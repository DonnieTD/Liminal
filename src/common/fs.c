#include "common/fs.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

bool fs_mkdir_if_missing(const char *path)
{
    if (mkdir(path, 0755) == 0)
        return true;

    if (errno == EEXIST)
        return true;

    return false;
}

bool fs_write_file(const char *path, const char *data, size_t len)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return false;

    fwrite(data, 1, len, f);
    fclose(f);
    return true;
}

FILE *fs_open_file(const char *path)
{
    return fopen(path, "w");
}
