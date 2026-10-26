#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include <stddef.h>

#define MAX_PAGES 500
#define MAX_FRAMES 20

typedef enum {
    STATUS_FAULT = 0,
    STATUS_HIT = 1
} PageStatus;

typedef struct {
    int page;
    int frames[MAX_FRAMES];
    PageStatus status;
} StepResult;

typedef struct {
    char algorithm[16];
    int page_count;
    int frame_count;
    int hits;
    int faults;
    StepResult steps[MAX_PAGES];
} SimulationResult;

int parse_page_string(
    const char *input,
    int pages[],
    int *count,
    char *error,
    size_t error_size
);

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

#endif
