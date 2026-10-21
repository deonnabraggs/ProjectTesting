#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "ipc.h"

static int read_all(int fd, void *buffer, size_t len)
{
    size_t received = 0;
    char *p = (char *)buffer;

    while (received < len)
    {
        ssize_t n = read(fd, p + received, len - received);

        if (n == 0)
            return 0;

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        received += (size_t)n;
    }

    return 1;
}

int main(void)
{
    int fifo_fd;
    LogMessage message;

    printf("\n");
    printf("====================================\n");
    printf("       LOGGER PROCESS STARTED\n");
    printf("====================================\n");

    printf("[LOGGER] Waiting for Core's named FIFO...\n");

    fifo_fd = open(FIFO_PATH, O_RDONLY);

    if (fifo_fd == -1)
    {
        perror("[LOGGER] open FIFO");
        return 1;
    }

    printf("[LOGGER] Named FIFO connected successfully\n");
    printf("[LOGGER] Waiting for messages from Core...\n");

    while (1)
    {
        memset(&message, 0, sizeof(message));

        int read_result = read_all(
            fifo_fd,
            &message,
            sizeof(message)
        );

        if (read_result == 0)
        {
            printf("[LOGGER] Core closed the FIFO\n");
            break;
        }

        if (read_result == -1)
        {
            perror("[LOGGER] read FIFO");
            break;
        }

        printf("\n");
        printf("====================================\n");
        printf("           LOGGER OUTPUT\n");
        printf("====================================\n");

        if (message.status == 0)
            printf("Status  : SUCCESS\n");
        else
            printf("Status  : ERROR\n");

        printf("Message : %s\n", message.message);

        if (message.result != -999999)
            printf("Result  : %d\n", message.result);

        printf("====================================\n");

        if (strcmp(
                message.message,
                "Core process shutting down") == 0)
        {
            printf("\n[LOGGER] Shutdown message received\n");
            break;
        }

        printf("[LOGGER] Waiting for next message...\n");
    }

    close(fifo_fd);

    printf("\n[LOGGER] Process terminated\n");

    return 0;
}
