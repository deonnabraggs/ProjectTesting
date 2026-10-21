#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
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
static int rear = -1;
static int count = 0;

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

static int write_memory(int address, int value)
{
    if (address < 0 || address >= MEMORY_SIZE)
        return -1;

    memory[address] = value;
    return 0;
}

static int read_memory(int address, int *value)
{
    if (address < 0 || address >= MEMORY_SIZE)
        return -1;

    *value = memory[address];
    return 0;
}

static int push(int value)
{
    if (top >= STACK_SIZE - 1)
        return -1;

    stack[++top] = value;
    return 0;
}

static int pop(int *value)
{
    if (top < 0)
        return -1;

    *value = stack[top--];
    return 0;
}

static int peek(int *value)
{
    if (top < 0)
        return -1;

    *value = stack[top];
    return 0;
}

static int enqueue(int value)
{
    if (count >= QUEUE_SIZE)
        return -1;

    rear = (rear + 1) % QUEUE_SIZE;
    queue[rear] = value;
    count++;
    return 0;
}

static int dequeue(int *value)
{
    if (count == 0)
        return -1;

    *value = queue[front];
    front = (front + 1) % QUEUE_SIZE;
    count--;
    return 0;
}

static int queue_peek(int *value)
{
    if (count == 0)
        return -1;

    *value = queue[front];
    return 0;
}

static int execute_cpu(const char *instruction, int a, int b, int *result)
{
    if (strcmp(instruction, "ADD") == 0)
        *result = a + b;
    else if (strcmp(instruction, "SUB") == 0)
        *result = a - b;
    else if (strcmp(instruction, "MUL") == 0)
        *result = a * b;
    else if (strcmp(instruction, "DIV") == 0)
    {
        if (b == 0 || (a == INT_MIN && b == -1))
            return -1;

        *result = a / b;
    }
    else
        return -1;

    return 0;
}

static void set_error(LogMessage *response, const char *message)
{
    response->status = 1;
    response->result = -999999;
    snprintf(response->message, sizeof(response->message), "%s", message);
}

static void set_success(LogMessage *response, const char *message,
                        int result)
{
    response->status = 0;
    response->result = result;
    snprintf(response->message, sizeof(response->message), "%s", message);
}

static void send_log(mqd_t logger_mq, int fifo_fd,
                     const LogMessage *message)
{
    if (mq_send(logger_mq, (const char *)message,
                sizeof(*message), 0) == -1)
        perror("[CORE] mq_send");

    ssize_t n = write(fifo_fd, message, sizeof(*message));

    if (n != (ssize_t)sizeof(*message))
        perror("[CORE] FIFO write");
}

int main(void)
{
    int server_fd, client_fd, fifo_fd;
    mqd_t logger_mq;
    struct sockaddr_un address;
    CoreMessage request;
    LogMessage response;

    memset(memory, 0, sizeof(memory));

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("[CORE] socket");
        return 1;
    }

    unlink(SOCKET_PATH);

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) == -1)
    {
        perror("[CORE] bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 1) == -1)
    {
        perror("[CORE] listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    logger_mq = mq_open(CORE_TO_LOGGER, O_WRONLY);
    if (logger_mq == (mqd_t)-1)
    {
        perror("[CORE] mq_open");
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1)
    {
        perror("[CORE] FIFO open");
        mq_close(logger_mq);
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("[CORE] Core process started\n");
    printf("[CORE] Waiting for UI connection...\n");
    fflush(stdout);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1)
    {
        perror("[CORE] accept");
        close(fifo_fd);
        mq_close(logger_mq);
        close(server_fd);
        unlink(SOCKET_PATH);
        return 1;
    }

    printf("[CORE] UI connected successfully\n");

    while (1)
    {
        memset(&request, 0, sizeof(request));
        memset(&response, 0, sizeof(response));
        response.result = -999999;

        if (receive_all(client_fd, &request, sizeof(request)) == -1)
        {
            printf("[CORE] UI disconnected\n");
            break;
        }

        printf("[CORE] Received instruction: %s\n", request.instruction);

        if (strcmp(request.instruction, "EXIT") == 0)
        {
            set_success(&response, "Core process shutting down",
                        -999999);

            send_all(client_fd, &response, sizeof(response));
            send_log(logger_mq, fifo_fd, &response);
            break;
        }
        else if (strcmp(request.instruction, "ADD") == 0 ||
                 strcmp(request.instruction, "SUB") == 0 ||
                 strcmp(request.instruction, "MUL") == 0 ||
                 strcmp(request.instruction, "DIV") == 0)
        {
            int result;

            if (execute_cpu(request.instruction, request.value1,
                            request.value2, &result) == 0)
            {
                char message[MAX_MESSAGE];

                snprintf(message, sizeof(message),
                         "%s operation executed successfully",
                         request.instruction);
                set_success(&response, message, result);
            }
            else
                set_error(&response,
                          "Invalid operation or division by zero");
        }
        else if (strcmp(request.instruction, "STORE") == 0)
        {
            if (write_memory(request.value1, request.value2) == 0)
                set_success(&response, "Value stored in memory",
                            request.value2);
            else
                set_error(&response, "Invalid memory address");
        }
        else if (strcmp(request.instruction, "LOAD") == 0)
        {
            int value;

            if (read_memory(request.value1, &value) == 0)
                set_success(&response, "Value loaded from memory", value);
            else
                set_error(&response, "Invalid memory address");
        }
        else if (strcmp(request.instruction, "PUSH") == 0)
        {
            if (push(request.value1) == 0)
                set_success(&response, "Value pushed onto stack",
                            request.value1);
            else
                set_error(&response, "Stack overflow");
        }
        else if (strcmp(request.instruction, "POP") == 0)
        {
            int value;

            if (pop(&value) == 0)
                set_success(&response, "Value popped from stack", value);
            else
                set_error(&response, "Stack is empty");
        }
        else if (strcmp(request.instruction, "PEEK") == 0)
        {
            int value;

            if (peek(&value) == 0)
                set_success(&response, "Top stack value", value);
            else
                set_error(&response, "Stack is empty");
        }
        else if (strcmp(request.instruction, "ENQUEUE") == 0)
        {
            if (enqueue(request.value1) == 0)
                set_success(&response, "Value added to queue",
                            request.value1);
            else
                set_error(&response, "Queue is full");
        }
        else if (strcmp(request.instruction, "DEQUEUE") == 0)
        {
            int value;

            if (dequeue(&value) == 0)
                set_success(&response, "Value removed from queue", value);
            else
                set_error(&response, "Queue is empty");
        }
        else if (strcmp(request.instruction, "QUEUE_PEEK") == 0)
        {
            int value;

            if (queue_peek(&value) == 0)
                set_success(&response, "Front queue value", value);
            else
                set_error(&response, "Queue is empty");
        }
        else
            set_error(&response, "Unknown instruction");

        if (send_all(client_fd, &response, sizeof(response)) == -1)
        {
            perror("[CORE] socket send");
            break;
        }

        send_log(logger_mq, fifo_fd, &response);
    }

    close(client_fd);
    close(server_fd);
    close(fifo_fd);
    mq_close(logger_mq);
    unlink(SOCKET_PATH);

    printf("[CORE] Process terminated\n");
    return 0;
}
