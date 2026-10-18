#ifndef DEADLOCK_H
#define DEADLOCK_H

typedef struct _cairo cairo_t;

#define MAX_PROCESSES 10
#define MAX_RESOURCES 10

extern int n, m;

extern int total[MAX_RESOURCES];
extern int available[MAX_RESOURCES];

extern int allocation[MAX_PROCESSES][MAX_RESOURCES];
extern int maximum[MAX_PROCESSES][MAX_RESOURCES];
extern int need[MAX_PROCESSES][MAX_RESOURCES];
extern int request[MAX_PROCESSES][MAX_RESOURCES];

extern int active[MAX_PROCESSES];

void create_processes(int count);
void create_resources(int count, const int totals[]);
void set_allocation(int pid, int rid, int value);
void set_maximum(int pid, int rid, int value);
void recalculate_state(void);
void load_sample_data(void);
float resource_utilization(int rid);

int is_safe(int sequence[]);
int detect_deadlock(int deadlocked[]);

#define REQ_GRANTED       0
#define REQ_INVALID       1
#define REQ_EXCEEDS_NEED  2
#define REQ_MUST_WAIT     3
#define REQ_UNSAFE        4

int request_resources(int pid, const int req[]);

#define REC_OK       0
#define REC_INVALID  1

int recover_from_deadlock(int pid);

void draw_resource_graph(cairo_t *cr, int width, int height);

void create_gui(int *argc, char ***argv);
void refresh_gui(void);
void show_message(const char *message);

#endif
