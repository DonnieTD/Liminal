#include "common/common.h"

#include <stdio.h>
#include <stdlib.h>

int read_entire_file(
    const char *path,
    char      **out_buf,
    size_t     *out_len
)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return 0;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }

    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return 0;
    }

    rewind(f);

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }

    size_t n = fread(buf, 1, (size_t)size, f);
    fclose(f);

    if (n != (size_t)size) {
        free(buf);
        return 0;
    }

    buf[size] = '\0';

    *out_buf = buf;
    *out_len = (size_t)size;
    return 1;
}
