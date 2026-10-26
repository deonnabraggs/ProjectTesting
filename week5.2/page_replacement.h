#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

#include <gtk/gtk.h>

/* Maximum allowed page requests and frames */
#define MAX_REQUESTS 100
#define MAX_FRAMES 10

/* Page request status */
#define PAGE_HIT 1
#define PAGE_FAULT 0

/* Store the results of one page replacement algorithm */
typedef struct {
    int frames[MAX_REQUESTS][MAX_FRAMES];
    int status[MAX_REQUESTS];

    int total_hits;
    int total_faults;
} SimulationResult;

/* Page replacement algorithm functions */
void fifo(
    const int pages[],
    int n,
    int frame_count,
    SimulationResult *result
);

void lru(
    const int pages[],
    int n,
    int frame_count,
    SimulationResult *result
);

void optimal(
    const int pages[],
    int n,
    int frame_count,
    SimulationResult *result
);

/* GUI function */
GtkWidget *create_main_window(void);

#endif
