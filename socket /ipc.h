#ifndef IPC_H
#define IPC_H

#define SOCKET_PATH "/tmp/ipc_core_socket"
#define CORE_TO_UI "/core_to_ui"
#define FIFO_PATH "/tmp/core_to_logger_fifo"

#define MAX_MESSAGE 256
#define MEMORY_SIZE 100
#define STACK_SIZE 100
#define QUEUE_SIZE 100

enum Command {
    CMD_ADD = 1,
    CMD_SUB,
    CMD_MUL,
    CMD_DIV,
    CMD_STORE,
    CMD_LOAD,
    CMD_PUSH,
    CMD_POP,
    CMD_PEEK,
    CMD_ENQUEUE,
    CMD_DEQUEUE,
    CMD_QUEUE_PEEK,
    CMD_EXIT
};

typedef struct {
    int command;
    int value1;
    int value2;
    char instruction[MAX_MESSAGE];
} CoreMessage;

typedef struct {
    int status;
    int result;
    char message[MAX_MESSAGE];
} LogMessage;

#endif
