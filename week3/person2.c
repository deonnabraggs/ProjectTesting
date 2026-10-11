#include <limits.h>
#include "scheduler.h"

static int valid_args(Process p[], int n,
                      Event events[], int *event_count)
{
    int i;

    if (event_count == 0)
        return 0;

    *event_count = 0;

    if (p == 0 || events == 0 ||
        n <= 0 || n > MAX_PROCESSES)
        return 0;

    for (i = 0; i < n; i++) {
        if (p[i].arrival_time < 0 || p[i].burst_time <= 0)
            return 0;

        p[i].waiting_time = 0;
        p[i].turnaround_time = 0;
        p[i].response_time = -1;
        p[i].completion_time = 0;
    }

    return 1;
}

static void record_event(Event events[], int *count,
                         int pid, int start, int end)
{
    if (events == 0 || count == 0 || end <= start)
        return;

    if (*count > 0 &&
        events[*count - 1].pid == pid &&
        events[*count - 1].end_time == start) {
        events[*count - 1].end_time = end;
    } else if (*count < MAX_EVENTS) {
        events[*count].pid = pid;
        events[*count].start_time = start;
        events[*count].end_time = end;
        (*count)++;
    }
}

static void finish_process(Process p[], int i, int time)
{
    p[i].completion_time = time;

    p[i].turnaround_time =
        time - p[i].arrival_time;

    p[i].waiting_time =
        p[i].turnaround_time - p[i].burst_time;
}

/* Shortest Remaining Time First */
void srtf(Process p[], int n,
          Event events[], int *event_count)
{
    int rem[MAX_PROCESSES];
    int i, done = 0, time = 0, running = -1;

    if (!valid_args(p, n, events, event_count))
        return;

    for (i = 0; i < n; i++)
        rem[i] = p[i].burst_time;

    while (done < n) {
        int best = -1;

        for (i = 0; i < n; i++) {
            if (rem[i] <= 0 || p[i].arrival_time > time)
                continue;

            if (best == -1 ||
                rem[i] < rem[best] ||
                (rem[i] == rem[best] && i == running)) {
                best = i;
            }
        }

        if (best == -1) {
            int next = INT_MAX;

            for (i = 0; i < n; i++) {
                if (rem[i] > 0 &&
                    p[i].arrival_time < next)
                    next = p[i].arrival_time;
            }

            if (next == INT_MAX)
                break;

            if (next > time) {
                record_event(events, event_count,
                             -1, time, next);
                time = next;
            }

            running = -1;
            continue;
        }

        if (p[best].response_time == -1)
            p[best].response_time =
                time - p[best].arrival_time;

        record_event(events, event_count,
                     p[best].pid, time, time + 1);

        rem[best]--;
        time++;
        running = best;

        if (rem[best] == 0) {
            finish_process(p, best, time);
            done++;
            running = -1;
        }
    }
}

/* Preemptive Priority Scheduling */
void priority_preemptive(
    Process p[], int n, Event events[], int *event_count)
{
    int rem[MAX_PROCESSES];
    int i, done = 0, time = 0, running = -1;

    if (!valid_args(p, n, events, event_count))
        return;

    for (i = 0; i < n; i++)
        rem[i] = p[i].burst_time;

    while (done < n) {
        int best = -1;

        for (i = 0; i < n; i++) {
            if (rem[i] <= 0 || p[i].arrival_time > time)
                continue;

            if (best == -1 ||
                p[i].priority < p[best].priority ||
                (p[i].priority == p[best].priority &&
                 i == running)) {
                best = i;
            }
        }

        if (best == -1) {
            int next = INT_MAX;

            for (i = 0; i < n; i++) {
                if (rem[i] > 0 &&
                    p[i].arrival_time < next)
                    next = p[i].arrival_time;
            }

            if (next == INT_MAX)
                break;

            if (next > time) {
                record_event(events, event_count,
                             -1, time, next);
                time = next;
            }

            running = -1;
            continue;
        }

        if (p[best].response_time == -1)
            p[best].response_time =
                time - p[best].arrival_time;

        record_event(events, event_count,
                     p[best].pid, time, time + 1);

        rem[best]--;
        time++;
        running = best;

        if (rem[best] == 0) {
            finish_process(p, best, time);
            done++;
            running = -1;
        }
    }
}

/* Round Robin Scheduling */
void round_robin(Process p[], int n, int quantum,
                 Event events[], int *event_count)
{
    int rem[MAX_PROCESSES];
    int queued[MAX_PROCESSES] = {0};
    int queue[MAX_PROCESSES + 1];

    int head = 0, tail = 0, qcount = 0;
    int i, done = 0, time = 0;

    if (!valid_args(p, n, events, event_count))
        return;

    if (quantum <= 0)
        return;

    for (i = 0; i < n; i++)
        rem[i] = p[i].burst_time;

    while (done < n) {
        /* Add processes that have already arrived. */
        for (i = 0; i < n; i++) {
            if (rem[i] > 0 && !queued[i] &&
                p[i].arrival_time <= time) {
                queue[tail] = i;
                tail = (tail + 1) % (MAX_PROCESSES + 1);
                qcount++;
                queued[i] = 1;
            }
        }

        if (qcount == 0) {
            int next = INT_MAX;

            for (i = 0; i < n; i++) {
                if (rem[i] > 0 &&
                    p[i].arrival_time < next)
                    next = p[i].arrival_time;
            }

            if (next == INT_MAX)
                break;

            if (next > time) {
                record_event(events, event_count,
                             -1, time, next);
                time = next;
            }

            continue;
        }

        i = queue[head];
        head = (head + 1) % (MAX_PROCESSES + 1);
        qcount--;
        queued[i] = 0;

        if (p[i].response_time == -1)
            p[i].response_time =
                time - p[i].arrival_time;

        {
            int slice = rem[i] < quantum ?
                        rem[i] : quantum;
            int start = time;
            int end = time + slice;
            int t, j;

            record_event(events, event_count,
                         p[i].pid, start, end);

            rem[i] -= slice;
            time = end;

            /* Enqueue arrivals in chronological order. */
            for (t = start + 1; t <= time; t++) {
                for (j = 0; j < n; j++) {
                    if (rem[j] > 0 &&
                        !queued[j] &&
                        j != i &&
                        p[j].arrival_time == t) {
                        queue[tail] = j;
                        tail = (tail + 1) %
                               (MAX_PROCESSES + 1);
                        qcount++;
                        queued[j] = 1;
                    }
                }
            }

            if (rem[i] == 0) {
                finish_process(p, i, time);
                done++;
            } else {
                /* An unfinished process goes to the queue's end. */
                queue[tail] = i;
                tail = (tail + 1) % (MAX_PROCESSES + 1);
                qcount++;
                queued[i] = 1;
            }
        }
    }
}
