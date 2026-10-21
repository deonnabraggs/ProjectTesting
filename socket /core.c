#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <mqueue.h>

#include "ipc.h"

static int memory[MEMORY_SIZE];
static int stack[STACK_SIZE];
static int top = -1;

static int queue[QUEUE_SIZE];
static int front = 0;
static int rear = 0;
static int count = 0;

static int read_all(int fd, void *buffer, size_t len)
{
    size_t done = 0;
    char *p = (char *)buffer;

    while (done < len)
    {
        ssize_t n = recv(fd, p + done, len - done, 0);

        if (n == 0)
            return 0;

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        done += (size_t)n;
    }

    return 1;
}

static int write_all(int fd, const void *buffer, size_t len)
{
    size_t done = 0;
    const char *p = (const char *)buffer;

    while (done < len)
    {
        ssize_t n = write(fd, p + done, len - done);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return -1;

        done += (size_t)n;
    }

    return 0;
}

/* ================= MEMORY ================= */

static void initialize_memory(void)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
        memory[i] = 0;

    printf("[MEMORY] Initialized\n");
}

static int write_memory(int address, int value)
{
    if (address < 0 || address >= MEMORY_SIZE)
    {
        printf("[MEMORY] Invalid address\n");
        return -1;
    }

    memory[address] = value;

    printf("[MEMORY] memory[%d] = %d\n", address, value);

    return 0;
}

static int read_memory(int address, int *value)
{
    if (address < 0 || address >= MEMORY_SIZE)
    {
        printf("[MEMORY] Invalid address\n");
        return -1;
    }

    *value = memory[address];

    printf("[MEMORY] memory[%d] = %d\n", address, *value);

    return 0;
}

/* ================= STACK ================= */

static void initialize_stack(void)
{
    top = -1;

    printf("[STACK] Initialized\n");
}

static int push(int value)
{
    if (top >= STACK_SIZE - 1)
    {
        printf("[STACK] Overflow\n");
        return -1;
    }

    stack[++top] = value;

    printf("[STACK] PUSH %d\n", value);

    return 0;
}

static int pop(int *value)
{
    if (top < 0)
    {
        printf("[STACK] Underflow\n");
        return -1;
    }

    *value = stack[top--];

    printf("[STACK] POP %d\n", *value);

    return 0;
}

static int peek(int *value)
{
    if (top < 0)
    {
        printf("[STACK] Empty\n");
        return -1;
    }

    *value = stack[top];

    printf("[STACK] PEEK %d\n", *value);

    return 0;
}

/* ================= QUEUE ================= */

static void initialize_queue(void)
{
    front = 0;
    rear = 0;
    count = 0;

    printf("[QUEUE] Initialized\n");
}

static int enqueue(int value)
{
    if (count >= QUEUE_SIZE)
    {
        printf("[QUEUE] Overflow\n");
        return -1;
    }

    queue[rear] = value;
    rear = (rear + 1) % QUEUE_SIZE;
    count++;

    printf("[QUEUE] ENQUEUE %d\n", value);

    return 0;
}

static int dequeue(int *value)
{
    if (count == 0)
    {
        printf("[QUEUE] Empty\n");
        return -1;
    }

    *value = queue[front];
    front = (front + 1) % QUEUE_SIZE;
    count--;

    printf("[QUEUE] DEQUEUE %d\n", *value);

    return 0;
}

static int queue_peek(int *value)
{
    if (count == 0)
    {
        printf("[QUEUE] Empty\n");
        return -1;
    }

    *value = queue[front];

    printf("[QUEUE] FRONT %d\n", *value);

    return 0;
}

/* ================= CPU ================= */

static int execute_cpu(
    const char *instruction,
    int value1,
    int value2,
    int *result)
{
    if (strcmp(instruction, "ADD") == 0)
    {
        *result = value1 + value2;
        return 0;
    }

    if (strcmp(instruction, "SUB") == 0)
    {
        *result = value1 - value2;
        return 0;
    }

    if (strcmp(instruction, "MUL") == 0)
    {
        *result = value1 * value2;
        return 0;
    }

    if (strcmp(instruction, "DIV") == 0)
    {
        if (value2 == 0)
        {
            printf("[CPU] Error: Division by zero\n");
            return -1;
        }

        *result = value1 / value2;
        return 0;
    }

    return -1;
}

