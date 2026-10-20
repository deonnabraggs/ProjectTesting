#include "deadlock.h"
#include <stdio.h>
#include <stdarg.h>

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
        GTK_TEXT_VIEW(output), &end, 0, FALSE, 0, 1);
}

static int ask_int(const char *title, const char *message,
                   int minimum, int maximum, int initial)
{
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title, GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK, NULL);

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
        result = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

    gtk_widget_destroy(dialog);
    return result;
}

static void show_state(void)
{
    if (n <= 0 || m <= 0)
    {
        gtk_label_set_text(GTK_LABEL(status_label),
                           "SYSTEM STATUS: NOT CONFIGURED");
        gtk_widget_queue_draw(graph_area);
        return;
    }

    int sequence[MAX_PROCESSES];
    int safe = is_safe(sequence);

    gtk_label_set_text(GTK_LABEL(status_label),
                       safe ? "SYSTEM STATUS: SAFE"
                            : "SYSTEM STATUS: UNSAFE");

    log_message("\n========== SYSTEM STATUS ==========");
    log_message("Processes: %d | Resource types: %d", n, m);

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
            char value[24];
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
            char value[24];
            snprintf(value, sizeof(value), " %d", maximum[i][j]);
            strncat(line, value, sizeof(line) - strlen(line) - 1);
        }
        log_message("%s", line);
    }

    log_message("\nNeed Matrix (Maximum - Allocation):");
    for (int i = 0; i < n; i++)
    {
        char line[512];
        snprintf(line, sizeof(line), "P%d:", i);

        for (int j = 0; j < m; j++)
        {
            char value[24];
            snprintf(value, sizeof(value), " %d", need[i][j]);
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
                char value[24];
                snprintf(value, sizeof(value), "P%d ", sequence[i]);
                strncat(line, value, sizeof(line) - strlen(line) - 1);
            }
        }
        log_message("%s", line);
    }
    else
        log_message("No safe sequence exists for the current state.");

    gtk_widget_queue_draw(graph_area);
}

/* Create all matrix cells in one dialog. */
static int edit_matrix(const char *title, int values[MAX_PROCESSES][MAX_RESOURCES],
                       int is_maximum)
{
    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return 0;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title, GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save Matrix", GTK_RESPONSE_OK, NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 520, 350);

    GtkWidget *content =
        gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(outer), 12);
    gtk_box_pack_start(GTK_BOX(content), outer, TRUE, TRUE, 0);

    GtkWidget *hint = gtk_label_new(
        "Click each cell and enter its value. Save when finished.");
    gtk_box_pack_start(GTK_BOX(outer), hint, FALSE, FALSE, 4);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(outer), scroll, TRUE, TRUE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 6);
    gtk_container_add(GTK_CONTAINER(scroll), grid);

    GtkWidget *label = gtk_label_new("Process");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    for (int j = 0; j < m; j++)
    {
        char text[24];
        snprintf(text, sizeof(text), "R%d", j);
        label = gtk_label_new(text);
        gtk_grid_attach(GTK_GRID(grid), label, j + 1, 0, 1, 1);
    }

    GtkWidget *cells[MAX_PROCESSES][MAX_RESOURCES];

    for (int i = 0; i < n; i++)
    {
        char text[24];
        snprintf(text, sizeof(text), "P%d", i);
        label = gtk_label_new(text);
        gtk_grid_attach(GTK_GRID(grid), label, 0, i + 1, 1, 1);

        for (int j = 0; j < m; j++)
        {
            int minimum = is_maximum ? allocation[i][j] : 0;
            int maximum_value = is_maximum ? total[j] : total[j];

            cells[i][j] =
                gtk_spin_button_new_with_range(minimum,
                                               maximum_value, 1);
            gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(cells[i][j]), TRUE);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(cells[i][j]),
                                      values[i][j]);

            gtk_entry_set_width_chars(
                GTK_ENTRY(gtk_bin_get_child(GTK_BIN(cells[i][j]))), 5);

            gtk_grid_attach(GTK_GRID(grid), cells[i][j],
                            j + 1, i + 1, 1, 1);
        }
    }

    gtk_widget_show_all(dialog);
    int saved = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK;

    if (saved)
    {
        int temp[MAX_PROCESSES][MAX_RESOURCES] = {{0}};
        int valid = 1;

        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < m; j++)
            {
                temp[i][j] = gtk_spin_button_get_value_as_int(
                    GTK_SPIN_BUTTON(cells[i][j]));

                if (is_maximum &&
                    (temp[i][j] < allocation[i][j] ||
                     temp[i][j] > total[j]))
                    valid = 0;
            }
        }

        if (!is_maximum)
        {
            for (int j = 0; j < m; j++)
            {
                int sum = 0;
                for (int i = 0; i < n; i++)
                    sum += temp[i][j];

                if (sum > total[j])
                    valid = 0;
            }
        }

        if (!valid)
        {
            GtkWidget *message = gtk_message_dialog_new(
                GTK_WINDOW(window), GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                is_maximum
                ? "Each maximum must be at least its allocation and no greater than the resource total."
                : "The allocation sum for each resource cannot exceed its total.");

            gtk_dialog_run(GTK_DIALOG(message));
            gtk_widget_destroy(message);
            saved = 0;
        }
        else
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < m; j++)
                    values[i][j] = temp[i][j];
            }
        }
    }

    gtk_widget_destroy(dialog);
    return saved;
}

