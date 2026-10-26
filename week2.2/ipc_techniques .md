
# IPC Techniques Used

## 1. Overview

Inter-Process Communication (IPC) allows independent processes to exchange data. Our Multi-Process Simulator uses three IPC techniques to communicate between the UI, Core, and Logger processes.

## 2. IPC Techniques

### 2.1 Unix Domain Socket

A Unix Domain Socket transfers operation requests from the UI Process to the Core Process. It sends the selected command and input values for processing.

**Communication:** UI → Core

### 2.2 POSIX Message Queue

A POSIX Message Queue transfers the operation result and response from the Core Process back to the UI Process. The UI receives the response and displays it to the user.

**Communication:** Core → UI

### 2.3 FIFO (Named Pipe)

A FIFO transfers execution log messages from the Core Process to the Logger Process. The Logger receives and displays information about the operations performed.

**Communication:** Core → Logger

## 3. Conclusion

The Multi-Process Simulator uses Unix Domain Sockets, POSIX Message Queues, and FIFO to enable communication between its independent processes. Each technique has a specific role in transferring requests, returning results, and recording execution logs.
