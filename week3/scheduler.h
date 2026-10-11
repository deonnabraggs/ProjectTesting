#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 100
#define MAX_EVENTS 10000

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int priority;

    int waiting_time;
    int turnaround_time;
    int response_time;
    int completion_time;
} Process;

typedef struct {
    int pid;  /* -1 means CPU is idle */
    int start_time;
    int end_time;
} Event;

/* Person 1: Non-preemptive algorithms */
void fcfs(Process p[], int n, Event events[], int *event_count);
void sjf(Process p[], int n, Event events[], int *event_count);
void priority_nonpreemptive(
    Process p[], int n, Event events[], int *event_count);

/* Person 2: Preemptive algorithms */
void srtf(Process p[], int n, Event events[], int *event_count);
void priority_preemptive(
    Process p[], int n, Event events[], int *event_count);
void round_robin(
    Process p[], int n, int quantum,
    Event events[], int *event_count);

/* Person 3: Output and calculations */
void display_gantt_chart(Event events[], int event_count);
void display_results(Process p[], int n);
void calculate_averages(Process p[], int n);
void calculate_cpu_utilization(
    Event events[], int event_count, int total_time);
void compare_algorithms(Process input[], int n, int quantum);

#endif