static void on_create_processes(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int count = ask_int("Create Processes",
                        "Enter the number of processes:",
                        1, MAX_PROCESSES, n > 0 ? n : 5);
    if (count < 0)
        return;

    create_processes(count);
    log_message("Created %d processes. Process matrices reset.", count);
    show_state();
}

static void on_create_resources(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int count = ask_int("Create Resources",
                        "Enter the number of resource types:",
                        1, MAX_RESOURCES, m > 0 ? m : 3);
    if (count < 0)
        return;

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Enter Resource Totals", GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_OK, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
    gtk_box_pack_start(GTK_BOX(content), grid, TRUE, TRUE, 0);

    GtkWidget *spins[MAX_RESOURCES];

    for (int j = 0; j < count; j++)
    {
        char label_text[32];
        snprintf(label_text, sizeof(label_text), "Total for R%d:", j);
        GtkWidget *label = gtk_label_new(label_text);
        spins[j] = gtk_spin_button_new_with_range(0, 1000, 1);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spins[j]),
                                  j < m ? total[j] : 5);
        gtk_grid_attach(GTK_GRID(grid), label, 0, j, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), spins[j], 1, j, 1, 1);
    }

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK)
    {
        int amounts[MAX_RESOURCES] = {0};
        for (int j = 0; j < count; j++)
            amounts[j] = gtk_spin_button_get_value_as_int(
                GTK_SPIN_BUTTON(spins[j]));

        create_resources(count, amounts);
        log_message("Created %d resource types.", count);
        show_state();
    }

    gtk_widget_destroy(dialog);
}

static void on_set_allocation(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int temp[MAX_PROCESSES][MAX_RESOURCES];
    for (int i = 0; i < MAX_PROCESSES; i++)
        for (int j = 0; j < MAX_RESOURCES; j++)
            temp[i][j] = allocation[i][j];

    if (!edit_matrix("Set Allocation Matrix", temp, 0))
        return;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
        {
            allocation[i][j] = temp[i][j];
            if (maximum[i][j] < allocation[i][j])
                maximum[i][j] = allocation[i][j];
        }

    recalculate_state();
    log_message("Allocation matrix saved.");
    show_state();
}

static void on_set_maximum(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int temp[MAX_PROCESSES][MAX_RESOURCES];
    for (int i = 0; i < MAX_PROCESSES; i++)
        for (int j = 0; j < MAX_RESOURCES; j++)
            temp[i][j] = maximum[i][j];

    if (!edit_matrix("Set Maximum Matrix", temp, 1))
        return;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            maximum[i][j] = temp[i][j];

    recalculate_state();
    log_message("Maximum matrix saved.");
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
            char value[24];
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

    int pid = ask_int("Resource Request", "Enter process number:",
                      0, n - 1, 0);
    if (pid < 0)
        return;

    int req[MAX_RESOURCES] = {0};

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Enter Resource Request", GTK_WINDOW(window), GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Submit Request", GTK_RESPONSE_OK, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
    gtk_box_pack_start(GTK_BOX(content), grid, TRUE, TRUE, 0);

    GtkWidget *spins[MAX_RESOURCES];

    for (int j = 0; j < m; j++)
    {
        char label_text[64];
        snprintf(label_text, sizeof(label_text), "Request for R%d:", j);
        GtkWidget *label = gtk_label_new(label_text);
        spins[j] = gtk_spin_button_new_with_range(0, 1000, 1);
        gtk_grid_attach(GTK_GRID(grid), label, 0, j, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), spins[j], 1, j, 1, 1);
    }

    gtk_widget_show_all(dialog);
    int accepted = gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK;

    if (accepted)
    {
        for (int j = 0; j < m; j++)
            req[j] = gtk_spin_button_get_value_as_int(
                GTK_SPIN_BUTTON(spins[j]));

        int result = request_resources(pid, req);

        switch (result)
        {
            case REQ_GRANTED:
                log_message("Request granted. System remains SAFE.");
                break;
            case REQ_MUST_WAIT:
                log_message("Insufficient resources. P%d must wait.", pid);
                break;
            case REQ_UNSAFE:
                log_message("Request denied because it would make the state unsafe.");
                break;
            case REQ_EXCEEDS_NEED:
                log_message("Request exceeds remaining need of P%d.", pid);
                break;
            default:
                log_message("Invalid resource request.");
                break;
        }
        show_state();
    }

    gtk_widget_destroy(dialog);
}

