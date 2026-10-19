#include "deadlock.h"
#include <stdio.h>

static int read_integer(const char *prompt, int min, int max, int *value)
{
    char line[256];
    int x;
    char extra;

    while (1)
    {
        printf("%s", prompt);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            return 0;

        if (sscanf(line, " %d %c", &x, &extra) == 1 &&
            x >= min && x <= max)
        {
            *value = x;
            return 1;
        }

        printf("Invalid input. Enter a value from %d to %d.\n",
               min, max);
    }
}

static int read_row(const char *label, int values[], int count)
{
    char line[512];
    int temp[MAX_RESOURCES];
    int used = 0;
    char *p;

    while (1)
    {
        printf("%s", label);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            return 0;

        p = line;
        used = 0;

        while (used < count)
        {
            int value;
            int consumed = 0;

            while (*p == ' ' || *p == '\t')
                p++;

            if (*p == '\n' || *p == '\0')
                break;

            if (sscanf(p, "%d%n", &value, &consumed) != 1)
                break;

            if (value < 0 || consumed == 0)
                break;

            temp[used++] = value;
            p += consumed;
        }

        while (*p == ' ' || *p == '\t' || *p == '\n')
            p++;

        if (used == count && *p == '\0')
        {
            for (int j = 0; j < count; j++)
                values[j] = temp[j];

            return 1;
        }

        printf("Enter exactly %d non-negative integers.\n", count);
    }
}

int terminal_input(void)
{
    int count;
    int resources[MAX_RESOURCES];

    printf("\n========================================\n");
    printf("     DEADLOCK SIMULATION SYSTEM\n");
    printf("        TERMINAL DATA INPUT\n");
    printf("========================================\n\n");

    if (!read_integer("Number of processes (1-10): ",
                      1, MAX_PROCESSES, &count))
        return 0;

    create_processes(count);

    if (!read_integer("Number of resource types (1-10): ",
                      1, MAX_RESOURCES, &count))
        return 0;

    m = count;

    if (!read_row("Total resources (space separated): ",
                  resources, m))
        return 0;

    create_resources(m, resources);

    printf("\nEnter Allocation Matrix rows.\n");
    printf("Enter %d values per row, in resource order R0, R1, ...\n\n",
           m);

    for (int i = 0; i < n; i++)
    {
        char prompt[80];
        int row[MAX_RESOURCES];

        snprintf(prompt, sizeof(prompt), "Allocation P%d: ", i);

        if (!read_row(prompt, row, m))
            return 0;

        for (int j = 0; j < m; j++)
        {
            if (row[j] > total[j])
            {
                printf("Invalid row: allocation exceeds total R%d.\n", j);
                i--;
                break;
            }

            allocation[i][j] = row[j];
        }

        if (i >= 0)
        {
            int valid = 1;

            for (int j = 0; j < m; j++)
            {
                int used = 0;

                for (int k = 0; k <= i; k++)
                    used += allocation[k][j];

                if (used > total[j])
                    valid = 0;
            }

            if (!valid)
            {
                printf("Total allocation exceeds available resources. "
                       "Re-enter P%d.\n", i);
                for (int j = 0; j < m; j++)
                    allocation[i][j] = 0;
                i--;
            }
        }
    }

    printf("\nEnter Maximum Matrix rows.\n\n");

    for (int i = 0; i < n; i++)
    {
        char prompt[80];
        int row[MAX_RESOURCES];
        int valid;

        do
        {
            snprintf(prompt, sizeof(prompt), "Maximum P%d: ", i);

            if (!read_row(prompt, row, m))
                return 0;

            valid = 1;

            for (int j = 0; j < m; j++)
            {
                if (row[j] < allocation[i][j] ||
                    row[j] > total[j])
                {
                    printf("Maximum for R%d must be between "
                           "allocation (%d) and total (%d).\n",
                           j, allocation[i][j], total[j]);
                    valid = 0;
                }
            }
        } while (!valid);

        for (int j = 0; j < m; j++)
            maximum[i][j] = row[j];
    }

    recalculate_state();

    printf("\nInitial data loaded successfully.\n");
    printf("Opening the GTK simulation window...\n\n");

    return 1;
}
