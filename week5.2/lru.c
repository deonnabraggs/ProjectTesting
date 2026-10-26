#include "replacement.h"
#include <string.h>

void run_lru(const int pages[], int count, int frame_count,
             SimulationResult *result)
{
    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES];
    int clock = 0;
    int i, j;

    memset(result, 0, sizeof(*result));
    strcpy(result->algorithm, "LRU");
    result->page_count = count;
    result->frame_count = frame_count;

    for (j = 0; j < frame_count; j++) {
        frames[j] = -1;
        last_used[j] = -1;
    }

    for (i = 0; i < count; i++) {
        int hit_index = -1;
        int empty = -1;

        for (j = 0; j < frame_count; j++) {
            if (frames[j] == pages[i])
                hit_index = j;

            if (frames[j] == -1 && empty == -1)
                empty = j;
        }

        if (hit_index != -1) {
            result->hits++;
            result->steps[i].status = STATUS_HIT;
            last_used[hit_index] = clock++;
        } else {
            int victim = empty;

            result->faults++;
            result->steps[i].status = STATUS_FAULT;

            if (victim == -1) {
                victim = 0;

                for (j = 1; j < frame_count; j++) {
                    if (last_used[j] < last_used[victim])
                        victim = j;
                }
            }

            frames[victim] = pages[i];
            last_used[victim] = clock++;
        }

        result->steps[i].page = pages[i];

        for (j = 0; j < frame_count; j++)
            result->steps[i].frames[j] = frames[j];
    }
}
