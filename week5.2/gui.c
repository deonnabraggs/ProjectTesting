#include "page_replacement.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

static GtkWidget *page_entry;
static GtkWidget *frame_spin;
static GtkWidget *algorithm_combo;
static GtkWidget *result_view;
static GtkWidget *status_label;
static GtkWidget *hits_value;
static GtkWidget *faults_value;
static GtkWidget *rate_value;

static const char *css =
"window { background: #f3f1eb; }"
".hero { background: #172b4d; border-radius: 18px; padding: 24px; }"
".brand { color: #ffffff; font-family: 'DejaVu Sans'; font-size: 26px; font-weight: 800; }"
".hero-sub { color: #d1dced; font-size: 12px; }"
".panel { background: #ffffff; border: 1px solid #dfe5ed; border-radius: 15px; padding: 17px; }"
".heading { color: #172b4d; font-size: 16px; font-weight: 800; }"
".muted { color: #667085; font-size: 12px; }"
".metric { background: #ffffff; border: 1px solid #dfe5ed; border-radius: 13px; padding: 14px; }"
".metric-label { color: #667085; font-size: 11px; font-weight: 700; }"
".metric-value { color: #087f8c; font-size: 26px; font-weight: 800; }"
"entry, spinbutton, combobox { background: #fbfcfe; color: #172b4d; border: 1px solid #cbd5e1; border-radius: 8px; padding: 9px; }"
"button { background: #e7edf5; color: #172b4d; border: 0; border-radius: 9px; padding: 10px 14px; font-weight: 700; }"
"button:hover { background: #d7e2ef; }"
"button.run { background: #087f8c; color: #ffffff; }"
"button.run:hover { background: #066b76; }"
"button.demo { background: #e4f5ef; color: #087f68; }"
"button.danger { background: #fff0ed; color: #b9382b; }"
"textview { background: #fbfcfe; color: #263449; font-family: 'DejaVu Sans Mono'; font-size: 11px; }"
".status { color: #087f68; font-weight: 700; }";

static void setup_css(void)
{
    GtkCssProvider *provider = gtk_css_provider_new();
    GError *error = NULL;

    gtk_css_provider_load_from_data(provider, css, -1, &error);

    if (error) {
        g_warning("CSS: %s", error->message);
        g_error_free(error);
    }

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

static GtkWidget *make_label(const char *text, const char *style)
{
    GtkWidget *label = gtk_label_new(text);

    gtk_widget_set_halign(label, GTK_ALIGN_START);

    if (style != NULL) {
        gtk_style_context_add_class(
            gtk_widget_get_style_context(label), style
        );
    }

    return label;
}

static void append_report(const char *text)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_view));

    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
}

static void clear_report(void)
{
    GtkTextBuffer *buffer =
        gtk_text_view_get_buffer(GTK_TEXT_VIEW(result_view));

    gtk_text_buffer_set_text(buffer, "", -1);
}

static GtkWidget *make_metric(
    const char *title,
    GtkWidget **value_label
)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 7);

    gtk_style_context_add_class(
        gtk_widget_get_style_context(box), "metric"
    );

    gtk_box_pack_start(
        GTK_BOX(box),
        make_label(title, "metric-label"),
        FALSE, FALSE, 0
    );

    *value_label = make_label("—", "metric-value");

    gtk_box_pack_start(
        GTK_BOX(box), *value_label, FALSE, FALSE, 0
    );

    return box;
}

static void update_metrics(int hits, int faults, int count)
{
    char value[40];

    snprintf(value, sizeof(value), "%d", hits);
    gtk_label_set_text(GTK_LABEL(hits_value), value);

    snprintf(value, sizeof(value), "%d", faults);
    gtk_label_set_text(GTK_LABEL(faults_value), value);

    if (count > 0) {
        snprintf(value, sizeof(value), "%.1f%%",
                 100.0 * hits / count);
    } else {
        snprintf(value, sizeof(value), "—");
    }

    gtk_label_set_text(GTK_LABEL(rate_value), value);
}

typedef void (*Algorithm)(
    const int[],
    int,
    int,
    SimulationResult *
);

