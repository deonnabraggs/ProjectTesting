#ifndef DEADLOCK_H
#define DEADLOCK_H

#include <gtk/gtk.h>

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

#define REQ_INVALID 0
#define REQ_EXCEEDS_NEED 1
#define REQ_MUST_WAIT 2
#define REQ_GRANTED 3
#define REQ_UNSAFE 4

#define REC_INVALID 0
#define REC_OK 1

extern int n, m;
extern int total[MAX_RESOURCES];
extern int available[MAX_RESOURCES];
extern int allocation[MAX_PROCESSES][MAX_RESOURCES];
extern int maximum[MAX_PROCESSES][MAX_RESOURCES];
extern int need[MAX_PROCESSES][MAX_RESOURCES];
extern int request[MAX_PROCESSES][MAX_RESOURCES];
extern int active[MAX_PROCESSES];

int is_safe(int sequence[]);
int detect_deadlock(int deadlocked[]);

int request_resources(int pid, const int req[]);
int recover_from_deadlock(int pid);

void create_processes(int count);
void create_resources(int count, const int amounts[]);
void set_allocation(int pid, int rid, int amount);
void set_maximum(int pid, int rid, int amount);
void recalculate_state(void);
void load_sample_data(void);
double resource_utilization(int rid);

void draw_resource_graph(cairo_t *cr, int width, int height);
void run_gui(int argc, char *argv[]);

#endif
