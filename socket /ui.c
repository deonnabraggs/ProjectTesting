#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "ipc.h"

static int send_all(int fd, const void *data, size_t size)
{
    const char *p = data;
    size_t sent = 0;

    while (sent < size)
    {
        ssize_t n = send(fd, p + sent, size - sent, 0);
        if (n <= 0)
            return -1;
        sent += (size_t)n;
    }

    return 0;
}

static int receive_all(int fd, void *data, size_t size)
{
    char *p = data;
    size_t received = 0;

    while (received < size)
    {
        ssize_t n = recv(fd, p + received, size - received, 0);
        if (n <= 0)
            return -1;
        received += (size_t)n;
    }

    return 0;
}

int main(void)
{
    int sockfd;
    struct sockaddr_un address;
    CoreMessage request;
    LogMessage response;

    printf("\n");
    printf("====================================\n");
    printf("          UI PROCESS STARTED\n");
    printf("====================================\n");

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        perror("[UI] socket");
        return 1;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);

    if (connect(sockfd, (struct sockaddr *)&address,
                sizeof(address)) == -1)
    {
        perror("[UI] connect");
        close(sockfd);
        return 1;
    }

    printf("[UI] Connected to Core using UNIX domain socket\n");

    while (1)
    {
        memset(&request, 0, sizeof(request));
        request.result = 0;

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
        fflush(stdout);

        if (scanf("%d", &request.command) != 1)
        {
            printf("[UI] Invalid input.\n");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            continue;
        }

        if (request.command < 1 || request.command > 13)
        {
            printf("[UI] Invalid choice.\n");
            continue;
        }

        if (request.command == 13)
        {
            strcpy(request.instruction, "EXIT");

            if (send_all(sockfd, &request, sizeof(request)) == -1)
                perror("[UI] socket send");
            else if (receive_all(sockfd, &response,
                                 sizeof(response)) == 0)
                printf("[UI] Core Response: %s\n", response.message);
            else
                perror("[UI] socket receive");

            break;
        }

        if (request.command <= 4)
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
                goto invalid_input;

            printf("Enter value 2: ");
            if (scanf("%d", &request.value2) != 1)
                goto invalid_input;
        }
        else if (request.command == 5)
        {
            strcpy(request.instruction, "STORE");

            printf("Enter memory address: ");
            if (scanf("%d", &request.value1) != 1)
                goto invalid_input;

            printf("Enter value: ");
            if (scanf("%d", &request.value2) != 1)
                goto invalid_input;
        }
        else if (request.command == 6)
        {
            strcpy(request.instruction, "LOAD");

            printf("Enter memory address: ");
            if (scanf("%d", &request.value1) != 1)
                goto invalid_input;
        }
        else if (request.command == 7)
        {
            strcpy(request.instruction, "PUSH");

            printf("Enter value: ");
            if (scanf("%d", &request.value1) != 1)
                goto invalid_input;
        }
        else if (request.command == 8)
            strcpy(request.instruction, "POP");
        else if (request.command == 9)
            strcpy(request.instruction, "PEEK");
        else if (request.command == 10)
        {
            strcpy(request.instruction, "ENQUEUE");

            printf("Enter value: ");
            if (scanf("%d", &request.value1) != 1)
                goto invalid_input;
        }
        else if (request.command == 11)
            strcpy(request.instruction, "DEQUEUE");
        else if (request.command == 12)
            strcpy(request.instruction, "QUEUE_PEEK");

        printf("\n");
        printf("====================================\n");
        printf("             UI COMMAND\n");
        printf("====================================\n");
        printf("Instruction : %s\n", request.instruction);
        printf("Value 1     : %d\n", request.value1);
        printf("Value 2     : %d\n", request.value2);

        if (send_all(sockfd, &request, sizeof(request)) == -1)
        {
            perror("[UI] socket send");
            break;
        }

        printf("\n[UI] Command sent to Core successfully\n");

        if (receive_all(sockfd, &response, sizeof(response)) == -1)
        {
            perror("[UI] socket receive");
            break;
        }

        printf("\n");
        printf("====================================\n");
        printf("            CORE RESPONSE\n");
        printf("====================================\n");

        printf("Status  : %s\n",
               response.status == 0 ? "SUCCESS" : "ERROR");
        printf("Message : %s\n", response.message);

        if (response.result != -999999)
            printf("Result  : %d\n", response.result);

        printf("====================================\n");
        continue;

invalid_input:
        printf("[UI] Invalid number. Please enter valid integers.\n");

        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
        }
    }

    close(sockfd);
    printf("\n[UI] Process terminated\n");

    return 0;
}
