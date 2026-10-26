#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "page_replacement.h"

static GtkWidget *page_entry;
static GtkWidget *frame_spin;
static GtkWidget *algorithm_combo;
static GtkWidget *output_view;
static GtkWidget *status_label;

typedef void (*AlgorithmFunction)(
    const int [], int, int, SimulationResult *
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

static void append_result(
    GString *report,
    const char *name,
    AlgorithmFunction algorithm,
    const int pages[],
    int count,
    int frame_count
)
{
    SimulationResult result = {0};

    algorithm(pages, count, frame_count, &result);

    g_string_append_printf(
        report,
        "\n%s\n"
        "================================================================\n",
        name
    );

    g_string_append(report, "Page |");

    for (int f = 0; f < frame_count; f++)
        g_string_append_printf(report, " F%-3d|", f + 1);

    g_string_append(report, " Result\n");

    for (int i = 0; i < count; i++) {
        g_string_append_printf(report, "%4d |", pages[i]);

        for (int f = 0; f < frame_count; f++) {
            int value = result.steps[i].frames[f];

            if (value == -1)
                g_string_append(report, "  -  |");
            else
                g_string_append_printf(report, " %-4d|", value);
        }

        g_string_append_printf(
            report, " %s\n",
            result.steps[i].status == STATUS_HIT ? "HIT" : "FAULT"
        );
    }

    g_string_append_printf(
        report,
        "\nPage requests : %d"
        "\nPage hits     : %d"
        "\nPage faults   : %d"
        "\nHit ratio     : %.2f%%"
        "\nFault ratio   : %.2f%%\n",
        count,
        result.total_hits,
        result.total_faults,
        100.0 * result.total_hits / count,
        100.0 * result.total_faults / count
    );
}

static void append_comparison(
    GString *report,
    const int pages[],
    int count,
    int frame_count
)
{
    const char *names[] = {"FIFO", "LRU", "OPTIMAL"};

    AlgorithmFunction algorithms[] = {
        run_fifo, run_lru, run_optimal
    };

    g_string_append(
        report,
        "\n\nCOMPARISON OF ALL ALGORITHMS\n"
        "===============================================================\n"
        "Algorithm | Hits | Faults | Hit ratio | Fault ratio\n"
        "---------------------------------------------------------------\n"
    );

    for (int i = 0; i < 3; i++) {
        SimulationResult result = {0};

        algorithms[i](pages, count, frame_count, &result);

        g_string_append_printf(
            report,
            "%-9s | %4d | %6d | %8.2f%% | %10.2f%%\n",
            names[i],
            result.total_hits,
            result.total_faults,
            100.0 * result.total_hits / count,
            100.0 * result.total_faults / count
        );
    }
}

static void on_run_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int pages[MAX_PAGES];
    int count = 0;
    char error[200];

    if (!parse_page_string(
            gtk_entry_get_text(GTK_ENTRY(page_entry)),
            pages, &count, error, sizeof(error))) {
        set_status(error);
        return;
    }

    int frames = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(frame_spin)
    );

    int choice = gtk_combo_box_get_active(
        GTK_COMBO_BOX(algorithm_combo)
    );

    if (choice < 0 || choice > 3) {
        set_status("Please choose option 1, 2, 3, or 4.");
        return;
    }

    GString *report = g_string_new(NULL);

    g_string_append(
        report,
        "PAGE REPLACEMENT SIMULATOR\n"
        "===============================================================\n"
    );

    g_string_append_printf(
        report,
        "Reference string: %s\n"
        "Requests: %d\n"
        "Frames: %d\n",
        gtk_entry_get_text(GTK_ENTRY(page_entry)),
        count,
        frames
    );

    if (choice == 0) {
        append_result(report, "FIFO - FIRST IN, FIRST OUT",
                      run_fifo, pages, count, frames);
    } else if (choice == 1) {
        append_result(report, "LRU - LEAST RECENTLY USED",
                      run_lru, pages, count, frames);
    } else if (choice == 2) {
        append_result(report, "OPTIMAL PAGE REPLACEMENT",
                      run_optimal, pages, count, frames);
    } else {
        append_result(report, "FIFO - FIRST IN, FIRST OUT",
                      run_fifo, pages, count, frames);
        append_result(report, "LRU - LEAST RECENTLY USED",
                      run_lru, pages, count, frames);
        append_result(report, "OPTIMAL PAGE REPLACEMENT",
                      run_optimal, pages, count, frames);
        append_comparison(report, pages, count, frames);
    }

    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));

    gtk_text_buffer_set_text(buffer, report->str, -1);
    g_string_free(report, TRUE);

    set_status("Simulation completed successfully.");
}

static void on_demo_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(
        GTK_ENTRY(page_entry),
        "7 0 1 2 0 3 0 4 2 3"
    );

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 3);

    clear_report();
    set_status("Demo loaded. Click Run Simulation.");
}

