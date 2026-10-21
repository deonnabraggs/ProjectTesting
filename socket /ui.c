#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mqueue.h>

#include "ipc.h"

/* Send a complete message through the socket. */
static int send_all(int fd, const void *buffer, size_t len)
{
    size_t sent = 0;
    const char *p = (const char *)buffer;

    while (sent < len)
    {
        ssize_t n = send(fd, p + sent, len - sent, 0);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return -1;

        sent += (size_t)n;
    }

    return 0;
}

/* Receive a response through the POSIX message queue. */
static int receive_response(
    mqd_t queue,
    LogMessage *response
)
{
    ssize_t n = mq_receive(
        queue,
        (char *)response,
        sizeof(*response),
        NULL
    );

    if (n == -1)
    {
        perror("[UI] mq_receive");
        return -1;
    }

    if ((size_t)n != sizeof(*response))
    {
        fprintf(stderr, "[UI] Incorrect response size\n");
        return -1;
    }

    return 0;
}

/* ================= UI PROCESS ================= */

int main(void)
{
    int socket_fd;
    mqd_t response_queue;

    struct sockaddr_un address;

    CoreMessage request;
    LogMessage response;

    printf("\n");
    printf("====================================\n");
    printf("          UI PROCESS STARTED\n");
    printf("====================================\n");

    /* UI -> Core: UNIX domain socket */

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd == -1)
    {
        perror("[UI] socket");
        return 1;
    }

    memset(&address, 0, sizeof(address));

    address.sun_family = AF_UNIX;

    snprintf(
        address.sun_path,
        sizeof(address.sun_path),
        "%s",
        SOCKET_PATH
    );

    if (connect(
            socket_fd,
            (struct sockaddr *)&address,
            sizeof(address)) == -1)
    {
        perror("[UI] connect to Core socket");

        close(socket_fd);

        fprintf(
            stderr,
            "[UI] Start Core first, then Logger, then UI.\n"
        );

        return 1;
    }

    /* Core -> UI: POSIX message queue */

    response_queue = mq_open(
        CORE_TO_UI,
        O_RDONLY
    );

    if (response_queue == (mqd_t)-1)
    {
        perror("[UI] mq_open CORE_TO_UI");
        close(socket_fd);
        return 1;
    }

    printf("[UI] UNIX domain socket connected successfully\n");
    printf("[UI] POSIX message queue opened successfully\n");

    while (1)
    {
        memset(&request, 0, sizeof(request));

        printf("\n");
        printf("====================================\n");
        printf("       MULTI-PROCESS SIMULATOR\n");
        printf("====================================\n");

        printf("1. ADD\n");
        printf("2. SUBTRACT\n");
        printf("3. MULTIPLY\n");
        printf("4. DIVIDE\n");
        printf("5. STORE\n");
        printf("6. LOAD\n");
        printf("7. PUSH\n");
        printf("8. POP\n");
        printf("9. PEEK\n");
        printf("10. ENQUEUE\n");
        printf("11. DEQUEUE\n");
        printf("12. QUEUE PEEK\n");
        printf("13. EXIT\n");

        printf("\nEnter your choice: ");

        if (scanf("%d", &request.command) != 1)
        {
            printf("[UI] Invalid input.\n");

            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            continue;
        }

        /* Check valid command */

        if (request.command < 1 || request.command > 13)
        {
            printf("[UI] Invalid choice.\n");
            continue;
        }

        /* ================= EXIT ================= */

        if (request.command == 13)
        {
            strcpy(request.instruction, "EXIT");
        }

        /* ================= CPU OPERATIONS ================= */

        else if (request.command >= 1 && request.command <= 4)
        {
            if (request.command == 1)
                strcpy(request.instruction, "ADD");

            else if (request.command == 2)
                strcpy(request.instruction, "SUB");

            else if (request.command == 3)
                strcpy(request.instruction, "MUL");

            else
                strcpy(request.instruction, "DIV");

            printf("Enter value 1: ");

            if (scanf("%d", &request.value1) != 1)
            {
                printf("[UI] Invalid integer.\n");

                int ch;

                while ((ch = getchar()) != '\n' && ch != EOF)
                    ;

                continue;
            }

            printf("Enter value 2: ");

            if (scanf("%d", &request.value2) != 1)
            {
                printf("[UI] Invalid integer.\n");

                int ch;

                while ((ch = getchar()) != '\n' && ch != EOF)
                    ;

                continue;
            }
        }

        /* ================= STORE ================= */

        else if (request.command == 5)
        {
            strcpy(request.instruction, "STORE");

            printf("Enter memory address: ");

            if (scanf("%d", &request.value1) != 1)
                continue;

            printf("Enter value: ");

            if (scanf("%d", &request.value2) != 1)
                continue;
        }

        /* ================= LOAD ================= */

        else if (request.command == 6)
        {
            strcpy(request.instruction, "LOAD");

            printf("Enter memory address: ");

            if (scanf("%d", &request.value1) != 1)
                continue;
        }

        /* ================= PUSH ================= */

        else if (request.command == 7)
        {
            strcpy(request.instruction, "PUSH");

            printf("Enter value: ");

            if (scanf("%d", &request.value1) != 1)
                continue;
        }

        /* ================= POP ================= */

        else if (request.command == 8)
        {
            strcpy(request.instruction, "POP");
        }

        /* ================= PEEK ================= */

        else if (request.command == 9)
        {
            strcpy(request.instruction, "PEEK");
        }

        /* ================= ENQUEUE ================= */

        else if (request.command == 10)
        {
            strcpy(request.instruction, "ENQUEUE");

            printf("Enter value: ");

            if (scanf("%d", &request.value1) != 1)
                continue;
        }

        /* ================= DEQUEUE ================= */

        else if (request.command == 11)
        {
            strcpy(request.instruction, "DEQUEUE");
        }

        /* ================= QUEUE PEEK ================= */

        else if (request.command == 12)
        {
            strcpy(request.instruction, "QUEUE_PEEK");
        }

        printf("\n");
        printf("====================================\n");
        printf("             UI COMMAND\n");
        printf("====================================\n");

        printf("Instruction : %s\n", request.instruction);
        printf("Value 1     : %d\n", request.value1);
        printf("Value 2     : %d\n", request.value2);

        /* Send command to Core through the socket */

        if (send_all(
                socket_fd,
                &request,
                sizeof(request)) == -1)
        {
            perror("[UI] socket send");
            break;
        }

        printf("\n[UI] Command sent to Core successfully\n");

        /* Wait for Core response through POSIX message queue */

        if (receive_response(
                response_queue,
                &response) == -1)
        {
            break;
        }

        printf("\n");
        printf("====================================\n");
        printf("            CORE RESPONSE\n");
        printf("====================================\n");

        if (response.status == 0)
            printf("Status  : SUCCESS\n");
        else
            printf("Status  : ERROR\n");

        printf("Message : %s\n", response.message);

        if
