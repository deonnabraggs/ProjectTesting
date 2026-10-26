#include <gtk/gtk.h>
#include "page_replacement.h"

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    GtkWidget *window = create_main_window();

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
