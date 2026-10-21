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
#include <limits.h>
#include "ipc.h"

static int memory[MEMORY_SIZE];
static int stack[STACK_SIZE];
static int top = -1;
static int queue[QUEUE_SIZE];
static int front = 0, rear = 0, count = 0;

static void initialize_data(void)
{
    memset(memory, 0, sizeof(memory));
    top = -1;
    front = rear = count = 0;
    puts("[CORE] Memory, stack and queue initialized");
}

static int write_memory(int address, int value)
{
    if (address < 0 || address >= MEMORY_SIZE) return -1;
    memory[address] = value;
    printf("[MEMORY] memory[%d] = %d\n", address, value);a
    return 0;
}

static int read_memory(int address, int *value)
{
    if (address < 0 || address >= MEMORY_SIZE) return -1;
    *value = memory[address];
    printf("[MEMORY] memory[%d] = %d\n", address, *value);
    return 0;
}

static int push(int value)
{
    if (top >= STACK_SIZE - 1) return -1;
    stack[++top] = value;
    printf("[STACK] PUSH %d\n", value);
    return 0;
}

static int pop(int *value)
{
    if (top < 0) return -1;
    *value = stack[top--];
    printf("[STACK] POP %d\n", *value);
    return 0;
}

static int peek(int *value)
{
    if (top < 0) return -1;
    *value = stack[top];
    printf("[STACK] PEEK %d\n", *value);
    return 0;
}

static int enqueue(int value)
{
    if (count >= QUEUE_SIZE) return -1;
    queue[rear] = value;
    rear = (rear + 1) % QUEUE_SIZE;
    count++;
    printf("[QUEUE] ENQUEUE %d\n", value);
    return 0;
}

static int dequeue(int *value)
{
    if (count == 0) return -1;
    *value = queue[front];
    front = (front + 1) % QUEUE_SIZE;
    count--;
    printf("[QUEUE] DEQUEUE %d\n", *value);
    return 0;
}

static int queue_peek(int *value)
{
    if (count == 0) return -1;
    *value = queue[front];
    printf("[QUEUE] FRONT %d\n", *value);
    return 0;
}

static int execute_cpu(const char *instruction, int a, int b, int *result)
{
    if (strcmp(instruction, "ADD") == 0) *result = a + b;
    else if (strcmp(instruction, "SUB") == 0) *result = a - b;
    else if (strcmp(instruction, "MUL") == 0) *result = a * b;
    else if (strcmp(instruction, "DIV") == 0)
    {
        if (b == 0) return -1;
        *result = a / b;
    }
    else return -1;
    return 0;
}

static int recv_all(int fd, void *buffer, size_t len)
{
    size_t received = 0;
    char *p = buffer;
    while (received < len)
    {
        ssize_t n = recv(fd, p + received, len - received, 0);
        if (n == 0) return 0;
        if (n < 0)
        {
            if (errno == EINTR) continue;
            return -1;
        }
        received += (size_t)n;
    }
    return 1;
}

