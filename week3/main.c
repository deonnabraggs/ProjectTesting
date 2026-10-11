#include <stdio.h>
#include <string.h>
#include "scheduler.h"

static int read_int(const char *prompt, int *value)
{
    int result;

    printf("%s", prompt);
    result = scanf("%d", value);

    if (result != 1) {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF) {
        }

        return 0;
    }

    return 1;
}

static void run_one(int choice, Process input[],
                    int n, int quantum)
{
    Process p[MAX_PROCESSES];
    Event events[MAX_EVENTS];
    int count = 0;
    int total_time = 0;
    int i;

    memcpy(p, input, sizeof(Process) * (size_t)n);

    switch (choice) {
        case 1:
            fcfs(p, n, events, &count);
            break;

        case 2:
            sjf(p, n, events, &count);
            break;

        case 3:
            priority_nonpreemptive(p, n, events, &count);
            break;

        case 4:
            srtf(p, n, events, &count);
            break;

        case 5:
            priority_preemptive(p, n, events, &count);
            break;

        case 6:
            round_robin(p, n, quantum, events, &count);
            break;

        default:
            return;
    }

    if (count == 0) {
        printf("\nScheduling failed. Check the process inputs.\n");
        return;
    }

    display_gantt_chart(events, count);
    display_results(p, n);
    calculate_averages(p, n);

    for (i = 0; i < count; i++) {
        if (events[i].end_time > total_time)
            total_time = events[i].end_time;
    }

    calculate_cpu_utilization(events, count, total_time);
}

int main(void)
{
    Process input[MAX_PROCESSES];
    int n, quantum, i, choice = -1;

    printf("========== CPU SCHEDULING SIMULATOR ==========\n");

    if (!read_int("Enter number of processes (1-100): ", &n) ||
        n < 1 || n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return 1;
    }

    for (i = 0; i < n; i++) {
        int j;

        printf("\nProcess %d\n", i + 1);

        if (!read_int("PID: ", &input[i].pid) ||
            !read_int("Arrival time: ", &input[i].arrival_time) ||
            !read_int("Burst time: ", &input[i].burst_time) ||
            !read_int("Priority: ", &input[i].priority)) {
            printf("Invalid input. Restart the program.\n");
            return 1;
        }

        if (input[i].arrival_time < 0 ||
            input[i].burst_time <= 0) {
            printf("Arrival time must be >= 0 and burst time > 0.\n");
            return 1;
        }

        for (j = 0; j < i; j++) {
            if (input[j].pid == input[i].pid) {
                printf("Each PID must be unique.\n");
                return 1;
            }
        }
    }

    if (!read_int(
            "\nEnter Round Robin time quantum (> 0): ",
            &quantum) || quantum <= 0) {
        printf("Time quantum must be greater than zero.\n");
        return 1;
    }

    while (choice != 0) {
        printf("\nChoose Algorithm:\n");
        printf("1. FCFS\n");
        printf("2. SJF (Non-Preemptive)\n");
        printf("3. Priority (Non-Preemptive)\n");
        printf("4. SRTF (Preemptive SJF)\n");
        printf("5. Priority (Preemptive)\n");
        printf("6. Round Robin\n");
        printf("7. Compare All Algorithms\n");
        printf("0. Exit\n");

        if (!read_int("Enter choice (0-7): ", &choice)) {
            printf("Invalid menu input.\n");
            choice = -1;
            continue;
        }

        if (choice >= 1 && choice <= 6) {
            run_one(choice, input, n, quantum);
        } else if (choice == 7) {
            compare_algorithms(input, n, quantum);
        } else if (choice != 0) {
            printf("Choose a number from 0 to 7.\n");
        }
    }

    printf("Exiting simulator.\n");
    return 0;
}
