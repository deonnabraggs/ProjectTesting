#include "replacement.h"
#include <string.h>

void run_fifo(const int pages[], int count, int frame_count,
              SimulationResult *result)
{
    int frames[MAX_FRAMES];
    int next = 0;
    int i, j;

    memset(result, 0, sizeof(*result));
    strcpy(result->algorithm, "FIFO");
    result->page_count = count;
    result->frame_count = frame_count;

    for (j = 0; j < frame_count; j++)
        frames[j] = -1;

    for (i = 0; i < count; i++) {
        int hit = 0;
        int empty = -1;

        for (j = 0; j < frame_count; j++) {
            if (frames[j] == pages[i]) {
                hit = 1;
                break;
            }

            if (frames[j] == -1 && empty == -1)
                empty = j;
        }

        if (hit) {
            result->hits++;
            result->steps[i].status = STATUS_HIT;
        } else {
            result->faults++;
            result->steps[i].status = STATUS_FAULT;

            if (empty != -1) {
                frames[empty] = pages[i];
            } else {
                frames[next] = pages[i];
                next = (next + 1) % frame_count;
            }
        }

        result->steps[i].page = pages[i];

        for (j = 0; j < frame_count; j++)
            result->steps[i].frames[j] = frames[j];
    }
}
