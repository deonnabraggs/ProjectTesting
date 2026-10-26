#include <gtk/gtk.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "page_replacement.h"

/* Algorithm functions from the separate C files */
void fifo(const int pages[], int n, int frame_count,
          SimulationResult *result);

void lru(const int pages[], int n, int frame_count,
         SimulationResult *result);

void optimal(const int pages[], int n, int frame_count,
             SimulationResult *result);

/* GUI widgets */
static GtkWidget *page_entry;
static GtkWidget *frame_spin;
static GtkWidget *results_view;
static GtkWidget *status_label;

/* Display text in the results area */
static void show_text(const char *text)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(results_view));

    gtk_text_buffer_set_text(buffer, text, -1);
}

/* Parse page requests separated by spaces, commas or semicolons */
static int parse_pages(const char *input, int pages[], int *count)
{
    const char *p = input;
    int n = 0;

    if (input == NULL || pages == NULL || count == NULL)
        return 0;

    while (*p != '\0')
    {
        char *end;
        long value;

        /* Skip separators */
        while (*p == ' ' || *p == '\t' || *p == '\n' ||
               *p == ',' || *p == ';')
        {
            p++;
        }

        if (*p == '\0')
            break;

        errno = 0;
        value = strtol(p, &end, 10);

        if (p == end || errno == ERANGE ||
            value < 0 || value > INT_MAX)
        {
            return 0;
        }

        if (n >= MAX_REQUESTS)
            return 0;

        pages[n++] = (int)value;
        p = end;

        /* Reject invalid characters between numbers */
        if (*p != '\0' && *p != ' ' && *p != '\t' &&
            *p != '\n' && *p != ',' && *p != ';')
        {
            return 0;
        }
    }

    if (n == 0)
        return 0;

    *count = n;
    return 1;
}

/* Add the frame table and totals for one algorithm */
static void append_algorithm(
    GString *output,
    const char *name,
    const int pages[],
    int n,
    int frame_count,
    const SimulationResult *result)
{
    int i, j;

    g_string_append_printf(
        output,
        "\n================================================\n"
        " %s PAGE REPLACEMENT\n"
        "================================================\n",
        name
    );

    g_string_append_printf(output, "%-8s", "Page");

    for (j = 0; j < frame_count; j++)
        g_string_append_printf(output, " F%-6d", j + 1);

    g_string_append(output, " Status\n");

    for (i = 0; i < n; i++)
    {
        g_string_append_printf(output, "%-8d", pages[i]);

        for (j = 0; j < frame_count; j++)
        {
            int value = result->frames[i][j];

            if (value == EMPTY_FRAME)
                g_string_append_printf(output, " %-7s", "-");
            else
                g_string_append_printf(output, " %-7d", value);
        }

        if (result->status[i] == PAGE_HIT)
            g_string_append(output, " Hit\n");
        else
            g_string_append(output, " Fault\n");
    }

    g_string_append_printf(
        output,
        "\nTotal hits: %d\n"
        "Total faults: %d\n",
        result->total_hits,
        result->total_faults
    );
}

