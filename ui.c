#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include "ipc.h"

int main(void)
{
    mqd_t ui_to_core;
    mqd_t core_to_ui;

    CoreMessage request;
    LogMessage response;

    struct mq_attr core_attr;
    struct mq_attr response_attr;

    /* UI -> Core queue */
    core_attr.mq_flags = 0;
    core_attr.mq_maxmsg = 10;
    core_attr.mq_msgsize = sizeof(CoreMessage);
    core_attr.mq_curmsgs = 0;

    /* Core -> UI queue */
    response_attr.mq_flags = 0;
    response_attr.mq_maxmsg = 10;
    response_attr.mq_msgsize = sizeof(LogMessage);
    response_attr.mq_curmsgs = 0;

    printf("\n");
    printf("====================================\n");
    printf("          UI PROCESS STARTED\n");
    printf("====================================\n");

    /* Open UI -> Core queue */
    ui_to_core = mq_open(
        UI_TO_CORE,
        O_CREAT | O_WRONLY,
        0666,
        &core_attr
    );

    if (ui_to_core == (mqd_t)-1)
    {
        perror("[UI] mq_open UI_TO_CORE");
        return 1;
    }

    /* Open Core -> UI queue */
    core_to_ui = mq_open(
        CORE_TO_UI,
        O_CREAT | O_RDONLY,
        0666,
        &response_attr
    );

    if (core_to_ui == (mqd_t)-1)
    {
        perror("[UI] mq_open CORE_TO_UI");
        mq_close(ui_to_core);
        return 1;
    }

    printf("[UI] Message queues opened successfully\n");

    while (1)
    {
        memset(&request, 0, sizeof(request));

        printf("\n");
        printf("====================================\n");
        printf("       MULTI-PROCESS SIMULATOR\n");
        printf("====================================\n");

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

        if (scanf("%d", &request.command) != 1)
        {
            printf("[UI] Invalid input.\n");

            while (getchar() != '\n')
                ;

            continue;
        }

        /* EXIT */
        if (request.command == 13)
        {
            strcpy(request.instruction, "EXIT");

            if (mq_send(
                    ui_to_core,
                    (const char *)&request,
                    sizeof(CoreMessage),
                    0) == -1)
            {
                perror("[UI] mq_send EXIT");
            }
            else
            {
                printf("\n[UI] EXIT command sent to Core\n");

                /* Wait for Core confirmation */
                if (mq_receive(
                        core_to_ui,
                        (char *)&response,
                        sizeof(LogMessage),
                        NULL) != -1)
                {
                    printf("[UI] Core Response: %s\n", response.message);
                }
            }

            break;
        }

        /* Check valid command */
        if (request.command < 1 || request.command > 13)
        {
            printf("[UI] Invalid choice.\n");
            continue;
        }

        /* CPU operations */
        if (request.command >= 1 && request.command <= 4)
        {
            if (request.command == 1)
                strcpy(request.instruction, "ADD");

            else if (request.command == 2)
                strcpy(request.instruction, "SUB");

            else if (request.command == 3)
                strcpy(request.instruction, "MUL");

            else
                strcpy(request.instruction, "DIV");

            printf("Enter value 1: ");
            scanf("%d", &request.value1);

            printf("Enter value 2: ");
            scanf("%d", &request.value2);
        }

        /* STORE */
        else if (request.command == 5)
        {
            strcpy(request.instruction, "STORE");

            printf("Enter memory address: ");
            scanf("%d", &request.value1);

            printf("Enter value: ");
            scanf("%d", &request.value2);
        }

        /* LOAD */
        else if (request.command == 6)
        {
            strcpy(request.instruction, "LOAD");

            printf("Enter memory address: ");
            scanf("%d", &request.value1);
        }

        /* PUSH */
        else if (request.command == 7)
        {
            strcpy(request.instruction, "PUSH");

            printf("Enter value: ");
            scanf("%d", &request.value1);
        }

        /* POP */
        else if (request.command == 8)
        {
            strcpy(request.instruction, "POP");
        }

        /* PEEK */
        else if (request.command == 9)
        {
            strcpy(request.instruction, "PEEK");
        }

        /* ENQUEUE */
        else if (request.command == 10)
        {
            strcpy(request.instruction, "ENQUEUE");

            printf("Enter value: ");
            scanf("%d", &request.value1);
        }

        /* DEQUEUE */
        else if (request.command == 11)
        {
            strcpy(request.instruction, "DEQUEUE");
        }

        /* QUEUE PEEK */
        else if (request.command == 12)
        {
            strcpy(request.instruction, "QUEUE_PEEK");
        }

        printf("\n");
        printf("====================================\n");
        printf("             UI COMMAND\n");
        printf("====================================\n");

        printf("Instruction : %s\n", request.instruction);
        printf("Value 1     : %d\n", request.value1);
        printf("Value 2     : %d\n", request.value2);

        /* Send command to Core */
        if (mq_send(
                ui_to_core,
                (const char *)&request,
                sizeof(CoreMessage),
                0) == -1)
        {
            perror("[UI] mq_send");
            break;
        }

        printf("\n[UI] Command sent to Core successfully\n");

        /* Wait for Core response */
        if (mq_receive(
                core_to_ui,
                (char *)&response,
                sizeof(LogMessage),
                NULL) == -1)
        {
            perror("[UI] mq_receive");
            break;
        }

        printf("\n");
        printf("====================================\n");
        printf("            CORE RESPONSE\n");
        printf("====================================\n");

        if (response.status == 0)
            printf("Status  : SUCCESS\n");
        else
            printf("Status  : ERROR\n");

        printf("Message : %s\n", response.message);

        if (response.result != -999999)
            printf("Result  : %d\n", response.result);

        printf("====================================\n");
    }

    mq_close(ui_to_core);
    mq_close(core_to_ui);

    printf("\n[UI] Process terminated\n");

    return 0;
}
