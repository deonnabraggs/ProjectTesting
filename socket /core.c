#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "ipc.h"

static int memory[MEMORY_SIZE];

static int stack[STACK_SIZE];
static int top = -1;

static int queue[QUEUE_SIZE];
static int front = 0;
static int rear = 0;
static int count = 0;

void initialize_memory(void)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
        memory[i] = 0;

    printf("[MEMORY] Initialized\n");
}

void initialize_stack(void)
{
    top = -1;
    printf("[STACK] Initialized\n");
}

void initialize_queue(void)
{
    front = rear = count = 0;
    printf("[QUEUE] Initialized\n");
}

int write_memory(int address, int value)
{
    if (address < 0 || address >= MEMORY_SIZE)
        return -1;

    memory[address] = value;
    return 0;
}

int read_memory(int address, int *value)
{
    if (address < 0 || address >= MEMORY_SIZE)
        return -1;

    *value = memory[address];
    return 0;
}

int push(int value)
{
    if (top >= STACK_SIZE - 1)
        return -1;

    stack[++top] = value;
    return 0;
}

int pop(int *value)
{
    if (top < 0)
        return -1;

    *value = stack[top--];
    return 0;
}

int peek(int *value)
{
    if (top < 0)
        return -1;

    *value = stack[top];
    return 0;
}

int enqueue(int value)
{
    if (count >= QUEUE_SIZE)
        return -1;

    queue[rear] = value;
    rear = (rear + 1) % QUEUE_SIZE;
    count++;
    return 0;
}

int dequeue(int *value)
{
    if (count == 0)
        return -1;

    *value = queue[front];
    front = (front + 1) % QUEUE_SIZE;
    count--;
    return 0;
}

int queue_peek(int *value)
{
    if (count == 0)
        return -1;

    *value = queue[front];
    return 0;
}

int execute_cpu(const char *instruction,
                int a, int b, int *result)
{
    if (strcmp(instruction, "ADD") == 0)
        *result = a + b;
    else if (strcmp(instruction, "SUB") == 0)
        *result = a - b;
    else if (strcmp(instruction, "MUL") == 0)
        *result = a * b;
    else if (strcmp(instruction, "DIV") == 0)
    {
        if (b == 0)
            return -1;

        *result = a / b;
    }
    else
        return -1;

    return 0;
}

static int recv_all(int fd, void *buffer, size_t length)
{
    size_t received = 0;
    char *p = buffer;

    while (received < length)
    {
        ssize_t n = recv(fd, p + received,
                         length - received, 0);

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

static int write_all(int fd, const void *buffer, size_t length)
{
    size_t sent = 0;
    const char *p = buffer;

    while (sent < length)
    {
        ssize_t n = write(fd, p + sent, length - sent);

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

static void send_result(mqd_t mq, int fifo_fd,
                        const LogMessage *response)
{
    if (mq_send(mq, (const char *)response,
                sizeof(*response), 0) == -1)
        perror("[CORE] mq_send");

    if (write_all(fifo_fd, response,
                  sizeof(*response)) == -1)
        perror("[CORE] FIFO write");
}

static void process_request(const CoreMessage *req,
                            LogMessage *res, int *exit_requested)
{
    int result = 0;
    int rc = -1;

    res->status = 0;
    res->result = -999999;
    res->message[0] = '\0';
    *exit_requested = 0;

    if (strcmp(req->instruction, "EXIT") == 0)
    {
        snprintf(res->message, MAX_MESSAGE,
                 "Core process shutting down");
        *exit_requested = 1;
        return;
    }

    if (strcmp(req->instruction, "ADD") == 0 ||
        strcmp(req->instruction, "SUB") == 0 ||
        strcmp(req->instruction, "MUL") == 0 ||
        strcmp(req->instruction, "DIV") == 0)
    {
        rc = execute_cpu(req->instruction,
                         req->value1, req->value2, &result);

        if (rc == 0)
        {
            res->result = result;
            snprintf(res->message, MAX_MESSAGE,
                     "CPU: %s result = %d",
                     req->instruction, result);
        }
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE,
                     "CPU error: invalid operation or division by zero");
        }
    }
    else if (strcmp(req->instruction, "STORE") == 0)
    {
        rc = write_memory(req->value1, req->value2);

        if (rc == 0)
            snprintf(res->message, MAX_MESSAGE,
                     "Memory: stored %d at address %d",
                     req->value2, req->value1);
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE,
                     "Memory: invalid address");
        }
    }
    else if (strcmp(req->instruction, "LOAD") == 0)
    {
        rc = read_memory(req->value1, &result);

        if (rc == 0)
        {
            res->result = result;
            snprintf(res->message, MAX_MESSAGE,
                     "Memory: address %d = %d",
                     req->value1, result);
        }
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE,
                     "Memory: invalid address");
        }
    }
    else if (strcmp(req->instruction, "PUSH") == 0)
    {
        rc = push(req->value1);

        if (rc == 0)
            snprintf(res->message, MAX_MESSAGE,
                     "Stack: pushed %d", req->value1);
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE, "Stack overflow");
        }
    }
    else if (strcmp(req->instruction, "POP") == 0)
    {
        rc = pop(&result);

        if (rc == 0)
        {
            res->result = result;
            snprintf(res->message, MAX_MESSAGE,
                     "Stack: popped %d", result);
        }
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE, "Stack underflow");
        }
    }
    else if (strcmp(req->instruction, "PEEK") == 0)
    {
        rc = peek(&result);

        if (rc == 0)
        {
            res->result = result;
            snprintf(res->message, MAX_MESSAGE,
                     "Stack top = %d", result);
        }
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE, "Stack is empty");
        }
    }
    else if (strcmp(req->instruction, "ENQUEUE") == 0)
    {
        rc = enqueue(req->value1);

        if (rc == 0)
            snprintf(res->message, MAX_MESSAGE,
                     "Queue: enqueued %d", req->value1);
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE, "Queue overflow");
        }
    }
    else if (strcmp(req->instruction, "DEQUEUE") == 0)
    {
        rc = dequeue(&result);

        if (rc == 0)
        {
            res->result = result;
            snprintf(res->message, MAX_MESSAGE,
                     "Queue: dequeued %d", result);
        }
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE, "Queue is empty");
        }
    }
    else if (strcmp(req->instruction, "QUEUE_PEEK") == 0)
    {
        rc = queue_peek(&result);

        if (rc == 0)
        {
            res->result = result;
            snprintf(res->message, MAX_MESSAGE,
                     "Queue front = %d", result);
        }
        else
        {
            res->status = 1;
            snprintf(res->message, MAX_MESSAGE, "Queue is empty");
        }
    }
    else
    {
        res->status = 1;
        snprintf(res->message, MAX_MESSAGE, "Unknown instruction");
    }
}

