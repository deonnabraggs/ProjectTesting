#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <mqueue.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include "ipc.h"

static int stack[MAX_STACK], top = -1;
static int queue[MAX_QUEUE], front = 0, count = 0;
static int memory[MEMORY_SIZE];

static int receive_request(int fd, Request *r)
{
    char *p = (char *)r;
    size_t left = sizeof(*r);

    while (left) {
        ssize_t n = recv(fd, p, left, 0);
        if (n == 0) return 0;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += n;
        left -= (size_t)n;
    }
    return 1;
}

static void log_message(int fd, const char *msg)
{
    char line[MAX_MESSAGE];
    int len = snprintf(line, sizeof(line), "%s\n", msg);
    if (len <= 0) return;
    if (len >= (int)sizeof(line)) len = sizeof(line) - 1;

    int done = 0;
    while (done < len) {
        ssize_t n = write(fd, line + done, (size_t)(len - done));
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("CORE: FIFO write");
            return;
        }
        done += (int)n;
    }
}

static void error_response(Response *r, const char *msg)
{
    r->status = 0;
    r->result = 0;
    snprintf(r->message, sizeof(r->message), "%s", msg);
}

static void execute(const Request *q, Response *r)
{
    int a = q->value1, b = q->value2;
    r->status = 1;
    r->result = 0;
    snprintf(r->message, sizeof(r->message), "Operation successful");

    switch (q->option) {
    case 1:
        if (__builtin_add_overflow(a, b, &r->result)) {
            error_response(r, "Addition overflow"); break;
        }
        snprintf(r->message, sizeof(r->message),
                 "CPU: ADD result = %d", r->result);
        break;

    case 2:
        if (__builtin_sub_overflow(a, b, &r->result)) {
            error_response(r, "Subtraction overflow"); break;
        }
        snprintf(r->message, sizeof(r->message),
                 "CPU: SUBTRACT result = %d", r->result);
        break;

    case 3:
        if (__builtin_mul_overflow(a, b, &r->result)) {
            error_response(r, "Multiplication overflow"); break;
        }
        snprintf(r->message, sizeof(r->message),
                 "CPU: MULTIPLY result = %d", r->result);
        break;

    case 4:
        if (b == 0) {
            error_response(r, "Cannot divide by zero"); break;
        }
        if (a == INT_MIN && b == -1) {
            error_response(r, "Division overflow"); break;
        }
        r->result = a / b;
        snprintf(r->message, sizeof(r->message),
                 "CPU: DIVIDE result = %d", r->result);
        break;

    case 5:
        if (top == MAX_STACK - 1) {
            error_response(r, "Stack is full"); break;
        }
        stack[++top] = a;
        r->result = a;
        snprintf(r->message, sizeof(r->message),
                 "Value pushed onto stack");
        break;

    case 6:
        if (top < 0) {
            error_response(r, "Stack is empty"); break;
        }
        r->result = stack[top--];
        snprintf(r->message, sizeof(r->message),
                 "Value popped from stack");
        break;

    case 7:
        if (count == MAX_QUEUE) {
            error_response(r, "Queue is full"); break;
        }
        queue[(front + count) % MAX_QUEUE] = a;
        count++;
        r->result = a;
        snprintf(r->message, sizeof(r->message),
                 "Value added to queue");
        break;

    case 8:
        if (count == 0) {
            error_response(r, "Queue is empty"); break;
        }
        r->result = queue[front];
        front = (front + 1) % MAX_QUEUE;
        count--;
        snprintf(r->message, sizeof(r->message),
                 "Value removed from queue");
        break;

    case 9:
        if (a < 0 || a >= MEMORY_SIZE) {
            error_response(r, "Invalid memory index (0-9)"); break;
        }
        memory[a] = b;
        r->result = b;
        snprintf(r->message, sizeof(r->message),
                 "Value stored in memory");
        break;

    case 10:
        if (a < 0 || a >= MEMORY_SIZE) {
            error_response(r, "Invalid memory index (0-9)"); break;
        }
        r->result = memory[a];
        snprintf(r->message, sizeof(r->message),
                 "Value loaded from memory");
        break;

    case 11:
        if (top < 0) {
            error_response(r, "Stack is empty"); break;
        }
        r->result = stack[top];
        snprintf(r->message, sizeof(r->message),
                 "Top value of stack");
        break;

    case 12:
        if (count == 0) {
            error_response(r, "Queue is empty"); break;
        }
        r->result = queue[front];
        snprintf(r->message, sizeof(r->message),
                 "Front value of queue");
        break;

    case 13:
        snprintf(r->message, sizeof(r->message),
                 "Simulator exit requested");
        break;

    default:
        error_response(r, "Invalid operation");
    }
}

int main(void)
{
    int server = -1, client = -1, fifo = -1;
    mqd_t mq = (mqd_t)-1;
    struct sockaddr_un addr;
    struct mq_attr attr;

    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Response);

    /* Remove stale IPC objects from an earlier run. */
    mq_unlink(MQ_NAME);
    unlink(SOCKET_PATH);

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("CORE: mkfifo");
        return 1;
    }

    printf("CORE: Waiting for Logger FIFO...\n");
    fflush(stdout);

    /* Wait for Logger to open the FIFO for reading. */
    fifo = open(FIFO_PATH, O_WRONLY);
    if (fifo == -1) {
        perror("CORE: open FIFO");
        goto cleanup;
    }

    printf("CORE: Logger connected.\n");

    /* Create response queue before UI starts. */
    mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("CORE: mq_open");
        goto cleanup;
    }

    server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server == -1) {
        perror("CORE: socket");
        goto cleanup;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("CORE: bind");
        goto cleanup;
    }

    if (listen(server, 1) == -1) {
        perror("CORE: listen");
        goto cleanup;
    }

    printf("CORE: Waiting for UI connection...\n");
    fflush(stdout);

    client = accept(server, NULL, NULL);
    if (client == -1) {
        perror("CORE: accept");
        goto cleanup;
    }

    printf("CORE: UI connected.\n");
    log_message(fifo, "Logger connected. Core is ready.");

    while (1) {
        Request req;
        Response res;
        char logline[MAX_MESSAGE];

        int got = receive_request(client, &req);
        if (got == 0) {
            printf("CORE: UI disconnected.\n");
            break;
        }
        if (got == -1) {
            perror("CORE: receive");
            break;
        }

        printf("CORE: Received option %d\n", req.option);
        execute(&req, &res);
        printf("CORE: %s\n", res.message);

        /* Core -> UI: POSIX Message Queue */
        if (mq_send(mq, (const char *)&res, sizeof(res), 0) == -1) {
            perror("CORE: mq_send");
            break;
        }

        /* Core -> Logger: FIFO */
        snprintf(logline, sizeof(logline), "%s | Result: %d",
                 res.message, res.result);
        log_message(fifo, logline);

        if (req.option == 13)
            break;
    }

cleanup:
    if (client != -1) close(client);
    if (server != -1) close(server);
    if (fifo != -1) close(fifo);
    if (mq != (mqd_t)-1) mq_close(mq);

    mq_unlink(MQ_NAME);
    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);

    printf("CORE: Shutting down.\n");
    return 0;
}