static void display_algorithm(
    const char *name,
    const int pages[],
    int count,
    int frame_count,
    Algorithm algorithm,
    int *hits,
    int *faults
)
{
    SimulationResult result;
    memset(&result, 0, sizeof(result));

    algorithm(pages, count, frame_count, &result);

    char line[512];

    snprintf(
        line, sizeof(line),
        "\n%s\n"
        "------------------------------------------------------------\n"
        "Page   Frames                         Status\n",
        name
    );

    append_report(line);

    for (int i = 0; i < count; i++) {
        char frames[220] = "";
        char part[24];
        size_t used = 0;

        for (int f = 0; f < frame_count; f++) {
            if (result.steps[i].frames[f] == -1) {
                snprintf(part, sizeof(part), " - ");
            } else {
                snprintf(
                    part, sizeof(part), " %d ",
                    result.steps[i].frames[f]
                );
            }

            size_t length = strlen(part);

            if (used + length < sizeof(frames) - 1) {
                memcpy(frames + used, part, length + 1);
                used += length;
            }
        }

        snprintf(
            line, sizeof(line), "%-6d %-30s %s\n",
            pages[i],
            frames,
            result.steps[i].status == STATUS_HIT
                ? "HIT" : "FAULT"
        );

        append_report(line);
    }

    snprintf(
        line, sizeof(line),
        "Hits: %d    Faults: %d    Hit rate: %.1f%%\n",
        result.total_hits,
        result.total_faults,
        count > 0 ? 100.0 * result.total_hits / count : 0.0
    );

    append_report(line);

    *hits = result.total_hits;
    *faults = result.total_faults;
}

static void on_run_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    int pages[MAX_PAGES];
    int count = 0;
    char error[256] = "";

    const char *input =
        gtk_entry_get_text(GTK_ENTRY(page_entry));

    if (!parse_page_string(
            input, pages, &count, error, sizeof(error))) {
        gtk_label_set_text(
            GTK_LABEL(status_label),
            error[0] ? error : "Enter a valid page-reference string."
        );
        return;
    }

    int frame_count = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(frame_spin)
    );

    gchar *choice = gtk_combo_box_text_get_active_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo)
    );

    if (choice == NULL) {
        gtk_label_set_text(
            GTK_LABEL(status_label), "Select an algorithm first."
        );
        return;
    }

    clear_report();

    append_report(
        "PAGELAB  /  SIMULATION REPORT\n"
        "============================================================\n"
    );

    char line[300];

    snprintf(
        line, sizeof(line),
        "Reference string: %s\nFrame count: %d    Requests: %d\n",
        input, frame_count, count
    );

    append_report(line);

    int hits = 0;
    int faults = 0;

    if (strstr(choice, "FIFO") != NULL) {
        display_algorithm(
            "FIFO", pages, count, frame_count,
            run_fifo, &hits, &faults
        );
    } else if (strstr(choice, "LRU") != NULL) {
        display_algorithm(
            "LRU", pages, count, frame_count,
            run_lru, &hits, &faults
        );
    } else if (strstr(choice, "Optimal") != NULL) {
        display_algorithm(
            "OPTIMAL", pages, count, frame_count,
            run_optimal, &hits, &faults
        );
    } else {
        int h1, f1, h2, f2, h3, f3;

        display_algorithm(
            "FIFO", pages, count, frame_count,
            run_fifo, &h1, &f1
        );

        display_algorithm(
            "LRU", pages, count, frame_count,
            run_lru, &h2, &f2
        );

        display_algorithm(
            "OPTIMAL", pages, count, frame_count,
            run_optimal, &h3, &f3
        );

        hits = h1;
        faults = f1;

        append_report("\nCOMPARISON\n");

        snprintf(
            line, sizeof(line),
            "FIFO faults: %d    LRU faults: %d    Optimal faults: %d\n",
            f1, f2, f3
        );

        append_report(line);

        if (f3 <= f1 && f3 <= f2) {
            append_report("Best result: OPTIMAL\n");
        } else if (f2 <= f1) {
            append_report("Best result: LRU\n");
        } else {
            append_report("Best result: FIFO\n");
        }
    }

    update_metrics(hits, faults, count);

    gtk_label_set_text(
        GTK_LABEL(status_label),
        "Simulation complete  •  Output refreshed"
    );

    g_free(choice);
}

static void on_sample_clicked(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(
        GTK_ENTRY(page_entry), "7 0 1 2 0 3 0 4 2 3"
    );

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), 3);

    gtk_label_set_text(
        GTK_LABEL(status_label),
        "Sample loaded. Press Run Simulation."
    );
}