static void on_clear_results_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    /* Keep the page string and frame count unchanged. */
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), -1);

    clear_report();
    set_status("Results and algorithm choice cleared.");
}

static void on_clear_all_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(GTK_ENTRY(page_entry), "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), -1);

    clear_report();
    set_status("Everything cleared.");
}

static void add_css(void)
{
    const char *css =
        "window { background: #101827; color: #eef2ff; }"
        "label { color: #e5eaf5; }"
        ".title { font-size: 25px; font-weight: bold; color: white; }"
        ".subtitle { color: #aab8d0; font-size: 11px; }"
        ".section { background: #1b2940; border-radius: 12px; padding: 14px; }"
        "entry, spinbutton, combobox, textview {"
        " background: #111c2e; color: #f8fafc;"
        " border: 1px solid #3b4d68; border-radius: 7px; padding: 8px; }"
        "button { background: #263852; color: white;"
        " border-radius: 8px; padding: 9px; }"
        "button:hover { background: #344b6c; }"
        "button.run { background: #6258e8; font-weight: bold; }"
        "button.danger { background: #703b4b; }"
        "textview { font-family: monospace; font-size: 11px; }";

    GtkCssProvider *provider = gtk_css_provider_new();

    gtk_css_provider_load_from_data(provider, css, -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

static GtkWidget *make_label(const char *text, const char *style_class)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    if (style_class != NULL)
        gtk_style_context_add_class(
            gtk_widget_get_style_context(label), style_class
        );

    return label;
}

GtkWidget *create_main_window(void)
{
    add_css();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "Page Replacement Simulator"
    );

    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 760);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(outer), 18);
    gtk_container_add(GTK_CONTAINER(window), outer);

    gtk_box_pack_start(
        GTK_BOX(outer),
        make_label("MEMORY LAB", "title"),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(outer),
        make_label("PAGE REPLACEMENT / OPERATING SYSTEMS LAB", "subtitle"),
        FALSE, FALSE, 0
    );

    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(card), "section"
    );
    gtk_box_pack_start(GTK_BOX(outer), card, FALSE, FALSE, 0);

    gtk_box_pack_start(
        GTK_BOX(card), make_label("SIMULATION SETUP", NULL),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(card), make_label("Page reference string", NULL),
        FALSE, FALSE, 0
    );

    page_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(page_entry),
        "Example: 7 0 1 2 0 3 0 4 2 3"
    );
    gtk_box_pack_start(GTK_BOX(card), page_entry, FALSE, FALSE, 0);

    gtk_box_pack_start(
        GTK_BOX(card), make_label("Number of frames", NULL),
        FALSE, FALSE, 0
    );

    frame_spin = gtk_spin_button_new_with_range(1, MAX_FRAMES, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);
    gtk_box_pack_start(GTK_BOX(card), frame_spin, FALSE, FALSE, 0);

    gtk_box_pack_start(
        GTK_BOX(card), make_label("Select algorithm (1-4)", NULL),
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
        GTK_COMBO_BOX_TEXT(algorithm_combo), "4. Compare All Algorithms"
    );
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), -1);
    gtk_box_pack_start(GTK_BOX(card), algorithm_combo, FALSE, FALSE, 0);

    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_pack_start(GTK_BOX(card), buttons, FALSE, FALSE, 0);

    GtkWidget *run_button = gtk_button_new_with_label("RUN SIMULATION");
    GtkWidget *demo_button = gtk_button_new_with_label("Load Demo Example");
    GtkWidget *clear_results_button =
        gtk_button_new_with_label("Clear Results / Choice");
    GtkWidget *clear_all_button =
        gtk_button_new_with_label("Clear Everything");

    gtk_style_context_add_class(
        gtk_widget_get_style_context(run_button), "run"
    );
    gtk_style_context_add_class(
        gtk_widget_get_style_context(clear_all_button), "danger"
    );

    gtk_box_pack_start(GTK_BOX(buttons), run_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), demo_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), clear_results_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), clear_all_button, TRUE, TRUE, 0);

    g_signal_connect(run_button, "clicked",
                     G_CALLBACK(on_run_clicked), NULL);
    g_signal_connect(demo_button, "clicked",
                     G_CALLBACK(on_demo_clicked), NULL);
    g_signal_connect(clear_results_button, "clicked",
                     G_CALLBACK(on_clear_results_clicked), NULL);
    g_signal_connect(clear_all_button, "clicked",
                     G_CALLBACK(on_clear_all_clicked), NULL);

    status_label = gtk_label_new("Ready. Enter pages and choose an algorithm.");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(outer), status_label, FALSE, FALSE, 0);

    GtkWidget *output_frame = gtk_frame_new("SIMULATION RESULTS");
    gtk_box_pack_start(GTK_BOX(outer), output_frame, TRUE, TRUE, 0);

    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(output_view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_NONE);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );

    gtk_container_add(GTK_CONTAINER(scroll), output_view);
    gtk_container_add(GTK_CONTAINER(output_frame), scroll);

    return window;
}
