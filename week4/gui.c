#include "deadlock.h"
#include <stdio.h>
#include <stdarg.h>

static GtkWidget *window;
static GtkWidget *output;
static GtkWidget *graph_area;
static GtkWidget *status_label;

/* Display a message in the output panel. */
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
}

/* Ask the user for an integer using a dialog box.
   Returns -1 if the user cancels. */
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

    GtkWidget *spin = gtk_spin_button_new_with_range(
        minimum, maximum, 1);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), initial);

    gtk_box_pack_start(GTK_BOX(content), label,
                       FALSE, FALSE, 8);

    gtk_box_pack_start(GTK_BOX(content), spin,
                       FALSE, FALSE, 8);

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

/* Display all matrices and resource information. */
static void show_state(void)
{
    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    int sequence[MAX_PROCESSES];
    int safe = is_safe(sequence);

    gtk_label_set_text(
        GTK_LABEL(status_label),
        safe ? "SYSTEM STATUS: SAFE" : "SYSTEM STATUS: UNSAFE");

    log_message("\n========== SYSTEM STATUS ==========");

    log_message("Processes: %d | Resource types: %d", n, m);

    log_message("\nAvailable Resource Vector:");

    for (int j = 0; j < m; j++)
        log_message("R%d = %d", j, available[j]);

    log_message("\nResource Utilization:");

    for (int j = 0; j < m; j++)
    {
        log_message("R%d: Total=%d, Available=%d, Used=%d, Utilization=%.1f%%",
                    j, total[j], available[j],
                    total[j] - available[j],
                    resource_utilization(j));
    }

    log_message("\nAllocation Matrix:");

    for (int i = 0; i < n; i++)
    {
        char line[256] = "";

        snprintf(line, sizeof(line), "P%d: ", i);

        for (int j = 0; j < m; j++)
        {
            char temp[24];

            snprintf(temp, sizeof(temp), "%d ",
                     allocation[i][j]);

            strncat(line, temp,
                    sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    log_message("\nMaximum Requirement Matrix:");

    for (int i = 0; i < n; i++)
    {
        char line[256] = "";

        snprintf(line, sizeof(line), "P%d: ", i);

        for (int j = 0; j < m; j++)
        {
            char temp[24];

            snprintf(temp, sizeof(temp), "%d ",
                     maximum[i][j]);

            strncat(line, temp,
                    sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    log_message("\nNeed Matrix:");

    for (int i = 0; i < n; i++)
    {
        char line[256] = "";

        snprintf(line, sizeof(line), "P%d: ", i);

        for (int j = 0; j < m; j++)
        {
            char temp[24];

            snprintf(temp, sizeof(temp), "%d ",
                     need[i][j]);

            strncat(line, temp,
                    sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    log_message("\nOutstanding Requests:");

    for (int i = 0; i < n; i++)
    {
        char line[256] = "";

        snprintf(line, sizeof(line), "P%d: ", i);

        for (int j = 0; j < m; j++)
        {
            char temp[24];

            snprintf(temp, sizeof(temp), "%d ",
                     request[i][j]);

            strncat(line, temp,
                    sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }

    log_message("\n%s",
                safe ? "System is SAFE."
                     : "System is UNSAFE.");

    if (safe)
    {
        char line[256] = "Safe Sequence: ";

        for (int i = 0; i < n; i++)
        {
            if (sequence[i] >= 0)
            {
                char temp[20];

                snprintf(temp, sizeof(temp),
                         "P%d ", sequence[i]);

                strncat(line, temp,
                        sizeof(line) - strlen(line) - 1);
            }
        }

        log_message("%s", line);
    }

    log_message("==================================\n");

    gtk_widget_queue_draw(graph_area);
}

/* 1. Create processes. */
static void on_create_processes(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int count = ask_int(
        "Create Processes",
        "Enter the number of processes:",
        1, MAX_PROCESSES, 5);

    if (count == -1)
        return;

    create_processes(count);

    log_message("Created %d processes.", count);
    show_state();
}

/* 2. Create resources. */
static void on_create_resources(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int count = ask_int(
        "Create Resources",
        "Enter the number of resource types:",
        1, MAX_RESOURCES, 3);

    if (count == -1)
        return;

    int amounts[MAX_RESOURCES] = {0};

    for (int j = 0; j < count; j++)
    {
        char message[100];

        snprintf(message, sizeof(message),
                 "Enter the total instances of R%d:", j);

        amounts[j] = ask_int(
            "Resource Quantity", message, 0, 10000, 5);

        if (amounts[j] == -1)
            return;
    }

    create_resources(count, amounts);

    log_message("Created %d resource types.", count);
    show_state();
}

/* 3. Enter the allocation matrix. */
static void on_enter_allocation(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    for (int i = 0; i < n; i++)
    {
        if (!active[i])
            continue;

        for (int j = 0; j < m; j++)
        {
            char message[120];

            snprintf(message, sizeof(message),
                     "Allocation for P%d, resource R%d:",
                     i, j);

            int amount = ask_int(
                "Allocation Matrix",
                message, 0, total[j], allocation[i][j]);

            if (amount == -1)
                return;

            int old = allocation[i][j];

            set_allocation(i, j, amount);

            if (allocation[i][j] != amount)
            {
                allocation[i][j] = old;
                recalculate_state();

                log_message(
                    "Invalid allocation. Total resources exceeded.");
                return;
            }
        }
    }

    log_message("Allocation matrix updated.");
    show_state();
}

/* 4. Enter the maximum requirement matrix. */
static void on_enter_maximum(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    for (int i = 0; i < n; i++)
    {
        if (!active[i])
            continue;

        for (int j = 0; j < m; j++)
        {
            char message[120];

            snprintf(message, sizeof(message),
                     "Maximum requirement for P%d, resource R%d:",
                     i, j);

            int amount = ask_int(
                "Maximum Matrix",
                message, allocation[i][j],
                total[j], maximum[i][j]);

            if (amount == -1)
                return;

            set_maximum(i, j, amount);
        }
    }

    log_message("Maximum requirement matrix updated.");
    show_state();
}

/* 5. Display the need matrix. */
static void on_display_need(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    log_message("\nNeed Matrix (Maximum - Allocation):");

    for (int i = 0; i < n; i++)
    {
        char line[256] = "";

        snprintf(line, sizeof(line), "P%d: ", i);

        for (int j = 0; j < m; j++)
        {
            char temp[24];

            snprintf(temp, sizeof(temp), "%d ", need[i][j]);

            strncat(line, temp,
                    sizeof(line) - strlen(line) - 1);
        }

        log_message("%s", line);
    }
}

/* 6. Display or refresh the Resource Allocation Graph. */
static void on_display_graph(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    gtk_widget_queue_draw(graph_area);

    log_message("Resource Allocation Graph refreshed.");
}

/* 7. Request resources. */
static void on_request(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0 || m <= 0)
    {
        log_message("Create processes and resources first.");
        return;
    }

    int pid = ask_int(
        "Resource Request",
        "Enter the process ID:",
        0, n - 1, 0);

    if (pid == -1)
        return;

    if (!active[pid])
    {
        log_message("This process is inactive.");
        return;
    }

    int req[MAX_RESOURCES] = {0};

    for (int j = 0; j < m; j++)
    {
        char message[100];

        snprintf(message, sizeof(message),
                 "How many instances of R%d does P%d request?",
                 j, pid);

        req[j] = ask_int(
            "Resource Request", message,
            0, total[j], 0);

        if (req[j] == -1)
            return;
    }

    int result = request_resources(pid, req);

    switch (result)
    {
        case REQ_GRANTED:
            log_message(
                "Request granted. System remains SAFE.");
            break;

        case REQ_MUST_WAIT:
            log_message(
                "Insufficient available resources. P%d must wait.",
                pid);
            break;

        case REQ_UNSAFE:
            log_message(
                "Request denied because it would make the state unsafe.");
            break;

        case REQ_EXCEEDS_NEED:
            log_message(
                "Request exceeds the remaining need of P%d.", pid);
            break;

        default:
            log_message("Invalid resource request.");
            break;
    }

    show_state();
}

/* 8. Run Banker's Algorithm. */
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
        char line[256] = "SAFE SEQUENCE: ";

        for (int i = 0; i < n; i++)
        {
            if (sequence[i] >= 0)
            {
                char temp[20];

                snprintf(temp, sizeof(temp), "P%d ",
                         sequence[i]);

                strncat(line, temp,
                        sizeof(line) - strlen(line) - 1);
            }
        }

        log_message("%s", line);
    }
    else
    {
        log_message(
            "UNSAFE STATE: No complete safe sequence exists.");
    }

    show_state();
}

/* 9. Detect deadlock. */
static void on_detect(GtkButton *button, gpointer data)
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

    if (count == 0)
    {
        log_message("No deadlocked processes detected.");
        return;
    }

    log_message("Deadlocked processes:");

    for (int i = 0; i < n; i++)
    {
        if (deadlocked[i])
            log_message("P%d is potentially deadlocked.", i);
    }
}

/* 10. Recover from deadlock by terminating a process. */
static void on_recover(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    if (n <= 0)
    {
        log_message("Create processes first.");
        return;
    }

    int pid = ask_int(
        "Deadlock Recovery",
        "Enter the process ID to terminate:",
        0, n - 1, 0);

    if (pid == -1)
        return;

    if (recover_from_deadlock(pid) == REC_OK)
    {
        log_message(
            "P%d terminated. Its allocated resources were released.",
            pid);

        show_state();
    }
    else
    {
        log_message("Invalid or inactive process.");
    }
}

/* 11. Display system status. */
static void on_status(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    show_state();
}

/* 12. Exit the program. */
static void on_exit(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_main_quit();
}

/* Draw the graph area. */
static gboolean on_draw_graph(GtkWidget *widget,
                              cairo_t *cr,
                              gpointer data)
{
    (void)widget;
    (void)data;

    GtkAllocation area;
    gtk_widget_get_allocation(graph_area, &area);

    draw_resource_graph(cr, area.width, area.height);

    return FALSE;
}

/* Create a button and connect its action. */
static GtkWidget *make_button(const char *label,
                              GCallback callback)
{
    GtkWidget *button = gtk_button_new_with_label(label);

    g_signal_connect(button, "clicked", callback, NULL);

    return button;
}

/* Build the complete GUI. */
void run_gui(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Week 4 - Deadlock Simulation System");

    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 800);

    g_signal_connect(window, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *root =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    gtk_container_set_border_width(GTK_CONTAINER(root), 12);
    gtk_container_add(GTK_CONTAINER(window), root);

    GtkWidget *title =
        gtk_label_new("DEADLOCK SIMULATION SYSTEM");

    PangoAttrList *attributes = pango_attr_list_new();

    PangoAttribute *font_size = pango_attr_size_new(18 * PANGO_SCALE);
    pango_attr_list_insert(attributes, font_size);

    gtk_label_set_attributes(GTK_LABEL(title), attributes);
    pango_attr_list_unref(attributes);

    gtk_widget_set_halign(title, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(root), title,
                       FALSE, FALSE, 4);

    GtkWidget *subtitle =
        gtk_label_new(
            "Operating Systems Project | Resource Management and Deadlock Detection");

    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(root), subtitle,
                       FALSE, FALSE, 0);

    status_label = gtk_label_new("SYSTEM STATUS: Load or create data");

    gtk_widget_set_halign(status_label, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(root), status_label,
                       FALSE, FALSE, 4);

    GtkWidget *buttons =
        gtk_grid_new();

    gtk_grid_set_row_spacing(GTK_GRID(buttons), 6);
    gtk_grid_set_column_spacing(GTK_GRID(buttons), 6);

    gtk_box_pack_start(GTK_BOX(root), buttons,
                       FALSE, FALSE, 4);

    const char *labels[] = {
        "1. Create Processes",
        "2. Create Resources",
        "3. Enter Allocation Matrix",
        "4. Enter Maximum Matrix",
        "5. Display Need Matrix",
        "6. Display Resource Graph",
        "7. Request Resources",
        "8. Run Banker's Algorithm",
        "9. Detect Deadlock",
        "10. Recover from Deadlock",
        "11. Display System Status",
        "12. Exit"
    };

    GCallback callbacks[] = {
        G_CALLBACK(on_create_processes),
        G_CALLBACK(on_create_resources),
        G_CALLBACK(on_enter_allocation),
        G_CALLBACK(on_enter_maximum),
        G_CALLBACK(on_display_need),
        G_CALLBACK(on_display_graph),
        G_CALLBACK(on_request),
        G_CALLBACK(on_banker),
        G_CALLBACK(on_detect),
        G_CALLBACK(on_recover),
        G_CALLBACK(on_status),
        G_CALLBACK(on_exit)
    };

    for (int i = 0; i < 12; i++)
    {
        GtkWidget *button =
            make_button(labels[i], callbacks[i]);

        gtk_widget_set_size_request(button, 200, 38);

        gtk_grid_attach(GTK_GRID(buttons),
                        button, i % 3, i / 3, 1, 1);
    }

    GtkWidget *paned =
        gtk_paned_new(GTK_ORIENTATION_VERTICAL);

    gtk_box_pack_start(GTK_BOX(root), paned,
                       TRUE, TRUE, 4);

    GtkWidget *scroll =
        gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);

    output = gtk_text_view_new();

    gtk_text_view_set_editable(GTK_TEXT_VIEW(output), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(output), TRUE);

    gtk_container_add(GTK_CONTAINER(scroll), output);

    gtk_widget_set_size_request(scroll, -1, 220);

    gtk_paned_pack1(GTK_PANED(paned), scroll, TRUE, FALSE);

    graph_area = gtk_drawing_area_new();

    gtk_widget_set_size_request(graph_area, 700, 260);

    g_signal_connect(graph_area, "draw",
                     G_CALLBACK(on_draw_graph), NULL);

    gtk_paned_pack2(GTK_PANED(paned), graph_area, TRUE, FALSE);

    gtk_widget_show_all(window);

    load_sample_data();

    log_message("Welcome to the Deadlock Simulation System.");
    log_message("Sample data loaded. You can use all 12 operations.");

    show_state();

    gtk_main();
}
