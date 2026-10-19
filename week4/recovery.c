#include "deadlock.h"

int request_resources(int pid, const int req[])
{
    int sequence[MAX_PROCESSES];

    if (req == NULL ||
        pid < 0 || pid >= n ||
        !active[pid])
    {
        return REQ_INVALID;
    }

    for (int j = 0; j < m; j++)
    {
        if (req[j] < 0)
            return REQ_INVALID;

        if (req[j] > need[pid][j])
            return REQ_EXCEEDS_NEED;
    }

    for (int j = 0; j < m; j++)
    {
        if (req[j] > available[j])
        {
            for (int k = 0; k < m; k++)
                request[pid][k] = req[k];

            return REQ_MUST_WAIT;
        }
    }

    for (int j = 0; j < m; j++)
    {
        available[j] -= req[j];
        allocation[pid][j] += req[j];
        need[pid][j] -= req[j];
        request[pid][j] = 0;
    }

    if (is_safe(sequence))
        return REQ_GRANTED;

    /* Roll back if the resulting state is unsafe. */
    for (int j = 0; j < m; j++)
    {
        available[j] += req[j];
        allocation[pid][j] -= req[j];
        need[pid][j] += req[j];
        request[pid][j] = 0;
    }

    return REQ_UNSAFE;
}

int recover_from_deadlock(int pid)
{
    if (pid < 0 || pid >= n || !active[pid])
        return REC_INVALID;

    active[pid] = 0;

    for (int j = 0; j < m; j++)
    {
        allocation[pid][j] = 0;
        maximum[pid][j] = 0;
        need[pid][j] = 0;
        request[pid][j] = 0;
    }

    recalculate_state();

    return REC_OK;
}
