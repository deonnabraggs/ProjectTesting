
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mqueue.h>

#include "ipc.h"

static int send_all(int fd, const void *buffer, size_t length)
{
    size_t sent = 0;
    const char *p = buffer;

    while (sent < length)
    {
        ssize_t n = send(fd, p + sent, length - sent, 0);

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

static int read_integer(const char *prompt, int *value)
{
    int ch;

    printf("%s", prompt);
    fflush(stdout);

    if (scanf("%d", value) == 1)
        return 0;

    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    printf("[UI] Please enter a valid integer.\n");
    return -1;
}

static int receive_response(mqd_t mq, LogMessage *response)
{
    ssize_t n;

    do
    {
        n = mq_receive(mq, (char *)response,
                       sizeof(*response), NULL);
    } while (n == -1 && errno == EINTR);

    if (n == -1)
    {
        perror("[UI] mq_receive");
        return -1;
    }

    if ((size_t)n != sizeof(*response))
    {
        fprintf(stderr, "[UI] Invalid response size\n");
        return -1;
    }

    response->message[MAX_MESSAGE - 1] = '\0';
    return 0;
}

int main(void)
{
    int socket_fd;
    int choice;
    mqd_t mq;
    struct sockaddr_un address;
    CoreMessage request;
    LogMessage response;

    printf("========== UI PROCESS ==========\n");

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd == -1)
    {
        perror("[UI] socket");
        return 1;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;

    snprintf(address.sun_path, sizeof(address.sun_path),
             "%s", SOCKET_PATH);

    if (connect(socket_fd, (struct sockaddr *)&address,
                sizeof(address)) == -1)
    {
        perror("[UI] connect to Core");
        close(socket_fd);
        fprintf(stderr, "[UI] Start Core and Logger first.\n");
        return 1;
    }

    mq = mq_open(CORE_TO_UI, O_RDONLY);

    if (mq == (mqd_t)-1)
    {
        perror("[UI] mq_open");
        close(socket_fd);
        return 1;
    }

    printf("[UI] Socket connected\n");
    printf("[UI] POSIX message queue connected\n");

    while (1)
    {
        memset(&request, 0, sizeof(request));

        printf("\n========== MULTI-PROCESS SIMULATOR ==========\n");
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

        if (read_integer("Enter your choice: ", &choice) == -1)
            continue;

        if (choice < 1 || choice > 13)
        {
            printf("[UI] Invalid choice.\n");
            continue;
        }

        request.command = choice;

        switch (choice)
        {
            case 1:
                strcpy(request.instruction, "ADD");
                break;
            case 2:
                strcpy(request.instruction, "SUB");
                break;
            case 3:
                strcpy(request.instruction, "MUL");
                break;
            case 4:
                strcpy(request.instruction, "DIV");
                break;
            case 5:
                strcpy(request.instruction, "STORE");
                break;
            case 6:
                strcpy(request.instruction, "LOAD");
                break;
            case 7:
                strcpy(request.instruction, "PUSH");
                break;
            case 8:
                strcpy(request.instruction, "POP");
                break;
            case 9:
                strcpy(request.instruction, "PEEK");
                break;
            case 10:
                strcpy(request.instruction, "ENQUEUE");
                break;
            case 11:
                strcpy(request.instruction, "DEQUEUE");
                break;
            case 12:
                strcpy(request.instruction, "QUEUE_PEEK");
                break;
            case 13:
                strcpy(request.instruction, "EXIT");
                break;
        }

        if (choice >= 1 && choice <= 4)
        {
            if (read_integer("Enter value 1: ",
                             &request.value1) == -1)
                continue;

            if (read_integer("Enter value 2: ",
                             &request.value2) == -1)
                continue;
        }
        else if (choice == 5)
        {
            if (read_integer("Enter memory address (0-99): ",
                             &request.value1) == -1)
                continue;

            if (read_integer("Enter value: ",
                             &request.value2) == -1)
                continue;
        }
        else if (choice == 6)
        {
            if (read_integer("Enter memory address (0-99): ",
                             &request.value1) == -1)
                continue;
        }
        else if (choice == 7 || choice == 10)
        {
            if (read_integer("Enter value: ",
                             &request.value1) == -1)
                continue;
        }

        printf("\n[UI] Sending: %s\n", request.instruction);

        if (send_all(socket_fd, &request, sizeof(request)) == -1)
        {
            perror("[UI] socket send");
            break;
        }

        if (receive_response(mq, &response) == -1)
            break;

        printf("\n========== CORE RESPONSE ==========\n");
        printf("Status  : %s\n",
               response.status == 0 ? "SUCCESS" : "ERROR");
        printf("Message : %s\n", response.message);

        if (response.result != -999999)
            printf("Result  : %d\n", response.result);

        if (choice == 13)
            break;
    }

    mq_close(mq);
    close(socket_fd);

    printf("[UI] Terminated\n");
    return 0;
}
