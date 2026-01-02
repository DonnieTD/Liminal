#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <time.h>

/* ------------------------------------------------------------------
 * Simple dynamic string set for system includes
 * ------------------------------------------------------------------ */

typedef struct {
    char **data;
    size_t count, cap;
} StringSet;

static StringSet sysincs = {0};

static int set_has(StringSet *s, const char *line) {
    for (size_t i = 0; i < s->count; i++) {
        if (strcmp(s->data[i], line) == 0) return 1;
    }
    return 0;
}

static void set_add(StringSet *s, const char *line) {
    if (set_has(s, line)) return;
    if (s->count == s->cap) {
        s->cap = s->cap ? s->cap * 2 : 16;
        s->data = realloc(s->data, s->cap * sizeof(char *));
    }
    s->data[s->count++] = strdup(line);
}

/* ------------------------------------------------------------------
 * Minifying emitter
 * Collapses whitespace, strips comments, keeps tokens
 * ------------------------------------------------------------------ */

static void emit_minified(const char *in, FILE *out) {
    static int in_block = 0;
    int last_space = 1;

    for (const char *p = in; *p; p++) {

        /* Strip // comments */
        if (!in_block && p[0]=='/' && p[1]=='/') break;

        /* Enter block comment */
        if (!in_block && p[0]=='/' && p[1]=='*') {
            in_block = 1;
            p++;
            continue;
        }

        /* Exit block comment */
        if (in_block && p[0]=='*' && p[1]=='/') {
            in_block = 0;
            p++;
            continue;
        }

        if (in_block) continue;

        unsigned char c = (unsigned char)*p;

        /* Collapse whitespace */
        if (isspace(c)) {
            if (!last_space) {
                fputc(' ', out);
                last_space = 1;
            }
            continue;
        }

        /* Punctuation */
        if (strchr("(){}[];,.", c)) {
            fputc(c, out);
            last_space = 1;
            continue;
        }

        /* Operators */
        if (strchr("+-*/%=<>!^|&", c)) {
            fputc(c, out);
            last_space = 0;
            continue;
        }

        /* Identifiers / literals */
        fputc(c, out);
        last_space = 0;
    }
}

/* ------------------------------------------------------------------
 * Filters
 * ------------------------------------------------------------------ */

static int is_system_include(const char *l) {
    return strncmp(l, "#include <", 10) == 0;
}

static int skip_line(const char *l) {
    return
        strstr(l, "#pragma once") ||
        strstr(l, "#ifndef ")     ||
        strstr(l, "#define ")     ||
        strcmp(l, "#endif\n") == 0 ||
        strstr(l, "#include \"");
}

/* ------------------------------------------------------------------
 * File flattening
 * ------------------------------------------------------------------ */

static void process_file(const char *path, FILE *body) {
    FILE *f = fopen(path, "r");
    if (!f) return;

    fprintf(body, "//@source %s\n", path);

    char buf[4096];
    while (fgets(buf, sizeof buf, f)) {

        if (is_system_include(buf)) {
            set_add(&sysincs, buf);
            continue;
        }
        if (skip_line(buf)) continue;

        emit_minified(buf, body);
    }

    fputc('\n', body);
    fclose(f);
}

/* ------------------------------------------------------------------
 * Directory walk
 * ------------------------------------------------------------------ */

static int is_c_file(const char *p) {
    size_t n = strlen(p);
    return n > 2 && strcmp(p + n - 2, ".c") == 0;
}

static void walk(const char *path, FILE *body) {
    struct stat st;
    if (stat(path, &st) != 0) return;

    if (S_ISREG(st.st_mode)) {
        if (is_c_file(path)) process_file(path, body);
        return;
    }

    if (!S_ISDIR(st.st_mode)) return;

    DIR *d = opendir(path);
    if (!d) return;

    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        char full[1024];
        snprintf(full, sizeof full, "%s/%s", path, e->d_name);
        walk(full, body);
    }
    closedir(d);
}

/* ------------------------------------------------------------------
 * Entry point
 * ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    if (argc < 4 || strcmp(argv[1], "--out") != 0) {
        fprintf(stderr, "usage: liminal-flatten --out <file> <paths...>\n");
        return 1;
    }

    const char *out_path = argv[2];
    FILE *out = fopen(out_path, "w");
    if (!out) { perror(out_path); return 1; }

    /* banner */
    time_t now = time(NULL);
    struct tm g;
    gmtime_r(&now, &g);
    char ts[32];
    strftime(ts, sizeof ts, "%Y%m%dT%H%M%SZ", &g);
    fprintf(out, "/* FLATTEN %s */\n", ts);

    /* flatten into temp */
    FILE *body = tmpfile();
    if (!body) { perror("tmpfile"); fclose(out); return 1; }

    /* walk each given path */
    for (int i = 3; i < argc; i++) {
        walk(argv[i], body);
    }

    /* write system includes once */
    for (size_t i = 0; i < sysincs.count; i++) {
        fputs(sysincs.data[i], out);
    }
    fputc('\n', out);

    /* append body */
    rewind(body);
    int c;
    while ((c = fgetc(body)) != EOF) fputc(c, out);

    fclose(body);
    fclose(out);
    return 0;
}
