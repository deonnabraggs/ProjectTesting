#include "page_replacement.h"
#include <string.h>

void run_fifo(
    const int pages[],
    int count,
    int frame_count,
    SimulationResult *result
)
{
    int frames[MAX_FRAMES];
    int next = 0;

    memset(result, 0, sizeof(*result));

    for (int j = 0; j < MAX_FRAMES; j++)
        frames[j] = -1;

    for (int i = 0; i < count; i++) {
        int hit = 0;
        int empty = -1;

        for (int j = 0; j < frame_count; j++) {
            if (frames[j] == pages[i]) {
                hit = 1;
                break;
            }

            if (frames[j] == -1 && empty == -1)
                empty = j;
        }

        if (hit) {
            result->steps[i].status = STATUS_HIT;
            result->total_hits++;
        } else {
            result->steps[i].status = STATUS_FAULT;
            result->total_faults++;

            if (empty != -1)
                frames[empty] = pages[i];
            else {
                frames[next] = pages[i];
                next = (next + 1) % frame_count;
            }
        }

        for (int j = 0; j < frame_count; j++)
            result->steps[i].frames[j] = frames[j];
    }
}
