
#include "ipc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <mqueue.h>

static int memory[MEMORY_SIZE];
static int memory_valid[MEMORY_SIZE];

static int stack_data[STACK_SIZE];
static int stack_top = -1;

static int queue_data[QUEUE_SIZE];
static int queue_front = 0;
static int queue_rear = 0;
static int queue_count = 0;

static int read_full(int fd, void *buffer, size_t size)
{
    size_t total = 0;
    char *p = buffer;

    while (total < size) {
        ssize_t n = read(fd, p + total, size - total);

        if (n == 0)
            return total == 0 ? 0 : -1;

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        total += (size_t)n;
    }

    return 1;
}

static int write_full(int fd, const void *buffer, size_t size)
{
    size_t total = 0;
    const char *p = buffer;

    while (total < size) {
        ssize_t n = write(fd, p + total, size - total);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0)
            return -1;

        total += (size_t)n;
    }

    return 0;
}

static void set_response(CoreResponse *r, int status,
                         int result, const char *message)
{
    r->status = status;
    r->result = result;
    snprintf(r->message, sizeof(r->message), "%s", message);
}

static void execute_command(const CoreMessage *req, CoreResponse *r)
{
    int a = req->value1;
    int b = req->value2;

    switch (req->command) {
    case CMD_ADD:
        set_response(r, 1, a + b, "Addition completed");
        break;

    case CMD_SUBTRACT:
        set_response(r, 1, a - b, "Subtraction completed");
        break;

    case CMD_MULTIPLY:
        set_response(r, 1, a * b, "Multiplication completed");
        break;

    case CMD_DIVIDE:
        if (b == 0) {
            set_response(r, 0, 0, "Error: division by zero");
        } else {
            set_response(r, 1, a / b, "Division completed (integer result)");
        }
        break;

    case CMD_PUSH:
        if (stack_top >= STACK_SIZE - 1) {
            set_response(r, 0, 0, "Error: stack is full");
        } else {
            stack_data[++stack_top] = a;
            set_response(r, 1, a, "Value pushed onto stack");
        }
        break;

    case CMD_POP:
        if (stack_top < 0) {
            set_response(r, 0, 0, "Error: stack is empty");
        } else {
            int value = stack_data[stack_top--];
            set_response(r, 1, value, "Value popped from stack");
        }
        break;

    case CMD_STACK_PEEK:
        if (stack_top < 0) {
            set_response(r, 0, 0, "Error: stack is empty");
        } else {
            set_response(r, 1, stack_data[stack_top],
                         "Top stack value");
        }
        break;

    case CMD_ENQUEUE:
        if (queue_count >= QUEUE_SIZE) {
            set_response(r, 0, 0, "Error: queue is full");
        } else {
            queue_data[queue_rear] = a;
            queue_rear = (queue_rear + 1) % QUEUE_SIZE;
            queue_count++;
            set_response(r, 1, a, "Value added to queue");
        }
        break;

    case CMD_DEQUEUE:
        if (queue_count == 0) {
            set_response(r, 0, 0, "Error: queue is empty");
        } else {
            int value = queue_data[queue_front];
            queue_front = (queue_front + 1) % QUEUE_SIZE;
            queue_count--;
            set_response(r, 1, value, "Value removed from queue");
        }
        break;

    case CMD_QUEUE_PEEK:
        if (queue_count == 0) {
            set_response(r, 0, 0, "Error: queue is empty");
        } else {
            set_response(r, 1, queue_data[queue_front],
                         "Front queue value");
        }
        break;

    case CMD_STORE:
        if (a < 0 || a >= MEMORY_SIZE) {
            set_response(r, 0, 0, "Error: invalid memory address");
        } else {
            memory[a] = b;
            memory_valid[a] = 1;
            set_response(r, 1, b, "Value stored in memory");
        }
        break;

    case CMD_LOAD:
        if (a < 0 || a >= MEMORY_SIZE) {
            set_response(r, 0, 0, "Error: invalid memory address");
        } else if (!memory_valid[a]) {
            set_response(r, 0, 0, "Error: memory address is empty");
        } else {
            set_response(r, 1, memory[a], "Value loaded from memory");
        }
        break;

    case CMD_EXIT:
        set_response(r, 1, 0, "Simulator shutting down");
        break;

    default:
        set_response(r, 0, 0, "Error: invalid command");
        break;
    }
}

int main(void)
{
    int server_fd = -1;
    int client_fd = -1;
    int fifo_fd = -1;
    mqd_t mq = (mqd_t)-1;
    int exit_requested = 0;

    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(CoreResponse);

    /* Remove stale IPC objects from an earlier run. */
    mq_unlink(MQ_NAME);
    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);

    mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("Core: mq_open");
        return 1;
    }

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("Core: mkfifo");
        mq_close(mq);
        mq_unlink(MQ_NAME);
        return 1;
    }

    /* Wait for Logger to open the FIFO for reading. */
    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) {
        perror("Core: open FIFO");
        mq_close(mq);
        mq_unlink(MQ_NAME);
        unlink(FIFO_PATH);
        return 1;
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Core: socket");
        goto cleanup;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Core: bind");
        goto cleanup;
    }

    if (listen(server_fd, 1) == -1) {
        perror("Core: listen");
        goto cleanup;
    }

    printf("CORE: Waiting for UI on Unix domain socket...\n");
    fflush(stdout);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Core: accept");
        goto cleanup;
    }

    printf("CORE: UI connected. Simulator is ready.\n");
    fflush(stdout);

    while (!exit_requested) {
        CoreMessage req;
        CoreResponse response;
        LogMessage log;

        int rc = read_full(client_fd, &req, sizeof(req));
        if (rc <= 0)
            break;

        execute_command(&req, &response);

        /* IPC technique 2: Core -> UI using POSIX message queue. */
        if (mq_send(mq, (const char *)&response,
                    sizeof(response), 0) == -1) {
            perror("Core: mq_send");
            break;
        }

        /* IPC technique 3: Core -> Logger using named FIFO. */
        memset(&log, 0, sizeof(log));
        log.status = response.status;
        log.command = req.command;
        log.result = response.result;
        snprintf(log.message, sizeof(log.message), "%s",
                 response.message);

        if (write_full(fifo_fd, &log, sizeof(log)) == -1) {
            perror("Core: write FIFO");
            break;
        }

        printf("CORE: Command %d -> %s\n",
               req.command, response.message);
        fflush(stdout);

        if (req.command == CMD_EXIT)
            exit_requested = 1;
    }

    if (client_fd >= 0) {
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        client_fd = -1;
    }

cleanup:
    if (client_fd >= 0)
        close(client_fd);
    if (server_fd >= 0)
        close(server_fd);
    if (fifo_fd >= 0) {
        close(fifo_fd);
        LogMessage stop;
        memset(&stop, 0, sizeof(stop));
        stop.status = -1;
        write(STDOUT_FILENO, "", 0);
        (void)stop;
    }

    if (mq != (mqd_t)-1)
        mq_close(mq);

    mq_unlink(MQ_NAME);
    unlink(SOCKET_PATH);
    unlink(FIFO_PATH);

    printf("CORE: Process stopped.\n");
    return 0;
}
