#include "deadlock.h"
#include <stdio.h>
#include <math.h>

static void draw_arrow(cairo_t *cr,
                       double x1, double y1,
                       double x2, double y2)
{
    double angle = atan2(y2 - y1, x2 - x1);
    double head = 8.0;

    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);

    cairo_move_to(cr, x2, y2);

    cairo_line_to(cr,
                  x2 - head * cos(angle - 0.45),
                  y2 - head * sin(angle - 0.45));

    cairo_line_to(cr,
                  x2 - head * cos(angle + 0.45),
                  y2 - head * sin(angle + 0.45));

    cairo_close_path(cr);
    cairo_fill(cr);
}

void draw_resource_graph(cairo_t *cr, int width, int height)
{
    if (n <= 0 || m <= 0)
    {
        cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
        cairo_move_to(cr, 20, 30);
        cairo_show_text(cr, "Create processes and resources first.");
        return;
    }

    double process_radius = 20;
    double resource_size = 28;
    double process_y = height * 0.25;
    double resource_y = height * 0.70;
    double process_gap = (double)width / (n + 1);
    double resource_gap = (double)width / (m + 1);

    /* Draw allocation and request edges first. */
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            double px = process_gap * (i + 1);
            double rx = resource_gap * (j + 1);

            if (allocation[i][j] > 0)
            {
                cairo_set_source_rgb(cr, 0.1, 0.3, 0.9);

                draw_arrow(cr,
                           rx, resource_y - resource_size / 2,
                           px, process_y + process_radius);
            }

            if (request[i][j] > 0)
            {
                cairo_set_source_rgb(cr, 0.9, 0.1, 0.1);

                draw_arrow(cr,
                           px, process_y + process_radius,
                           rx, resource_y - resource_size / 2);
            }
        }
    }

    /* Draw processes. */
    for (int i = 0; i < n; i++)
    {
        double x = process_gap * (i + 1);

        if (active[i])
            cairo_set_source_rgb(cr, 0.2, 0.7, 0.3);
        else
            cairo_set_source_rgb(cr, 0.65, 0.65, 0.65);

        cairo_arc(cr, x, process_y, process_radius, 0, 2 * G_PI);
        cairo_fill_preserve(cr);

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_stroke(cr);

        char label[20];
        snprintf(label, sizeof(label), "P%d", i);

        cairo_move_to(cr, x - 9, process_y + 5);
        cairo_show_text(cr, label);
    }

    /* Draw resource nodes. */
    for (int j = 0; j < m; j++)
    {
        double x = resource_gap * (j + 1);

        cairo_set_source_rgb(cr, 1.0, 0.75, 0.2);

        cairo_rectangle(cr,
                        x - resource_size / 2,
                        resource_y - resource_size / 2,
                        resource_size, resource_size);

        cairo_fill_preserve(cr);

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_stroke(cr);

        char label[20];
        snprintf(label, sizeof(label), "R%d", j);

        cairo_move_to(cr, x - 9, resource_y + 5);
        cairo_show_text(cr, label);
    }

    cairo_set_source_rgb(cr, 0.1, 0.3, 0.9);
    cairo_move_to(cr, 15, height - 25);
    cairo_show_text(cr, "Blue: Allocation");

    cairo_set_source_rgb(cr, 0.9, 0.1, 0.1);
    cairo_move_to(cr, 180, height - 25);
    cairo_show_text(cr, "Red: Request");
}
