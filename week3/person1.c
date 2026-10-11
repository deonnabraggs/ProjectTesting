#include <stdio.h>
#include "scheduler.h"

static void add_event(Event events[], int *count,
                      int pid, int start, int end)
{
    if (events == NULL || count == NULL || start >= end)
        return;

    if (*count > 0 &&
        events[*count - 1].pid == pid &&
        events[*count - 1].end_time == start) {
        events[*count - 1].end_time = end;
        return;
    }

    if (*count < MAX_EVENTS) {
        events[*count].pid = pid;
        events[*count].start_time = start;
        events[*count].end_time = end;
        (*count)++;
    }
}

static int select_process(Process p[], int n, int time,
                          int completed[], int mode)
{
    int i, best = -1;

    for (i = 0; i < n; i++) {
        if (completed[i] || p[i].arrival_time > time)
            continue;

        if (best == -1) {
            best = i;
        } else if (mode == 0) {
            /* FCFS: earliest arrival */
            if (p[i].arrival_time < p[best].arrival_time)
                best = i;
        } else if (mode == 1) {
            /* SJF: shortest burst */
            if (p[i].burst_time < p[best].burst_time)
                best = i;
        } else {
            /* Priority: smaller number means higher priority */
            if (p[i].priority < p[best].priority)
                best = i;
        }
    }

    return best;
}

static void run_nonpreemptive(Process p[], int n,
                              Event events[], int *event_count,
                              int mode)
{
    int completed[MAX_PROCESSES] = {0};
    int time = 0, finished = 0, i;

    if (event_count == NULL)
        return;

    *event_count = 0;

    if (p == NULL || events == NULL ||
        n <= 0 || n > MAX_PROCESSES)
        return;

    for (i = 0; i < n; i++) {
        p[i].waiting_time = 0;
        p[i].turnaround_time = 0;
        p[i].response_time = 0;
        p[i].completion_time = 0;
    }

    while (finished < n) {
        int selected = select_process(
            p, n, time, completed, mode);

        if (selected == -1) {
            int next_arrival = -1;

            for (i = 0; i < n; i++) {
                if (!completed[i] &&
                    (next_arrival == -1 ||
                     p[i].arrival_time < next_arrival)) {
                    next_arrival = p[i].arrival_time;
                }
            }

            if (next_arrival > time) {
                add_event(events, event_count, -1,
                          time, next_arrival);
                time = next_arrival;
            }

            continue;
        }

        {
            int start = time;
            int end = start + p[selected].burst_time;

            add_event(events, event_count, p[selected].pid,
                      start, end);

            p[selected].completion_time = end;
            p[selected].turnaround_time =
                end - p[selected].arrival_time;
            p[selected].waiting_time =
                p[selected].turnaround_time -
                p[selected].burst_time;
            p[selected].response_time =
                start - p[selected].arrival_time;

            completed[selected] = 1;
            finished++;
            time = end;
        }
    }
}

void fcfs(Process p[], int n, Event events[], int *event_count)
{
    run_nonpreemptive(p, n, events, event_count, 0);
}

void sjf(Process p[], int n, Event events[], int *event_count)
{
    run_nonpreemptive(p, n, events, event_count, 1);
}

void priority_nonpreemptive(
    Process p[], int n, Event events[], int *event_count)
{
    run_nonpreemptive(p, n, events, event_count, 2);
}
