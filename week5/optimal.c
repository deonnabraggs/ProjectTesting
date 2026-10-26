#include "page_replacement.h"

void optimal(const int pages[], int n, int frame_count,
             SimulationResult *result)
{
    int frames[MAX_FRAMES];
    int i, j, k;

    /* Check for invalid input */
    if (pages == NULL || result == NULL)
        return;

    if (n < 1 || n > MAX_REQUESTS ||
        frame_count < 1 || frame_count > MAX_FRAMES)
        return;

    /* Initialize frames as empty */
    for (i = 0; i < frame_count; i++)
    {
        frames[i] = EMPTY_FRAME;
    }

    result->total_hits = 0;
    result->total_faults = 0;

    /* Process each page request */
    for (i = 0; i < n; i++)
    {
        int hit = 0;
        int empty_frame = -1;

        /* Check whether the requested page is already present */
        for (j = 0; j < frame_count; j++)
        {
            if (frames[j] == pages[i])
            {
                hit = 1;
                break;
            }

            /* Remember the lowest-index empty frame */
            if (frames[j] == EMPTY_FRAME && empty_frame == -1)
            {
                empty_frame = j;
            }
        }

        if (hit)
        {
            result->status[i] = PAGE_HIT;
            result->total_hits++;
        }
        else
        {
            result->status[i] = PAGE_FAULT;
            result->total_faults++;

            /* Fill an empty frame before replacing any page */
            if (empty_frame != -1)
            {
                frames[empty_frame] = pages[i];
            }
            else
            {
                int replace_index = 0;
                int farthest_use = -1;

                /* Find the resident page used farthest in the future */
                for (j = 0; j < frame_count; j++)
                {
                    int next_use = -1;

                    /* Search only requests AFTER the current request */
                    for (k = i + 1; k < n; k++)
                    {
                        if (pages[k] == frames[j])
                        {
                            next_use = k;
                            break;
                        }
                    }

                    /*
                     * If next_use is -1, this page is never used again.
                     * Such a page is the best replacement candidate.
                     * Ties are resolved by keeping the lowest frame index.
                     */
                    if (next_use == -1)
                    {
                        replace_index = j;
                        break;
                    }

                    if (next_use > farthest_use)
                    {
                        farthest_use = next_use;
                        replace_index = j;
                    }
                }

                /* Replace the page in the same frame slot */
                frames[replace_index] = pages[i];
            }
        }

        /* Save frame contents AFTER processing this request */
        for (j = 0; j < frame_count; j++)
        {
            result->frames[i][j] = frames[j];
        }
    }
}
