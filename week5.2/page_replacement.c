#include "replacement.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    if (!input || !pages || !count) {
        snprintf(error, error_size, "Invalid input.");
        return 0;
    }

    while (*p) {
        char *end;
        long value;

        while (isspace((unsigned char)*p) || *p == ',')
            p++;

        if (*p == '\0')
            break;

        if (!isdigit((unsigned char)*p)) {
            snprintf(error, error_size,
                     "Enter non-negative integers separated by commas.");
            return 0;
        }

        value = strtol(p, &end, 10);

        if (end == p || value < 0 || value > 2147483647L) {
            snprintf(error, error_size, "Invalid page number.");
            return 0;
        }

        if (n >= MAX_PAGES) {
            snprintf(error, error_size,
                     "Maximum 500 page requests allowed.");
            return 0;
        }

        pages[n++] = (int)value;
        p = end;

        if (*p && !isspace((unsigned char)*p) && *p != ',') {
            snprintf(error, error_size,
                     "Separate page numbers with commas or spaces.");
            return 0;
        }
    }

    if (n == 0) {
        snprintf(error, error_size,
                 "Please enter at least one page number.");
        return 0;
    }

    *count = n;
    return 1;
}