int main(void)
{
    int server_fd = -1, client_fd = -1, fifo_fd = -1;
    mqd_t mq = (mqd_t)-1;
    int exit_requested = 0;

    struct mq_attr attr;
    struct sockaddr_un address;
    CoreMessage request;
    LogMessage response;

    printf("========== CORE PROCESS ==========\n");

    initialize_memory();
    initialize_stack();
    initialize_queue();

    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(LogMessage);

    mq_unlink(CORE_TO_UI);
    mq = mq_open(CORE_TO_UI, O_CREAT | O_WRONLY, 0666, &attr);

    if (mq == (mqd_t)-1)
    {
        perror("[CORE] mq_open");
        return 1;
    }

    unlink(FIFO_PATH);

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST)
    {
        perror("[CORE] mkfifo");
        goto cleanup;
    }

    printf("[CORE] Waiting for Logger to connect...\n");
    fifo_fd = open(FIFO_PATH, O_WRONLY);

    if (fifo_fd == -1)
    {
        perror("[CORE] open FIFO");
        goto cleanup;
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("[CORE] socket");
        goto cleanup;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path),
             "%s", SOCKET_PATH);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) == -1)
    {
        perror("[CORE] bind");
        goto cleanup;
    }

    if (listen(server_fd, 1) == -1)
    {
        perror("[CORE] listen");
        goto cleanup;
    }

    printf("[CORE] Waiting for UI socket connection...\n");
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1)
    {
        perror("[CORE] accept");
        goto cleanup;
    }

    printf("[CORE] UI connected. Ready for commands.\n");

    while (1)
    {
        int received = recv_all(client_fd, &request, sizeof(request));

        if (received == 0)
        {
            printf("[CORE] UI disconnected\n");
            break;
        }

        if (received < 0)
        {
            perror("[CORE] recv");
            break;
        }

        printf("[CORE] Command: %s\n", request.instruction);

        process_request(&request, &response, &exit_requested);
        send_result(mq, fifo_fd, &response);

        printf("[CORE] %s\n", response.message);

        if (exit_requested)
            break;
    }

cleanup:
    if (client_fd >= 0)
        close(client_fd);
    if (server_fd >= 0)
        close(server_fd);
    if (fifo_fd >= 0)
        close(fifo_fd);
    if (mq != (mqd_t)-1)
        mq_close(mq);

    mq_unlink(CORE_TO_UI);
    unlink(FIFO_PATH);
    unlink(SOCKET_PATH);

    printf("[CORE] Terminated\n");
    return 0;
}
