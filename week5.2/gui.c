
#include "page_replacement.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

static GtkWidget *page_entry;
static GtkWidget *frame_spin;
static GtkWidget *algorithm_combo;
static GtkWidget *output_view;
static GtkWidget *hits_label;
static GtkWidget *faults_label;
static GtkWidget *rate_label;

static void setup_css(void)
{
    GtkCssProvider *css = gtk_css_provider_new();

    const char *style =
        "window {"
        " background-color: #f3f1eb;"
        " font-family: 'DejaVu Sans';"
        " }"

        ".hero {"
        " background-color: #172b4d;"
        " color: white;"
        " border-radius: 14px;"
        " padding: 22px;"
        " }"

        ".hero-title {"
        " color: white;"
        " font-size: 30px;"
        " font-weight: bold;"
        " }"

        ".hero-subtitle {"
        " color: #e0e8f2;"
        " font-size: 16px;"
        " }"

        ".panel {"
        " background-color: white;"
        " border-radius: 12px;"
        " padding: 18px;"
        " }"

        ".section-title {"
        " color: #172b4d;"
        " font-size: 21px;"
        " font-weight: bold;"
        " }"

        "label { font-size: 16px; }"

        "entry, spinbutton, combobox {"
        " font-size: 18px;"
        " padding: 10px;"
        " }"

        "button {"
        " font-size: 17px;"
        " font-weight: bold;"
        " padding: 12px 18px;"
        " border-radius: 8px;"
        " }"

        ".run-button {"
        " background-color: #087f8c;"
        " color: white;"
        " }"

        ".sample-button {"
        " background-color: #e1f1e9;"
        " color: #176345;"
        " }"

        ".clear-button {"
        " background-color: #fae8e7;"
        " color: #9b3030;"
        " }"

        ".metric-title {"
        " color: #526176;"
        " font-size: 16px;"
        " }"

        ".metric-value {"
        " color: #087f8c;"
        " font-size: 30px;"
        " font-weight: bold;"
        " }"

        "textview {"
        " background-color: #ffffff;"
        " color: #172b4d;"
        " font-family: 'DejaVu Sans Mono';"
        " font-size: 17px;"
        " }";

    gtk_css_provider_load_from_data(css, style, -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(css);
}

static GtkWidget *make_metric(const char *title, GtkWidget **value_label)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *heading = gtk_label_new(title);
    GtkWidget *value = gtk_label_new("—");

    gtk_widget_set_halign(heading, GTK_ALIGN_START);
    gtk_widget_set_halign(value, GTK_ALIGN_START);

    gtk_style_context_add_class(
        gtk_widget_get_style_context(heading), "metric-title");

    gtk_style_context_add_class(
        gtk_widget_get_style_context(value), "metric-value");

    gtk_box_pack_start(GTK_BOX(box), heading, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), value, FALSE, FALSE, 0);

    *value_label = value;
    return box;
}

static void append_algorithm_result(
    GString *text,
    const char *name,
    const int pages[],
    int count,
    int frame_count,
    void (*algorithm)(const int[], int, int, SimulationResult *)
)
{
    SimulationResult result = {0};

    algorithm(pages, count, frame_count, &result);

    g_string_append_printf(
        text,
        "\n============================================================\n"
        "  %s RESULTS\n"
        "============================================================\n\n",
        name
    );

    g_string_append_printf(
        text,
        "%-10s", "Page:"
    );

    for (int i = 0; i < count; i++)
        g_string_append_printf(text, "%5d", pages[i]);

    g_string_append(text, "\n");

    for (int f = 0; f < frame_count; f++) {
        g_string_append_printf(text, "Frame %-4d", f + 1);

        for (int i = 0; i < count; i++) {
            int page = result.steps[i].frames[f];

            if (page < 0)
                g_string_append_printf(text, "%5s", "-");
            else
                g_string_append_printf(text, "%5d", page);
        }

        g_string_append(text, "\n");
    }

    g_string_append_printf(text, "%-10s", "Status:");

    for (int i = 0; i < count; i++) {
        if (result.steps[i].status == STATUS_HIT)
            g_string_append_printf(text, "%5s", "HIT");
        else
            g_string_append_printf(text, "%5s", "FAULT");
    }

    double hit_rate = count > 0
        ? (double)result.total_hits * 100.0 / count
        : 0.0;

    g_string_append_printf(
        text,
        "\n\nTotal Page Hits   : %d"
        "\nTotal Page Faults : %d"
        "\nHit Rate          : %.2f%%\n",
        result.total_hits,
        result.total_faults,
        hit_rate
    );
}

