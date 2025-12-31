#ifndef LIMINAL_FS_H
#define LIMINAL_FS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool fs_mkdir_if_missing(const char *path);
bool fs_write_file(const char *path, const char *data, size_t len);
FILE *fs_open_file(const char *path);

#endif
