#ifndef IPC_H
#define IPC_H

#define UI_TO_CORE "/ui_to_core"
#define CORE_TO_UI "/core_to_ui"
#define CORE_TO_LOGGER "/core_to_logger"

#define MAX_MESSAGE 256

typedef struct
{
    int command;
    int value1;
    int value2;
    char instruction[MAX_MESSAGE];
} CoreMessage;

typedef struct
{
    int status;
    int result;
    char message[MAX_MESSAGE];
} LogMessage;

#endif
