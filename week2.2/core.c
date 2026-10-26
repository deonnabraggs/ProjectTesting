
#include "ipc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <mqueue.h>

static int memory[MEMORY_SIZE];
static int valid[MEMORY_SIZE];
static int stack[STACK_SIZE], top = -1;
static int queue[QUEUE_SIZE], front = 0, rear = 0, count = 0;

static int read_full(int fd, void *buf, size_t size)
{
    size_t nread = 0;
    while (nread < size) {
        ssize_t n = read(fd, (char *)buf + nread, size - nread);
        if (n == 0) return nread == 0 ? 0 : -1;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        nread += (size_t)n;
    }
    return 1;
}

static int write_full(int fd, const void *buf, size_t size)
{
    size_t nwritten = 0;
    while (nwritten < size) {
        ssize_t n = write(fd, (const char *)buf + nwritten,
                          size - nwritten);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1;
        nwritten += (size_t)n;
    }
    return 0;
}

static void reply(CoreResponse *r, int ok, int value, const char *msg)
{
    r->status = ok;
    r->result = value;
    snprintf(r->message, sizeof(r->message), "%s", msg);
}

static void execute(CoreMessage *q, CoreResponse *r)
{
    int a = q->value1, b = q->value2;

    switch (q->command) {
    case CMD_ADD:
        reply(r, 1, a + b, "Addition completed"); break;
    case CMD_SUBTRACT:
        reply(r, 1, a - b, "Subtraction completed"); break;
    case CMD_MULTIPLY:
        reply(r, 1, a * b, "Multiplication completed"); break;
    case CMD_DIVIDE:
        if (b == 0) reply(r, 0, 0, "Error: division by zero");
        else reply(r, 1, a / b, "Integer division completed");
        break;
    case CMD_PUSH:
        if (top == STACK_SIZE - 1) reply(r, 0, 0, "Error: stack full");
        else {
            stack[++top] = a;
            reply(r, 1, a, "Value pushed onto stack");
        }
        break;
    case CMD_POP:
        if (top < 0) reply(r, 0, 0, "Error: stack empty");
        else reply(r, 1, stack[top--], "Value popped from stack");
        break;
    case CMD_STACK_PEEK:
        if (top < 0) reply(r, 0, 0, "Error: stack empty");
        else reply(r, 1, stack[top], "Top stack value");
        break;
    case CMD_ENQUEUE:
        if (count == QUEUE_SIZE) reply(r, 0, 0, "Error: queue full");
        else {
            queue[rear] = a;
            rear = (rear + 1) % QUEUE_SIZE;
            count++;
            reply(r, 1, a, "Value added to queue");
        }
        break;
    case CMD_DEQUEUE:
        if (count == 0) reply(r, 0, 0, "Error: queue empty");
        else {
            int value = queue[front];
            front = (front + 1) % QUEUE_SIZE;
            count--;
            reply(r, 1, value, "Value removed from queue");
        }
        break;
    case CMD_QUEUE_PEEK:
        if (count == 0) reply(r, 0, 0, "Error: queue empty");
        else reply(r, 1, queue[front], "Front queue value");
        break;
    case CMD_STORE:
        if (a < 0 || a >= MEMORY_SIZE)
            reply(r, 0, 0, "Error: invalid memory address");
        else {
            memory[a] = b;
            valid[a] = 1;
            reply(r, 1, b, "Value stored in memory");
        }
        break;
    case CMD_LOAD:
        if (a < 0 || a >= MEMORY_SIZE)
            reply(r, 0, 0, "Error: invalid memory address");
        else if (!valid[a])
            reply(r, 0, 0, "Error: memory address is empty");
        else reply(r, 1, memory[a], "Value loaded from memory");
        break;
    case CMD_EXIT:
        reply(r, 1, 0, "Simulator shutting down"); break;
    default:
        reply(r, 0, 0, "Invalid command"); break;
    }
}

int main(void)
{
    int server = -1, client = -1, fifo = -1;
    mqd_t mq = (mqd_t)-1;
    int exit_requested = 0;

    struct mq_attr attr = {0};
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(CoreResponse);

    mq_unlink(MQ_NAME);
    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);

    mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0600, &attr);
    if (mq == (mqd_t)-1) {
        perror("Core mq_open");
        return 1;
    }

    if (mkfifo(FIFO_PATH, 0600) == -1 && errno != EEXIST) {
        perror("Core mkfifo");
        goto cleanup;
    }

    printf("CORE: Waiting for Logger FIFO...\n");
    fflush(stdout);
    fifo = open(FIFO_PATH, O_WRONLY);
    if (fifo == -1) {
        perror("Core FIFO open");
        goto cleanup;
    }

    server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server == -1) {
        perror("Core socket");
        goto cleanup;
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) == -1 ||
        listen(server, 1) == -1) {
        perror("Core bind/listen");
        goto cleanup;
    }

    printf("CORE: Waiting for UI connection...\n");
    fflush(stdout);

    client = accept(server, NULL, NULL);
    if (client == -1) {
        perror("Core accept");
        goto cleanup;
    }

    printf("CORE: UI connected.\n");

    while (!exit_requested) {
        CoreMessage req;
        CoreResponse res;
        LogMessage log;

        int rc = read_full(client, &req, sizeof(req));
        if (rc <= 0) break;

        execute(&req, &res);

        if (mq_send(mq, (char *)&res, sizeof(res), 0) == -1) {
            perror("Core mq_send");
            break;
        }

        memset(&log, 0, sizeof(log));
        log.status = res.status;
        log.command = req.command;
        log.result = res.result;
        snprintf(log.message, sizeof(log.message), "%s", res.message);

        if (write_full(fifo, &log, sizeof(log)) == -1) {
            perror("Core FIFO write");
            break;
        }

        printf("CORE: %s\n", res.message);
        fflush(stdout);

        if (req.command == CMD_EXIT) exit_requested = 1;
    }

cleanup:
    if (client >= 0) close(client);
    if (server >= 0) close(server);

    if (fifo >= 0) {
        LogMessage stop = {0};
        stop.status = -1;
        write_full(fifo, &stop, sizeof(stop));
        close(fifo);
    }

    if (mq != (mqd_t)-1) mq_close(mq);
    mq_unlink(MQ_NAME);
    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);

    printf("CORE: Stopped.\n");
    return 0;
}
