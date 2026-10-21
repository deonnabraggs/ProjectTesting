#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ipc.h"

static int read_all(int fd, void *buffer, size_t length)
{
    char *p = buffer;
    while (length > 0) {
        ssize_t n = read(fd, p, length);
        if (n < 0) { if (errno == EINTR) continue; return -1; }
        if (n == 0) return 0;
        p += n;
        length -= (size_t)n;
    }
    return 1;
}

int main(void)
{
    int fifo_fd;
    LogMessage message;

    printf("\n====================================\n");
    printf("LOGGER PROCESS STARTED\n");
    printf("====================================\n");
    printf("[LOGGER] Waiting for Core FIFO: %s\n", FIFO_PATH);

    fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("[LOGGER] open FIFO (start ./core first)");
        return 1;
    }

    printf("[LOGGER] FIFO opened. Waiting for log messages...\n");
    for (;;) {
        int result;
        memset(&message, 0, sizeof(message));
        result = read_all(fifo_fd, &message, sizeof(message));
        if (result == 0) {
            printf("[LOGGER] Core closed the FIFO. Logger stopping.\n");
            break;
        }
        if (result == -1) {
            perror("[LOGGER] read FIFO");
            break;
        }

        printf("\n====================================\n");
        printf("LOGGER OUTPUT (NAMED PIPE / FIFO)\n");
        printf("====================================\n");
        printf("Status  : %s\n", message.status == 0 ? "SUCCESS" : "ERROR");
        printf("Message : %s\n", message.message);
        if (message.result != -999999)
            printf("Result  : %d\n", message.result);
        printf("====================================\n");

        if (strcmp(message.message, "Core process shutting down") == 0)
            break;
    }

    close(fifo_fd);
    printf("[LOGGER] Process terminated.\n");
    return 0;
}
