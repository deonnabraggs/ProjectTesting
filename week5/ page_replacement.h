#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

/*
 * PAGE REPLACEMENT ALGORITHMS SIMULATOR
 * Shared interface for the whole team.
 *
 * Language: C
 * GUI: GTK 3 (team leader's files)
 *
 * Only main.c may contain the final program's main() function.
 * Algorithm files must not contain main() or GUI code.
 */

#define MAX_REQUESTS 100
#define MAX_FRAMES 20

#define PAGE_HIT 1
#define PAGE_FAULT 0
#define EMPTY_FRAME -1

typedef struct {
    /*
     * frames[step][frame_index] stores every frame's contents AFTER
     * pages[step] has been processed.
     * Use EMPTY_FRAME (-1) for an empty frame.
     */
    int frames[MAX_REQUESTS][MAX_FRAMES];

    /* PAGE_HIT or PAGE_FAULT for each request. */
    int status[MAX_REQUESTS];

    int total_hits;
    int total_faults;
} SimulationResult;

/*
 * CONTRACT FOR ALL ALGORITHMS
 *
 * - pages must not be NULL.
 * - result must not be NULL.
 * - n must be between 1 and MAX_REQUESTS inclusive.
 * - frame_count must be between 1 and MAX_FRAMES inclusive.
 * - Page numbers must be non-negative; -1 is reserved for EMPTY_FRAME.
 * - Check all arguments before writing to result. If any argument is
 *   invalid, return immediately without accessing invalid memory.
 * - Initialize the result fields used by the simulation.
 * - Start every frame empty.
 * - Process pages[0] through pages[n - 1] in order.
 * - A hit does not replace a page.
 * - On a fault, fill the lowest-index empty frame first.
 * - If memory is full, put the incoming page in the SAME frame slot
 *   as the page being replaced. Never shift/reorder frame slots.
 * - Save all frame contents after every request.
 * - Set status for every request and calculate both totals.
 *
 * Deterministic tie-break for Optimal:
 * If multiple resident pages will never be requested again, replace
 * the page in the lowest-index frame among those pages.
 *
 * Function signatures are fixed. Do not change them without approval
 * from the team leader.
 */
void fifo(const int pages[], int n, int frame_count,
          SimulationResult *result);

void lru(const int pages[], int n, int frame_count,
         SimulationResult *result);

void optimal(const int pages[], int n, int frame_count,
             SimulationResult *result);

#endif /* PAGE_REPLACEMENT_H */
