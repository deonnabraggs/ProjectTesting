
#include <stdio.h>
#include <string.h>
#include "scheduler.h"

void display_gantt_chart(Event events[], int event_count)
{
    int i;

    if (events == NULL || event_count <= 0) {
        printf("\nNo events to display.\n");
        return;
    }

    printf("\n========== GANTT CHART ==========\n\n");

    for (i = 0; i < event_count; i++) {
        if (events[i].pid == -1)
            printf("| IDLE ");
        else
            printf("| P%d ", events[i].pid);
    }
    printf("|\n");

    if (events[0].start_time == 0)
        printf("0");
    else
        printf("%d", events[0].start_time);

    for (i = 0; i < event_count; i++)
        printf("\t%d", events[i].end_time);

    printf("\n");
}

void display_results(Process p[], int n)
{
    int i;

    printf("\n================ PROCESS RESULTS ================\n");

    printf("%-6s %-5s %-5s %-8s %-6s %-7s %-6s %-6s\n",
           "PID", "AT", "BT", "Priority",
           "CT", "TAT", "WT", "RT");

    for (i = 0; i < n; i++) {
        printf("%-6d %-5d %-5d %-8d %-6d %-7d %-6d %-6d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].priority,
               p[i].completion_time,
               p[i].turnaround_time,
               p[i].waiting_time,
               p[i].response_time);
    }
}

void calculate_averages(Process p[], int n)
{
    int i;
    double wt = 0, tat = 0, rt = 0;

    if (p == NULL || n <= 0) {
        printf("\nNo processes available.\n");
        return;
    }

    for (i = 0; i < n; i++) {
        wt += p[i].waiting_time;
        tat += p[i].turnaround_time;
        rt += p[i].response_time;
    }

    printf("\n========== AVERAGES ==========\n");
    printf("Average Waiting Time    : %.2f\n", wt / n);
    printf("Average Turnaround Time : %.2f\n", tat / n);
    printf("Average Response Time   : %.2f\n", rt / n);
}

void calculate_cpu_utilization(
    Event events[], int event_count, int total_time)
{
    int i, busy = 0, idle = 0;

    if (events == NULL || event_count <= 0 || total_time <= 0) {
        printf("\nCPU utilization cannot be calculated.\n");
        return;
    }

    for (i = 0; i < event_count; i++) {
        int duration =
            events[i].end_time - events[i].start_time;

        if (events[i].pid == -1)
            idle += duration;
        else
            busy += duration;
    }

    printf("\n========== CPU UTILIZATION ==========\n");
    printf("CPU Busy Time  : %d\n", busy);
    printf("CPU Idle Time  : %d\n", idle);
    printf("Total Time     : %d\n", total_time);
    printf("CPU Utilization: %.2f%%\n",
           (double)busy / total_time * 100.0);
}

static double average_metric(Process p[], int n, int metric)
{
    int i;
    double total = 0;

    for (i = 0; i < n; i++) {
        if (metric == 0)
            total += p[i].waiting_time;
        else if (metric == 1)
            total += p[i].turnaround_time;
        else
            total += p[i].response_time;
    }

    return total / n;
}

static int get_total_time(Event events[], int count)
{
    int i, total = 0;

    for (i = 0; i < count; i++) {
        if (events[i].end_time > total)
            total = events[i].end_time;
    }

    return total;
}

static double get_cpu_usage(Event events[], int count, int total_time)
{
    int i, busy = 0;

    if (total_time <= 0)
        return 0.0;

    for (i = 0; i < count; i++) {
        if (events[i].pid != -1)
            busy += events[i].end_time - events[i].start_time;
    }

    return (double)busy / total_time * 100.0;
}

void compare_algorithms(Process input[], int n, int quantum)
{
    Process p[6][MAX_PROCESSES];
    Event events[6][MAX_EVENTS];
    int counts[6] = {0};
    int total_time[6];
    int i;

    const char *names[] = {
        "FCFS",
        "SJF Non-Preemptive",
        "Priority Non-Preemptive",
        "SRTF",
        "Priority Preemptive",
        "Round Robin"
    };

    if (input == NULL || n <= 0 || n > MAX_PROCESSES) {
        printf("\nInvalid process data.\n");
        return;
    }

    if (quantum <= 0) {
        printf("\nTime quantum must be greater than zero.\n");
        return;
    }

    for (i = 0; i < 6; i++)
        memcpy(p[i], input, sizeof(Process) * n);

    fcfs(p[0], n, events[0], &counts[0]);
    sjf(p[1], n, events[1], &counts[1]);
    priority_nonpreemptive(p[2], n, events[2], &counts[2]);
    srtf(p[3], n, events[3], &counts[3]);
    priority_preemptive(p[4], n, events[4], &counts[4]);
    round_robin(p[5], n, quantum, events[5], &counts[5]);

    printf("\n================ ALGORITHM COMPARISON ================\n");

    printf("%-25s %12s %12s %12s %12s\n",
           "Algorithm", "Avg WT", "Avg TAT",
           "Avg RT", "CPU Usage");

    for (i = 0; i < 6; i++) {
        total_time[i] = get_total_time(events[i], counts[i]);

        printf("%-25s %12.2f %12.2f %12.2f %11.2f%%\n",
               names[i],
               average_metric(p[i], n, 0),
               average_metric(p[i], n, 1),
               average_metric(p[i], n, 2),
               get_cpu_usage(events[i], counts[i], total_time[i]));
    }

    printf("\nRound Robin Time Quantum: %d\n", quantum);
}
