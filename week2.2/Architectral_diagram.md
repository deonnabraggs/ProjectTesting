
# Multi-Process Simulator – Architectural Diagram

## Architecture Overview

The Multi-Process Simulator consists of three independent processes: UI, Core, and Logger. The processes communicate using Unix Domain Sockets, POSIX Message Queues, and FIFOs.

- **UI Process:** Accepts user input, sends operation requests, and displays results.
- **Core Process:** Executes operations and manages CPU execution, memory, stack, and queue.
- **Logger Process:** Receives and displays execution log messages.

## Architecture Diagram

```text
                            USER
                              |
                              v
                 +--------------------------+
                 |        UI PROCESS        |
                 |--------------------------|
                 | User Input               |
                 | Command Selection        |
                 | Send Requests            |
                 | Display Results          |
                 +------------+-------------+
                              |
                              | Unix Domain Socket
                              | Operation Request
                              v
                 +--------------------------+
                 |       CORE PROCESS       |
                 |--------------------------|
                 | CPU / Instruction        |
                 | Execution                |
                 | Memory Management        |
                 | Stack Management         |
                 | Queue Management         |
                 | Operation Processing     |
                 +------------+-------------+
                              |
                   +----------+----------+
                   |                     |
                   | POSIX Message Queue | FIFO / Named Pipe
                   | Response / Result   | Execution Logs
                   v                     v
       +--------------------------+  +--------------------------+
       |        UI PROCESS        |  |      LOGGER PROCESS      |
       |--------------------------|  |--------------------------|
       | Receive Response         |  | Receive Log Messages     |
       | Display Status           |  | Display Operation        |
       | Display Result           |  | Status and Details       |
       +--------------------------+  +--------------------------+

```

## Working Flow

1. The user selects an operation and enters the required input through the UI.
2. The UI sends the request to the Core Process using a Unix Domain Socket.
3. The Core Process executes the requested operation using its CPU, memory, stack, or queue components.
4. The Core Process sends the response to the UI through a POSIX Message Queue.
5. The Core Process sends execution logs to the Logger Process through a FIFO (Named Pipe).
6. The UI displays the result, while the Logger displays execution details.

## IPC Mechanisms

| IPC Mechanism | Communication | Purpose |
|---|---|---|
| Unix Domain Socket | UI → Core | Transfers operation requests |
| POSIX Message Queue | Core → UI | Returns responses and results |
| FIFO (Named Pipe) | Core → Logger | Transfers execution logs |
