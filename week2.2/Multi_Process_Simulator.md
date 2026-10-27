# MULTI-PROCESS SIMULATOR & IPC

## 1. Performance Metrics

- Execution Time: Time taken by a process to run.
- CPU Usage: Percentage of CPU used by a process.
- User CPU Time: Time spent executing user-level code.
- System CPU Time: Time spent performing operating system tasks.
- Memory Usage: Maximum resident memory used by a process.
- Exit Status: Indicates whether a process terminated successfully.

## 2. Standalone Simulator — Performance Results

| Performance Metric | Result |
|---|---|
| Execution Time | 23.99 seconds |
| CPU Usage | 0% |
| User CPU Time | 0.00 s |
| System CPU Time | 0.00 s |
| Maximum Memory Usage | 1,912 KB |
| Exit Status | 0 |

## 3. Multi-Process Simulator — Performance Results

| Performance Metric | UI Process | Core Process | Logger Process |
|---|---:|---:|---:|
| Execution Time | 58.32 s | 65.82 s | 76.36 s |
| CPU Usage | 0% | 0% | 0% |
| User CPU Time | 0.00 s | 0.00 s | 0.00 s |
| System CPU Time | 0.00 s | 0.00 s | 0.00 s |
| Maximum Memory Usage | 1,848 KB | 1,660 KB | 1,528 KB |
| Exit Status | 0 | 0 | 0 |

## 4. Comparison Between Architectures

| Feature | Standalone Simulator | Multi-Process Simulator |
|---|---|---|
| Architecture | Single process | Three independent processes |
| Components | UI, processing, and logging in one process | Separate UI, Core, and Logger processes |
| Communication | Direct function calls | IPC mechanisms |
| Modularity | Lower | Higher |
| Communication overhead | Minimal | Additional IPC overhead |
| Maximum memory usage | 1,912 KB | UI: 1,848 KB; Core: 1,660 KB; Logger: 1,528 KB |

## 5. Performance Analysis

The standalone simulator performs all operations within one process, avoiding inter-process communication overhead.

The multi-process simulator separates the UI, Core, and Logger into independent processes. This improves modularity and makes each component easier to manage and maintain.

### IPC Overhead

IPC overhead refers to the additional processing and communication work required when separate processes exchange data.

In the multi-process simulator, the UI sends requests to Core through a Unix Domain Socket. Core sends results to the UI through a POSIX Message Queue and sends log messages to Logger through a FIFO (Named Pipe).

These communication mechanisms require additional operations compared with direct function calls in the standalone simulator.

The exact numerical IPC overhead was not measured separately in this project. Therefore, its precise time or percentage cannot be determined from the recorded performance results alone.

The recorded CPU usage was 0%, with user and system CPU times displayed as 0.00 seconds.

## 6. Conclusion

The Week 4 Multi-Process Simulator demonstrates the separation of UI, Core, and Logger into three independent processes.

The project implements Unix Domain Sockets, POSIX Message Queues, and FIFO for inter-process communication. This architecture improves modularity but introduces additional communication work.

The performance comparison presents the recorded execution time, CPU usage, and memory usage for the standalone and multi-process simulators. IPC overhead is discussed qualitatively because it was not measured separately.
