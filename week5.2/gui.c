#include "replacement.h"
#include <gtk/gtk.h>
#include <stdio.h>

static GtkWidget *page_entry, *frame_spin;
static GtkWidget *output_view, *status_label;
static GtkWidget *fifo_summary, *lru_summary, *optimal_summary;

static void set_text(GtkWidget *widget, const char *text)
{
    gtk_label_set_text(GTK_LABEL(widget), text);
}

static GtkWidget *make_card(const char *title, GtkWidget **summary)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *heading = gtk_label_new(title);

    *summary = gtk_label_new("Hits: —   Faults: —");

    gtk_widget_set_halign(heading, GTK_ALIGN_START);
    gtk_widget_set_halign(*summary, GTK_ALIGN_START);

    gtk_box_pack_start(GTK_BOX(box), heading, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), *summary, FALSE, FALSE, 0);

    gtk_style_context_add_class(
        gtk_widget_get_style_context(box), "card");

    return box;
}

static void add_result(GString *report, const SimulationResult *r)
{
    int i, j;

    g_string_append_printf(
        report,
        "\n%s   |   Hits: %d   |   Faults: %d   |   Hit ratio: %.1f%%\n",
        r->algorithm, r->hits, r->faults,
        100.0 * r->hits / r->page_count
    );

    g_string_append(report,
        "---------------------------------------------------------------\n");

    g_string_append(report, "Step  Page  ");

    for (j = 0; j < r->frame_count; j++)
        g_string_append_printf(report, "F%d    ", j + 1);

    g_string_append(report, "Status\n");

    for (i = 0; i < r->page_count; i++) {
        const StepResult *step = &r->steps[i];

        g_string_append_printf(
            report, "%-5d %-5d ", i + 1, step->page);

        for (j = 0; j < r->frame_count; j++) {
            if (step->frames[j] == -1)
                g_string_append(report, "-     ");
            else
                g_string_append_printf(
                    report, "%-6d", step->frames[j]);
        }

        g_string_append_printf(
            report, "%s\n",
            step->status == STATUS_HIT ? "HIT" : "FAULT");
    }
}

static void run_simulation(GtkButton *button, gpointer data)
{
    int pages[MAX_PAGES], count, frames;
    char error[256] = "";
    const char *input;
    SimulationResult fifo, lru, optimal;
    GString *report;
    GtkTextBuffer *buffer;

    (void)button;
    (void)data;

    input = gtk_entry_get_text(GTK_ENTRY(page_entry));
    frames = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(frame_spin));

    if (!parse_page_string(
            input, pages, &count, error, sizeof(error))) {
        set_text(status_label, error);
        return;
    }

    if (frames < 1 || frames > MAX_FRAMES) {
        set_text(status_label, "Frames must be between 1 and 20.");
        return;
    }

    run_fifo(pages, count, frames, &fifo);
    run_lru(pages, count, frames, &lru);
    run_optimal(pages, count, frames, &optimal);

    {
        char text[100];

        snprintf(text, sizeof(text),
                 "Hits: %d   |   Faults: %d",
                 fifo.hits, fifo.faults);
        set_text(fifo_summary, text);

        snprintf(text, sizeof(text),
                 "Hits: %d   |   Faults: %d",
                 lru.hits, lru.faults);
        set_text(lru_summary, text);

        snprintf(text, sizeof(text),
                 "Hits: %d   |   Faults: %d",
                 optimal.hits, optimal.faults);
        set_text(optimal_summary, text);
    }

    report = g_string_new("MEMORY LAB — SIMULATION REPORT\n");

    g_string_append_printf(
        report,
        "Reference string: %s\nFrames: %d   Requests: %d\n",
        input, frames, count);

    add_result(report, &fifo);
    add_result(report, &lru);
    add_result(report, &optimal);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    gtk_text_buffer_set_text(buffer, report->str, -1);

    g_string_free(report, TRUE);

    set_text(status_label,
             "Simulation complete. FIFO, LRU and Optimal compared.");
}

static void clear_simulation(GtkButton *button, gpointer data)
{
    GtkTextBuffer *buffer;

    (void)button;
    (void)data;

    gtk_entry_set_text(GTK_ENTRY(page_entry), "");
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(frame_spin), 3);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    gtk_text_buffer_set_text(
        buffer, "Enter page numbers and click RUN SIMULATION.", -1);

    set_text(fifo_summary, "Hits: —   |   Faults: —");
    set_text(lru_summary, "Hits: —   |   Faults: —");
    set_text(optimal_summary, "Hits: —   |   Faults: —");
    set_text(status_label, "Ready for a new simulation.");
}

static void load_demo(GtkButton *button, gpointer data)
{
    (void)button;
    (void)data;

    gtk_entry_set_text(
        GTK_ENTRY(page_entry),
        "7, 0, 1, 2, 0, 3, 0, 4, 2, 3");

    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(frame_spin), 3);

    run_simulation(NULL, NULL);
}

