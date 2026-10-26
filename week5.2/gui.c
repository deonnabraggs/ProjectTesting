#include <gtk/gtk.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "page_replacement.h"

static GtkWidget *page_entry;
static GtkWidget *frame_spin;
static GtkWidget *algorithm_combo;
static GtkWidget *output_view;
static GtkWidget *status_label;

typedef void (*AlgorithmFunction)(
    const int[], int, int, SimulationResult *
);

static void set_status(const char *message)
{
    gtk_label_set_text(GTK_LABEL(status_label), message);
}

static void clear_report(void)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));

    gtk_text_buffer_set_text(buffer, "", -1);
}

static int parse_pages(const char *input, int pages[], int *count)
{
    const char *p = input;
    int n = 0;

    while (*p) {
        while (isspace((unsigned char)*p) ||
               *p == ',' || *p == ';') {
            p++;
        }

        if (*p == '\0')
            break;

        if (!isdigit((unsigned char)*p))
            return 0;

        errno = 0;
        char *end = NULL;
        long value = strtol(p, &end, 10);

        if (errno == ERANGE || end == p ||
            value < 0 || value > 2147483647L) {
            return 0;
        }

        if (n >= MAX_REQUESTS)
            return -1;

        pages[n++] = (int)value;
        p = end;

        if (*p != '\0' &&
            !isspace((unsigned char)*p) &&
            *p != ',' && *p != ';') {
            return 0;
        }
    }

    if (n == 0)
        return 0;

    *count = n;
    return 1;
}

static void append_algorithm_result(
    GString *report,
    const char *name,
    AlgorithmFunction algorithm,
    const int pages[],
    int n,
    int frame_count)
{
    SimulationResult result = {0};

    algorithm(pages, n, frame_count, &result);

    g_string_append_printf(
        report,
        "\n%s\n"
        "------------------------------------------------------------\n",
        name
    );

    g_string_append(report, "Page |");

    for (int f = 0; f < frame_count; f++)
        g_string_append_printf(report, " F%-3d|", f + 1);

    g_string_append(report, " Status\n");

    for (int i = 0; i < n; i++) {
        g_string_append_printf(report, "%4d |", pages[i]);

        for (int f = 0; f < frame_count; f++) {
            int page = result.frames[i][f];

            if (page == -1)
                g_string_append(report, "  -  |");
            else
                g_string_append_printf(report, " %-4d|", page);
        }

        g_string_append_printf(
            report, " %s\n",
            result.status[i] == PAGE_HIT ? "HIT" : "FAULT"
        );
    }

    g_string_append_printf(
        report,
        "\nTotal page requests : %d"
        "\nPage hits           : %d"
        "\nPage faults         : %d"
        "\nHit ratio           : %.2f%%"
        "\nFault ratio         : %.2f%%\n",
        n,
        result.total_hits,
        result.total_faults,
        100.0 * result.total_hits / n,
        100.0 * result.total_faults / n
    );
}

static void append_comparison(
    GString *report,
    const int pages[],
    int n,
    int frame_count)
{
    const char *names[] = {"FIFO", "LRU", "OPTIMAL"};

    AlgorithmFunction algorithms[] = {fifo, lru, optimal};

    g_string_append(
        report,
        "\n\nCOMPARISON OF ALL ALGORITHMS\n"
        "====================================================================\n"
        "Algorithm | Hits | Faults | Hit Ratio | Fault Ratio\n"
        "--------------------------------------------------------------------\n"
    );

    for (int a = 0; a < 3; a++) {
        SimulationResult result = {0};

        algorithms[a](pages, n, frame_count, &result);

        g_string_append_printf(
            report,
            "%-9s | %4d | %6d | %8.2f%% | %10.2f%%\n",
            names[a],
            result.total_hits,
            result.total_faults,
            100.0 * result.total_hits / n,
            100.0 * result.total_faults / n
        );
    }

    g_string_append(
        report,
        "\nHits = requested page was already in memory."
        "\nFault = requested page had to be loaded into memory.\n"
    );
}

