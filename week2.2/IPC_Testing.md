# IPC TESTING

## 1. Objective

The objective of IPC testing is to verify that the UI, Core, and Logger processes communicate correctly using the selected Inter-Process Communication (IPC) mechanisms.

## 2. IPC Mechanisms Tested

| Communication | IPC Mechanism       |
| ------------- | ------------------- |
| UI → Core     | Unix Domain Socket  |
| Core → UI     | POSIX Message Queue |
| Core → Logger | FIFO (Named Pipe)   |

## 3. IPC Test Cases and Results

| Test Case                    | Expected Result                       | Actual Result                                                                            | Status     |
| ---------------------------- | ------------------------------------- | ---------------------------------------------------------------------------------------- | ---------- |
| UI to Core communication     | Core receives the user's input        | Core received option 1 for the ADD operation through the UI connection                   | Passed     |
| Core to UI communication     | UI receives the computed result       | UI displayed SUCCESS and the addition result 5                                           | Passed\*   |
| Core to Logger communication | Logger receives the operation details | Logger received the addition message and result 5                                        | Passed     |
| Addition operation           | The simulator calculates 2 + 3        | The simulator returned 5                                                                 | Passed     |
| Logging operation            | Logger displays the operation details | Logger displayed `CPU: ADD result = 5` and `Result: 5`                                   | Passed     |
| UI connection                | UI connects to the Core process       | Core displayed `UI connected.`                                                           | Passed     |
| Logger connection            | Core connects to the Logger process   | Core displayed `Logger connected.`                                                       | Passed     |
| UI termination               | UI disconnects after the exit request | Exit request and disconnection were not shown in the provided output                     | Not tested |
| Core and Logger termination  | Both processes terminate successfully | Core was interrupted using Ctrl+C; normal termination of both processes was not verified | Not tested |

\*The UI displayed the result, but the terminal output alone does not confirm that a POSIX message queue was used.

## 4. Test Summary

The IPC tests demonstrated communication between the UI, Core, and Logger processes during the addition operation. The Core process received the ADD request from the UI, performed the calculation using inputs 2 and 3, and generated the correct result, 5.

The UI displayed the successful result, and the Logger received and displayed the operation details. The terminal outputs also confirmed that the Core process connected to both the UI and Logger.

The exit procedure and normal termination of all processes have not yet been verified. The use of a POSIX message queue for Core-to-UI communication must also be confirmed from the source code.

## 5. Conclusion

The IPC testing confirmed that the simulator successfully performed the addition operation, returned the result to the UI, and sent operation details to the Logger. The observed outputs support successful communication between the processes during the tested operation. Additional testing is required to verify normal process termination and confirm the specific IPC mechanism used for Core-to-UI communication.
