
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include "ipc.h"

static int stack_data[MAX_STACK];
static int stack_top = -1;

static int queue_data[MAX_QUEUE];
static int queue_front = 0;
static int queue_rear = -1;
static int queue_count = 0;

static int memory_data[MEMORY_SIZE];
static int memory_used[MEMORY_SIZE];

static int log_fd = -1;

static int read_full(int fd, void *buffer, size_t length)
{
    size_t received = 0;

    while (received < length) {
        ssize_t n = read(
            fd, (char *)buffer + received, length - received
        );

        if (n == 0)
            return 0;

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        received += (size_t)n;
    }

    return 1;
}

static void log_line(const char *format, ...)
{
    char message[1024];
    va_list args;
    int length;

    if (log_fd < 0)
        return;

    va_start(args, format);
    length = vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    if (length < 0)
        return;

    if ((size_t)length >= sizeof(message))
        length = (int)sizeof(message) - 1;

    (void)write(log_fd, message, (size_t)length);
}

static const char *instruction_name(int option)
{
    switch (option) {
        case 1:  return "ADD";
        case 2:  return "SUBTRACT";
        case 3:  return "MULTIPLY";
        case 4:  return "DIVIDE";
        case 5:  return "STACK_PUSH";
        case 6:  return "STACK_POP";
        case 7:  return "QUEUE_ENQUEUE";
        case 8:  return "QUEUE_DEQUEUE";
        case 9:  return "MEMORY_STORE";
        case 10: return "MEMORY_LOAD";
        case 11: return "STACK_PEEK";
        case 12: return "QUEUE_PEEK";
        case 13: return "EXIT";
        default: return "UNKNOWN";
    }
}

static void set_response(Response *response, int success,
                         int result, const char *message)
{
    response->status = success;
    response->result = result;

    snprintf(response->message, sizeof(response->message),
             "%s", message);
}

static void process_request(const Request *request,
                            Response *response)
{
    int value;

    memset(response, 0, sizeof(*response));

    response->value1 = request->value1;
    response->value2 = request->value2;

    snprintf(response->instruction,
             sizeof(response->instruction), "%s",
             instruction_name(request->option));

    switch (request->option) {
        case 1:
            set_response(response, 1,
                request->value1 + request->value2,
                "Addition completed successfully");
            break;

        case 2:
            set_response(response, 1,
                request->value1 - request->value2,
                "Subtraction completed successfully");
            break;

        case 3:
            set_response(response, 1,
                request->value1 * request->value2,
                "Multiplication completed successfully");
            break;

        case 4:
            if (request->value2 == 0) {
                set_response(response, 0, 0,
                    "Error: division by zero is not allowed");
            } else {
                set_response(response, 1,
                    request->value1 / request->value2,
                    "Division completed successfully (integer result)");
            }
            break;

        case 5:
            if (stack_top >= MAX_STACK - 1) {
                set_response(response, 0, -1,
                    "Error: stack is full");
            } else {
                stack_data[++stack_top] = request->value1;
                set_response(response, 1, request->value1,
                    "Value pushed onto stack");
            }
            break;

        case 6:
            if (stack_top < 0) {
                set_response(response, 0, -1,
                    "Error: stack is empty");
            } else {
                value = stack_data[stack_top--];
                set_response(response, 1, value,
                    "Value popped from stack");
            }
            break;

        case 7:
            if (queue_count >= MAX_QUEUE) {
                set_response(response, 0, -1,
                    "Error: queue is full");
            } else {
                queue_rear = (queue_rear + 1) % MAX_QUEUE;
                queue_data[queue_rear] = request->value1;
                queue_count++;

                set_response(response, 1, request->value1,
                    "Value added to queue");
            }
            break;

        case 8:
            if (queue_count == 0) {
                set_response(response, 0, -1,
                    "Error: queue is empty");
            } else {
                value = queue_data[queue_front];
                queue_front = (queue_front + 1) % MAX_QUEUE;
                queue_count--;

                set_response(response, 1, value,
                    "Value removed from queue");
            }
            break;

        case 9:
            if (request->value1 < 0 ||
                request->value1 >= MEMORY_SIZE) {
                set_response(response, 0, -1,
                    "Error: memory address must be between 0 and 9");
            } else {
                memory_data[request->value1] = request->value2;
                memory_used[request->value1] = 1;

                set_response(response, 1, request->value2,
                    "Value stored in memory");
            }
            break;

        case 10:
            if (request->value1 < 0 ||
                request->value1 >= MEMORY_SIZE) {
                set_response(response, 0, -1,
                    "Error: memory address must be between 0 and 9");
            } else if (!memory_used[request->value1]) {
                set_response(response, 0, -1,
                    "Error: memory address has no stored value");
            } else {
                set_response(response, 1,
                    memory_data[request->value1],
                    "Value loaded from memory");
            }
            break;

        case 11:
            if (stack_top < 0) {
                set_response(response, 0, -1,
                    "Error: stack is empty");
            } else {
                set_response(response, 1, stack_data[stack_top],
                    "Top stack value retrieved");
            }
            break;

        case 12:
            if (queue_count == 0) {
                set_response(response, 0, -1,
                    "Error: queue is empty");
            } else {
                set_response(response, 1, queue_data[queue_front],
                    "Front queue value retrieved");
            }
            break;

        case 13:
            set_response(response, 1, 0,
                "Simulator exit requested");
            break;

        default:
            set_response(response, 0, -1,
                "Error: invalid menu option");
            break;
    }
}