/* ================= CORE PROCESS ================= */

int main(void)
{
    int server_fd = -1;
    int client_fd = -1;
    int fifo_fd = -1;

    mqd_t response_queue = (mqd_t)-1;

    struct sockaddr_un address;
    struct mq_attr attr;

    CoreMessage request;
    LogMessage response;

    int exit_requested = 0;

    printf("\n");
    printf("====================================\n");
    printf("         CORE PROCESS STARTED\n");
    printf("====================================\n");

    initialize_memory();
    initialize_stack();
    initialize_queue();

    /* Create POSIX message queue: Core -> UI */

    memset(&attr, 0, sizeof(attr));

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(LogMessage);

    mq_unlink(CORE_TO_UI);

    response_queue = mq_open(
        CORE_TO_UI,
        O_CREAT | O_WRONLY,
        0666,
        &attr
    );

    if (response_queue == (mqd_t)-1)
    {
        perror("[CORE] mq_open CORE_TO_UI");
        goto cleanup;
    }

    /* Create named FIFO: Core -> Logger */

    unlink(FIFO_PATH);

    if (mkfifo(FIFO_PATH, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("[CORE] mkfifo");
            goto cleanup;
        }
    }

    /* Create UNIX domain socket: UI -> Core */

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("[CORE] socket");
        goto cleanup;
    }

    memset(&address, 0, sizeof(address));

    address.sun_family = AF_UNIX;

    snprintf(
        address.sun_path,
        sizeof(address.sun_path),
        "%s",
        SOCKET_PATH
    );

    unlink(SOCKET_PATH);

    if (bind(
            server_fd,
            (struct sockaddr *)&address,
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

    printf("[CORE] UNIX domain socket ready: %s\n", SOCKET_PATH);
    printf("[CORE] Waiting for Logger to open FIFO...\n");

    fifo_fd = open(FIFO_PATH, O_WRONLY);

    if (fifo_fd == -1)
    {
        perror("[CORE] open FIFO");
        goto cleanup;
    }

    printf("[CORE] Named FIFO connected successfully\n");
    printf("[CORE] Waiting for UI connection...\n");

    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1)
    {
        perror("[CORE] accept");
        goto cleanup;
    }

    printf("[CORE] UI connected successfully\n");
    printf("\n[CORE] Waiting for commands from UI...\n");

    while (!exit_requested)
    {
        memset(&request, 0, sizeof(request));
        memset(&response, 0, sizeof(response));

        /* Receive UI command through UNIX socket */

        int received = read_all(
            client_fd,
            &request,
            sizeof(request)
        );

        if (received == 0)
        {
            printf("[CORE] UI disconnected\n");
            break;
        }

        if (received == -1)
        {
            perror("[CORE] socket receive");
            break;
        }

        printf("\n");
        printf("====================================\n");
        printf("         COMMAND RECEIVED\n");
        printf("====================================\n");

        printf("Instruction : %s\n", request.instruction);
        printf("Value 1     : %d\n", request.value1);
        printf("Value 2     : %d\n", request.value2);

        response.status = 0;
        response.result = -999999;

        /* ================= EXIT ================= */

        if (strcmp(request.instruction, "EXIT") == 0)
        {
            snprintf(
                response.message,
                MAX_MESSAGE,
                "Core process shutting down"
            );

            exit_requested = 1;
        }

        /* ================= CPU ================= */

        else if (
            strcmp(request.instruction, "ADD") == 0 ||
            strcmp(request.instruction, "SUB") == 0 ||
            strcmp(request.instruction, "MUL") == 0 ||
            strcmp(request.instruction, "DIV") == 0)
        {
            int result;

            printf("[CORE] Sending instruction to CPU...\n");

            if (execute_cpu(
                    request.instruction,
                    request.value1,
                    request.value2,
                    &result) == 0)
            {
                response.result = result;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "CPU: %s result = %d",
                    request.instruction,
                    result
                );

                printf("[CPU] Result = %d\n", result);
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "CPU: Error executing %s",
                    request.instruction
                );
            }
        }

        /* ================= STORE ================= */

        else if (strcmp(request.instruction, "STORE") == 0)
        {
            if (write_memory(
                    request.value1,
                    request.value2) == 0)
            {
                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Memory: stored %d at address %d",
                    request.value2,
                    request.value1
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Memory: invalid address"
                );
            }
        }

        /* ================= LOAD ================= */

        else if (strcmp(request.instruction, "LOAD") == 0)
        {
            int value;

            if (read_memory(request.value1, &value) == 0)
            {
                response.result = value;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Memory: address %d = %d",
                    request.value1,
                    value
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Memory: invalid address"
                );
            }
        }

        /* ================= PUSH ================= */

        else if (strcmp(request.instruction, "PUSH") == 0)
        {
            if (push(request.value1) == 0)
            {
                response.result = request.value1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Stack: pushed %d",
                    request.value1
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Stack: overflow"
                );
            }
        }

        /* ================= POP ================= */

        else if (strcmp(request.instruction, "POP") == 0)
        {
            int value;

            if (pop(&value) == 0)
            {
                response.result = value;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Stack: popped %d",
                    value
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Stack: underflow"
                );
            }
        }

        /* ================= PEEK ================= */

        else if (strcmp(request.instruction, "PEEK") == 0)
        {
            int value;

            if (peek(&value) == 0)
            {
                response.result = value;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Stack: top = %d",
                    value
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Stack: empty"
                );
            }
        }

        /* ================= ENQUEUE ================= */

        else if (strcmp(request.instruction, "ENQUEUE") == 0)
        {
            if (enqueue(request.value1) == 0)
            {
                response.result = request.value1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Queue: enqueued %d",
                    request.value1
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Queue: overflow"
                );
            }
        }

        /* ================= DEQUEUE ================= */

        else if (strcmp(request.instruction, "DEQUEUE") == 0)
        {
            int value;

            if (dequeue(&value) == 0)
            {
                response.result = value;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Queue: dequeued %d",
                    value
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Queue: empty"
                );
            }
        }

        /* ================= QUEUE PEEK ================= */

        else if (strcmp(request.instruction, "QUEUE_PEEK") == 0)
        {
            int value;

            if (queue_peek(&value) == 0)
            {
                response.result = value;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Queue: front = %d",
                    value
                );
            }
            else
            {
                response.status = 1;

                snprintf(
                    response.message,
                    MAX_MESSAGE,
                    "Queue: empty"
                );
            }
        }

        /* ================= UNKNOWN ================= */

        else
        {
            response.status = 1;

            snprintf(
                response.message,
                MAX_MESSAGE,
                "Unknown instruction: %s",
                request.instruction
            );
        }

        /* Send response to UI through POSIX message queue */

        if (mq_send(
                response_queue,
                (const char *)&response,
                sizeof(response),
                0) == -1)
        {
            perror("[CORE] mq_send UI");
        }

        /* Send log to Logger through named FIFO */

        if (write_all(
                fifo_fd,
                &response,
                sizeof(response)) == -1)
        {
            perror("[CORE] FIFO write");
            break;
        }

        printf("[CORE] Response sent to UI\n");
        printf("[CORE] Log sent to Logger\n");

        if (!exit_requested)
            printf("\n[CORE] Waiting for next command...\n");
    }

cleanup:

    if (client_fd != -1)
        close(client_fd);

    if (server_fd != -1)
        close(server_fd);

    if (fifo_fd != -1)
        close(fifo_fd);

    if (response_queue != (mqd_t)-1)
        mq_close(response_queue);

    mq_unlink(CORE_TO_UI);
    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);

    printf("\n[CORE] Process terminated\n");

    return 0;
}
