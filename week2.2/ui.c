
#include "ipc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mqueue.h>
#include <limits.h>

static int read_int(const char *prompt, int *value)
{
    char line[128], extra;
    long n;

    printf("%s", prompt);
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin)) return 0;

    if (sscanf(line, " %ld %c", &n, &extra) != 1 ||
        n < INT_MIN || n > INT_MAX) {
        printf("Invalid integer. Try again.\n");
        return -1;
    }

    *value = (int)n;
    return 1;
}

static int send_full(int fd, const void *buf, size_t size)
{
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = write(fd, (const char *)buf + sent, size - sent);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

static void menu(void)
{
    printf("\n========== WEEK 4 SIMULATOR ==========\n");
    printf("1. Add\n2. Subtract\n3. Multiply\n4. Divide\n");
    printf("5. Stack Push\n6. Stack Pop\n7. Queue Enqueue\n");
    printf("8. Queue Dequeue\n9. Memory Store\n10. Memory Load\n");
    printf("11. Stack Peek\n12. Queue Peek\n13. Exit\n");
    printf("======================================\n");
}

int main(void)
{
    mqd_t mq = mq_open(MQ_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("UI: Start Core and Logger first");
        return 1;
    }

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("UI socket");
        mq_close(mq);
        return 1;
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("UI connect");
        close(sock);
        mq_close(mq);
        return 1;
    }

    printf("UI: Connected to Core.\n");

    while (1) {
        CoreMessage req = {0};
        CoreResponse res;
        int choice, rc;

        menu();
        rc = read_int("Select option (1-13): ", &choice);
        if (rc == 0) break;
        if (rc < 0) continue;

        if (choice < 1 || choice > 13) {
            printf("Choose a number from 1 to 13.\n");
            continue;
        }

        req.command = choice;

        if (choice >= 1 && choice <= 4) {
            rc = read_int("Enter first number: ", &req.value1);
            if (rc <= 0) { if (!rc) break; else continue; }
            rc = read_int("Enter second number: ", &req.value2);
            if (rc <= 0) { if (!rc) break; else continue; }
        } else if (choice == 5 || choice == 7) {
            rc = read_int("Enter value: ", &req.value1);
            if (rc <= 0) { if (!rc) break; else continue; }
        } else if (choice == 9) {
            rc = read_int("Enter memory address (0-99): ", &req.value1);
            if (rc <= 0) { if (!rc) break; else continue; }
            rc = read_int("Enter value to store: ", &req.value2);
            if (rc <= 0) { if (!rc) break; else continue; }
        } else if (choice == 10) {
            rc = read_int("Enter memory address (0-99): ", &req.value1);
            if (rc <= 0) { if (!rc) break; else continue; }
        }

        if (send_full(sock, &req, sizeof(req)) == -1) {
            perror("UI send");
            break;
        }

        ssize_t n;
        do {
            n = mq_receive(mq, (char *)&res, sizeof(res), NULL);
        } while (n == -1 && errno == EINTR);

        if (n != sizeof(res)) {
            perror("UI mq_receive");
            break;
        }

        printf("\n------------- RESULT -------------\n");
        printf("Status : %s\n", res.status ? "SUCCESS" : "ERROR");
        printf("Message: %s\n", res.message);
        if (res.status && choice != CMD_EXIT)
            printf("Result : %d\n", res.result);
        printf("----------------------------------\n");

        if (choice == CMD_EXIT) break;
    }

    close(sock);
    mq_close(mq);
    printf("UI: Stopped.\n");
    return 0;
}
