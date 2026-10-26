
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ipc.h"

int main(void)
{
    int fifo_fd;
    char buffer[1024];
    ssize_t count;

    printf("LOGGER: Process started.\n");
    printf("LOGGER: Waiting for Core process (FIFO)...\n");
    fflush(stdout);

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("LOGGER: mkfifo failed");
        return EXIT_FAILURE;
    }

    fifo_fd = open(FIFO_PATH, O_RDONLY);

    if (fifo_fd == -1) {
        perror("LOGGER: Could not open FIFO");
        return EXIT_FAILURE;
    }

    printf("LOGGER: Connected to Core. Waiting for messages...\n");
    printf("--------------------------------------------------\n");
    fflush(stdout);

    while ((count = read(fifo_fd, buffer, sizeof(buffer))) > 0) {
        size_t offset = 0;

        while (offset < (size_t)count) {
            size_t written = fwrite(
                buffer + offset, 1,
                (size_t)count - offset, stdout
            );

            if (written == 0) {
                perror("LOGGER: Output error");
                close(fifo_fd);
                return EXIT_FAILURE;
            }

            offset += written;
        }

        fflush(stdout);
    }

    if (count == -1) {
        perror("LOGGER: FIFO read failed");
    }

    close(fifo_fd);
    printf("\nLOGGER: FIFO closed. Logger process stopped.\n");

    return count == -1 ? EXIT_FAILURE : EXIT_SUCCESS;
}
