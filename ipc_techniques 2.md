
# IPC Techniques

## 1. Overview

The Multi-Process Simulator uses **Inter-Process Communication (IPC)** to enable communication between three independent processes:

- **UI Process**
- **Core Process**
- **Logger Process**

The project implements three IPC techniques: **Unix Domain Sockets, POSIX Message Queues, and FIFO (Named Pipe)**.

---

## 2. IPC Techniques Used

### 2.1 Unix Domain Socket

Unix Domain Sockets are used to transfer operation requests from the UI Process to the Core Process.

The implementation uses:

```c
#include <sys/socket.h>
#include <sys/un.h>
```

| Communication | Purpose |
|---|---|
| UI → Core | Sends commands and input values |

### 2.2 POSIX Message Queue

POSIX Message Queues are used to transfer operation responses from the Core Process to the UI Process.

The implementation uses:

```c
#include <mqueue.h>
```

| Communication | Purpose |
|---|---|
| Core → UI | Sends operation results, status, and response messages |

### 2.3 FIFO (Named Pipe)

A FIFO is used to transfer execution log messages from the Core Process to the Logger Process.

The implementation uses:

```c
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
```

| Communication | Purpose |
|---|---|
| Core → Logger | Transfers execution logs and operation details |

---

## 3. IPC Communication Flow

```text
                 UI PROCESS
                     |
                     | Unix Domain Socket
                     | Operation Request
                     v
                CORE PROCESS
                 /         \
                /           \
               v             v
      POSIX Message Queue   FIFO / Named Pipe
               |             |
               v             v
          UI PROCESS    LOGGER PROCESS
         (Response)      (Execution Logs)
```

The UI Process sends commands and input values to the Core Process through a Unix Domain Socket.

The Core Process executes the requested CPU, Memory, Stack, or Queue operation.

After processing, the Core sends the response to the UI through a POSIX Message Queue and sends execution logs to the Logger through a FIFO.

---

## 4. Message Structures

The project uses structures defined in `ipc.h` to exchange request and response data.

### Request

The `Request` structure carries the selected operation and its input values from the UI to the Core Process.

```c
typedef struct {
    int option;
    int value1;
    int value2;
} Request;
```

### Response

The `Response` structure carries the operation status, result, input values, instruction, and explanatory message from the Core Process to the UI.

```c
typedef struct {
    int status;
    int result;
    int value1;
    int value2;
    char instruction[64];
    char message[MAX_MESSAGE];
} Response;
```

---

## 5. IPC Functions Used

| Function | Purpose |
|---|---|
| `socket()` | Creates a socket for communication |
| `bind()` | Associates the socket with a local address |
| `listen()` | Waits for incoming socket connections |
| `accept()` | Accepts a connection from the UI |
| `connect()` | Connects the UI to the Core |
| `mq_open()` | Opens or creates a POSIX message queue |
| `mq_send()` | Sends a message through a message queue |
| `mq_receive()` | Receives a message from a message queue |
| `mq_close()` | Closes a message queue descriptor |
| `mq_unlink()` | Removes a named message queue |
| `mkfifo()` | Creates a named pipe |
| `open()` | Opens the FIFO for reading or writing |
| `read()` | Reads data from the FIFO |
| `write()` | Writes data to the FIFO |

---

## 6. Advantages of the IPC Techniques

The selected IPC techniques provide:

- **Process independence:** Each process runs separately.
- **Efficient communication:** Requests, responses, and logs use dedicated communication channels.
- **Structured data exchange:** Requests and responses use defined C structures.
- **Separation of responsibilities:** The UI handles interaction, the Core performs operations, and the Logger handles execution logs.
- **Modular design:** Each process can be developed, tested, and maintained separately.

---

## 7. Conclusion

The Multi-Process Simulator uses three IPC techniques to coordinate its independent processes:

- **Unix Domain Socket:** UI → Core
- **POSIX Message Queue:** Core → UI
- **FIFO (Named Pipe):** Core → Logger

This architecture enables the simulator to process user commands, return results, and record execution information through separate communication channels.
