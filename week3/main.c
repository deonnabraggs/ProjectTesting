#include <stdio.h>
#include "scheduler.h"

int main(void)
{
    Process p[MAX_PROCESSES];
    Event events[MAX_EVENTS];

    int n, i, choice, quantum = 2;
    int event_count = 0;

    printf("====================================\n");
    printf("      CPU SCHEDULING SIMULATOR\n");
    printf("====================================\n");

    printf("Enter number of processes (1-%d): ", MAX_PROCESSES);
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (i = 0; i < n; i++) {
        printf("\nProcess P%d\n", i + 1);
        p[i].pid = i + 1;

        printf("Arrival Time: ");
        if (scanf("%d", &p[i].arrival_time) != 1 || p[i].arrival_time < 0) {
            printf("Arrival time must be zero or greater.\n");
            return 1;
        }

        printf("Burst Time: ");
        if (scanf("%d", &p[i].burst_time) != 1 || p[i].burst_time <= 0) {
            printf("Burst time must be greater than zero.\n");
            return 1;
        }

        printf("Priority: ");
        if (scanf("%d", &p[i].priority) != 1) {
            printf("Invalid priority.\n");
            return 1;
        }
    }

    printf("\nEnter Time Quantum for Round Robin: ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        quantum = 2;
        printf("Invalid quantum. Defaulting to 2.\n");
    }

    printf("\nChoose Algorithm:\n");
    printf("1. FCFS\n");
    printf("2. SJF (Non-Preemptive)\n");
    printf("3. Priority (Non-Preemptive)\n");
    printf("4. SRTF (Preemptive SJF)\n");
    printf("5. Priority (Preemptive)\n");
    printf("6. Round Robin\n");
    printf("7. Compare All Algorithms\n");
    printf("Enter choice (1-7): ");
    
    if (scanf("%d", &choice) != 1) {
        printf("Invalid choice.\n");
        return 1;
    }

    switch (choice) {
        case 1:
            fcfs(p, n, events, &event_count);
            break;
        case 2:
            sjf(p, n, events, &event_count);
            break;
        case 3:
            priority_nonpreemptive(p, n, events, &event_count);
            break;
        case 4:
            srtf(p, n, events, &event_count);
            break;
        case 5:
            priority_preemptive(p, n, events, &event_count);
            break;
        case 6:
            round_robin(p, n, quantum, events, &event_count);
            break;
        case 7:
            compare_algorithms(p, n, quantum);
            return 0;
        default:
            printf("Invalid selection.\n");
            return 1;
    }

    display_gantt_chart(events, event_count);
    display_results(p, n);
    calculate_averages(p, n);
    
    if (event_count > 0) {
        calculate_cpu_utilization(events, event_count, events[event_count - 1].end_time);
    }

    return 0;
}
