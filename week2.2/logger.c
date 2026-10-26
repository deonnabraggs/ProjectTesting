
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "ipc.h"

int main(void)
{
    int fifo;
    char buffer[256];
    ssize_t bytes_read;

    printf("Logger process starting...\n");
    printf("Waiting for Core process...\n");
    fflush(stdout);

    /* Open the named pipe for reading */
    fifo = open(FIFO_PATH, O_RDONLY);

    if (fifo == -1)
    {
        perror("Logger: Failed to open FIFO");
        return EXIT_FAILURE;
    }

    printf("Logger connected. Waiting for messages...\n");
    printf("------------------------------------------\n");
    fflush(stdout);

    /* Read and display messages sent by Core */
    while ((bytes_read = read(fifo, buffer, sizeof(buffer))) > 0)
    {
        size_t written = fwrite(buffer, 1, (size_t)bytes_read, stdout);

        if (written != (size_t)bytes_read)
        {
            perror("Logger: Failed to display message");
            close(fifo);
            return EXIT_FAILURE;
        }

        fflush(stdout);
    }

    if (bytes_read == -1)
    {
        perror("Logger: Failed to read FIFO");
        close(fifo);
        return EXIT_FAILURE;
    }

    close(fifo);

    printf("\n------------------------------------------\n");
    printf("Logger process stopped.\n");

    return EXIT_SUCCESS;
}
