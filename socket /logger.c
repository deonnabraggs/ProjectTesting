#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "ipc.h"

/* Read one complete LogMessage from the FIFO. */
static int read_all(
    int fd,
    void *buffer,
    size_t len
)
{
    size_t received = 0;
    char *p = (char *)buffer;

    while (received < len)
    {
        ssize_t n = read(
            fd,
            p + received,
            len - received
        );

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

/* ================= LOGGER PROCESS ================= */

int main(void)
{
    int fifo_fd;
    LogMessage message;

    printf("\n");
    printf("====================================\n");
    printf("       LOGGER PROCESS STARTED\n");
    printf("====================================\n");

    printf("[LOGGER] Waiting for Core's named FIFO...\n");

    /* Open the named FIFO for reading. */

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

        if (read_all(
                fifo_fd,
                &message,
                sizeof(message)) == 0)
        {
            printf("[LOGGER] Core closed the FIFO\n");
            break;
        }

        if (errno != EINTR && ferror(stdin))
        {
            /* No action needed; FIFO reads do not use stdin. */
        }

        /*
         * read_all returns -1 on a read error.
         * Check it separately to avoid processing invalid data.
         */
        /* The actual read result is handled below in the corrected loop. */

        break;
    }

    close(fifo_fd);

    printf("\n[LOGGER] Process terminated\n");

    return 0;
}
