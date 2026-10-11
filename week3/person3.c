
#include <stdio.h>
#include <string.h>
#include "scheduler.h"

/* =========================================================
   PERSON 3: GANTT CHART, RESULTS, AVERAGES AND COMPARISON
   ========================================================= */


/* ---------------------------------------------------------
   1. DISPLAY GANTT CHART
   --------------------------------------------------------- */
void display_gantt_chart(Event events[], int event_count)
{
    int i;

    if (events == NULL || event_count <= 0)
    {
        printf("\nNo Gantt chart events available.\n");
        return;
    }

    printf("\n========== GANTT CHART ==========\n");

    /* Display process blocks */
    printf("|");

    for (i = 0; i < event_count; i++)
    {
        if (events[i].pid == -1)
            printf(" IDLE |");
        else
            printf(" P%d |", events[i].pid);
    }

    printf("\n");

    /* Display time boundaries */
    printf("%d", events[0].start_time);

    for (i = 0; i < event_count; i++)
    {
        printf("     %d", events[i].end_time);
    }

    printf("\n=================================\n");
}


/* ---------------------------------------------------------
   2. DISPLAY INDIVIDUAL PROCESS RESULTS
   --------------------------------------------------------- */
void display_results(Process p[], int n)
{
    int i;

    if (p == NULL || n <= 0)
    {
        printf("\nNo process results available.\n");
        return;
    }

    printf("\n================ PROCESS RESULTS ================\n");

    printf("%-8s %-8s %-8s %-8s %-8s %-8s %-8s\n",
           "PID", "AT", "BT", "CT", "TAT", "WT", "RT");

    printf("-------------------------------------------------\n");

    for (i = 0; i < n; i++)
    {
        printf("%-8d %-8d %-8d %-8d %-8d %-8d %-8d\n",
               p[i].pid,
               p[i].arrival_time,
               p[i].burst_time,
               p[i].completion_time,
               p[i].turnaround_time,
               p[i].waiting_time,
               p[i].response_time);
    }

    printf("=================================================\n");
}


/* ---------------------------------------------------------
   3. CALCULATE AND DISPLAY AVERAGES
   --------------------------------------------------------- */
void calculate_averages(Process p[], int n)
{
    int i;
    double total_wt = 0.0;
    double total_tat = 0.0;
    double total_rt = 0.0;

    if (p == NULL || n <= 0)
    {
        printf("\nCannot calculate averages: no processes.\n");
        return;
    }

    for (i = 0; i < n; i++)
    {
        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
        total_rt += p[i].response_time;
    }

    printf("\n========== AVERAGE RESULTS ==========\n");

    printf("Average Waiting Time    : %.2f\n",
           total_wt / n);

    printf("Average Turnaround Time : %.2f\n",
           total_tat / n);

    printf("Average Response Time   : %.2f\n",
           total_rt / n);

    printf("=====================================\n");
}


/* ---------------------------------------------------------
   4. CALCULATE TOTAL ELAPSED TIME
   --------------------------------------------------------- */
static int get_total_time(Event events[], int event_count)
{
    int i;
    int first_time;
    int last_time;

    if (events == NULL || event_count <= 0)
        return 0;

    first_time = events[0].start_time;
    last_time = events[0].end_time;

    for (i = 0; i < event_count; i++)
    {
        if (events[i].start_time < first_time)
            first_time = events[i].start_time;

        if (events[i].end_time > last_time)
            last_time = events[i].end_time;
    }

    if (last_time <= first_time)
        return 0;

    return last_time - first_time;
}


/* ---------------------------------------------------------
   5. CALCULATE CPU UTILIZATION
   --------------------------------------------------------- */
static double get_cpu_usage(Event events[], int event_count,
                            int total_time)
{
    int i;
    int busy_time = 0;

    if (events == NULL || event_count <= 0 || total_time <= 0)
        return 0.0;

    for (i = 0; i < event_count; i++)
    {
        if (events[i].pid != -1 &&
            events[i].end_time > events[i].start_time)
        {
            busy_time += events[i].end_time - events[i].start_time;
        }
    }

    return ((double) busy_time / total_time) * 100.0;
}


/* ---------------------------------------------------------
   6. DISPLAY CPU UTILIZATION
   --------------------------------------------------------- */
