
#include "ipc.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

static int read_full(int fd, void *buffer, size_t size)
{
    size_t total = 0;
    char *p = buffer;

    while (total < size) {
        ssize_t n = read(fd, p + total, size - total);

        if (n == 0)
            return total == 0 ? 0 : -1;

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        total += (size_t)n;
    }

    return 1;
}

int main(void)
{
    /* Start Logger before Core; FIFO open waits for Core's writer. */
    int fifo_fd = open(FIFO_PATH, O_RDONLY);

    if (fifo_fd == -1) {
        perror("Logger: open FIFO - start Core after creating FIFO");
        return 1;
    }

    FILE *log_file = fopen("execution.log", "a");
    if (!log_file) {
        perror("Logger: execution.log");
        close(fifo_fd);
        return 1;
    }

    printf("LOGGER: Running and waiting for Core messages...\n");
    fflush(stdout);

    while (1) {
        LogMessage log;
        int rc = read_full(fifo_fd, &log, sizeof(log));

        if (rc <= 0)
            break;

        if (log.status == -1) {
            printf("LOGGER: Shutdown received.\n");
            break;
        }

        const char *status = log.status ? "SUCCESS" : "ERROR";

        printf("[LOGGER] %s | Command: %d | %s | Result: %d\n",
               status, log.command, log.message, log.result);

        fprintf(log_file,
                "%s | Command: %d | %s | Result: %d\n",
                status, log.command, log.message, log.result);

        fflush(log_file);
        fflush(stdout);
    }

    fclose(log_file);
    close(fifo_fd);

    printf("LOGGER: Process stopped.\n");
    return 0;
}
