#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================= MEMORY ================= */

#define MEMORY_SIZE 100

int memory[MEMORY_SIZE];

void initialize_memory(void)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
        memory[i] = 0;

    printf("[MEMORY] Initialized\n");
}

int store_memory(int address, int value)
{
    if (address < 0 || address >= MEMORY_SIZE)
    {
        printf("[MEMORY] Invalid address\n");
        return -1;
    }

    memory[address] = value;

    printf("[MEMORY] Stored %d at address %d\n",
           value, address);

    return 0;
}

int load_memory(int address, int *value)
{
    if (address < 0 || address >= MEMORY_SIZE)
    {
        printf("[MEMORY] Invalid address\n");
        return -1;
    }

    *value = memory[address];

    printf("[MEMORY] Loaded %d from address %d\n",
           *value, address);

    return 0;
}


/* ================= STACK ================= */

#define STACK_SIZE 50

int stack[STACK_SIZE];
int top = -1;

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

int queue[QUEUE_SIZE];
int front = 0;
int rear = 0;
int count = 0;

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
            printf("[CPU] Division by zero\n");
            return -1;
        }

        *result = value1 / value2;
        return 0;
    }

    return -1;
}


/* ================= MAIN ================= */

int main(void)
{
    int choice;
    int value1;
    int value2;
    int result;

    initialize_memory();
    initialize_stack();
    initialize_queue();

    printf("\n");
    printf("====================================\n");
    printf("       STANDALONE SIMULATOR\n");
    printf("====================================\n");

    while (1)
    {
        printf("\n");
        printf("1. ADD\n");
        printf("2. SUBTRACT\n");
        printf("3. MULTIPLY\n");
        printf("4. DIVIDE\n");
        printf("5. STORE\n");
        printf("6. LOAD\n");
        printf("7. PUSH\n");
        printf("8. POP\n");
        printf("9. PEEK\n");
        printf("10. ENQUEUE\n");
        printf("11. DEQUEUE\n");
        printf("12. QUEUE PEEK\n");
        printf("13. EXIT\n");

        printf("\nEnter your choice: ");
        scanf("%d", &choice);

        /* CPU operations */

        if (choice >= 1 && choice <= 4)
        {
            printf("Enter value 1: ");
            scanf("%d", &value1);

            printf("Enter value 2: ");
            scanf("%d", &value2);

            if (choice == 1)
            {
                execute_cpu(
                    "ADD",
                    value1,
                    value2,
                    &result);

                printf("[CPU] ADD result = %d\n",
                       result);
            }

            else if (choice == 2)
            {
                execute_cpu(
                    "SUB",
                    value1,
                    value2,
                    &result);

                printf("[CPU] SUB result = %d\n",
                       result);
            }

            else if (choice == 3)
            {
                execute_cpu(
                    "MUL",
                    value1,
                    value2,
                    &result);

                printf("[CPU] MUL result = %d\n",
                       result);
            }

            else
            {
                if (execute_cpu(
                        "DIV",
                        value1,
                        value2,
                        &result) == 0)
                {
                    printf("[CPU] DIV result = %d\n",
                           result);
                }
            }
        }

        /* STORE */

        else if (choice == 5)
        {
            printf("Enter memory address: ");
            scanf("%d", &value1);

            printf("Enter value: ");
            scanf("%d", &value2);

            store_memory(value1, value2);
        }

        /* LOAD */

        else if (choice == 6)
        {
            printf("Enter memory address: ");
            scanf("%d", &value1);

            if (load_memory(value1, &result) == 0)
            {
                printf("Result = %d\n", result);
            }
        }

        /* PUSH */

        else if (choice == 7)
        {
            printf("Enter value: ");
            scanf("%d", &value1);

            push(value1);
        }

        /* POP */

        else if (choice == 8)
        {
            if (pop(&result) == 0)
            {
                printf("Result = %d\n", result);
            }
        }

        /* PEEK */

        else if (choice == 9)
        {
            if (peek(&result) == 0)
            {
                printf("Result = %d\n", result);
            }
        }

        /* ENQUEUE */

        else if (choice == 10)
        {
            printf("Enter value: ");
            scanf("%d", &value1);

            enqueue(value1);
        }

        /* DEQUEUE */

        else if (choice == 11)
        {
            if (dequeue(&result) == 0)
            {
                printf("Result = %d\n", result);
            }
        }

        /* QUEUE PEEK */

        else if (choice == 12)
        {
            if (queue_peek(&result) == 0)
            {
                printf("Result = %d\n", result);
            }
        }

        /* EXIT */

        else if (choice == 13)
        {
            printf("\n[STANDALONE] Simulator terminated\n");
            break;
        }

        else
        {
            printf("[ERROR] Invalid choice\n");
        }
    }

    return 0;
}
