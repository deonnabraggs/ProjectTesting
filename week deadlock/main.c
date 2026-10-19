#include "deadlock.h"
#include <string.h>

int n = 0;
int m = 0;

int total[MAX_RESOURCES] = {0};
int available[MAX_RESOURCES] = {0};

int allocation[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
int maximum[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
int need[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
int request[MAX_PROCESSES][MAX_RESOURCES] = {{0}};

int active[MAX_PROCESSES] = {0};

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "--gui-only") == 0)
    {
        run_gui(argc, argv);
        return 0;
    }

    if (!terminal_input())
        return 1;

    run_gui(argc, argv);
    return 0;
}