int main(void)
{
    int server_fd = -1;
    int client_fd = -1;
    int exit_requested = 0;

    struct sockaddr_un address;
    mqd_t response_queue = (mqd_t)-1;

    memset(memory_used, 0, sizeof(memory_used));

    printf("CORE: Process started.\n");
    printf("CORE: Memory initialized (%d slots).\n", MEMORY_SIZE);
    printf("CORE: Stack initialized (capacity %d).\n", MAX_STACK);
    printf("CORE: Queue initialized (capacity %d).\n", MAX_QUEUE);
    fflush(stdout);

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("CORE: mkfifo failed");
        return EXIT_FAILURE;
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("CORE: socket failed");
        unlink(FIFO_PATH);
        return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;

    snprintf(address.sun_path, sizeof(address.sun_path),
             "%s", SOCKET_PATH);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) == -1) {
        perror("CORE: bind failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    if (listen(server_fd, 1) == -1) {
        perror("CORE: listen failed");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    printf("CORE: Waiting for Logger FIFO...\n");
    fflush(stdout);

    log_fd = open(FIFO_PATH, O_WRONLY);

    if (log_fd == -1) {
        perror("CORE: Could not open Logger FIFO");
        close(server_fd);
        unlink(SOCKET_PATH);
        return EXIT_FAILURE;
    }

    log_line("LOGGER: Core connected through FIFO. Logger is ready.\n");

    printf("CORE: Logger connected.\n");
    printf("CORE: Waiting for UI connection...\n");
    fflush(stdout);

    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd == -1) {
        perror("CORE: accept failed");
        goto cleanup;
    }

    printf("CORE: UI connected.\n");
    log_line("CORE: UI connected through Unix domain socket.\n");

    response_queue = mq_open(MQ_NAME, O_WRONLY);

    if (response_queue == (mqd_t)-1) {
        perror("CORE: mq_open failed; start UI so it creates the response queue");
        goto cleanup;
    }

    while (!exit_requested) {
        Request request;
        Response response;

        int read_status = read_full(
            client_fd, &request, sizeof(request)
        );

        if (read_status == 0) {
            printf("CORE: UI disconnected.\n");
            break;
        }

        if (read_status < 0) {
            perror("CORE: Failed to receive request");
            break;
        }

        printf("\nCORE: Command received from UI.\n");
        printf("CORE: Instruction: %s\n",
               instruction_name(request.option));

        if (request.option >= 1 && request.option <= 4) {
            printf("CORE: Value 1: %d\n", request.value1);
            printf("CORE: Value 2: %d\n", request.value2);
        } else if (request.option == 9) {
            printf("CORE: Memory address: %d\n", request.value1);
            printf("CORE: Value to store: %d\n", request.value2);
        } else if (request.option == 10) {
            printf("CORE: Memory address: %d\n", request.value1);
        } else if (request.option == 5 || request.option == 7) {
            printf("CORE: Value: %d\n", request.value1);
        }

        fflush(stdout);

        log_line("CORE: Command received. Instruction: %s\n",
                 instruction_name(request.option));

        process_request(&request, &response);

        if (request.option >= 1 && request.option <= 4) {
            printf("CORE: CPU result = %d\n", response.result);
            log_line("CORE: CPU result = %d\n", response.result);
        } else {
            printf("CORE: %s\n", response.message);
            log_line("CORE: Operation result: %s\n",
                     response.message);
        }

        log_line("LOGGER: Status: %s\n",
                 response.status ? "SUCCESS" : "ERROR");
        log_line("LOGGER: Message: %s\n", response.message);
        log_line("LOGGER: Result: %d\n", response.result);
        log_line("LOGGER: Waiting for next message...\n\n");

        if (mq_send(response_queue, (const char *)&response,
                    sizeof(response), 0) == -1) {
            perror("CORE: Failed to send response through message queue");
            break;
        }

        printf("CORE: Response sent to UI through POSIX message queue.\n");
        printf("CORE: Log sent to Logger through FIFO.\n");
        printf("CORE: Waiting for next command...\n");
        fflush(stdout);

        if (request.option == 13)
            exit_requested = 1;
    }

cleanup:
    if (response_queue != (mqd_t)-1)
        mq_close(response_queue);

    if (client_fd != -1)
        close(client_fd);

    if (server_fd != -1)
        close(server_fd);

    if (log_fd != -1)
        close(log_fd);

    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);
    mq_unlink(MQ_NAME);

    printf("CORE: Process stopped.\n");

    return EXIT_SUCCESS;
}
