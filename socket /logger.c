#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <poll.h>

#include "ipc.h"

int main(void)
{
    mqd_t logger_mq;
    struct mq_attr attr;
    LogMessage message;
    int fifo_fd;

    printf("\n");
    printf("====================================\n");
    printf("        LOGGER PROCESS STARTED\n");
    printf("====================================\n");

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(LogMessage);
    attr.mq_curmsgs = 0;

    mq_unlink(CORE_TO_LOGGER);

    logger_mq = mq_open(CORE_TO_LOGGER,
                        O_CREAT | O_RDONLY, 0666, &attr);

    if (logger_mq == (mqd_t)-1)
    {
        perror("[LOGGER] mq_open");
        return 1;
    }

    unlink(FIFO_PATH);

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST)
    {
        perror("[LOGGER] mkfifo");
        mq_close(logger_mq);
        mq_unlink(CORE_TO_LOGGER);
        return 1;
    }

    printf("[LOGGER] Message queue created\n");
    printf("[LOGGER] Waiting for Core to open FIFO...\n");
    fflush(stdout);

    fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1)
    {
        perror("[LOGGER] FIFO open");
        mq_close(logger_mq);
        mq_unlink(CORE_TO_LOGGER);
        unlink(FIFO_PATH);
        return 1;
    }

    printf("[LOGGER] FIFO opened successfully\n");

    struct pollfd fds[2];

    fds[0].fd = (int)logger_mq;
    fds[0].events = POLLIN;

    fds[1].fd = fifo_fd;
    fds[1].events = POLLIN;

    while (1)
    {
        int ready = poll(fds, 2, -1);

        if (ready == -1)
        {
            if (errno == EINTR)
                continue;

            perror("[LOGGER] poll");
            break;
        }

        if (fds[0].revents & POLLIN)
        {
            ssize_t n = mq_receive(logger_mq, (char *)&message,
                                   sizeof(message), NULL);

            if (n == (ssize_t)sizeof(message))
            {
                printf("\n[LOGGER][MESSAGE QUEUE] %s\n",
                       message.message);
                fflush(stdout);

                if (strcmp(message.message,
                           "Core process shutting down") == 0)
                    break;
            }
            else if (n == -1 && errno != EINTR)
                perror("[LOGGER] mq_receive");
        }

        if (fds[1].revents & POLLIN)
        {
            ssize_t n = read(fifo_fd, &message, sizeof(message));

            if (n == (ssize_t)sizeof(message))
            {
                printf("[LOGGER][FIFO] %s\n", message.message);
                fflush(stdout);

                if (strcmp(message.message,
                           "Core process shutting down") == 0)
                    break;
            }
            else if (n == 0)
            {
                printf("[LOGGER] FIFO writer disconnected\n");
                break;
            }
            else if (n == -1 && errno != EINTR)
                perror("[LOGGER] FIFO read");
        }
    }

    close(fifo_fd);
    mq_close(logger_mq);
    mq_unlink(CORE_TO_LOGGER);
    unlink(FIFO_PATH);

    printf("[LOGGER] Process terminated\n");
    return 0;
}
