#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipc.h"

static int read_number(const char *prompt, int *value)
{
    printf("%s", prompt);
    fflush(stdout);

    if (scanf("%d", value) != 1) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}
        printf("Invalid input. Enter a number.\n");
        return 0;
    }
    return 1;
}

static int send_request(int fd, const Request *r)
{
    const char *p = (const char *)r;
    size_t left = sizeof(*r);

    while (left) {
        ssize_t n = send(fd, p, left, 0);
        if (n < 0) return -1;
        if (n == 0) return -1;
        p += n;
        left -= (size_t)n;
    }
    return 0;
}

int main(void)
{
    int sock;
    struct sockaddr_un addr;
    mqd_t mq;
    Request req;
    Response res;

    /* Core must be started first so this queue already exists. */
    mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("UI: mq_open (start Core first)");
        return 1;
    }

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("UI: socket");
        mq_close(mq);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("UI: connect (start Core first)");
        close(sock);
        mq_close(mq);
        return 1;
    }

    printf("UI: Connected to Core.\n");

    while (1) {
        printf("\n====== MULTI-PROCESS SIMULATOR ======\n");
        printf("1. Add\n2. Subtract\n3. Multiply\n4. Divide\n");
        printf("5. Stack Push\n6. Stack Pop\n");
        printf("7. Queue Enqueue\n8. Queue Dequeue\n");
        printf("9. Memory Store\n10. Memory Load\n");
        printf("11. Stack Peek\n12. Queue Peek\n13. Exit\n");
        printf("=====================================\n");

        if (!read_number("Select an option: ", &req.option))
            continue;

        if (req.option < 1 || req.option > 13) {
            printf("Choose an option from 1 to 13.\n");
            continue;
        }

        req.value1 = 0;
        req.value2 = 0;

        if (req.option >= 1 && req.option <= 4) {
            if (!read_number("Enter first number: ", &req.value1) ||
                !read_number("Enter second number: ", &req.value2))
                continue;
        } else if (req.option == 5 || req.option == 7) {
            if (!read_number("Enter value: ", &req.value1))
                continue;
        } else if (req.option == 9) {
            if (!read_number("Enter memory index (0-9): ", &req.value1) ||
                !read_number("Enter value to store: ", &req.value2))
                continue;
        } else if (req.option == 10) {
            if (!read_number("Enter memory index (0-9): ", &req.value1))
                continue;
        }

        /* IPC 1: UI -> Core using Unix Domain Socket */
        if (send_request(sock, &req) == -1) {
            perror("UI: send");
            break;
        }

        /* IPC 2: Core -> UI using POSIX Message Queue */
        if (mq_receive(mq, (char *)&res, sizeof(res), NULL) == -1) {
            perror("UI: mq_receive");
            break;
        }

        printf("\n------------- RESULT -------------\n");
        printf("Status : %s\n", res.status ? "SUCCESS" : "ERROR");
        printf("Message: %s\n", res.message);
        printf("Result : %d\n", res.result);
        printf("----------------------------------\n");

        if (req.option == 13)
            break;
    }

    close(sock);
    mq_close(mq);
    printf("UI: Exiting.\n");
    return 0;
}
