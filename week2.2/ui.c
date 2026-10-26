
#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "ipc.h"

static int write_full(int fd, const void *buffer, size_t length)
{
    size_t sent = 0;

    while (sent < length) {
        ssize_t n = write(
            fd, (const char *)buffer + sent, length - sent
        );

        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        sent += (size_t)n;
    }

    return 0;
}

static void print_menu(void)
{
    printf("\n========== WEEK 4 SIMULATOR ==========\n");
    printf("1. Add\n");
    printf("2. Subtract\n");
    printf("3. Multiply\n");
    printf("4. Divide\n");
    printf("5. Stack Push\n");
    printf("6. Stack Pop\n");
    printf("7. Queue Enqueue\n");
    printf("8. Queue Dequeue\n");
    printf("9. Memory Store\n");
    printf("10. Memory Load\n");
    printf("11. Stack Peek\n");
    printf("12. Queue Peek\n");
    printf("13. Exit\n");
    printf("======================================\n");
}

static int read_integer(const char *prompt, int *value)
{
    char line[128];
    char extra;

    printf("%s", prompt);
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL)
        return 0;

    if (sscanf(line, " %d %c", value, &extra) != 1) {
        printf("Invalid input. Please enter a whole number.\n");
        return -1;
    }

    return 1;
}

int main(void)
{
    int socket_fd = -1;
    mqd_t response_queue = (mqd_t)-1;

    struct sockaddr_un address;
    struct mq_attr attributes;

    int running = 1;

    /* Create the response queue before connecting to Core. */
    mq_unlink(MQ_NAME);

    memset(&attributes, 0, sizeof(attributes));
    attributes.mq_maxmsg = 10;
    attributes.mq_msgsize = sizeof(Response);

    response_queue = mq_open(
        MQ_NAME, O_CREAT | O_RDONLY, 0666, &attributes
    );

    if (response_queue == (mqd_t)-1) {
        perror("UI: Could not create POSIX message queue");
        return EXIT_FAILURE;
    }

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        perror("UI: socket failed");
        mq_close(response_queue);
        mq_unlink(MQ_NAME);
        return EXIT_FAILURE;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;

    snprintf(address.sun_path, sizeof(address.sun_path),
             "%s", SOCKET_PATH);

    if (connect(socket_fd, (struct sockaddr *)&address,
                sizeof(address)) == -1) {
        perror("UI: Could not connect to Core. Start Core and Logger first");
        close(socket_fd);
        mq_close(response_queue);
        mq_unlink(MQ_NAME);
        return EXIT_FAILURE;
    }

    printf("UI: Connected to Core through Unix domain socket.\n");

    while (running) {
        Request request;
        Response response;
        int input_status;

        memset(&request, 0, sizeof(request));

        print_menu();

        input_status = read_integer(
            "Select option (1-13): ", &request.option
        );

        if (input_status == 0)
            break;

        if (input_status < 0)
            continue;

        if (request.option < 1 || request.option > 13) {
            printf("Invalid option. Choose a number from 1 to 13.\n");
            continue;
        }

        if (request.option >= 1 && request.option <= 4) {
            input_status = read_integer(
                "Enter first value: ", &request.value1
            );

            if (input_status == 0)
                break;
            if (input_status < 0)
                continue;

            input_status = read_integer(
                "Enter second value: ", &request.value2
            );

            if (input_status == 0)
                break;
            if (input_status < 0)
                continue;

        } else if (request.option == 5 || request.option == 7) {
            input_status = read_integer(
                "Enter value: ", &request.value1
            );

            if (input_status == 0)
                break;
            if (input_status < 0)
                continue;

        } else if (request.option == 9) {
            input_status = read_integer(
                "Enter memory address (0-9): ", &request.value1
            );

            if (input_status == 0)
                break;
            if (input_status < 0)
                continue;

            input_status = read_integer(
                "Enter value to store: ", &request.value2
            );

            if (input_status == 0)
                break;
            if (input_status < 0)
                continue;

        } else if (request.option == 10) {
            input_status = read_integer(
                "Enter memory address (0-9): ", &request.value1
            );

            if (input_status == 0)
                break;
            if (input_status < 0)
                continue;
        }

        if (write_full(socket_fd, &request, sizeof(request)) == -1) {
            perror("UI: Failed to send request to Core");
            break;
        }

        if (mq_receive(response_queue, (char *)&response,
                       sizeof(response), NULL) == -1) {
            perror("UI: Failed to receive response from Core message queue");
            break;
        }

        printf("\n------------- RESULT -------------\n");
        printf("Status : %s\n",
               response.status ? "SUCCESS" : "ERROR");
        printf("Message: %s\n", response.message);
        printf("Result : %d\n", response.result);
        printf("----------------------------------\n");

        if (request.option == 13)
            running = 0;
    }

    close(socket_fd);
    mq_close(response_queue);
    mq_unlink(MQ_NAME);

    printf("UI: Disconnected.\n");

    return EXIT_SUCCESS;
}