static void on_run_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    int pages[MAX_REQUESTS];
    int n = 0;

    int parsed = parse_pages(
        gtk_entry_get_text(GTK_ENTRY(page_entry)),
        pages,
        &n
    );

    if (parsed != 1) {
        set_status(
            parsed == -1
            ? "Too many requests. Maximum allowed is 100."
            : "Enter non-negative whole page numbers separated by spaces or commas."
        );
        return;
    }

    int frame_count = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(frame_spin)
    );

    if (frame_count < 1 || frame_count > MAX_FRAMES) {
        set_status("Frame count is outside the allowed range.");
        return;
    }

    /* 0 = FIFO, 1 = LRU, 2 = Optimal, 3 = Compare all. */
    int choice = gtk_combo_box_get_active(
        GTK_COMBO_BOX(algorithm_combo)
    );

    if (choice < 0 || choice > 3) {
        set_status("Select FIFO, LRU, Optimal, or Compare all.");
        return;
    }

    GString *report = g_string_new(NULL);

    g_string_append(
        report,
        "MEMORY LAB  |  PAGE REPLACEMENT SIMULATION\n"
        "============================================================\n"
    );

    g_string_append_printf(
        report,
        "Page reference string: %s\n",
        gtk_entry_get_text(GTK_ENTRY(page_entry))
    );

    g_string_append_printf(
        report, "Number of page requests: %d\n", n
    );

    g_string_append_printf(
        report, "Number of frames: %d\n", frame_count
    );

    if (choice == 0) {
        append_algorithm_result(
            report, "FIFO — FIRST IN, FIRST OUT",
            fifo, pages, n, frame_count
        );
    }
    else if (choice == 1) {
        append_algorithm_result(
            report, "LRU — LEAST RECENTLY USED",
            lru, pages, n, frame_count
        );
    }
    else if (choice == 2) {
        append_algorithm_result(
            report, "OPTIMAL PAGE REPLACEMENT",
            optimal, pages, n, frame_count
        );
    }
    else {
        append_algorithm_result(
            report, "FIFO — FIRST IN, FIRST OUT",
            fifo, pages, n, frame_count
        );

        append_algorithm_result(
            report, "LRU — LEAST RECENTLY USED",
            lru, pages, n, frame_count
        );

        append_algorithm_result(
            report, "OPTIMAL PAGE REPLACEMENT",
            optimal, pages, n, frame_count
        );

        append_comparison(report, pages, n, frame_count);
    }

    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));

    gtk_text_buffer_set_text(buffer, report->str, -1);

    g_string_free(report, TRUE);

    set_status(
        choice == 3
        ? "Comparison completed: FIFO, LRU, and Optimal."
        : "Simulation completed. Choose another algorithm to compare."
    );
}

static void on_load_demo_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    gtk_entry_set_text(
        GTK_ENTRY(page_entry), "7 0 1 2 0 3 0 4 2 3"
    );

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(algorithm_combo), 3
    );

    clear_report();

    set_status("Demo loaded. You can edit the sample before running.");
}

/* Clear everything: inputs, selection, report and status. */
static void on_clear_all_clicked(GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    gtk_entry_set_text(GTK_ENTRY(page_entry), "");

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(algorithm_combo), -1
    );

    clear_report();

    set_status(
        "Everything cleared. Enter a new page string and choose an algorithm."
    );
}

/* Clear results and selection, keeping the inputs. */
static void on_clear_results_clicked(
    GtkButton *button, gpointer user_data)
{
    (void)button;
    (void)user_data;

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(algorithm_combo), -1
    );

    clear_report();

    set_status(
        "Results and algorithm choice cleared. Inputs are unchanged."
    );
}

static void add_css(void)
{
    const char *css =
        "window {"
        " background: #101827;"
        " color: #eef2ff;"
        "}"

        "window label {"
        " color: #e5eaf5;"
        " font-size: 16px;"
        "}"

        "window label.title {"
        " font-size: 28px;"
        " font-weight: bold;"
        " color: #ffffff;"
        "}"

        "window label.subtitle {"
        " color: #aab8d0;"
        " font-size: 14px;"
        "}"

        ".section {"
        " background: #1b2940;"
        " border-radius: 12px;"
        " padding: 14px;"
        "}"

        "entry, spinbutton, combobox, combobox box, combobox button {"
        " background: #111c2e;"
        " background-image: none;"
        " color: #f8fafc;"
        " border: 1px solid #3b4d68;"
        " border-radius: 7px;"
        " padding: 9px;"
        " font-size: 16px;"
        "}"

        "combobox *, spinbutton * {"
        " color: #f8fafc;"
        " font-size: 16px;"
        "}"

        "button {"
        " background: #263852;"
        " background-image: none;"
        " color: #ffffff;"
        " border-radius: 8px;"
        " padding: 10px 12px;"
        " border: 1px solid #405575;"
        " font-size: 15px;"
        "}"

        "button:hover {"
        " background: #344b6c;"
        " background-image: none;"
        "}"

        "button.run {"
        " background: #6258e8;"
        " background-image: none;"
        " color: #ffffff;"
        " font-size: 16px;"
        " font-weight: bold;"
        "}"

        "button.run:hover {"
        " background: #766df3;"
        " background-image: none;"
        "}"

        "button.danger {"
        " background: #703b4b;"
        " background-image: none;"
        "}"

        "textview, textview text {"
        " background: #111c2e;"
        " background-image: none;"
        " color: #f8fafc;"
        " font-family: monospace;"
        " font-size: 16px;"
        "}"

        "scrolledwindow {"
        " background: #111c2e;"
        "}"

        "frame > label {"
        " color: #e5eaf5;"
        " font-size: 16px;"
        "}";

    GtkCssProvider *provider = gtk_css_provider_new();

    gtk_css_provider_load_from_data(
        provider, css, -1, NULL
    );

    GdkScreen *screen = gdk_screen_get_default();

    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    g_object_unref(provider);
}

static GtkWidget *make_label(
    const char *text, const char *style_class)
{
    GtkWidget *label = gtk_label_new(text);

    gtk_widget_set_halign(label, GTK_ALIGN_START);

    if (style_class) {
        gtk_style_context_add_class(
            gtk_widget_get_style_context(label),
            style_class
        );
    }

    return label;
}