static void on_banker(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int sequence[MAX_PROCESSES];
    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    if (is_safe(sequence))
    {
        char line[512] = "BANKER'S ALGORITHM: SAFE\nSafe sequence: ";
        for (int i = 0; i < n; i++)
        {
            if (sequence[i] >= 0)
            {
                char value[24];
                snprintf(value, sizeof(value), "P%d ", sequence[i]);
                strncat(line, value, sizeof(line) - strlen(line) - 1);
            }
        }
        log_message("%s", line);
    }
    else
        log_message("BANKER'S ALGORITHM: UNSAFE STATE.");

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
        log_message("Deadlock detection failed: invalid system state.");
    else if (count == 0)
        log_message("No deadlocked processes detected.");
    else
    {
        log_message("Potentially deadlocked processes:");
        for (int i = 0; i < n; i++)
            if (deadlocked[i])
                log_message("P%d is potentially deadlocked.", i);
    }
    gtk_widget_queue_draw(graph_area);
}

static void on_recover(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int pid = ask_int("Recovery", "Enter process number to terminate:",
                      0, n > 0 ? n - 1 : 0, 0);
    if (pid < 0)
        return;

    if (recover_from_deadlock(pid) == REC_OK)
        log_message("P%d terminated. Its resources have been released.", pid);
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

    gtk_text_buffer_set_text(
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output)), "", -1);
}

static void on_exit_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;
    gtk_main_quit();
}

static gboolean on_graph_draw(GtkWidget *widget, cairo_t *cr,
                              gpointer data)
{
    (void)data;
    GtkAllocation a;
    gtk_widget_get_allocation(widget, &a);
    draw_resource_graph(cr, a.width, a.height);
    return FALSE;
}

static void add_button(GtkWidget *grid, const char *label,
                       GCallback callback, int column, int row)
{
    GtkWidget *button = gtk_button_new_with_label(label);
    gtk_widget_set_hexpand(button, TRUE);
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
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 10);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    GtkWidget *heading = gtk_label_new(
        "OPERATING SYSTEMS - DEADLOCK SIMULATION");
    gtk_box_pack_start(GTK_BOX(main_box), heading, FALSE, FALSE, 4);

    status_label = gtk_label_new("SYSTEM STATUS: NOT CONFIGURED");
    gtk_box_pack_start(GTK_BOX(main_box), status_label, FALSE, FALSE, 4);

    GtkWidget *content_box =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(main_box), content_box, TRUE, TRUE, 4);

    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_widget_set_size_request(left_box, 380, -1);
    gtk_box_pack_start(GTK_BOX(content_box), left_box, FALSE, FALSE, 0);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);
    gtk_box_pack_start(GTK_BOX(left_box), grid, FALSE, FALSE, 0);

    add_button(grid, "Create Processes", G_CALLBACK(on_create_processes), 0, 0);
    add_button(grid, "Create Resources", G_CALLBACK(on_create_resources), 1, 0);
    add_button(grid, "Set Allocation", G_CALLBACK(on_set_allocation), 0, 1);
    add_button(grid, "Set Maximum", G_CALLBACK(on_set_maximum), 1, 1);
    add_button(grid, "Show Need", G_CALLBACK(on_show_need), 0, 2);
    add_button(grid, "Refresh Graph", G_CALLBACK(on_refresh_graph), 1, 2);
    add_button(grid, "Request Resources", G_CALLBACK(on_request_resources), 0, 3);
    add_button(grid, "Banker's Algorithm", G_CALLBACK(on_banker), 1, 3);
    add_button(grid, "Detect Deadlock", G_CALLBACK(on_detect_deadlock), 0, 4);
    add_button(grid, "Recover / Terminate", G_CALLBACK(on_recover), 1, 4);
    add_button(grid, "Show System Status", G_CALLBACK(on_show_status), 0, 5);
    add_button(grid, "Clear Output", G_CALLBACK(on_clear_output), 1, 5);
    add_button(grid, "Exit", G_CALLBACK(on_exit_clicked), 0, 6);

    GtkWidget *graph_frame = gtk_frame_new("Resource Allocation Graph");
    gtk_box_pack_start(GTK_BOX(left_box), graph_frame, TRUE, TRUE, 4);

    graph_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(graph_area, 360, 300);
    g_signal_connect(graph_area, "draw", G_CALLBACK(on_graph_draw), NULL);
    gtk_container_add(GTK_CONTAINER(graph_frame), graph_area);

    GtkWidget *output_frame = gtk_frame_new("Simulation Output");
    gtk_box_pack_start(GTK_BOX(content_box), output_frame, TRUE, TRUE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
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
