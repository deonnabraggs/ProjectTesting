#include "deadlock.h"

int request_resources(int pid, const int req[])
{
    int sequence[MAX_PROCESSES];

    if (req == NULL || pid < 0 || pid >= n ||
        !active[pid] || m < 1 || m > MAX_RESOURCES)
    {
        return REQ_INVALID;
    }

    /* A request cannot exceed the process's remaining need. */
    for (int j = 0; j < m; j++)
    {
        if (req[j] < 0)
            return REQ_INVALID;

        if (req[j] > need[pid][j])
            return REQ_EXCEEDS_NEED;
    }

    /* If resources are unavailable, keep the request pending. */
    for (int j = 0; j < m; j++)
    {
        if (req[j] > available[j])
        {
            for (int k = 0; k < m; k++)
                request[pid][k] = req[k];

            return REQ_MUST_WAIT;
        }
    }

    /* Temporarily grant the request. */
    for (int j = 0; j < m; j++)
    {
        available[j] -= req[j];
        allocation[pid][j] += req[j];
        need[pid][j] -= req[j];
        request[pid][j] = 0;
    }

    /* Grant only if the resulting state is safe. */
    if (is_safe(sequence))
        return REQ_GRANTED;

    /* Unsafe: undo the temporary allocation. */
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

    /*
     * Terminate the process and clear its resource data.
     * Recalculation returns its held resources to available.
     */
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
