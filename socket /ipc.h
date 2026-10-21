#ifndef IPC_H
#define IPC_H

#define SOCKET_PATH "/tmp/ipc_core_socket"
#define CORE_TO_LOGGER "/core_to_logger"
#define FIFO_PATH "/tmp/ipc_core_logger_fifo"

#define MAX_MESSAGE 256
#define MEMORY_SIZE 100
#define STACK_SIZE 50
#define QUEUE_SIZE 50

typedef struct
{
    int command;
    int value1;
    int value2;
    char instruction[MAX_MESSAGE];
} CoreMessage;

typedef struct
{
    int status;
    int result;
    char message[MAX_MESSAGE];
} LogMessage;

#endif
