#include "deadlock.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static GtkWidget *window;
static GtkWidget *output;
static GtkWidget *graph_area;
static GtkWidget *status_label;

static void log_message(const char *format, ...)
{
    char message[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, message, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", 1);

    gtk_text_buffer_get_end_iter(buffer, &end);

    gtk_text_view_scroll_to_iter(
        GTK_TEXT_VIEW(output), &end, 0.0, FALSE, 0.0, 1.0);
}

static int ask_int(const char *title,
                   const char *message,
                   int minimum,
                   int maximum,
                   int initial)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title,
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK,
        NULL);

    GtkWidget *content =
        gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *label = gtk_label_new(message);
    GtkWidget *spin =
        gtk_spin_button_new_with_range(minimum, maximum, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), initial);

    gtk_box_pack_start(GTK_BOX(content), label, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(content), spin, FALSE, FALSE, 8);

    gtk_widget_show_all(dialog);

    int result = -1;

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        result = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(spin));
    }

    gtk_widget_destroy(dialog);
    return result;
}

static void show_state(void)
{
    if (n <= 0 || m <= 0)
    {
        gtk_label_set_text(
            GTK_LABEL(status_label), "SYSTEM STATUS: NOT CONFIGURED");

        log_message("Create processes and resources first.");
        gtk_widget_queue_draw(graph_area);
        return;
    }

    int sequence[MAX_PROCESSES];
    int safe = is_safe(sequence);

    gtk_label_set_text(
        GTK_LABEL(status_label),
        safe ? "SYSTEM STATUS: SAFE" : "SYSTEM STATUS: UNSAFE");

    log_message("\n========== SYSTEM STATUS ==========");
    log_message("Processes: %d | Resource types: %d", n, m);
    log_message("Available resources:");

    for (int j = 0; j < m; j++)
        log_message("R%d: Total=%d Available=%d Used=%d Utilization=%.1f%%",
                    j, total[j], available[j],
                    total[j] - available[j], resource_utilization(j));

    log_message("\nAllocation Matrix:");

    for (int i = 0; i < n; i++)
    {
        char line[512];
        snprintf(line, sizeof(line), "P%d%s:", i,
                 active[i] ? "" : " (terminated)");

        for (int j = 0; j < m; j++)
        {
            char value[32];
            snprintf(value, sizeof(value), " %d", allocation[i][j]);
            strncat(line, value, sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    log_message("\nMaximum Matrix:");

    for (int i = 0; i < n; i++)
    {
        char line[512];
        snprintf(line, sizeof(line), "P%d:", i);

        for (int j = 0; j < m; j++)
        {
            char value[32];
            snprintf(value, sizeof(value), " %d", maximum[i][j]);
            strncat(line, value, sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    log_message("\nNeed Matrix:");

    for (int i = 0; i < n; i++)
    {
        char line[512];
        snprintf(line, sizeof(line), "P%d:", i);

        for (int j = 0; j < m; j++)
        {
            char value[32];
            snprintf(value, sizeof(value), " %d", need[i][j]);
            strncat(line, value, sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    log_message("\nPending Request Matrix:");

    for (int i = 0; i < n; i++)
    {
        char line[512];
        snprintf(line, sizeof(line), "P%d:", i);

        for (int j = 0; j < m; j++)
        {
            char value[32];
            snprintf(value, sizeof(value), " %d", request[i][j]);
            strncat(line, value, sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    if (safe)
    {
        char line[512] = "Safe sequence: ";

        for (int i = 0; i < n; i++)
        {
            if (sequence[i] >= 0)
            {
                char value[32];
                snprintf(value, sizeof(value), "P%d ", sequence[i]);
                strncat(line, value, sizeof(line) - strlen(line) - 1);
            }
        }

        log_message("%s", line);
    }
    else
    {
        log_message("No safe sequence exists for the current state.");
    }

    gtk_widget_queue_draw(graph_area);
}

static void on_create_processes(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int count = ask_int("Create Processes",
                        "Enter the number of processes:",
                        1, MAX_PROCESSES, n > 0 ? n : 5);

    if (count == -1)
        return;

    create_processes(count);
    log_message("Created %d processes. Previous process matrices were reset.",
                count);
    show_state();
}

static void on_create_resources(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int count = ask_int("Create Resources",
                        "Enter the number of resource types:",
                        1, MAX_RESOURCES, m > 0 ? m : 3);

    if (count == -1)
        return;

    int amounts[MAX_RESOURCES] = {0};

    for (int j = 0; j < count; j++)
    {
        char prompt[100];
        snprintf(prompt, sizeof(prompt),
                 "Enter total quantity of resource R%d:", j);

        int amount = ask_int("Resource Quantity", prompt, 0, 1000, 5);

        if (amount == -1)
            return;

        amounts[j] = amount;
    }

    create_resources(count, amounts);
    log_message("Created %d resource types. Previous resource matrices were reset.",
                count);
    show_state();
}

static void on_set_allocation(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    int pid = ask_int("Allocation", "Enter process number:", 0, n - 1, 0);
    if (pid == -1)
        return;

    int rid = ask_int("Allocation", "Enter resource number:", 0, m - 1, 0);
    if (rid == -1)
        return;

    int amount = ask_int("Allocation", "Enter allocation quantity:",
                         0, 1000, allocation[pid][rid]);

    if (amount == -1)
        return;

    int old = allocation[pid][rid];
    set_allocation(pid, rid, amount);

    if (allocation[pid][rid] != amount)
        log_message("Allocation rejected. It exceeds the available resources or is invalid.");
    else
        log_message("Updated allocation of R%d for P%d: %d -> %d.",
                    rid, pid, old, amount);

    show_state();
}

static void on_set_maximum(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    int pid = ask_int("Maximum", "Enter process number:", 0, n - 1, 0);
    if (pid == -1)
        return;

    int rid = ask_int("Maximum", "Enter resource number:", 0, m - 1, 0);
    if (rid == -1)
        return;

    int amount = ask_int("Maximum", "Enter maximum quantity:",
                         allocation[pid][rid], total[rid],
                         maximum[pid][rid]);

    if (amount == -1)
        return;

    set_maximum(pid, rid, amount);
    log_message("Updated maximum claim for P%d, R%d.", pid, rid);
    show_state();
}

static void on_show_need(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    log_message("Need = Maximum - Allocation");

    for (int i = 0; i < n; i++)
    {
        char line[512];
        snprintf(line, sizeof(line), "P%d:", i);

        for (int j = 0; j < m; j++)
        {
            char value[32];
            snprintf(value, sizeof(value), " %d", need[i][j]);
            strncat(line, value, sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }
}

static void on_refresh_graph(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_widget_queue_draw(graph_area);
    log_message("Resource Allocation Graph refreshed.");
}

static void on_request_resources(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    int pid = ask_int("Resource Request",
                      "Enter requesting process number:",
                      0, n - 1, 0);

    if (pid == -1)
        return;

    int req[MAX_RESOURCES] = {0};

    for (int j = 0; j < m; j++)
    {
        char prompt[100];
        snprintf(prompt, sizeof(prompt),
                 "Quantity of resource R%d requested by P%d:",
                 j, pid);

        int amount = ask_int("Resource Request", prompt,
                             0, 1000, 0);

        if (amount == -1)
            return;

        req[j] = amount;
    }

    int result = request_resources(pid, req);

    switch (result)
    {
        case REQ_GRANTED:
            log_message("Request granted. System remains SAFE.");
            break;

        case REQ_MUST_WAIT:
            log_message("Insufficient available resources. P%d must wait.",
                        pid);
            break;

        case REQ_UNSAFE:
            log_message("Request denied because it would make the state unsafe.");
            break;

        case REQ_EXCEEDS_NEED:
            log_message("Request exceeds the remaining need of P%d.", pid);
            break;

        default:
            log_message("Invalid resource request.");
            break;
    }

    show_state();
}

static void on_banker(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    int sequence[MAX_PROCESSES];

    if (is_safe(sequence))
    {
        char line[512] = "BANKER'S ALGORITHM: SAFE\nSafe sequence: ";

        for (int i = 0; i < n; i++)
        {
            if (sequence[i] >= 0)
            {
                char value[32];
                snprintf(value, sizeof(value), "P%d ", sequence[i]);
                strncat(line, value, sizeof(line) - strlen(line) - 1);
            }
        }

        log_message("%s", line);
    }
    else
    {
        log_message("BANKER'S ALGORITHM: UNSAFE STATE.");
    }

    show_state();
}

static void on_detect_deadlock(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    int deadlocked[MAX_PROCESSES];
    int count = detect_deadlock(deadlocked);

    if (count < 0)
    {
        log_message("Deadlock detection failed: invalid system state.");
    }
    else if (count == 0)
    {
        log_message("No deadlocked processes detected.");
    }
    else
    {
        log_message("Potentially deadlocked processes:");

        for (int i = 0; i < n; i++)
        {
            if (deadlocked[i])
                log_message("P%d is potentially deadlocked.", i);
        }
    }

    gtk_widget_queue_draw(graph_area);
}

static void on_recover(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0)
    {
        log_message("Create processes first.");
        return;
    }

    int pid = ask_int("Recovery",
                      "Enter process number to terminate:",
                      0, n - 1, 0);

    if (pid == -1)
        return;

    if (recover_from_deadlock(pid) == REC_OK)
        log_message("P%d terminated. Its allocated resources have been released.",
                    pid);
    else
        log_message("Recovery failed. Check that the process is active.");

    show_state();
}

static void on_show_status(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    show_state();
}

static void on_clear_output(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output));

    gtk_text_buffer_set_text(buffer, "", -1);
    log_message("Output cleared.");
}

static void on_exit_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_main_quit();
}

static gboolean on_graph_draw(GtkWidget *widget,
                              cairo_t *cr,
                              gpointer data)
{
    (void)data;

    GtkAllocation allocation_info;
    gtk_widget_get_allocation(widget, &allocation_info);

    draw_resource_graph(cr,
                        allocation_info.width,
                        allocation_info.height);

    return FALSE;
}

static void add_button(GtkWidget *grid,
                       const char *label,
                       GCallback callback,
                       int column,
                       int row)
{
    GtkWidget *button = gtk_button_new_with_label(label);

    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_vexpand(button, FALSE);

    g_signal_connect(button, "clicked", callback, NULL);

    gtk_grid_attach(GTK_GRID(grid), button, column, row, 1, 1);
}

void run_gui(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),
                         "Interactive Deadlock Simulation System");
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 750);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    g_signal_connect(window, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *heading = gtk_label_new(
        "OPERATING SYSTEMS - DEADLOCK SIMULATION");

    PangoAttrList *attrs = pango_attr_list_new();
    PangoAttribute *weight =
        pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(attrs, weight);
    gtk_label_set_attributes(GTK_LABEL(heading), attrs);
    pango_attr_list_unref(attrs);

    gtk_box_pack_start(GTK_BOX(main_box), heading,
                       FALSE, FALSE, 4);

    status_label = gtk_label_new("SYSTEM STATUS: NOT CONFIGURED");
    gtk_box_pack_start(GTK_BOX(main_box), status_label,
                       FALSE, FALSE, 4);

    GtkWidget *content_box =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    gtk_box_pack_start(GTK_BOX(main_box), content_box,
                       TRUE, TRUE, 4);

    GtkWidget *left_box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    gtk_widget_set_size_request(left_box, 380, -1);

    gtk_box_pack_start(GTK_BOX(content_box), left_box,
                       FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);

    gtk_box_pack_start(GTK_BOX(left_box), grid,
                       FALSE, FALSE, 0);

    add_button(grid, "Create Processes",
               G_CALLBACK(on_create_processes), 0, 0);
    add_button(grid, "Create Resources",
               G_CALLBACK(on_create_resources), 1, 0);
    add_button(grid, "Set Allocation",
               G_CALLBACK(on_set_allocation), 0, 1);
    add_button(grid, "Set Maximum",
               G_CALLBACK(on_set_maximum), 1, 1);
    add_button(grid, "Show Need",
               G_CALLBACK(on_show_need), 0, 2);
    add_button(grid, "Refresh Graph",
               G_CALLBACK(on_refresh_graph), 1, 2);
    add_button(grid, "Request Resources",
               G_CALLBACK(on_request_resources), 0, 3);
    add_button(grid, "Banker's Algorithm",
               G_CALLBACK(on_banker), 1, 3);
    add_button(grid, "Detect Deadlock",
               G_CALLBACK(on_detect_deadlock), 0, 4);
    add_button(grid, "Recover / Terminate",
               G_CALLBACK(on_recover), 1, 4);
    add_button(grid, "Show System Status",
               G_CALLBACK(on_show_status), 0, 5);
    add_button(grid, "Clear Output",
               G_CALLBACK(on_clear_output), 1, 5);
    add_button(grid, "Exit",
               G_CALLBACK(on_exit_clicked), 0, 6);

    GtkWidget *graph_frame =
        gtk_frame_new("Resource Allocation Graph");

    gtk_box_pack_start(GTK_BOX(left_box), graph_frame,
                       TRUE, TRUE, 4);

    graph_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(graph_area, 360, 300);

    g_signal_connect(graph_area, "draw",
                     G_CALLBACK(on_graph_draw), NULL);

    gtk_container_add(GTK_CONTAINER(graph_frame), graph_area);

    GtkWidget *output_frame = gtk_frame_new("Simulation Output");

    gtk_box_pack_start(GTK_BOX(content_box), output_frame,
                       TRUE, TRUE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);

    gtk_container_add(GTK_CONTAINER(output_frame), scroll);

    output = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output), GTK_WRAP_WORD_CHAR);

    gtk_container_add(GTK_CONTAINER(scroll), output);

    gtk_widget_show_all(window);

    show_state();

    gtk_main();
}
