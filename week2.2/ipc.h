
#ifndef IPC_H
#define IPC_H

#define SOCKET_PATH "/tmp/week4_simulator.sock"
#define FIFO_PATH   "/tmp/week4_simulator_fifo"
#define MQ_NAME     "/week4_simulator_response"

#define MAX_MESSAGE 256
#define MAX_STACK   10
#define MAX_QUEUE   10
#define MEMORY_SIZE 10

typedef struct {
    int option;
    int value1;
    int value2;
} Request;

typedef struct {
    int status;
    int result;
    int value1;
    int value2;
    char instruction[64];
    char message[MAX_MESSAGE];
} Response;

#endif
