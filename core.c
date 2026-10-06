#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "ipc.h"

/* ================= MEMORY ================= */

#define MEMORY_SIZE 100

static int memory[MEMORY_SIZE];

void initialize_memory(void)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
        memory[i] = 0;

    printf("[MEMORY] Initialized\n");
}

int write_memory(int address, int value)
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

int read_memory(int address, int *value)
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

#define STACK_SIZE 50

static int stack[STACK_SIZE];
static int top = -1;

void initialize_stack(void)
{
    top = -1;

    printf("[STACK] Initialized\n");
}

int push(int value)
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

int pop(int *value)
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

int peek(int *value)
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

#define QUEUE_SIZE 50

static int queue[QUEUE_SIZE];
static int front = 0;
static int rear = 0;
static int count = 0;

void initialize_queue(void)
{
    front = 0;
    rear = 0;
    count = 0;

    printf("[QUEUE] Initialized\n");
}

int enqueue(int value)
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

int dequeue(int *value)
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

int queue_peek(int *value)
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

int execute_cpu(
    const char *instruction,
    int value1,
    int value2,
    int *result
)
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
    mqd_t ui_queue;
    mqd_t response_queue;
    mqd_t logger_queue;

    CoreMessage request;
    LogMessage response;

    struct mq_attr core_attr;
    struct mq_attr response_attr;
    struct mq_attr logger_attr;

    /* UI -> Core */
    core_attr.mq_flags = 0;
    core_attr.mq_maxmsg = 10;
    core_attr.mq_msgsize = sizeof(CoreMessage);
    core_attr.mq_curmsgs = 0;

    /* Core -> UI */
    response_attr.mq_flags = 0;
    response_attr.mq_maxmsg = 10;
    response_attr.mq_msgsize = sizeof(LogMessage);
    response_attr.mq_curmsgs = 0;

    /* Core -> Logger */
    logger_attr.mq_flags = 0;
    logger_attr.mq_maxmsg = 10;
    logger_attr.mq_msgsize = sizeof(LogMessage);
    logger_attr.mq_curmsgs = 0;

    printf("\n");
    printf("====================================\n");
    printf("         CORE PROCESS STARTED\n");
    printf("====================================\n");

    /* Open UI -> Core */
    ui_queue = mq_open(
        UI_TO_CORE,
        O_CREAT | O_RDONLY,
        0666,
        &core_attr
    );

    if (ui_queue == (mqd_t)-1)
    {
        perror("[CORE] mq_open UI_TO_CORE");
        return 1;
    }

    /* Open Core -> UI */
    response_queue = mq_open(
        CORE_TO_UI,
        O_CREAT | O_WRONLY,
        0666,
        &response_attr
    );

    if (response_queue == (mqd_t)-1)
    {
        perror("[CORE] mq_open CORE_TO_UI");

        mq_close(ui_queue);

        return 1;
    }

    /* Open Core -> Logger */
    logger_queue = mq_open(
        CORE_TO_LOGGER,
        O_CREAT | O_WRONLY,
        0666,
        &logger_attr
    );

    if (logger_queue == (mqd_t)-1)
    {
        perror("[CORE] mq_open CORE_TO_LOGGER");

        mq_close(ui_queue);
        mq_close(response_queue);

        return 1;
    }

    initialize_memory();
    initialize_stack();
    initialize_queue();

    printf("\n[CORE] Waiting for commands from UI...\n");

    /* Keep running */
    while (1)
    {
        memset(&request, 0, sizeof(request));
        memset(&response, 0, sizeof(response));

        /* Receive command */
        if (mq_receive(
                ui_queue,
                (char *)&request,
                sizeof(CoreMessage),
                NULL) == -1)
        {
            perror("[CORE] mq_receive");
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
            strcpy(
                response.message,
                "Core process shutting down"
            );

            mq_send(
                response_queue,
                (const char *)&response,
                sizeof(LogMessage),
                0
            );

            mq_send(
                logger_queue,
                (const char *)&response,
                sizeof(LogMessage),
                0
            );

            break;
        }


        /* ================= CPU ================= */

        if (strcmp(request.instruction, "ADD") == 0 ||
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

                strcpy(
                    response.message,
                    "Memory: invalid address"
                );
            }
        }


        /* ================= LOAD ================= */

        else if (strcmp(request.instruction, "LOAD") == 0)
        {
            int value;

            if (read_memory(
                    request.value1,
                    &value) == 0)
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

                strcpy(
                    response.message,
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

                strcpy(
                    response.message,
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

                strcpy(
                    response.message,
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

                strcpy(
                    response.message,
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

                strcpy(
                    response.message,
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

                strcpy(
                    response.message,
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

                strcpy(
                    response.message,
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


        /* Send response to UI */
        if (mq_send(
                response_queue,
                (const char *)&response,
                sizeof(LogMessage),
                0) == -1)
        {
            perror("[CORE] mq_send UI");
        }

        /* Send response to Logger */
        if (mq_send(
                logger_queue,
                (const char *)&response,
                sizeof(LogMessage),
                0) == -1)
        {
            perror("[CORE] mq_send Logger");
        }

        printf("[CORE] Response sent to UI\n");
        printf("[CORE] Log sent to Logger\n");

        printf("\n[CORE] Waiting for next command...\n");
    }

    mq_close(ui_queue);
    mq_close(response_queue);
    mq_close(logger_queue);

    mq_unlink(UI_TO_CORE);
    mq_unlink(CORE_TO_UI);
    mq_unlink(CORE_TO_LOGGER);

    printf("\n[CORE] Process terminated\n");

    return 0;
}
