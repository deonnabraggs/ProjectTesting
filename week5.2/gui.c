#define MAX_PAGES 100
#define MAX_FRAMES 20

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

void run_fifo(const int pages[], int count, int frame_count,
              SimulationResult *result);

void run_lru(const int pages[], int count, int frame_count,
             SimulationResult *result);

void run_optimal(const int pages[], int count, int frame_count,
                 SimulationResult *result);

GtkWidget *create_main_window(void);