GtkWidget *create_main_window(void)
{
    add_css();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Memory Lab — Page Replacement Simulator"
    );

    gtk_window_set_default_size(GTK_WINDOW(window), 1120, 780);

    gtk_window_set_position(
        GTK_WINDOW(window), GTK_WIN_POS_CENTER
    );

    GtkWidget *outer =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    gtk_container_set_border_width(
        GTK_CONTAINER(outer), 20
    );

    gtk_container_add(GTK_CONTAINER(window), outer);

    /* Header */
    GtkWidget *header =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);

    gtk_box_pack_start(
        GTK_BOX(outer), header, FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(header),
        make_label("MEMORY LAB", "title"),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(header),
        make_label(
            "PAGE REPLACEMENT  /  OPERATING SYSTEMS LAB",
            "subtitle"
        ),
        FALSE, FALSE, 0
    );

    /* Input section */
    GtkWidget *input_card =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 9);

    gtk_style_context_add_class(
        gtk_widget_get_style_context(input_card),
        "section"
    );

    gtk_box_pack_start(
        GTK_BOX(outer), input_card, FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(input_card),
        make_label("SIMULATION SETUP", NULL),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(input_card),
        make_label("Page reference string", NULL),
        FALSE, FALSE, 0
    );

    page_entry = gtk_entry_new();

    gtk_entry_set_text(
        GTK_ENTRY(page_entry),
        "7 0 1 2 0 3 0 4 2 3"
    );

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(page_entry),
        "Example: 7 0 1 2 0 3 0 4 2 3"
    );

    gtk_box_pack_start(
        GTK_BOX(input_card), page_entry, FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(input_card),
        make_label("Number of frames", NULL),
        FALSE, FALSE, 0
    );

    frame_spin = gtk_spin_button_new_with_range(
        1, MAX_FRAMES, 1
    );

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(frame_spin), 3
    );

    gtk_box_pack_start(
        GTK_BOX(input_card), frame_spin, FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(input_card),
        make_label("Select the algorithm (1–4)", NULL),
        FALSE, FALSE, 0
    );

    algorithm_combo = gtk_combo_box_text_new();

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "1. FIFO"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "2. LRU"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "3. Optimal"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo),
        "4. Compare all algorithms"
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(algorithm_combo), 3
    );

    gtk_box_pack_start(
        GTK_BOX(input_card),
        algorithm_combo,
        FALSE, FALSE, 0
    );

    /* Buttons */
    GtkWidget *buttons =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    gtk_box_pack_start(
        GTK_BOX(input_card), buttons, FALSE, FALSE, 3
    );

    GtkWidget *run_button =
        gtk_button_new_with_label("▶  RUN SIMULATION");

    GtkWidget *demo_button =
        gtk_button_new_with_label("Load Demo Example");

    GtkWidget *clear_results_button =
        gtk_button_new_with_label("Clear Results / Choice");

    GtkWidget *clear_all_button =
        gtk_button_new_with_label("Clear Everything");

    gtk_style_context_add_class(
        gtk_widget_get_style_context(run_button), "run"
    );

    gtk_style_context_add_class(
        gtk_widget_get_style_context(clear_all_button),
        "danger"
    );

    gtk_box_pack_start(
        GTK_BOX(buttons), run_button, TRUE, TRUE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(buttons), demo_button, TRUE, TRUE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(buttons), clear_results_button, TRUE, TRUE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(buttons), clear_all_button, TRUE, TRUE, 0
    );

    g_signal_connect(
        run_button, "clicked",
        G_CALLBACK(on_run_clicked), NULL
    );

    g_signal_connect(
        demo_button, "clicked",
        G_CALLBACK(on_load_demo_clicked), NULL
    );

    g_signal_connect(
        clear_results_button, "clicked",
        G_CALLBACK(on_clear_results_clicked), NULL
    );

    g_signal_connect(
        clear_all_button, "clicked",
        G_CALLBACK(on_clear_all_clicked), NULL
    );

    /* Status message */
    status_label = make_label(
        "Sample input is ready. Select an algorithm and run the simulation.",
        NULL
    );

    gtk_box_pack_start(
        GTK_BOX(outer), status_label, FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(outer),
        make_label("SIMULATION REPORT", NULL),
        FALSE, FALSE, 0
    );

    /* Output area */
    GtkWidget *scroll =
        gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );

    gtk_widget_set_vexpand(scroll, TRUE);

    gtk_box_pack_start(
        GTK_BOX(outer), scroll, TRUE, TRUE, 0
    );

    output_view = gtk_text_view_new();

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(output_view), FALSE
    );

    gtk_text_view_set_cursor_visible(
        GTK_TEXT_VIEW(output_view), FALSE
    );

    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(output_view), TRUE
    );

    gtk_text_view_set_wrap_mode(
        GTK_TEXT_VIEW(output_view), GTK_WRAP_NONE
    );

    gtk_container_add(
        GTK_CONTAINER(scroll), output_view
    );

    g_signal_connect(
        window, "destroy",
        G_CALLBACK(gtk_main_quit), NULL
    );

    return window;
}