static void set_output(const char *text)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));

    gtk_text_buffer_set_text(buffer, text, -1);
}

static void clear_statistics(void)
{
    gtk_label_set_text(GTK_LABEL(hits_label), "—");
    gtk_label_set_text(GTK_LABEL(faults_label), "—");
    gtk_label_set_text(GTK_LABEL(rate_label), "—");
}

static void run_simulation(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    const char *input =
        gtk_entry_get_text(GTK_ENTRY(page_entry));

    int pages[MAX_PAGES];
    int count = 0;
    char error[256];

    if (parse_page_string(
            input, pages, &count, error, sizeof(error)) != 0) {
        GtkWidget *dialog = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "%s",
            error
        );

        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    int frame_count =
        gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(frame_spin));

    int choice =
        gtk_combo_box_get_active(
            GTK_COMBO_BOX(algorithm_combo));

    GString *report = g_string_new(
        "PAGE REPLACEMENT SIMULATION REPORT\n"
        "==================================\n\n"
    );

    g_string_append_printf(
        report,
        "Reference string: %s\n"
        "Number of pages : %d\n"
        "Number of frames: %d\n",
        input, count, frame_count
    );

    if (choice == 0) {
        append_algorithm_result(
            report, "FIFO", pages, count, frame_count, run_fifo);
    } else if (choice == 1) {
        append_algorithm_result(
            report, "LRU", pages, count, frame_count, run_lru);
    } else if (choice == 2) {
        append_algorithm_result(
            report, "OPTIMAL", pages, count, frame_count, run_optimal);
    } else {
        append_algorithm_result(
            report, "FIFO", pages, count, frame_count, run_fifo);

        append_algorithm_result(
            report, "LRU", pages, count, frame_count, run_lru);

        append_algorithm_result(
            report, "OPTIMAL", pages, count, frame_count, run_optimal);
    }

    set_output(report->str);

    if (choice >= 0 && choice <= 2) {
        SimulationResult result = {0};

        if (choice == 0)
            run_fifo(pages, count, frame_count, &result);
        else if (choice == 1)
            run_lru(pages, count, frame_count, &result);
        else
            run_optimal(pages, count, frame_count, &result);

        double rate = count > 0
            ? (double)result.total_hits * 100.0 / count
            : 0.0;

        char hits[32], faults[32], hit_rate[32];

        snprintf(hits, sizeof(hits), "%d", result.total_hits);
        snprintf(faults, sizeof(faults), "%d", result.total_faults);
        snprintf(hit_rate, sizeof(hit_rate), "%.2f%%", rate);

        gtk_label_set_text(GTK_LABEL(hits_label), hits);
        gtk_label_set_text(GTK_LABEL(faults_label), faults);
        gtk_label_set_text(GTK_LABEL(rate_label), hit_rate);
    } else {
        clear_statistics();
    }

    g_string_free(report, TRUE);
}

static void load_sample(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(
        GTK_ENTRY(page_entry), "7 0 1 2 0 3 0 4 2 3");

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(frame_spin), 3);
}

static void clear_results(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    set_output(
        "Your simulation results will appear here.\n\n"
        "Enter a page reference string, select an algorithm,\n"
        "and click RUN SIMULATION."
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(algorithm_combo), -1);

    clear_statistics();
}