void calculate_cpu_utilization(Event events[], int event_count,
                               int total_time)
{
    double utilization;

    if (events == NULL || event_count <= 0 || total_time <= 0)
    {
        printf("\nCPU Utilization: Cannot calculate.\n");
        return;
    }

    utilization = get_cpu_usage(events, event_count, total_time);

    printf("\nCPU Utilization: %.2f%%\n", utilization);
}


/* ---------------------------------------------------------
   7. HELPER: DISPLAY ONE ALGORITHM'S COMPARISON RESULTS
   --------------------------------------------------------- */
static void report_one(const char *name,
                       Process p[],
                       int n,
                       Event events[],
                       int event_count)
{
    int i;
    double total_wt = 0.0;
    double total_tat = 0.0;
    double total_rt = 0.0;
    double cpu_usage = 0.0;
    int total_time;

    for (i = 0; i < n; i++)
    {
        total_wt += p[i].waiting_time;
        total_tat += p[i].turnaround_time;
        total_rt += p[i].response_time;
    }

    total_time = get_total_time(events, event_count);

    if (total_time > 0)
        cpu_usage = get_cpu_usage(events, event_count, total_time);

    printf("%-23s %-12.2f %-12.2f %-12.2f %-12.2f\n",
           name,
           total_wt / n,
           total_tat / n,
           total_rt / n,
           cpu_usage);
}


/* ---------------------------------------------------------
   8. COMPARE ALL SIX SCHEDULING ALGORITHMS
   --------------------------------------------------------- */
void compare_algorithms(Process input[], int n, int quantum)
{
    Process p_fcfs[MAX_PROCESSES];
    Process p_sjf[MAX_PROCESSES];
    Process p_priority[MAX_PROCESSES];
    Process p_srtf[MAX_PROCESSES];
    Process p_priority_pre[MAX_PROCESSES];
    Process p_rr[MAX_PROCESSES];

    Event e_fcfs[MAX_EVENTS];
    Event e_sjf[MAX_EVENTS];
    Event e_priority[MAX_EVENTS];
    Event e_srtf[MAX_EVENTS];
    Event e_priority_pre[MAX_EVENTS];
    Event e_rr[MAX_EVENTS];

    int c_fcfs = 0;
    int c_sjf = 0;
    int c_priority = 0;
    int c_srtf = 0;
    int c_priority_pre = 0;
    int c_rr = 0;

    if (input == NULL || n <= 0 || n > MAX_PROCESSES)
    {
        printf("\nInvalid process data for comparison.\n");
        return;
    }

    if (quantum <= 0)
    {
        printf("\nTime quantum must be greater than zero.\n");
        return;
    }

    /* Preserve the original input for every algorithm. */
    memcpy(p_fcfs, input, n * sizeof(Process));
    memcpy(p_sjf, input, n * sizeof(Process));
    memcpy(p_priority, input, n * sizeof(Process));
    memcpy(p_srtf, input, n * sizeof(Process));
    memcpy(p_priority_pre, input, n * sizeof(Process));
    memcpy(p_rr, input, n * sizeof(Process));

    /* Run each algorithm independently. */
    fcfs(p_fcfs, n, e_fcfs, &c_fcfs);

    sjf(p_sjf, n, e_sjf, &c_sjf);

    priority_nonpreemptive(p_priority, n,
                           e_priority, &c_priority);

    srtf(p_srtf, n, e_srtf, &c_srtf);

    priority_preemptive(p_priority_pre, n,
                        e_priority_pre, &c_priority_pre);

    round_robin(p_rr, n, quantum, e_rr, &c_rr);

    printf("\n");
    printf("=============================================================="
           "================================\n");
    printf("                  ALGORITHM COMPARISON\n");
    printf("=============================================================="
           "================================\n");

    printf("%-23s %-12s %-12s %-12s %-12s\n",
           "Algorithm",
           "Avg WT",
           "Avg TAT",
           "Avg RT",
           "CPU Util %");

    printf("----------------------------------------------------------------"
           "--------------------------------\n");

    report_one("FCFS",
               p_fcfs, n, e_fcfs, c_fcfs);

    report_one("SJF Non-Preemptive",
               p_sjf, n, e_sjf, c_sjf);

    report_one("Priority Non-Preemptive",
               p_priority, n, e_priority, c_priority);

    report_one("SRTF",
               p_srtf, n, e_srtf, c_srtf);

    report_one("Priority Preemptive",
               p_priority_pre, n, e_priority_pre, c_priority_pre);

    report_one("Round Robin",
               p_rr, n, e_rr, c_rr);

    printf("=============================================================="
           "================================\n");
}
