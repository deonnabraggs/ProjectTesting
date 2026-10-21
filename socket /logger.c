#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "ipc.h"

static int read_all(int fd, void *buffer, size_t length)
{
    size_t received = 0;
    char *p = buffer;

    while (received < length)
    {
        ssize_t n = read(fd, p + received, length - received);

        if (n == 0)
        {
            if (received == 0)
                return 0;

            fprintf(stderr, "[LOGGER] Incomplete message\n");
            return -1;
        }

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

    printf("========== LOGGER PROCESS ==========\n");
    printf("[LOGGER] Waiting for Core's named FIFO...\n");

    fifo_fd = open(FIFO_PATH, O_RDONLY);

    if (fifo_fd == -1)
    {
        perror("[LOGGER] open FIFO");
        return 1;
    }

    printf("[LOGGER] Connected. Waiting for logs...\n");

    while (1)
    {
        memset(&message, 0, sizeof(message));

        int result = read_all(fifo_fd, &message, sizeof(message));

        if (result == 0)
        {
            printf("[LOGGER] Core closed the FIFO.\n");
            break;
        }

        if (result == -1)
        {
            perror("[LOGGER] FIFO read");
            break;
        }

        message.message[MAX_MESSAGE - 1] = '\0';

        printf("\n========== OPERATION LOG ==========\n");
        printf("Status  : %s\n",
               message.status == 0 ? "SUCCESS" : "ERROR");
        printf("Message : %s\n", message.message);

        if (message.result != -999999)
            printf("Result  : %d\n", message.result);
    }

    close(fifo_fd);

    printf("[LOGGER] Terminated\n");
    return 0;
}
