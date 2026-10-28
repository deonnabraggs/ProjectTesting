# MULTI-PROCESS SIMULATOR & IPC

## 1. Performance Metrics

- Execution Time: The time taken by a process to complete its execution.
- CPU Usage: The percentage of CPU resources utilized by a process during execution.
- User CPU Time: The amount of time the CPU spends executing instructions in user mode for a process.
- System CPU Time: The amount of time the CPU spends performing operating system tasks on behalf of a process.
- Maximum Memory Usage: The maximum amount of physical memory occupied by a process during execution.
- Exit Status: A value that indicates whether a process terminated successfully or encountered an error.

## 2. Standalone Simulator — Performance Results

| Performance Metric   | Result        |
| -------------------- | ------------- |
| Execution Time       | 23.99 seconds |
| CPU Usage            | 0%            |
| User CPU Time        | 0.00 seconds  |
| System CPU Time      | 0.00 seconds  |
| Maximum Memory Usage | 1,912 KB      |
| Exit Status          | 0             |

The standalone simulator completed its execution in 23.99 seconds, reported 0% CPU usage, and used a maximum of 1,912 KB of memory. Its exit status was 0, indicating successful termination.

## 3. Multi-Process Simulator — Performance Results

| Performance Metric   | UI Process    | Core Process  | Logger Process |
| -------------------- | ------------- | ------------- | -------------- |
| Execution Time       | 17.10 seconds | 25.24 seconds | 32.50 seconds  |
| CPU Usage            | 0%            | 0%            | 0%             |
| User CPU Time        | 0.00 seconds  | 0.00 seconds  | 0.00 seconds   |
| System CPU Time      | 0.00 seconds  | 0.00 seconds  | 0.00 seconds   |
| Maximum Memory Usage | 1,836 KB      | 1,780 KB      | 1,584 KB       |
| Exit Status          | 0             | 0             | 0              |

The UI process completed in 17.10 seconds and used a maximum of 1,836 KB of memory. The Core process completed in 25.24 seconds and used a maximum of 1,780 KB of memory. The Logger process completed in 32.50 seconds and used a maximum of 1,584 KB of memory. All three processes reported an exit status of 0, indicating successful termination.

## 4. Comparison Between Architectures

| Feature                | Standalone Simulator                                                    | Multi-Process Simulator                                                                           |
| ---------------------- | ----------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| Architecture           | The simulator executes within a single process.                         | The simulator executes through three independent processes.                                       |
| Components             | The UI, processing logic, and logging operate within one process.       | The UI, Core, and Logger operate as separate processes.                                           |
| Communication          | Components communicate through direct function calls.                   | Processes communicate through inter-process communication mechanisms.                             |
| Modularity             | The components are less independent because they belong to one process. | The components are more independent and easier to maintain separately.                            |
| Communication Overhead | The simulator has minimal inter-process communication overhead.         | The simulator introduces additional communication overhead between processes.                     |
| Maximum Memory Usage   | The recorded maximum memory usage is 1,912 KB.                          | The recorded maximum memory usage is 1,836 KB for UI, 1,780 KB for Core, and 1,584 KB for Logger. |

## 5. Performance Analysis

The standalone simulator performs all operations within a single process, so it does not require communication between independent processes.

The multi-process simulator separates the user interface, processing logic, and logging functionality into three independent processes. This architecture improves modularity because each process handles a specific responsibility.

The UI process receives user input and sends operation requests to the Core process. The Core process performs the requested operations and sends the results back to the UI. The Logger process receives log messages from the Core process and displays them in the terminal.

### IPC Overhead

Inter-process communication overhead refers to the additional processing required when independent processes exchange data.

The simulator uses a Unix Domain Socket for communication from UI to Core, a POSIX Message Queue for communication from Core to UI, and a FIFO (Named Pipe) for communication from Core to Logger.

These communication mechanisms require additional operations compared with direct function calls in a standalone simulator.

The recorded UI execution time was 17.10 seconds, the Core execution time was 25.24 seconds, and the Logger execution time was 32.50 seconds. These times may include waiting for input, connections, and messages, so they should not be interpreted as pure processing time or used to calculate exact IPC overhead.

All three processes reported 0% CPU usage, with user and system CPU times displayed as 0.00 seconds. These values are rounded measurements and do not indicate that the processes performed no work.

## 6. Conclusion

The Week 2.2 Multi-Process Simulator demonstrates how a program can be divided into three independent processes to improve modularity and separate responsibilities.

The project implements Unix Domain Sockets, POSIX Message Queues, and FIFO for communication between the UI, Core, and Logger processes.

The UI successfully performed addition, subtraction, multiplication, and division, producing the expected results of 25, 9, 36, and 4, respectively.

The benchmark results show that the UI process completed in 17.10 seconds, the Core process completed in 25.24 seconds, and the Logger process completed in 32.50 seconds. Their maximum memory usages were 1,836 KB, 1,780 KB, and 1,584 KB, respectively.

All three processes terminated successfully with exit status 0. The results demonstrate the functionality of the multi-process architecture, while the exact numerical IPC overhead remains unmeasured.