/* Run all three page replacement algorithms */
static void on_run_clicked(GtkButton *button, gpointer data)
{
    int pages[MAX_REQUESTS];
    int n = 0;
    int frame_count;

    SimulationResult fifo_result = {0};
    SimulationResult lru_result = {0};
    SimulationResult optimal_result = {0};

    GString *output;

    (void)button;
    (void)data;

    frame_count = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(frame_spin)
    );

    /* Validate the page-reference input */
    if (!parse_pages(
            gtk_entry_get_text(GTK_ENTRY(page_entry)),
            pages,
            &n))
    {
        gtk_label_set_text(
            GTK_LABEL(status_label),
            "Invalid input. Enter non-negative page numbers."
        );

        show_text(
            "Example input:\n"
            "7, 0, 1, 2, 0, 3, 0, 4, 2, 3\n\n"
            "Separate numbers with commas, spaces or semicolons."
        );

        return;
    }

    if (frame_count < 1 || frame_count > MAX_FRAMES)
    {
        gtk_label_set_text(
            GTK_LABEL(status_label),
            "Invalid number of frames."
        );
        return;
    }

    /* Execute the algorithms */
    fifo(pages, n, frame_count, &fifo_result);
    lru(pages, n, frame_count, &lru_result);
    optimal(pages, n, frame_count, &optimal_result);

    /* Build the output */
    output = g_string_new(NULL);

    g_string_append_printf(
        output,
        "PAGE REPLACEMENT SIMULATOR\n"
        "Page requests: %d    Number of frames: %d\n",
        n,
        frame_count
    );

    append_algorithm(
        output, "FIFO", pages, n, frame_count, &fifo_result
    );

    append_algorithm(
        output, "LRU", pages, n, frame_count, &lru_result
    );

    append_algorithm(
        output, "OPTIMAL", pages, n, frame_count, &optimal_result
    );

    /* Comparison table */
    g_string_append(
        output,
        "\n================================================\n"
        " ALGORITHM COMPARISON\n"
        "================================================\n"
    );

    g_string_append_printf(
        output,
        "%-12s %-12s %-12s\n",
        "Algorithm", "Hits", "Faults"
    );

    g_string_append_printf(
        output, "%-12s %-12d %-12d\n",
        "FIFO",
        fifo_result.total_hits,
        fifo_result.total_faults
    );

    g_string_append_printf(
        output, "%-12s %-12d %-12d\n",
        "LRU",
        lru_result.total_hits,
        lru_result.total_faults
    );

    g_string_append_printf(
        output, "%-12s %-12d %-12d\n",
        "Optimal",
        optimal_result.total_hits,
        optimal_result.total_faults
    );

    show_text(output->str);

    gtk_label_set_text(
        GTK_LABEL(status_label),
        "Simulation completed successfully."
    );

    g_string_free(output, TRUE);
}

/* Clear the results and restore default frame count */
static void on_clear_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(GTK_ENTRY(page_entry), "");

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(frame_spin), 3
    );

    show_text(
        "Enter page requests and click Run Simulation."
    );

    gtk_label_set_text(
        GTK_LABEL(status_label), "Ready."
    );
}

