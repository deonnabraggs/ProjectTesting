#include "deadlock.h"

int is_safe(int sequence[])
{
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int count = 0;

    for (int j = 0; j < m; j++)
        work[j] = available[j];

    for (int i = 0; i < n; i++)
        sequence[i] = -1;

    while (count < n)
    {
        int found = 0;

        for (int i = 0; i < n; i++)
        {
            if (!active[i] || finish[i])
                continue;

            int possible = 1;

            for (int j = 0; j < m; j++)
            {
                if (need[i][j] > work[j])
                {
                    possible = 0;
                    break;
                }
            }

            if (possible)
            {
                for (int j = 0; j < m; j++)
                    work[j] += allocation[i][j];

                sequence[count++] = i;
                finish[i] = 1;
                found = 1;
            }
        }

        if (!found)
            break;
    }

    for (int i = 0; i < n; i++)
    {
        if (active[i] && !finish[i])
            return 0;
    }

    return 1;
}

int detect_deadlock(int deadlocked[])
{
    int work[MAX_RESOURCES];
    int finish[MAX_PROCESSES] = {0};
    int count = 0;

    for (int j = 0; j < m; j++)
        work[j] = available[j];

    for (int i = 0; i < n; i++)
    {
        deadlocked[i] = 0;

        if (!active[i])
        {
            finish[i] = 1;
            continue;
        }

        int has_allocation = 0;

        for (int j = 0; j < m; j++)
        {
            if (allocation[i][j] > 0)
                has_allocation = 1;
        }

        if (!has_allocation)
            finish[i] = 1;
    }

    int changed;

    do
    {
        changed = 0;

        for (int i = 0; i < n; i++)
        {
            if (finish[i])
                continue;

            int possible = 1;

            for (int j = 0; j < m; j++)
            {
                if (request[i][j] > work[j])
                {
                    possible = 0;
                    break;
                }
            }

            if (possible)
            {
                for (int j = 0; j < m; j++)
                    work[j] += allocation[i][j];

                finish[i] = 1;
                changed = 1;
            }
        }
    } while (changed);

    for (int i = 0; i < n; i++)
    {
        if (active[i] && !finish[i])
        {
            deadlocked[i] = 1;
            count++;
        }
    }

    return count;
}
