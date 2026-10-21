#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mqueue.h>
#include <fcntl.h>
#include "ipc.h"

static int send_all(int fd, const void *buffer, size_t length)
{
    const char *p = buffer;
    while (length > 0) {
        ssize_t n = send(fd, p, length, 0);
        if (n < 0) { if (errno == EINTR) continue; return -1; }
        if (n == 0) return -1;
        p += n;
        length -= (size_t)n;
    }
    return 0;
}

static int read_integer(const char *prompt, int *value)
{
    printf("%s", prompt);
    fflush(stdout);
    if (scanf("%d", value) != 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        printf("[UI] Invalid input. Please enter a number.\n");
        return -1;
    }
    return 0;
}

int main(void)
{
    int sockfd = -1;
    mqd_t response_queue = (mqd_t)-1;
    struct sockaddr_un address;
    CoreMessage request;
    LogMessage response;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) { perror("[UI] socket"); return 1; }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path), "%s", SOCKET_PATH);

    printf("[UI] Connecting to Core socket...\n");
    if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("[UI] connect (start ./core first)");
        close(sockfd);
        return 1;
    }

    response_queue = mq_open(CORE_TO_UI, O_RDONLY);
    if (response_queue == (mqd_t)-1) {
        perror("[UI] mq_open CORE_TO_UI (start ./core first)");
        close(sockfd);
        return 1;
    }

    printf("\n====================================\n");
    printf("       UI PROCESS STARTED\n");
    printf("====================================\n");

    while (1) {
        memset(&request, 0, sizeof(request));
        request.command = -1;

        printf("\n====================================\n");
        printf("       MULTI-PROCESS SIMULATOR\n");
        printf("====================================\n");
        printf("1. ADD\n2. SUBTRACT\n3. MULTIPLY\n4. DIVIDE\n");
        printf("5. STORE\n6. LOAD\n7. PUSH\n8. POP\n9. PEEK\n");
        printf("10. ENQUEUE\n11. DEQUEUE\n12. QUEUE PEEK\n13. EXIT\n");

        if (read_integer("\nEnter your choice: ", &request.command) != 0)
            continue;

        if (request.command < 1 || request.command > 13) {
            printf("[UI] Invalid choice.\n");
            continue;
        }

        switch (request.command) {
        case 1: strcpy(request.instruction, "ADD"); break;
        case 2: strcpy(request.instruction, "SUB"); break;
        case 3: strcpy(request.instruction, "MUL"); break;
        case 4: strcpy(request.instruction, "DIV"); break;
        case 5: strcpy(request.instruction, "STORE"); break;
        case 6: strcpy(request.instruction, "LOAD"); break;
        case 7: strcpy(request.instruction, "PUSH"); break;
        case 8: strcpy(request.instruction, "POP"); break;
        case 9: strcpy(request.instruction, "PEEK"); break;
        case 10: strcpy(request.instruction, "ENQUEUE"); break;
        case 11: strcpy(request.instruction, "DEQUEUE"); break;
        case 12: strcpy(request.instruction, "QUEUE_PEEK"); break;
        case 13: strcpy(request.instruction, "EXIT"); break;
        }

        if (request.command == 1 || request.command == 2 ||
            request.command == 3 || request.command == 4) {
            if (read_integer("Enter value 1: ", &request.value1) != 0) continue;
            if (read_integer("Enter value 2: ", &request.value2) != 0) continue;
        } else if (request.command == 5) {
            if (read_integer("Enter memory address: ", &request.value1) != 0) continue;
            if (read_integer("Enter value: ", &request.value2) != 0) continue;
        } else if (request.command == 6) {
            if (read_integer("Enter memory address: ", &request.value1) != 0) continue;
        } else if (request.command == 7 || request.command == 10) {
            if (read_integer("Enter value: ", &request.value1) != 0) continue;
        }

        printf("\n[UI] Instruction: %s\n", request.instruction);
        if (send_all(sockfd, &request, sizeof(request)) == -1) {
            perror("[UI] socket send");
            break;
        }
        printf("[UI] Command sent to Core through Unix domain socket.\n");

        if (mq_receive(response_queue, (char *)&response, sizeof(response), NULL) == -1) {
            perror("[UI] mq_receive");
            break;
        }

        printf("\n====================================\n");
        printf("            CORE RESPONSE\n");
        printf("====================================\n");
        printf("Status  : %s\n", response.status == 0 ? "SUCCESS" : "ERROR");
        printf("Message : %s\n", response.message);
        if (response.result != -999999)
            printf("Result  : %d\n", response.result);
        printf("=====================================\n");

        if (request.command == 13)
            break;
    }

    if (response_queue != (mqd_t)-1) mq_close(response_queue);
    if (sockfd != -1) close(sockfd);
    printf("\n[UI] Process terminated\n");
    return 0;
}
