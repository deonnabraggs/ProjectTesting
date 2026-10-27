static void style_report_heading(GtkTextBuffer *buffer)
{
    GtkTextTagTable *table;
    GtkTextTag *tag;
    GtkTextIter start, end;

    table = gtk_text_buffer_get_tag_table(buffer);
    tag = gtk_text_tag_table_lookup(table, "report_heading");

    if (tag == NULL) {
        tag = gtk_text_buffer_create_tag(
            buffer,
            "report_heading",
            "foreground", "#FFFFFF",
            "background", "#123B70",
            "paragraph-background", "#123B70",
            "weight", PANGO_WEIGHT_BOLD,
            "size-points", 16.0,
            NULL
        );
    }

    gtk_text_buffer_get_start_iter(buffer, &start);
    end = start;

    if (gtk_text_iter_forward_to_line_end(&end)) {
        gtk_text_buffer_apply_tag(buffer, tag, &start, &end);
    }
}
