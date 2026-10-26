#include <gtk/gtk.h>

/* Function implemented in GUI.c */
GtkWidget *create_gui(void);

int main(int argc, char *argv[])
{
    GtkWidget *window;

    gtk_init(&argc, &argv);

    window = create_gui();

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
