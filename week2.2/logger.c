
#include "ipc.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

static int read_full(int fd, void *buf, size_t size)
{
    size_t total = 0;

    while (total < size) {
        ssize_t n = read(fd, (char *)buf + total, size - total);
        if (n == 0) return total == 0 ? 0 : -1;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        total += (size_t)n;
    }

    return 1;
}

int main(void)
{
    int fifo = open(FIFO_PATH, O_RDONLY);
    if (fifo == -1) {
        perror("Logger FIFO open");
        return 1;
    }

    FILE *file = fopen("execution.log", "a");
    if (!file) {
        perror("Logger log file");
        close(fifo);
        return 1;
    }

    printf("LOGGER: Waiting for Core records...\n");
    fflush(stdout);

    while (1) {
        LogMessage log;
        int rc = read_full(fifo, &log, sizeof(log));
        if (rc <= 0) break;
        if (log.status == -1) {
            printf("LOGGER: Shutdown received.\n");
            break;
        }

        printf("[LOGGER] %s | Command %d | %s | Result %d\n",
               log.status ? "SUCCESS" : "ERROR",
               log.command, log.message, log.result);

        fprintf(file, "[%s] Command %d | %s | Result %d\n",
                log.status ? "SUCCESS" : "ERROR",
                log.command, log.message, log.result);
        fflush(file);
    }

    fclose(file);
    close(fifo);
    printf("LOGGER: Stopped.\n");
    return 0;
}
