#include "deadlock.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static GtkWidget *window;
static GtkWidget *output;
static GtkWidget *graph;
static GtkWidget *status;

static void logmsg(const char *fmt, ...)
{
    char s[1024];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(s, sizeof(s), fmt, ap);
    va_end(ap);

    GtkTextBuffer *b =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));

    GtkTextIter e;
    gtk_text_buffer_get_end_iter(b, &e);
    gtk_text_buffer_insert(b, &e, s, -1);
    gtk_text_buffer_insert(b, &e, "\n", 1);

    gtk_text_buffer_get_end_iter(b, &e);
    gtk_text_view_scroll_to_iter(
        GTK_TEXT_VIEW(output), &e, 0.0, FALSE, 0.0, 1.0);
}

static int ask(const char *title, const char *prompt,
               int min, int max, int initial)
{
    GtkWidget *d = gtk_dialog_new_with_buttons(
        title, GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK, NULL);

    GtkWidget *c = gtk_dialog_get_content_area(GTK_DIALOG(d));
    GtkWidget *l = gtk_label_new(prompt);
    GtkWidget *sp = gtk_spin_button_new_with_range(min, max, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(sp), initial);
    gtk_box_pack_start(GTK_BOX(c), l, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(c), sp, FALSE, FALSE, 8);
    gtk_widget_show_all(d);

    int v = -1;

    if (gtk_dialog_run(GTK_DIALOG(d)) == GTK_RESPONSE_OK)
        v = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sp));

    gtk_widget_destroy(d);
    return v;
}

static void refresh(void)
{
    gtk_widget_queue_draw(graph);

    if (n <= 0 || m <= 0)
    {
        gtk_label_set_text(GTK_LABEL(status),
                           "SYSTEM STATUS: NOT CONFIGURED");
        return;
    }

    int seq[MAX_PROCESSES];
    int safe = is_safe(seq);

    gtk_label_set_text(
        GTK_LABEL(status),
        safe ? "SYSTEM STATUS: SAFE" : "SYSTEM STATUS: UNSAFE");

    logmsg("\n========== SYSTEM STATUS ==========");
    logmsg("Processes: %d | Resource types: %d", n, m);

    for (int j = 0; j < m; j++)
    {
        logmsg("R%d Total=%d Available=%d Used=%d Utilization=%.1f%%",
               j, total[j], available[j],
               total[j] - available[j], resource_utilization(j));
    }

#define SHOW_MATRIX(title, matrix)                       \
    do {                                                 \
        logmsg(title);                                   \
        for (int i = 0; i < n; i++)                     \
        {                                                \
            char line[256];                              \
            int len = snprintf(line, sizeof(line),       \
                               "P%d%s:", i,              \
                               active[i] ? "" : " (terminated)"); \
            for (int j = 0; j < m; j++)                 \
                len += snprintf(line + len,              \
                    sizeof(line) - (size_t)len,          \
                    " %d", matrix[i][j]);                \
            logmsg("%s", line);                          \
        }                                                \
    } while (0)

    SHOW_MATRIX("\nAllocation Matrix:", allocation);
    SHOW_MATRIX("\nMaximum Matrix:", maximum);
    SHOW_MATRIX("\nNeed Matrix:", need);
    SHOW_MATRIX("\nPending Request Matrix:", request);

#undef SHOW_MATRIX

    if (safe)
    {
        char line[256] = "Safe sequence: ";

        for (int i = 0; i < n; i++)
        {
            if (seq[i] >= 0)
            {
                char x[16];
                snprintf(x, sizeof(x), "P%d ", seq[i]);
                strncat(line, x, sizeof(line) - strlen(line) - 1);
            }
        }

        logmsg("%s", line);
    }
    else
    {
        logmsg("No safe sequence exists.");
    }
}

static void on_status(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;
    refresh();
}

static void on_need(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;

    if (n <= 0 || m <= 0)
    {
        logmsg("Enter process and resource data first.");
        return;
    }

    logmsg("Need = Maximum - Allocation");

    for (int i = 0; i < n; i++)
    {
        char line[256];
        int len = snprintf(line, sizeof(line), "P%d:", i);

        for (int j = 0; j < m; j++)
            len += snprintf(line + len, sizeof(line) - (size_t)len,
                            " %d", need[i][j]);

        logmsg("%s", line);
    }
}

static void on_banker(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;

    int seq[MAX_PROCESSES];

    if (is_safe(seq))
    {
        char line[256] = "BANKER'S ALGORITHM: SAFE. Sequence: ";

        for (int i = 0; i < n; i++)
        {
            if (seq[i] >= 0)
            {
                char x[16];
                snprintf(x, sizeof(x), "P%d ", seq[i]);
                strncat(line, x, sizeof(line) - strlen(line) - 1);
            }
        }

        logmsg("%s", line);
    }
    else
    {
        logmsg("BANKER'S ALGORITHM: UNSAFE STATE.");
    }

    refresh();
}

static void on_detect(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;

    if (n <= 0 || m <= 0)
    {
        logmsg("Enter process and resource data first.");
        return;
    }

    int dead[MAX_PROCESSES];
    int count = detect_deadlock(dead);

    if (count < 0)
        logmsg("Detection failed: invalid system state.");
    else if (count == 0)
        logmsg("No deadlocked processes detected.");
    else
    {
        logmsg("Potentially deadlocked processes:");

        for (int i = 0; i < n; i++)
        {
            if (dead[i])
                logmsg("P%d is potentially deadlocked.", i);
        }
    }

    gtk_widget_queue_draw(graph);
}

