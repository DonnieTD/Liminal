#ifndef LIMINAL_FILE_H
#define LIMINAL_FILE_H

#include <stddef.h>

/*
 * Read entire file into heap-allocated buffer.
 *
 * On success:
 *   - *out_buf points to malloc'd memory
 *   - *out_len contains byte length
 *
 * Caller owns the buffer.
 */
int read_entire_file(
    const char *path,
    char      **out_buf,
    size_t     *out_len
);

#endif /* LIMINAL_FILE_H */
