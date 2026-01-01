#include <stdio.h>
#include <string.h>

size_t timeline_diff_first_line(
    FILE *a,
    FILE *b
)
{
    char la[512];
    char lb[512];
    size_t line = 0;

    rewind(a);
    rewind(b);

    while (1) {
        char *ra = fgets(la, sizeof la, a);
        char *rb = fgets(lb, sizeof lb, b);

        if (!ra && !rb)
            return (size_t)-1; /* identical */

        if (!ra || !rb)
            return line;

        if (strcmp(la, lb) != 0)
            return line;

        line++;
    }
}
