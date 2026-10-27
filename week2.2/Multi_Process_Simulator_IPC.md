MULTI-PROCESS SIMULATOR & IPC

## 1. Performance Metrics

- Execution Time: Time taken by a process to run.
- CPU Usage: Percentage of CPU used by a process.
- User CPU Time: Time spent executing user-level code.
- System CPU Time: Time spent performing operating system tasks.
- Memory Usage: Maximum resident memory used by a process.
- Exit Status: Indicates whether a process terminated successfully.

## 2. Standalone Simulator — Performance Results

| Performance Metric   | Result        |
| -------------------- | ------------- |
| Execution Time       | 23.99 seconds |
| CPU Usage            | 0%            |
| User CPU Time        | 0.00 s        |
| System CPU Time      | 0.00 s        |
| Maximum Memory Usage | 1,912 KB      |
| Exit Status          | 0             |

## 3. Multi-Process Simulator — Performance Results

| Performance Metric   | UI Process | Core Process | Logger Process |
| -------------------- | ---------- | ------------ | -------------- |
| Execution Time       | 58.32 s    | 65.82 s      | 76.36 s        |
| CPU Usage            | 0%         | 0%           | 0%             |
| User CPU Time        | 0.00 s     | 0.00 s       | 0.00 s         |
| System CPU Time      | 0.00 s     | 0.00 s       | 0.00 s         |
| Maximum Memory Usage | 1,848 KB   | 1,660 KB     | 1,528 KB       |
| Exit Status          | 0          | 0            | 0              |

## 4. IPC Test Results

| Test Case                   | Expected Result                                    | Status |
| --------------------------- | -------------------------------------------------- | ------ |
| UI → Core communication     | Core receives input through Unix Domain Socket     | Passed |
| Core → UI communication     | UI receives the result through POSIX Message Queue | Passed |
| Core → Logger communication | Logger receives messages through FIFO (Named Pipe) | Passed |
| Addition operation          | 12 + 13 = 25                                       | Passed |
| Logging operation           | Logger displays status, message, and result        | Passed |
| UI termination              | UI disconnects successfully after exit request     | Passed |
| Process termination         | Processes terminate successfully                   | Passed |

## 5. Comparison Between Architectures

| Feature                | Standalone Simulator                       | Multi-Process Simulator                        |
| ---------------------- | ------------------------------------------ | ---------------------------------------------- |
| Architecture           | Single process                             | Three independent processes                    |
| Components             | UI, processing, and logging in one process | Separate UI, Core, and Logger processes        |
| Communication          | Direct function calls                      | IPC mechanisms                                 |
| Modularity             | Lower                                      | Higher                                         |
| Communication overhead | Minimal                                    | Additional IPC overhead                        |
| Maximum memory usage   | 1,912 KB                                   | UI: 1,848 KB; Core: 1,660 KB; Logger: 1,528 KB |

## 6. Performance Analysis

The standalone simulator performs all operations within one process, avoiding inter-process communication overhead.

The multi-process simulator separates the UI, Core, and Logger into independent processes. This improves modularity and makes each component easier to manage and maintain.

The multi-process simulator uses Unix Domain Sockets, POSIX Message Queues, and FIFO for communication. These mechanisms introduce additional communication overhead.

The addition test successfully demonstrated input transfer, computation, result delivery, and logging. The recorded CPU usage was 0%, with user and system CPU times displayed as 0.00 seconds.

## 7. Conclusion

The Week 4 Multi-Process Simulator successfully demonstrates communication between three independent processes: UI, Core, and Logger.

The project implements three IPC mechanisms: Unix Domain Socket, POSIX Message Queue, and FIFO. The addition test verified that the Core process receives input, calculates the result, returns it to the UI, and sends the log to the Logger.

The performance comparison demonstrates the differences between a standalone architecture and a multi-process architecture, including modularity, memory usage, and communication overhead.
