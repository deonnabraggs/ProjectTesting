#ifndef IPC_H
#define IPC_H

#define _POSIX_C_SOURCE 200809L

#define SOCKET_PATH "/tmp/week2_2_core.sock"
#define FIFO_PATH   "/tmp/week2_2_logger.fifo"
#define MQ_NAME     "/week2_2_response"

#define MAX_STACK 10
#define MAX_QUEUE 10
#define MEMORY_SIZE 10
#define MAX_MESSAGE 256

typedef struct {
    int option;
    int value1;
    int value2;
} Request;

typedef struct {
    int status;
    int result;
    char message[128];
} Response;

#endif