static void apply_style(void)
{
    GtkCssProvider *css = gtk_css_provider_new();

    const char *style =
        "window { background: #0b1120; }"
        "label { color: #e5e7eb; }"
        ".title { color: #ffffff; font-size: 26px; font-weight: bold; }"
        ".subtitle { color: #94a3b8; font-size: 13px; }"
        ".card { background: #17243b; border-radius: 12px; padding: 18px; }"
        "entry, spinbutton { background: #111c31; color: white;"
        " border: 1px solid #475569; border-radius: 7px; padding: 9px; }"
        "button { background: #263752; color: white; padding: 10px;"
        " border-radius: 7px; }"
        "button.run { background: #7c3aed; color: white; font-weight: bold; }"
        "textview, textview text { background: #0f172a; color: #dbeafe; }";

    gtk_css_provider_load_from_data(css, style, -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_object_unref(css);
}

GtkWidget *create_gui(void)
{
    GtkWidget *window, *root, *header, *title_box;
    GtkWidget *title, *subtitle, *body, *left, *right;
    GtkWidget *setup, *label, *run, *demo, *clear;
    GtkWidget *cards, *report_card, *scroll;
    GtkWidget *explanation;
    GtkAdjustment *adjustment;

    apply_style();

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(
        GTK_WINDOW(window), "Memory Lab | Page Replacement Simulator");
    gtk_window_set_default_size(GTK_WINDOW(window), 1280, 850);
    gtk_window_maximize(GTK_WINDOW(window));

    g_signal_connect(
        window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_margin_start(root, 24);
    gtk_widget_set_margin_end(root, 24);
    gtk_widget_set_margin_top(root, 20);
    gtk_widget_set_margin_bottom(root, 20);
    gtk_container_add(GTK_CONTAINER(window), root);

    header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    title = gtk_label_new("◈  MEMORY LAB");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(title), "title");

    subtitle = gtk_label_new(
        "PAGE REPLACEMENT  /  OPERATING SYSTEMS LAB");
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(subtitle), "subtitle");

    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), header, FALSE, FALSE, 0);

    body = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 18);
    gtk_box_pack_start(GTK_BOX(root), body, TRUE, TRUE, 0);

    left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_size_request(left, 300, -1);
    gtk_box_pack_start(GTK_BOX(body), left, FALSE, FALSE, 0);

    setup = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(setup), "card");

    label = gtk_label_new("SIMULATION SETUP");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(setup), label, FALSE, FALSE, 0);

    label = gtk_label_new("Page reference string");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(setup), label, FALSE, FALSE, 0);

    page_entry = gtk_entry_new();
    gtk_entry_set_text(
        GTK_ENTRY(page_entry), "7, 0, 1, 2, 0, 3, 0, 4, 2, 3");
    gtk_box_pack_start(GTK_BOX(setup), page_entry, FALSE, FALSE, 0);

    label = gtk_label_new("Number of frames");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(setup), label, FALSE, FALSE, 0);

    adjustment = GTK_ADJUSTMENT(
        gtk_adjustment_new(3, 1, MAX_FRAMES, 1, 1, 0));

    frame_spin = gtk_spin_button_new(adjustment, 1, 0);
    gtk_box_pack_start(GTK_BOX(setup), frame_spin, FALSE, FALSE, 0);

    run = gtk_button_new_with_label("▶  RUN SIMULATION");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(run), "run");

    demo = gtk_button_new_with_label("Load demo example");
    clear = gtk_button_new_with_label("Clear");

    gtk_box_pack_start(GTK_BOX(setup), run, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(setup), demo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(setup), clear, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(left), setup, FALSE, FALSE, 0);

    explanation = gtk_label_new(
        "FIFO  — replaces the oldest page.\n\n"
        "LRU  — replaces the least recently used page.\n\n"
        "OPTIMAL  — replaces the page needed farthest in the future.\n\n"
        "HIT  — page already in memory.\n\n"
        "FAULT  — page must be loaded.");
    gtk_label_set_line_wrap(GTK_LABEL(explanation), TRUE);
    gtk_label_set_xalign(GTK_LABEL(explanation), 0.0);

    {
        GtkWidget *help = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
        gtk_style_context_add_class(
            gtk_widget_get_style_context(help), "card");
        gtk_box_pack_start(GTK_BOX(help), explanation, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(left), help, FALSE, FALSE, 0);
    }

    right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_box_pack_start(GTK_BOX(body), right, TRUE, TRUE, 0);

    cards = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);

    {
        GtkWidget *c1 = make_card("FIFO", &fifo_summary);
        GtkWidget *c2 = make_card("LRU", &lru_summary);
        GtkWidget *c3 = make_card("OPTIMAL", &optimal_summary);

        gtk_box_pack_start(GTK_BOX(cards), c1, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(cards), c2, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(cards), c3, TRUE, TRUE, 0);
    }

    gtk_box_pack_start(GTK_BOX(right), cards, FALSE, FALSE, 0);

    report_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(report_card), "card");

    label = gtk_label_new("SIMULATION REPORT");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(report_card), label, FALSE, FALSE, 0);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(
        GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_view), GTK_WRAP_NONE);

    gtk_container_add(GTK_CONTAINER(scroll), output_view);
    gtk_box_pack_start(GTK_BOX(report_card), scroll, TRUE, TRUE, 0);
    gtk_widget_set_vexpand(scroll, TRUE);

    gtk_box_pack_start(GTK_BOX(right), report_card, TRUE, TRUE, 0);

    status_label = gtk_label_new("Ready.");
    gtk_widget_set_halign(status_label, GTK_ALIGN_START);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(status_label), "subtitle");

    gtk_box_pack_start(GTK_BOX(right), status_label, FALSE, FALSE, 0);

    g_signal_connect(run, "clicked", G_CALLBACK(run_simulation), NULL);
    g_signal_connect(demo, "clicked", G_CALLBACK(load_demo), NULL);
    g_signal_connect(clear, "clicked", G_CALLBACK(clear_simulation), NULL);

    run_simulation(NULL, NULL);

    return window;
}
