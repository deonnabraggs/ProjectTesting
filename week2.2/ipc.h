
#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define SOCKET_PATH "/tmp/ipc_core_socket"
#define FIFO_PATH   "/tmp/core_to_logger_fifo"
#define MQ_NAME     "/week4_ipc_queue"

#define MAX_MESSAGE 256
#define MEMORY_SIZE 100
#define STACK_SIZE  50
#define QUEUE_SIZE  50

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

/* Core sends operation results to UI through the POSIX MQ. */
typedef struct {
    int status;       /* 1 = success, 0 = error */
    int result;
    char message[MAX_MESSAGE];
} CoreResponse;

/* Core sends execution/error records to Logger through FIFO. */
typedef struct {
    int status;       /* -1 = logger shutdown */
    int command;
    int result;
    char message[MAX_MESSAGE];
} LogMessage;

#endif
