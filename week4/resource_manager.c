#include "deadlock.h"

void create_processes(int count)
{
    if (count < 1 || count > MAX_PROCESSES)
        return;

    n = count;

    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        active[i] = (i < count);

        for (int j = 0; j < MAX_RESOURCES; j++)
        {
            allocation[i][j] = 0;
            maximum[i][j] = 0;
            need[i][j] = 0;
            request[i][j] = 0;
        }
    }

    recalculate_state();
}

void create_resources(int count, const int amounts[])
{
    if (count < 1 || count > MAX_RESOURCES || amounts == NULL)
        return;

    for (int j = 0; j < count; j++)
    {
        if (amounts[j] < 0)
            return;
    }

    m = count;

    for (int j = 0; j < MAX_RESOURCES; j++)
        total[j] = (j < count) ? amounts[j] : 0;

    /* A new resource setup starts with no allocations. */
    for (int i = 0; i < MAX_PROCESSES; i++)
    {
        for (int j = 0; j < MAX_RESOURCES; j++)
        {
            allocation[i][j] = 0;
            maximum[i][j] = 0;
            need[i][j] = 0;
            request[i][j] = 0;
        }
    }

    recalculate_state();
}

void set_allocation(int pid, int rid, int amount)
{
    if (pid < 0 || pid >= n ||
        rid < 0 || rid >= m ||
        amount < 0 || !active[pid])
    {
        return;
    }

    int used_by_others = total[rid] - available[rid]
                         - allocation[pid][rid];

    if (used_by_others + amount > total[rid])
        return;

    int old_allocation = allocation[pid][rid];
    int old_maximum = maximum[pid][rid];

    allocation[pid][rid] = amount;

    if (maximum[pid][rid] < amount)
        maximum[pid][rid] = amount;

    recalculate_state();

    if (available[rid] < 0)
    {
        allocation[pid][rid] = old_allocation;
        maximum[pid][rid] = old_maximum;
        recalculate_state();
    }
}

void set_maximum(int pid, int rid, int amount)
{
    if (pid < 0 || pid >= n ||
        rid < 0 || rid >= m ||
        !active[pid] ||
        amount < allocation[pid][rid] ||
        amount > total[rid])
    {
        return;
    }

    maximum[pid][rid] = amount;
    recalculate_state();
}

void recalculate_state(void)
{
    for (int j = 0; j < m; j++)
    {
        int used = 0;

        for (int i = 0; i < n; i++)
        {
            if (active[i])
                used += allocation[i][j];
        }

        available[j] = total[j] - used;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            need[i][j] = maximum[i][j] - allocation[i][j];

            if (need[i][j] < 0)
                need[i][j] = 0;
        }
    }
}

void load_sample_data(void)
{
    const int amounts[] = {10, 5, 7};

    const int alloc[5][3] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2}
    };

    const int max_need[5][3] = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };

    create_processes(5);
    create_resources(3, amounts);

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            allocation[i][j] = alloc[i][j];
            maximum[i][j] = max_need[i][j];
            request[i][j] = 0;
        }
    }

    recalculate_state();
}

double resource_utilization(int rid)
{
    if (rid < 0 || rid >= m || total[rid] <= 0)
        return 0.0;

    return 100.0 * (total[rid] - available[rid]) / total[rid];
}
