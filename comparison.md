# Performance Benchmark – Standalone vs Multi-Process Simulator

## 1. Objective

The objective of this benchmark is to compare the performance of the Standalone Simulator with the Multi-Process Simulator. The Standalone Simulator performs all operations within a single process, while the Multi-Process Simulator separates the system into UI, Core and Logger processes and uses POSIX Message Queues for communication.

Measured metrics: execution time, CPU usage, memory usage and IPC overhead. The Linux `/usr/bin/time -v` command was used.

## 2. Benchmark Results

### Standalone Simulator

Tested operations: ADD (12 + 13 = 25), SUBTRACT (23 - 12 = 11), MULTIPLY (12 × 4 = 48), and DIVIDE (12 ÷ 2 = 6).

| Metric | Standalone |
|---|---|
| Execution Time | 23.99 seconds |
| User CPU Time | 0.00 seconds |
| System CPU Time | 0.00 seconds |
| CPU Usage | 0% |
| Maximum Memory Usage | 1912 KB |

### Multi-Process Simulator

| Process | Execution Time | CPU Usage | Maximum Memory |
|---|---:|---:|---:|
| UI | 35.31 seconds | 0% | 1784 KB |
| Core | 29.21 seconds | 0% | 1660 KB |
| Logger | 23.98 seconds | 0% | 1656 KB |

## 3. Comparison

| Performance Metric | Standalone | Multi-Process |
|---|---|---|
| Execution Time | 23.99 s | UI: 35.31 s; Core: 29.21 s; Logger: 23.98 s |
| CPU Usage | 0% | 0% for each process |
| Maximum Memory | 1912 KB | UI: 1784 KB; Core: 1660 KB; Logger: 1656 KB |
| Processes | 1 | 3 |
| IPC | Not required | POSIX Message Queues |

The individual UI, Core and Logger elapsed times must not be added together because the processes can run concurrently and their elapsed times can overlap. The Standalone measurement also includes manual user input time, so the current measurements are not a controlled end-to-end comparison.

## 4. IPC Overhead

The Multi-Process Simulator introduces IPC overhead because the UI, Core and Logger processes communicate through POSIX Message Queues.

IPC Overhead = Multi-Process End-to-End Time − Standalone End-to-End Time

An exact numerical IPC overhead cannot be calculated from the current measurements because the Multi-Process Simulator was timed separately for UI, Core and Logger rather than with one common start and end point. Therefore, the three process times should not be summed.

## 5. Conclusion

The benchmark demonstrates the architectural difference between the two versions. The Standalone Simulator performs all operations inside one process without IPC, while the Multi-Process Simulator distributes the work among UI, Core and Logger and communicates using POSIX Message Queues.

The Standalone Simulator recorded 23.99 seconds of elapsed time and 1912 KB of maximum resident memory. The Multi-Process measurements recorded 35.31 seconds for UI, 29.21 seconds for Core and 23.98 seconds for Logger, with maximum memory values of 1784 KB, 1660 KB and 1656 KB respectively.

The Multi-Process architecture provides modularity and process separation, but introduces additional process-management and IPC coordination. For an exact end-to-end performance and IPC-overhead comparison, an automated benchmark using identical fixed inputs and a common start/end measurement should be performed.
