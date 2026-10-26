#include "replacement.h"
#include <string.h>

void run_optimal(const int pages[], int count, int frame_count,
                 SimulationResult *result)
{
    int frames[MAX_FRAMES];
    int i, j;

    memset(result, 0, sizeof(*result));
    strcpy(result->algorithm, "Optimal");
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
                int victim = -1;
                int farthest = -1;

                for (j = 0; j < frame_count; j++) {
                    int k;
                    int next_use = -1;

                    for (k = i + 1; k < count; k++) {
                        if (pages[k] == frames[j]) {
                            next_use = k;
                            break;
                        }
                    }

                    if (next_use == -1) {
                        victim = j;
                        break;
                    }

                    if (next_use > farthest) {
                        farthest = next_use;
                        victim = j;
                    }
                }

                frames[victim] = pages[i];
            }
        }

        result->steps[i].page = pages[i];

        for (j = 0; j < frame_count; j++)
            result->steps[i].frames[j] = frames[j];
    }
}