static int write_all(int fd, const void *buffer, size_t len)
{
    size_t sent = 0;
    const char *p = buffer;
    while (sent < len)
    {
        ssize_t n = write(fd, p + sent, len - sent);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

static void send_result(mqd_t response_queue, int fifo_fd, const LogMessage *response)
{
    if (mq_send(response_queue, (const char *)response, sizeof(*response), 0) == -1)
        perror("[CORE] mq_send UI");
    if (write_all(fifo_fd, response, sizeof(*response)) == -1)
        perror("[CORE] write Logger FIFO");
}

static void handle_request(const CoreMessage *request, LogMessage *response)
{
    int result = 0;
    response->status = 0;
    response->result = -999999;
    response->message[0] = '\0';

    if (strcmp(request->instruction, "EXIT") == 0)
        snprintf(response->message, sizeof(response->message), "Core process shutting down");
    else if (strcmp(request->instruction, "ADD") == 0 ||
             strcmp(request->instruction, "SUB") == 0 ||
             strcmp(request->instruction, "MUL") == 0 ||
             strcmp(request->instruction, "DIV") == 0)
    {
        if (execute_cpu(request->instruction, request->value1, request->value2, &result) == 0)
        {
            response->result = result;
            snprintf(response->message, sizeof(response->message), "CPU: %.8s result = %d",
                     request->instruction, result);
        }
        else
        {
            response->status = 1;
            snprintf(response->message, sizeof(response->message),
                     "CPU: Error executing %.8s (check division by zero)", request->instruction);
        }
    }
    else if (strcmp(request->instruction, "STORE") == 0)
    {
        if (write_memory(request->value1, request->value2) == 0)
            snprintf(response->message, sizeof(response->message), "Memory: stored %d at address %d",
                     request->value2, request->value1);
        else { response->status = 1; snprintf(response->message, sizeof(response->message),
                                               "Memory: invalid address (0-%d)", MEMORY_SIZE - 1); }
    }
    else if (strcmp(request->instruction, "LOAD") == 0)
    {
        if (read_memory(request->value1, &result) == 0)
        {
            response->result = result;
            snprintf(response->message, sizeof(response->message), "Memory: address %d = %d",
                     request->value1, result);
        }
        else { response->status = 1; snprintf(response->message, sizeof(response->message),
                                               "Memory: invalid address (0-%d)", MEMORY_SIZE - 1); }
    }
    else if (strcmp(request->instruction, "PUSH") == 0)
    {
        if (push(request->value1) == 0)
            snprintf(response->message, sizeof(response->message), "Stack: pushed %d", request->value1);
        else { response->status = 1; snprintf(response->message, sizeof(response->message), "Stack overflow"); }
    }
    else if (strcmp(request->instruction, "POP") == 0)
    {
        if (pop(&result) == 0) { response->result = result; snprintf(response->message, sizeof(response->message), "Stack: popped %d", result); }
        else { response->status = 1; snprintf(response->message, sizeof(response->message), "Stack underflow: stack is empty"); }
    }
    else if (strcmp(request->instruction, "PEEK") == 0)
    {
        if (peek(&result) == 0) { response->result = result; snprintf(response->message, sizeof(response->message), "Stack top = %d", result); }
        else { response->status = 1; snprintf(response->message, sizeof(response->message), "Stack is empty"); }
    }
    else if (strcmp(request->instruction, "ENQUEUE") == 0)
    {
        if (enqueue(request->value1) == 0)
            snprintf(response->message, sizeof(response->message), "Queue: enqueued %d", request->value1);
        else { response->status = 1; snprintf(response->message, sizeof(response->message), "Queue overflow"); }
    }
    else if (strcmp(request->instruction, "DEQUEUE") == 0)
    {
        if (dequeue(&result) == 0) { response->result = result; snprintf(response->message, sizeof(response->message), "Queue: dequeued %d", result); }
        else { response->status = 1; snprintf(response->message, sizeof(response->message), "Queue is empty"); }
    }
    else if (strcmp(request->instruction, "QUEUE_PEEK") == 0)
    {
        if (queue_peek(&result) == 0) { response->result = result; snprintf(response->message, sizeof(response->message), "Queue front = %d", result); }
        else { response->status = 1; snprintf(response->message, sizeof(response->message), "Queue is empty"); }
    }
    else
    {
        response->status = 1;
        snprintf(response->message, sizeof(response->message), "Unknown command");
    }
}

int main(void)
{
    int server_fd = -1, client_fd = -1, fifo_fd = -1;
    mqd_t response_queue = (mqd_t)-1;
    struct mq_attr attr;
    struct sockaddr_un address;
    CoreMessage request;
    LogMessage response;

    puts("\n====================================\n         CORE PROCESS STARTED\n====================================");

    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(LogMessage);
    mq_unlink(CORE_TO_UI);
    response_queue = mq_open(CORE_TO_UI, O_CREAT | O_WRONLY, 0666, &attr);
    if (response_queue == (mqd_t)-1) { perror("[CORE] mq_open"); return 1; }

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST)
    {
        perror("[CORE] mkfifo"); goto cleanup;
    }
    puts("[CORE] Waiting for Logger to open FIFO...");
    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) { perror("[CORE] open FIFO"); goto cleanup; }
    puts("[CORE] Logger connected");

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) { perror("[CORE] socket"); goto cleanup; }
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    snprintf(address.sun_path, sizeof(address.sun_path), "%s", SOCKET_PATH);
    unlink(SOCKET_PATH);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1)
    { perror("[CORE] bind"); goto cleanup; }
    if (listen(server_fd, 1) == -1) { perror("[CORE] listen"); goto cleanup; }

    initialize_data();
    printf("[CORE] Socket ready: %s\n[CORE] Waiting for UI...\n", SOCKET_PATH);
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) { perror("[CORE] accept"); goto cleanup; }

    puts("[CORE] UI connected");
    while (1)
    {
        int received = recv_all(client_fd, &request, sizeof(request));
        if (received == 0) { puts("[CORE] UI disconnected"); break; }
        if (received < 0) { perror("[CORE] recv"); break; }

        request.instruction[MAX_MESSAGE - 1] = '\0';
        printf("\n[CORE] Command: %s | value1=%d | value2=%d\n",
               request.instruction, request.value1, request.value2);
        handle_request(&request, &response);
        send_result(response_queue, fifo_fd, &response);
        printf("[CORE] Response: %s\n", response.message);
        if (strcmp(request.instruction, "EXIT") == 0) break;
    }

cleanup:
    if (client_fd >= 0) close(client_fd);
    if (server_fd >= 0) close(server_fd);
    if (fifo_fd >= 0) close(fifo_fd);
    if (response_queue != (mqd_t)-1) mq_close(response_queue);
    mq_unlink(CORE_TO_UI);
    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);
    puts("[CORE] Process terminated");
    return 0;
}