static void on_clear_selection_clicked(
    GtkButton *button, gpointer data
)
{
    (void)button;
    (void)data;

    clear_report();
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), -1);
    update_metrics(0, 0, 0);

    gtk_label_set_text(
        GTK_LABEL(status_label),
        "Results and selection cleared; inputs kept."
    );
}

static void on_clear_everything_clicked(
    GtkButton *button, gpointer data
)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(GTK_ENTRY(page_entry), "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);
    gtk_combo_box_set_active(GTK_COMBO_BOX(algorithm_combo), -1);

    clear_report();
    update_metrics(0, 0, 0);

    gtk_label_set_text(
        GTK_LABEL(status_label), "Everything cleared."
    );
}

GtkWidget *create_main_window(void)
{
    setup_css();

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(
        GTK_WINDOW(window),
        "PageLab | Page Replacement Simulator"
    );

    gtk_window_set_default_size(GTK_WINDOW(window), 1120, 820);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    g_signal_connect(
        window, "destroy", G_CALLBACK(gtk_main_quit), NULL
    );

    GtkWidget *outer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);

    gtk_container_set_border_width(GTK_CONTAINER(outer), 22);
    gtk_container_add(GTK_CONTAINER(window), outer);

    /* Header */
    GtkWidget *hero = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);

    gtk_style_context_add_class(
        gtk_widget_get_style_context(hero), "hero"
    );

    GtkWidget *brand_box =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    gtk_box_pack_start(
        GTK_BOX(brand_box),
        make_label("PAGELAB", "brand"),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(brand_box),
        make_label(
            "MEMORY MANAGEMENT  /  PAGE REPLACEMENT WORKBENCH",
            "hero-sub"
        ),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(hero), brand_box, TRUE, TRUE, 0
    );

    gtk_box_pack_end(
        GTK_BOX(hero),
        make_label("FIFO   •   LRU   •   OPTIMAL", "hero-sub"),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(GTK_BOX(outer), hero, FALSE, FALSE, 0);

    /* Introduction */
    gtk_box_pack_start(
        GTK_BOX(outer),
        make_label("Simulation workspace", "heading"),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(outer),
        make_label(
            "Configure your input, select a method, and inspect the frame-by-frame trace.",
            "muted"
        ),
        FALSE, FALSE, 0
    );

    /* Input panel */
    GtkWidget *form = gtk_grid_new();

    gtk_style_context_add_class(
        gtk_widget_get_style_context(form), "panel"
    );

    gtk_grid_set_row_spacing(GTK_GRID(form), 9);
    gtk_grid_set_column_spacing(GTK_GRID(form), 14);

    gtk_grid_attach(
        GTK_GRID(form),
        make_label("PAGE REFERENCE STRING", "metric-label"),
        0, 0, 2, 1
    );

    page_entry = gtk_entry_new();

    gtk_entry_set_placeholder_text(
        GTK_ENTRY(page_entry),
        "Example: 7 0 1 2 0 3 0 4 2 3"
    );

    gtk_entry_set_text(
        GTK_ENTRY(page_entry), "7 0 1 2 0 3 0 4 2 3"
    );

    gtk_grid_attach(GTK_GRID(form), page_entry, 0, 1, 2, 1);

    gtk_grid_attach(
        GTK_GRID(form),
        make_label("FRAME COUNT", "metric-label"),
        2, 0, 1, 1
    );

    frame_spin = gtk_spin_button_new_with_range(
        1, MAX_FRAMES, 1
    );

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);

    gtk_grid_attach(GTK_GRID(form), frame_spin, 2, 1, 1, 1);

    gtk_grid_attach(
        GTK_GRID(form),
        make_label("ALGORITHM", "metric-label"),
        3, 0, 1, 1
    );

    algorithm_combo = gtk_combo_box_text_new();

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "1  ·  FIFO"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "2  ·  LRU"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "3  ·  Optimal"
    );

    gtk_combo_box_text_append_text(
        GTK_COMBO_BOX_TEXT(algorithm_combo), "4  ·  Compare All"
    );

    gtk_combo_box_set_active(
        GTK_COMBO_BOX(algorithm_combo), 3
    );

    gtk_grid_attach(
        GTK_GRID(form), algorithm_combo, 3, 1, 1, 1
    );

    gtk_box_pack_start(GTK_BOX(outer), form, FALSE, FALSE, 0);

    /* Action buttons */
    GtkWidget *actions =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 9);

    GtkWidget *run_button =
        gtk_button_new_with_label("▶  Run Simulation");

    gtk_style_context_add_class(
        gtk_widget_get_style_context(run_button), "run"
    );

    g_signal_connect(
        run_button, "clicked", G_CALLBACK(on_run_clicked), NULL
    );

    GtkWidget *sample_button =
        gtk_button_new_with_label("Load Sample");

    gtk_style_context_add_class(
        gtk_widget_get_style_context(sample_button), "demo"
    );

    g_signal_connect(
        sample_button, "clicked", G_CALLBACK(on_sample_clicked), NULL
    );

    GtkWidget *clear_button =
        gtk_button_new_with_label("Clear Results / Choice");

    g_signal_connect(
        clear_button, "clicked",
        G_CALLBACK(on_clear_selection_clicked), NULL
    );

    GtkWidget *clear_all_button =
        gtk_button_new_with_label("Clear Everything");

    gtk_style_context_add_class(
        gtk_widget_get_style_context(clear_all_button), "danger"
    );

    g_signal_connect(
        clear_all_button, "clicked",
        G_CALLBACK(on_clear_everything_clicked), NULL
    );

    gtk_box_pack_start(
        GTK_BOX(actions), run_button, FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(actions), sample_button, FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(actions), clear_button, FALSE, FALSE, 0
    );

    gtk_box_pack_end(
        GTK_BOX(actions), clear_all_button, FALSE, FALSE, 0
    );

    gtk_box_pack_start(GTK_BOX(outer), actions, FALSE, FALSE, 0);

    /* Statistics cards */
    GtkWidget *stats = gtk_grid_new();

    gtk_grid_set_column_spacing(GTK_GRID(stats), 12);

    gtk_grid_attach(
        GTK_GRID(stats),
        make_metric("PAGE HITS", &hits_value),
        0, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(stats),
        make_metric("PAGE FAULTS", &faults_value),
        1, 0, 1, 1
    );

    gtk_grid_attach(
        GTK_GRID(stats),
        make_metric("HIT RATE", &rate_value),
        2, 0, 1, 1
    );

    gtk_box_pack_start(GTK_BOX(outer), stats, FALSE, FALSE, 0);

    /* Results panel */
    GtkWidget *output =
        gtk_box_new(GTK_ORIENTATION_VERTICAL, 9);

    gtk_style_context_add_class(
        gtk_widget_get_style_context(output), "panel"
    );

    GtkWidget *output_header =
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    gtk_box_pack_start(
        GTK_BOX(output_header),
        make_label("SIMULATION OUTPUT", "heading"),
        TRUE, TRUE, 0
    );

    gtk_box_pack_end(
        GTK_BOX(output_header),
        make_label("Detailed frame-by-frame trace", "muted"),
        FALSE, FALSE, 0
    );

    gtk_box_pack_start(
        GTK_BOX(output), output_header, FALSE, FALSE, 0
    );

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC,
        GTK_POLICY_AUTOMATIC
    );

    gtk_widget_set_size_request(scroll, -1, 220);

    result_view = gtk_text_view_new();

    gtk_text_view_set_editable(
        GTK_TEXT_VIEW(result_view), FALSE
    );

    gtk_text_view_set_cursor_visible(
        GTK_TEXT_VIEW(result_view), FALSE
    );

    gtk_text_view_set_monospace(
        GTK_TEXT_VIEW(result_view), TRUE
    );

    gtk_text_view_set_left_margin(
        GTK_TEXT_VIEW(result_view), 12
    );

    gtk_text_view_set_right_margin(
        GTK_TEXT_VIEW(result_view), 12
    );

    gtk_text_view_set_top_margin(
        GTK_TEXT_VIEW(result_view), 12
    );

    gtk_container_add(GTK_CONTAINER(scroll), result_view);

    gtk_box_pack_start(
        GTK_BOX(output), scroll, TRUE, TRUE, 0
    );

    gtk_box_pack_start(GTK_BOX(outer), output, TRUE, TRUE, 0);

    status_label = make_label(
        "Ready  •  Sample input is loaded", "status"
    );

    gtk_box_pack_start(
        GTK_BOX(outer), status_label, FALSE, FALSE, 0
    );

    update_metrics(0, 0, 0);

    return window;
}