static void on_request(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;

    if (n <= 0 || m <= 0)
    {
        logmsg("Enter process and resource data first.");
        return;
    }

    int p = ask("Request Resources", "Process number:",
                0, n - 1, 0);

    if (p < 0)
        return;

    int req[MAX_RESOURCES] = {0};

    for (int j = 0; j < m; j++)
    {
        char prompt[80];

        snprintf(prompt, sizeof(prompt),
                 "Units of R%d requested by P%d:", j, p);

        req[j] = ask("Request Resources", prompt, 0, 1000, 0);

        if (req[j] < 0)
            return;
    }

    int result = request_resources(p, req);

    switch (result)
    {
        case REQ_GRANTED:
            logmsg("Request granted; state remains safe.");
            break;

        case REQ_MUST_WAIT:
            logmsg("Not enough available resources. P%d must wait.", p);
            break;

        case REQ_UNSAFE:
            logmsg("Request denied because it would make the state unsafe.");
            break;

        case REQ_EXCEEDS_NEED:
            logmsg("Request exceeds P%d's remaining need.", p);
            break;

        default:
            logmsg("Invalid resource request.");
            break;
    }

    refresh();
}

static void on_recover(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;

    if (n <= 0)
    {
        logmsg("No processes configured.");
        return;
    }

    int p = ask("Recover / Terminate",
                "Process number to terminate:", 0, n - 1, 0);

    if (p < 0)
        return;

    if (recover_from_deadlock(p) == REC_OK)
        logmsg("P%d terminated; its resources were released.", p);
    else
        logmsg("Recovery failed: process is not active.");

    refresh();
}

static void on_clear(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output)), "", -1);
}

static void on_exit(GtkButton *b, gpointer d)
{
    (void)b;
    (void)d;
    gtk_main_quit();
}

static gboolean draw_cb(GtkWidget *w, cairo_t *cr, gpointer d)
{
    (void)d;

    GtkAllocation a;
    gtk_widget_get_allocation(w, &a);

    draw_resource_graph(cr, a.width, a.height);
    return FALSE;
}

static void add_button(GtkWidget *grid, const char *label,
                       GCallback callback, int x, int y)
{
    GtkWidget *b = gtk_button_new_with_label(label);

    gtk_widget_set_hexpand(b, TRUE);
    g_signal_connect(b, "clicked", callback, NULL);
    gtk_grid_attach(GTK_GRID(grid), b, x, y, 1, 1);
}

void run_gui(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(
        GTK_WINDOW(window), "Interactive Deadlock Simulation System");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 720);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    g_signal_connect(window, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *base = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(base), 10);
    gtk_container_add(GTK_CONTAINER(window), base);

    GtkWidget *heading =
        gtk_label_new("OPERATING SYSTEMS - DEADLOCK SIMULATION");
    gtk_box_pack_start(GTK_BOX(base), heading, FALSE, FALSE, 4);

    status = gtk_label_new("SYSTEM STATUS: READY");
    gtk_box_pack_start(GTK_BOX(base), status, FALSE, FALSE, 4);

    GtkWidget *body = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(base), body, TRUE, TRUE, 0);

    GtkWidget *left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_size_request(left, 390, -1);
    gtk_box_pack_start(GTK_BOX(body), left, FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_box_pack_start(GTK_BOX(left), grid, FALSE, FALSE, 0);

    add_button(grid, "Show System Status",
               G_CALLBACK(on_status), 0, 0);
    add_button(grid, "Show Need",
               G_CALLBACK(on_need), 1, 0);
    add_button(grid, "Banker's Algorithm",
               G_CALLBACK(on_banker), 0, 1);
    add_button(grid, "Request Resources",
               G_CALLBACK(on_request), 1, 1);
    add_button(grid, "Detect Deadlock",
               G_CALLBACK(on_detect), 0, 2);
    add_button(grid, "Recover / Terminate",
               G_CALLBACK(on_recover), 1, 2);
    add_button(grid, "Refresh Graph",
               G_CALLBACK(on_status), 0, 3);
    add_button(grid, "Clear Output",
               G_CALLBACK(on_clear), 1, 3);
    add_button(grid, "Exit",
               G_CALLBACK(on_exit), 0, 4);

    GtkWidget *frame = gtk_frame_new("Resource Allocation Graph");
    gtk_box_pack_start(GTK_BOX(left), frame, TRUE, TRUE, 4);

    graph = gtk_drawing_area_new();
    gtk_widget_set_size_request(graph, 360, 300);

    g_signal_connect(graph, "draw", G_CALLBACK(draw_cb), NULL);
    gtk_container_add(GTK_CONTAINER(frame), graph);

    GtkWidget *out_frame = gtk_frame_new("Simulation Output");
    gtk_box_pack_start(GTK_BOX(body), out_frame, TRUE, TRUE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(out_frame), scroll);

    output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output), FALSE);
    gtk_text_view_set_wrap_mode(
        GTK_TEXT_VIEW(output), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), output);

    gtk_widget_show_all(window);

    logmsg("Initial data loaded. Use the buttons to run the simulation.");
    refresh();

    gtk_main();
}
