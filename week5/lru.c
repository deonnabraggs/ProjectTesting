#include <stddef.h>
#include "page_replacement.h"

void lru(const int pages[], int n, int frame_count,
         SimulationResult *result)
{
    int frame[MAX_FRAMES];
    int last_used[MAX_FRAMES];
    int i, j;

    /* Validate everything before modifying result. */
    if (pages == NULL || result == NULL ||
        n < 1 || n > MAX_REQUESTS ||
        frame_count < 1 || frame_count > MAX_FRAMES) {
        return;
    }
    for (i = 0; i < n; ++i) {
        if (pages[i] < 0) {
            return;
        }
    }

    result->total_hits = 0;
    result->total_faults = 0;

    for (j = 0; j < frame_count; ++j) {
        frame[j] = EMPTY_FRAME;
        last_used[j] = -1;
    }

    for (i = 0; i < n; ++i) {
        int hit_frame = -1;
        int empty_frame = -1;

        /* Find a hit and the first (lowest-index) empty frame. */
        for (j = 0; j < frame_count; ++j) {
            if (frame[j] == pages[i]) {
                hit_frame = j;
                break;
            }
            if (frame[j] == EMPTY_FRAME && empty_frame == -1) {
                empty_frame = j;
            }
        }

        if (hit_frame != -1) {
            result->status[i] = PAGE_HIT;
            ++result->total_hits;
            last_used[hit_frame] = i;
        } else {
            int target_frame;

            result->status[i] = PAGE_FAULT;
            ++result->total_faults;

            if (empty_frame != -1) {
                target_frame = empty_frame;
            } else {
                /* Least-recently-used resident page; ties keep lowest index. */
                target_frame = 0;
                for (j = 1; j < frame_count; ++j) {
                    if (last_used[j] < last_used[target_frame]) {
                        target_frame = j;
                    }
                }
            }

            frame[target_frame] = pages[i];
            last_used[target_frame] = i;
        }

        /* Record the frame contents after this request. */
        for (j = 0; j < frame_count; ++j) {
            result->frames[i][j] = frame[j];
        }
    }
}
