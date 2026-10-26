
#ifndef IPC_H
#define IPC_H

#define SOCKET_PATH "/tmp/week4_core_socket"
#define FIFO_PATH "/tmp/week4_logger_fifo"
#define MQ_NAME "/week4_result_queue"

#define MEMORY_SIZE 100
#define STACK_SIZE 50
#define QUEUE_SIZE 50
#define MAX_MESSAGE 256

typedef enum {
    CMD_ADD = 1,
    CMD_SUBTRACT,
    CMD_MULTIPLY,
    CMD_DIVIDE,
    CMD_PUSH,
    CMD_POP,
    CMD_ENQUEUE,
    CMD_DEQUEUE,
    CMD_STORE,
    CMD_LOAD,
    CMD_STACK_PEEK,
    CMD_QUEUE_PEEK,
    CMD_EXIT
} Command;

typedef struct {
    int command;
    int value1;
    int value2;
} CoreMessage;

typedef struct {
    int status;
    int result;
    char message[MAX_MESSAGE];
} CoreResponse;

typedef struct {
    int status;
    int command;
    int result;
    char message[MAX_MESSAGE];
} LogMessage;

#endif
