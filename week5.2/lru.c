#include "page_replacement.h"
#include <string.h>

void run_lru(
    const int pages[],
    int count,
    int frame_count,
    SimulationResult *result
)
{
    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES];

    memset(result, 0, sizeof(*result));

    for (int j = 0; j < MAX_FRAMES; j++) {
        frames[j] = -1;
        last_used[j] = -1;
    }

    for (int i = 0; i < count; i++) {
        int hit = 0;
        int empty = -1;
        int position = -1;

        for (int j = 0; j < frame_count; j++) {
            if (frames[j] == pages[i]) {
                hit = 1;
                position = j;
                break;
            }

            if (frames[j] == -1 && empty == -1)
                empty = j;
        }

        if (hit) {
            result->steps[i].status = STATUS_HIT;
            result->total_hits++;
            last_used[position] = i;
        } else {
            result->steps[i].status = STATUS_FAULT;
            result->total_faults++;

            if (empty != -1) {
                position = empty;
            } else {
                position = 0;

                for (int j = 1; j < frame_count; j++) {
                    if (last_used[j] < last_used[position])
                        position = j;
                }
            }

            frames[position] = pages[i];
            last_used[position] = i;
        }

        for (int j = 0; j < frame_count; j++)
            result->steps[i].frames[j] = frames[j];
    }
}
