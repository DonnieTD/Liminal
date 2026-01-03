#include <stdlib.h>
#include <string.h>
#include "./fix_surface.h"


static void add_cause(FixSurface *fs, const CauseKey *key)
{
    if (fs->count == fs->capacity) {
        size_t nc = fs->capacity ? fs->capacity * 2 : 4;
        fs->causes = realloc(fs->causes, nc * sizeof(*fs->causes));
        fs->capacity = nc;
    }
    fs->causes[fs->count++] = *key;
}

FixSurface build_fix_surface(const ConvergenceMap *map)
{
    FixSurface fs = {0};

    /* In current architecture:
       every diagnostic belongs to exactly one cause.
       So minimal fix surface = all unique causes.
     */

    for (size_t i = 0; i < map->count; i++) {
        add_cause(&fs, &map->entries[i].key);
    }

    return fs;
}
