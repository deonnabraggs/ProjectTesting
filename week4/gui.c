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
