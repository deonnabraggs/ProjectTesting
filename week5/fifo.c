#include "page_replacement.h"
#include <stddef.h>

void fifo(const int pages[], int n, int frame_count,
          SimulationResult *result)
{
    int frames[MAX_FRAMES];
    int next_to_replace = 0;
    int i, j;

    if (pages == NULL || result == NULL ||
        n < 1 || n > MAX_REQUESTS ||
        frame_count < 1 || frame_count > MAX_FRAMES) {
        return;
    }

    for (i = 0; i < frame_count; ++i) {
        frames[i] = EMPTY_FRAME;
    }

    result->total_hits = 0;
    result->total_faults = 0;

    for (i = 0; i < n; ++i) {
        int hit = 0;
        int empty_index = -1;

        for (j = 0; j < frame_count; ++j) {
            if (frames[j] == pages[i]) {
                hit = 1;
                break;
            }
            if (frames[j] == EMPTY_FRAME && empty_index == -1) {
                empty_index = j;
            }
        }

        if (hit) {
            result->status[i] = PAGE_HIT;
            result->total_hits++;
        } else {
            result->status[i] = PAGE_FAULT;
            result->total_faults++;

            if (empty_index != -1) {
                frames[empty_index] = pages[i];
                next_to_replace = (empty_index + 1) % frame_count;
            } else {
                frames[next_to_replace] = pages[i];
                next_to_replace = (next_to_replace + 1) % frame_count;
            }
        }

        for (j = 0; j < frame_count; ++j) {
            result->frames[i][j] = frames[j];
        }
    }
}
