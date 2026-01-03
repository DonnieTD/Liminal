#include <stdio.h>
#include <stdlib.h>

typedef struct TimelineEvent {
    unsigned long time;
    int step;
    unsigned int ast;
} TimelineEvent;

int load_timeline(const char *path,
                  TimelineEvent **out,
                  size_t *out_count)
{
    if (!path || !out || !out_count)
        return 1;

    FILE *f = fopen(path, "r");
    if (!f)
        return 2;

    TimelineEvent *buf = calloc(256, sizeof(TimelineEvent));
    size_t count = 0;

    while (count < 256) {
        TimelineEvent e;
        int n = fscanf(
            f,
            "{\"time\":%lu,\"step\":%d,\"ast\":%u}\n",
            &e.time,
            &e.step,
            &e.ast
        );
        if (n != 3)
            break;
        buf[count++] = e;
    }

    fclose(f);

    *out = buf;
    *out_count = count;
    return 0;
}
