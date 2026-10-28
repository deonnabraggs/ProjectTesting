#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include "ipc.h"

int main(void)
{
    int fd;
    char buffer[MAX_MESSAGE];
    size_t used = 0;

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("LOGGER: mkfifo");
        return 1;
    }

    printf("Logger process starting...\n");
    printf("Waiting for Core process...\n");
    fflush(stdout);

    fd = open(FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror("LOGGER: open FIFO");
        return 1;
    }

    printf("Logger connected. Waiting for messages...\n");
    fflush(stdout);

    while (1) {
        char ch;
        ssize_t n = read(fd, &ch, 1);

        if (n == 0)
            break;

        if (n < 0) {
            if (errno == EINTR) continue;
            perror("LOGGER: read");
            break;
        }

        if (ch == '\n') {
            buffer[used] = '\0';
            printf("------------------------------------------\n");
            printf("%s\n", buffer);
            fflush(stdout);
            used = 0;
        } else if (used < sizeof(buffer) - 1) {
            buffer[used++] = ch;
        }
    }

    close(fd);
    printf("Logger process exiting.\n");
    return 0;
}
