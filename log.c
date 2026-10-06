#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "ipc.h"

int main(void)
{
    mqd_t logger_queue;

    LogMessage message;

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(LogMessage);
    attr.mq_curmsgs = 0;

    printf("\n");
    printf("====================================\n");
    printf("       LOGGER PROCESS STARTED\n");
    printf("====================================\n");

    logger_queue = mq_open(
        CORE_TO_LOGGER,
        O_CREAT | O_RDONLY,
        0666,
        &attr
    );

    if (logger_queue == (mqd_t)-1)
    {
        perror("[LOGGER] mq_open");
        return 1;
    }

    printf("[LOGGER] Message queue opened successfully\n");
    printf("[LOGGER] Waiting for messages from Core...\n");

    while (1)
    {
        memset(&message, 0, sizeof(message));

        if (mq_receive(
                logger_queue,
                (char *)&message,
                sizeof(LogMessage),
                NULL) == -1)
        {
            perror("[LOGGER] mq_receive");
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

        /* Stop logger when Core sends EXIT message */
        if (strcmp(
                message.message,
                "Core process shutting down") == 0)
        {
            printf("\n[LOGGER] Shutdown message received\n");
            break;
        }

        printf("[LOGGER] Waiting for next message...\n");
    }

    mq_close(logger_queue);

    printf("\n[LOGGER] Process terminated\n");

    return 0;
}
