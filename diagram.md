# IPC TECHNIQUES

## 1. POSIX MESSAGE QUEUES

The Multi-Process Simulator uses **POSIX Message Queues** as the primary Inter-Process Communication (IPC) technique.

POSIX Message Queues allow the independent **UI, Core, and Logger processes** to exchange structured messages through operating-system-managed queues.

### Message Queues Used

| Message Queue | Communication | Purpose |
|---|---|---|
| `/ui_to_core` | UI → Core | Sends user commands and input values |
| `/core_to_ui` | Core → UI | Sends operation results and status information back to the UI |
| `/core_to_logger` | Core → Logger | Sends operation results and status information to the Logger |

### Message Structures

The UI sends a `CoreMessage` to the Core process.

```c
typedef struct
{
    int command;
    int value1;
    int value2;
    char instruction[MAX_MESSAGE];
} CoreMessage;
```

The Core sends a `LogMessage` to the UI and Logger processes.

```c
typedef struct
{
    int status;
    int result;
    char message[MAX_MESSAGE];
} LogMessage;
```

### POSIX MESSAGE QUEUE FUNCTIONS USED

| Function | Purpose |
|---|---|
| `mq_open()` | Creates or opens a POSIX message queue |
| `mq_send()` | Sends a message through the queue |
| `mq_receive()` | Receives a message from the queue |
| `mq_close()` | Closes the message queue |
| `mq_unlink()` | Removes the message queue |

---

# 2. IPC COMMUNICATION FLOW

The simulator uses three POSIX Message Queues to provide communication between the three independent processes.

### UI → CORE

The **UI Process** accepts the user's command and input values and sends them to the **Core Process** through `/ui_to_core` using `CoreMessage`.

### CORE → UI

After executing the requested operation, the **Core Process** sends the result and status information back to the **UI Process** through `/core_to_ui` using `LogMessage`.

### CORE → LOGGER

The **Core Process** also sends the operation result and status information to the **Logger Process** through `/core_to_logger` using `LogMessage`.

---

# 3. ARCHITECTURE DIAGRAM

```mermaid
flowchart TD

    UI["UI PROCESS<br/>User Input and Commands"]

    Q1[["/ui_to_core<br/>POSIX Message Queue"]]

    CORE["CORE PROCESS<br/>Command Execution"]

    CPU["CPU<br/>ADD • SUB • MUL • DIV"]
    MEMORY["MEMORY<br/>STORE • LOAD"]
    STACK["STACK<br/>PUSH • POP • PEEK"]
    QUEUE["QUEUE<br/>ENQUEUE • DEQUEUE • PEEK"]

    Q2[["/core_to_ui<br/>POSIX Message Queue"]]

    Q3[["/core_to_logger<br/>POSIX Message Queue"]]

    LOGGER["LOGGER PROCESS<br/>Status and Result Logging"]

    UI -->|"CoreMessage"| Q1
    Q1 -->|"mq_receive()"| CORE

    CORE --> CPU
    CORE --> MEMORY
    CORE --> STACK
    CORE --> QUEUE

    CORE -->|"LogMessage"| Q2
    Q2 -->|"mq_receive()"| UI

    CORE -->|"LogMessage"| Q3
    Q3 -->|"mq_receive()"| LOGGER
```

---

# 4. ARCHITECTURE EXPLANATION

The Multi-Process Simulator consists of **three independent processes: UI, Core, and Logger**. The processes communicate using POSIX Message Queues.

### 1. UI PROCESS

- Accepts commands and input values from the user.
- Creates a `CoreMessage`.
- Sends the message to the Core Process through `/ui_to_core`.
- Receives the operation result through `/core_to_ui`.
- Displays the result and status to the user.

### 2. CORE PROCESS

- Acts as the main processing unit of the simulator.
- Receives commands from the UI Process.
- Performs CPU operations:
  - ADD
  - SUB
  - MUL
  - DIV
- Handles Memory operations:
  - STORE
  - LOAD
- Handles Stack operations:
  - PUSH
  - POP
  - PEEK
- Handles Queue operations:
  - ENQUEUE
  - DEQUEUE
  - QUEUE PEEK
- Creates a `LogMessage` containing the operation status and result.
- Sends the result to the UI Process through `/core_to_ui`.
- Sends the result to the Logger Process through `/core_to_logger`.

### 3. LOGGER PROCESS

- Receives `LogMessage` from the Core Process.
- Displays the operation status and result.
- Records the activity of the simulator.
- Does not directly communicate with the UI Process.

### 4. IPC CONNECTION

The three POSIX Message Queues provide the following communication paths:

- `/ui_to_core` → UI sends commands to Core.
- `/core_to_ui` → Core sends results back to UI.
- `/core_to_logger` → Core sends results to Logger.

The **CPU, Memory, Stack, and Queue are internal components of the Core Process**. They are not separate processes and therefore do not use IPC between them.
