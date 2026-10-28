
#include "ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mqueue.h>

static int read_int(const char *prompt, int *value)
{
    char line[128];
    char extra;
    long number;

    printf("%s", prompt);
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin))
        return 0;

    if (sscanf(line, " %ld %c", &number, &extra) != 1 ||
        number < -2147483647L - 1 || number > 2147483647L) {
        printf("Invalid input. Please enter an integer.\n");
        return -1;
    }

    *value = (int)number;
    return 1;
}

static int send_full(int fd, const void *buffer, size_t size)
{
    size_t sent = 0;
    const char *p = buffer;

    while (sent < size) {
        ssize_t n = write(fd, p + sent, size - sent);

        if (n < 0) {
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

static void show_menu(void)
{
    printf("\n====================================\n");
    printf("   WEEK 4 MULTI-PROCESS SIMULATOR\n");
    printf("====================================\n");
    printf("1.  Add\n");
    printf("2.  Subtract\n");
    printf("3.  Multiply\n");
    printf("4.  Divide\n");
    printf("5.  Stack Push\n");
    printf("6.  Stack Pop\n");
    printf("7.  Queue Enqueue\n");
    printf("8.  Queue Dequeue\n");
    printf("9.  Memory Store\n");
    printf("10. Memory Load\n");
    printf("11. Stack Peek\n");
    printf("12. Queue Peek\n");
    printf("13. Exit\n");
    printf("====================================\n");
}

int main(void)
{
    int sock = -1;
    mqd_t mq = (mqd_t)-1;

    /* Core must be started before UI. */
    mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("UI: mq_open - start Core first");
        return 1;
    }

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("UI: socket");
        mq_close(mq);
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("UI: connect - check that Core is running");
        close(sock);
        mq_close(mq);
        return 1;
    }

    printf("UI: Connected to Core through Unix domain socket.\n");

    while (1) {
        CoreMessage req;
        CoreResponse response;
        int choice;
        int rc;

        show_menu();

        rc = read_int("Select an option (1-13): ", &choice);
        if (rc == 0)
            break;
        if (rc < 0)
            continue;

        if (choice < 1 || choice > 13) {
            printf("Please select an option from 1 to 13.\n");
            continue;
        }

        memset(&req, 0, sizeof(req));
        req.command = choice;

        if (choice >= 1 && choice <= 4) {
            rc = read_int("Enter first number: ", &req.value1);
            if (rc <= 0) {
                if (rc == 0) break;
                continue;
            }

            rc = read_int("Enter second number: ", &req.value2);
            if (rc <= 0) {
                if (rc == 0) break;
                continue;
            }
        } else if (choice == 5 || choice == 7) {
            rc = read_int("Enter value: ", &req.value1);
            if (rc <= 0) {
                if (rc == 0) break;
                continue;
            }
        } else if (choice == 9) {
            rc = read_int("Enter memory address (0-99): ",
                          &req.value1);
            if (rc <= 0) {
                if (rc == 0) break;
                continue;
            }

            rc = read_int("Enter value to store: ", &req.value2);
            if (rc <= 0) {
                if (rc == 0) break;
                continue;
            }
        } else if (choice == 10) {
            rc = read_int("Enter memory address (0-99): ",
                          &req.value1);
            if (rc <= 0) {
                if (rc == 0) break;
                continue;
            }
        }

        /* IPC technique 1: UI -> Core using Unix domain socket. */
        if (send_full(sock, &req, sizeof(req)) == -1) {
            perror("UI: send");
            break;
        }

        /* IPC technique 2: receive Core's response via POSIX MQ. */
        ssize_t received;
        do {
            received = mq_receive(mq, (char *)&response,
                                  sizeof(response), NULL);
        } while (received == -1 && errno == EINTR);

        if (received != (ssize_t)sizeof(response)) {
            perror("UI: mq_receive");
            break;
        }

        printf("\n--------------- OUTPUT ---------------\n");
        printf("Process      : UI -> Core\n");
        printf("Status       : %s\n",
               response.status ? "SUCCESS" : "ERROR");
        printf("Message      : %s\n", response.message);

        if (response.status && choice != 13)
            printf("Result       : %d\n", response.result);

        printf("--------------------------------------\n");

        if (choice == 13)
            break;
    }

    close(sock);
    mq_close(mq);

    printf("UI: Process stopped.\n");
    return 0;
}
