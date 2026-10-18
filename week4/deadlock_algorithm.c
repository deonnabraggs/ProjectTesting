#include "deadlock.h"

/*
 * Banker's Safety Algorithm
 * Returns 1 if the system is safe, otherwise returns 0.
 */
int is_safe(int sequence[])
{
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int count = 0;
    int i, j;
    int found;

    /* Initialize the output sequence. */
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        sequence[i] = -1;
    }

    /* Copy available resources into work[]. */
    for (j = 0; j < m; j++)
    {
        work[j] = available[j];
    }

    /* Inactive processes do not need to finish. */
    for (i = 0; i < n; i++)
    {
        if (!active[i])
        {
            finish[i] = 1;
        }
    }

    /* Find a process whose remaining need can be satisfied. */
    do
    {
        found = 0;

        for (i = 0; i < n; i++)
        {
            if (finish[i])
            {
                continue;
            }

            int can_finish = 1;

            for (j = 0; j < m; j++)
            {
                if (need[i][j] > work[j])
                {
                    can_finish = 0;
                    break;
                }
            }

            if (can_finish)
            {
                /* Release its allocated resources after finishing. */
                for (j = 0; j < m; j++)
                {
                    work[j] += allocation[i][j];
                }

                finish[i] = 1;
                sequence[count++] = i;
                found = 1;
            }
        }
    } while (found);

    /* Check whether every active process can finish. */
    for (i = 0; i < n; i++)
    {
        if (!finish[i])
        {
            sequence[0] = -1;
            return 0;
        }
    }

    sequence[count] = -1;
    return 1;
}

/*
 * Standard Deadlock Detection Algorithm
 * Returns the number of deadlocked processes.
 */
int detect_deadlock(int deadlocked[])
{
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int count = 0;
    int i, j;
    int found;

    /* Initialize the output list. */
    for (i = 0; i < MAX_PROCESSES; i++)
    {
        deadlocked[i] = -1;
    }

    /* Start with currently available resources. */
    for (j = 0; j < m; j++)
    {
        work[j] = available[j];
    }

    /*
     * A process holding no resources can be considered
     * finished for deadlock detection.
     * Inactive processes are also ignored.
     */
    for (i = 0; i < n; i++)
    {
        if (!active[i])
        {
            finish[i] = 1;
            continue;
        }

        finish[i] = 1;

        for (j = 0; j < m; j++)
        {
            if (allocation[i][j] != 0)
            {
                finish[i] = 0;
                break;
            }
        }
    }

    /* Try to satisfy outstanding requests. */
    do
    {
        found = 0;

        for (i = 0; i < n; i++)
        {
            if (finish[i])
            {
                continue;
            }

            int can_finish = 1;

            for (j = 0; j < m; j++)
            {
                if (request[i][j] > work[j])
                {
                    can_finish = 0;
                    break;
                }
            }

            if (can_finish)
            {
                /* Assume this process finishes and releases resources. */
                for (j = 0; j < m; j++)
                {
                    work[j] += allocation[i][j];
                }

                finish[i] = 1;
                found = 1;
            }
        }
    } while (found);

    /* Any unfinished active process is deadlocked. */
    for (i = 0; i < n; i++)
    {
        if (active[i] && !finish[i])
        {
            deadlocked[count++] = i;
        }
    }

    deadlocked[count] = -1;
    return count;
}
