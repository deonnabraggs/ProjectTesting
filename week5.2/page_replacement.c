#include "page_replacement.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int parse_page_string(
    const char *input,
    int pages[],
    int *count,
    char *error,
    size_t error_size
)
{
    const char *p = input;
    int n = 0;

    if (input == NULL || pages == NULL || count == NULL) {
        if (error && error_size)
            snprintf(error, error_size, "Invalid input.");
        return 0;
    }

    while (isspace((unsigned char)*p) ||
           *p == ',' || *p == ';') {
        p++;
    }

    while (*p != '\0') {
        char *end = NULL;
        long value;

        if (!isdigit((unsigned char)*p)) {
            if (error && error_size)
                snprintf(error, error_size,
                         "Use non-negative page numbers separated by spaces or commas.");
            return 0;
        }

        errno = 0;
        value = strtol(p, &end, 10);

        if (errno == ERANGE || end == p ||
            value < 0 || value > 2147483647L) {
            if (error && error_size)
                snprintf(error, error_size, "Invalid page number.");
            return 0;
        }

        if (n >= MAX_PAGES) {
            if (error && error_size)
                snprintf(error, error_size,
                         "Maximum of %d page requests allowed.", MAX_PAGES);
            return 0;
        }

        pages[n++] = (int)value;
        p = end;

        if (*p != '\0' &&
            !isspace((unsigned char)*p) &&
            *p != ',' && *p != ';') {
            if (error && error_size)
                snprintf(error, error_size,
                         "Separate page numbers with spaces or commas.");
            return 0;
        }

        while (isspace((unsigned char)*p) ||
               *p == ',' || *p == ';') {
            p++;
        }
    }

    if (n == 0) {
        if (error && error_size)
            snprintf(error, error_size, "Enter at least one page number.");
        return 0;
    }

    *count = n;

    if (error && error_size)
        error[0] = '\0';

    return 1;
}
