#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

#include <gtk/gtk.h>
#include <stddef.h>

#define MAX_PAGES 500
#define MAX_REQUESTS MAX_PAGES
#define MAX_FRAMES 10

#define STATUS_HIT 1
#define STATUS_FAULT 0

typedef struct {
    int frames[MAX_FRAMES];
    int status;
} PageStep;

typedef struct {
    PageStep steps[MAX_PAGES];
    int total_hits;
    int total_faults;
} SimulationResult;

void run_fifo(
    const int pages[],
    int count,
    int frame_count,
    SimulationResult *result
);

void run_lru(
    const int pages[],
    int count,
    int frame_count,
    SimulationResult *result
);

void run_optimal(
    const int pages[],
    int count,
    int frame_count,
    SimulationResult *result
);

int parse_page_string(
    const char *input,
    int pages[],
    int *count,
    char *error,
    size_t error_size
);

GtkWidget *create_main_window(void);

#endif
