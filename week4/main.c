#include "deadlock.h"
#include <gtk/gtk.h>

/* Shared system data */
int n = 0;
int m = 0;

int total[MAX_RESOURCES] = {0};
int available[MAX_RESOURCES] = {0};

int allocation[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
int maximum[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
int need[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
int request[MAX_PROCESSES][MAX_RESOURCES] = {{0}};

int active[MAX_PROCESSES] = {0};

/* GUI entry point implemented in gui.c */
void run_gui(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    run_gui(argc, argv);
    return 0;
}
