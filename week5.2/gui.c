
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

typedef void (*AlgorithmFunction)(const int[], int, int, SimulationResult *);

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
        while (isspace((unsigned char)*p) || *p == ',' || *p == ';')
            p++;

        if (*p == '\0')
            break;

        if (!isdigit((unsigned char)*p))
            return 0;

        errno = 0;
        char *end = NULL;
        long value = strtol(p, &end, 10);

        if (errno == ERANGE || end == p || value < 0 || value > 2147483647L)
            return 0;

        if (n >= MAX_REQUESTS)
            return -1;

        pages[n++] = (int)value;
        p = end;

        if (*p != '\0' && !isspace((unsigned char)*p) &&
            *p != ',' && *p != ';')
            return 0;
    }

    if (n == 0)
        return 0;

    *count = n;
    return 1;
}

static void append_algorithm_result(GString *report, const char *name,
                                    AlgorithmFunction algorithm,
                                    const int pages[], int n, int frame_count)
{
    SimulationResult result = {0};
    algorithm(pages, n, frame_count, &result);

    g_string_append_printf(report,
        "\n%s\n"
        "------------------------------------------------------------\n",
        name);

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

        g_string_append_printf(report, " %s\n",