static void clear_everything(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(GTK_ENTRY(page_entry), "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);

    clear_results(NULL, NULL);
}

GtkWidget *create_main_window(void)
{
    setup_css();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(window), "Page Replacement Simulator");

    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 850);
    gtk_window_maximize(GTK_WINDOW(window));

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_container_set_border_width(GTK_CONTAINER(outer), 18);
    gtk_container_add(GTK_CONTAINER(window), outer);

    GtkWidget *hero = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(hero), "hero");

    GtkWidget *title = gtk_label_new("PAGELAB");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(title), "hero-title");

    GtkWidget *subtitle = gtk_label_new(
        "Interactive Page Replacement Algorithm Simulator");
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(subtitle), "hero-subtitle");

    gtk_box_pack_start(GTK_BOX(hero), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hero), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(outer), hero, FALSE, FALSE, 0);

    GtkWidget *input_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(input_panel), "panel");

    GtkWidget *input_title = gtk_label_new("Simulation Settings");
    gtk_widget_set_halign(input_title, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(input_title), "section-title");

    gtk_box_pack_start(
        GTK_BOX(input_panel), input_title, FALSE, FALSE, 0);

    GtkWidget *input_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(input_grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(input_grid), 16);

    GtkWidget *page_label = gtk_label_new("Page Reference String");
    gtk_widget_set_halign(page_label, GTK_ALIGN_START);

    page_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(
        GTK_ENTRY(page_entry), "Example: 7 0 1 2 0 3 0 4");

    GtkWidget *frame_label = gtk_label_new("Number of Frames");
    gtk_widget_set_halign(frame_label, GTK_ALIGN_START);

    frame_spin = gtk_spin_button_new_with_range(1, MAX_FRAMES, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);

    GtkWidget *algo_label = gtk_label_new("Select Algorithm");
    gtk_widget_set_halign(algo_label, GTK_ALIGN_START);

    algorithm_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "Option 1 - FIFO");
    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "Option 2 - LRU");
    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "Option 3 - Optimal");
    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "Option 4 - Compare All");
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 0);

    gtk_grid_attach(GTK_GRID(input_grid), page_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), page_entry, 1, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(input_grid), frame_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), frame_spin, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), algo_label, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(input_grid), algorithm_combo, 3, 1, 1, 1);

    gtk_widget_set_hexpand(page_entry, TRUE);
    gtk_widget_set_hexpand(algorithm_combo, TRUE);

    gtk_box_pack_start(
        GTK_BOX(input_panel), input_grid, FALSE, FALSE, 0);

    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    GtkWidget *run_button = gtk_button_new_with_label("RUN SIMULATION");
    GtkWidget *sample_button = gtk_button_new_with_label("Load Sample");
    GtkWidget *clear_button = gtk_button_new_with_label("Clear Results / Choice");
    GtkWidget *clear_all_button = gtk_button_new_with_label("Clear Everything");

    gtk_style_context_add_class(
        gtk_widget_get_style_context(run_button), "run-button");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(sample_button), "sample-button");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(clear_button), "clear-button");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(clear_all_button), "clear-button");

    gtk_box_pack_start(GTK_BOX(buttons), run_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), sample_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), clear_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(buttons), clear_all_button, FALSE, FALSE, 0);

    gtk_box_pack_start(
        GTK_BOX(input_panel), buttons, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(outer), input_panel, FALSE, FALSE, 0);

    GtkWidget *metrics = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 14);

    GtkWidget *hits = make_metric("PAGE HITS", &hits_label);
    GtkWidget *faults = make_metric("PAGE FAULTS", &faults_label);
    GtkWidget *rate = make_metric("HIT RATE", &rate_label);

    gtk_style_context_add_class(
        gtk_widget_get_style_context(hits), "panel");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(faults), "panel");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(rate), "panel");

    gtk_box_pack_start(GTK_BOX(metrics), hits, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(metrics), faults, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(metrics), rate, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(outer), metrics, FALSE, FALSE, 0);

    GtkWidget *output_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(output_panel), "panel");

    GtkWidget *output_title = gtk_label_new("Simulation Output");
    gtk_widget_set_halign(output_title, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(output_title), "section-title");

    gtk_box_pack_start(
        GTK_BOX(output_panel), output_title, FALSE, FALSE, 0);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC);

    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_NONE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(output_view), 18);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(output_view), 18);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(output_view), 18);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(output_view), 18);

    gtk_container_add(GTK_CONTAINER(scroll), output_view);
    gtk_box_pack_start(GTK_BOX(output_panel), scroll, TRUE, TRUE, 0);

    gtk_widget_set_size_request(scroll, -1, 300);
    gtk_widget_set_vexpand(output_panel, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);

    gtk_box_pack_start(GTK_BOX(outer), output_panel, TRUE, TRUE, 0);

    set_output(
        "Your simulation results will appear here.\n\n"
        "Enter a page reference string, select an algorithm,\n"
        "and click RUN SIMULATION."
    );

    g_signal_connect(run_button, "clicked",
                     G_CALLBACK(run_simulation), NULL);
    g_signal_connect(sample_button, "clicked",
                     G_CALLBACK(load_sample), NULL);
    g_signal_connect(clear_button, "clicked",
                     G_CALLBACK(clear_results), NULL);
    g_signal_connect(clear_all_button, "clicked",
                     G_CALLBACK(clear_everything), NULL);

    return window;
}
