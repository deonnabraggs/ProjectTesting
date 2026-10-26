#include <gtk/gtk.h>

/*
 * PAGE REPLACEMENT SIMULATOR
 * Main application entry point.
 *
 * GUI implementation: gui.c
 * Algorithms: fifo.c, lru.c, optimal.c
 * Shared definitions: page_replacement.h
 */

GtkWidget *create_main_window(void);

int main(int argc, char *argv[])
{
    GtkWidget *window;

    /* Initialize the GTK library */
    gtk_init(&argc, &argv);

    /* Create the main window */
    window = create_main_window();

    /* Display the window and all child widgets */
    gtk_widget_show_all(window);

    /* Start handling GUI events */
    gtk_main();

    return 0;
}