/* Create the GTK desktop application window */
GtkWidget *create_main_window(void)
{
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *heading;
    GtkWidget *description;
    GtkWidget *input_grid;
    GtkWidget *page_label;
    GtkWidget *frame_label;
    GtkWidget *buttons_box;
    GtkWidget *run_button;
    GtkWidget *clear_button;
    GtkWidget *scroll;
    GtkAdjustment *frame_adjustment;

    /* Main window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Page Replacement Simulator | FIFO, LRU, Optimal"
    );

    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 720);

    gtk_window_set_position(
        GTK_WINDOW(window), GTK_WIN_POS_CENTER
    );

    /* Main vertical layout */
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    gtk_container_set_border_width(
        GTK_CONTAINER(main_box), 18
    );

    gtk_container_add(
        GTK_CONTAINER(window), main_box
    );

    /* Application heading */
    heading = gtk_label_new(
        "PAGE REPLACEMENT SIMULATOR"
    );

    gtk_widget_set_halign(heading, GTK_ALIGN_START);
    gtk_label_set_xalign(GTK_LABEL(heading), 0.0f);

    gtk_box_pack_start(
        GTK_BOX(main_box), heading, FALSE, FALSE, 0
    );

    description = gtk_label_new(
        "Simulate and compare FIFO, LRU and Optimal algorithms."
    );

    gtk_widget_set_halign(description, GTK_ALIGN_START);
    gtk_label_set_xalign(GTK_LABEL(description), 0.0f);

    gtk_box_pack_start(
        GTK_BOX(main_box), description, FALSE, FALSE, 0
    );

    /* Input grid */
    input_grid = gtk_grid_new();

    gtk_grid_set_row_spacing(GTK_GRID(input_grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(input_grid), 12);

    gtk_box_pack_start(
        GTK_BOX(main_box), input_grid, FALSE, FALSE, 0
    );

    /* Page-reference entry */
    page_label = gtk_label_new("Page reference string:");

    gtk_widget_set_halign(page_label, GTK_ALIGN_START);

    gtk_grid_attach(
        GTK_GRID(input_grid),
        page_label,
        0, 0, 1, 1
    );

    page_entry = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(page_entry),
        "Example: 7, 0, 1, 2, 0, 3, 0, 4, 2, 3"
    );

    gtk_entry_set_text(
        GTK_ENTRY(page_entry),
        "7, 0, 1, 2, 0, 3, 0, 4, 2, 3"
    );

    gtk_widget_set_hexpand(page_entry, TRUE);

    gtk_grid_attach(
        GTK_GRID(input_grid),
        page_entry,
        1, 0, 2, 1
    );

    /* Frame-count selector */
    frame_label = gtk_label_new("Number of frames:");

    gtk_widget_set_halign(frame_label, GTK_ALIGN_START);

    gtk_grid_attach(
        GTK_GRID(input_grid),
        frame_label,
        0, 1, 1, 1
    );

    frame_adjustment = GTK_ADJUSTMENT(
        gtk_adjustment_new(
            3,          /* Default value */
            1,          /* Minimum */
            MAX_FRAMES, /* Maximum */
            1, 1, 0
        )
    );

    frame_spin = gtk_spin_button_new(
        frame_adjustment, 1, 0
    );

    gtk_grid_attach(
        GTK_GRID(input_grid),
        frame_spin,
        1, 1, 1, 1
    );

    /* Buttons */
    buttons_box = gtk_box_new(
        GTK_ORIENTATION_HORIZONTAL, 10
    );

    gtk_box_pack_start(
        GTK_BOX(main_box),
        buttons_box,
        FALSE, FALSE, 0
    );

    run_button = gtk_button_new_with_label(
        "Run Simulation"
    );

    gtk_widget_set_size_request(
        run_button, 150, 38
    );

    gtk_box_pack_start(
        GTK_BOX(buttons_box),
        run_button,
        FALSE, FALSE, 0
    );

    clear_button = gtk_button_new_with_label("Clear");

    gtk_widget_set_size_request(
        clear_button, 100, 38
    );

    gtk_box_pack_start(
        GTK_BOX(buttons_box),
        clear_button,
        FALSE, FALSE, 0
    );

    /* Status label */
    status_label = gtk_label_new("Ready.");

    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_label_set_xalign(GTK_LABEL(status_label), 0.0f);

    gtk_box_pack_start(
        GTK_BOX(main_box),
        status_label,
        FALSE, FALSE, 0
    );

    /* Scrollable results panel */
    scroll = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );

    gtk_widget_set_vexpand(scroll, TRUE);

    gtk_box_pack_start(
        GTK_BOX(main_box),
        scroll,
        TRUE, TRUE, 0
    );

    results_view = gtk_text_view_new();

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(results_view), FALSE
    );

    gtk_text_view_set_cursor_visible(
        GTK_TEXT_VIEW(results_view), FALSE
    );

    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(results_view), TRUE
    );

    gtk_text_view_set_wrap_mode(
        GTK_TEXT_VIEW(results_view), GTK_WRAP_NONE
    );

    gtk_container_add(
        GTK_CONTAINER(scroll), results_view
    );

    show_text(
        "Enter page requests and click Run Simulation."
    );

    /* Connect GUI buttons to their callbacks */
    g_signal_connect(
        run_button,
        "clicked",
        G_CALLBACK(on_run_clicked),
        NULL
    );

    g_signal_connect(
        clear_button,
        "clicked",
        G_CALLBACK(on_clear_clicked),
        NULL
    );

    g_signal_connect(
        window,
        "destroy",
        G_CALLBACK(gtk_main_quit),
        NULL
    );

    return window;
}
