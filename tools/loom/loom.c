/* tools/loom/loom.c */
#include "loom.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

/* ============================================================
 * Config (mirrors Makefile variables)
 * ============================================================ */

#define CC_PATH     "cc"
#define BIN_NAME    "liminal"
#define BUILD_DIR   "build"

#define STANDARDISE_PATH "tools/standardise/standardise"
#define STYLE_GUIDE_PATH "CodeStyleGuide.md"

/* CFLAGS: -std=c99 -Wall -Wextra -Wpedantic -g -Isrc */
static const char *CFLAGS_ARR[] = {
    "-std=c99",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-g",
    "-Isrc",
    "-MMD",
    "-MP",
    NULL
};

/* LDFLAGS is empty in your Makefile */
#define LDFLAGS_STR ""

/* ============================================================
 * Tiny helpers
 * ============================================================ */

typedef struct {
    char **items;
    size_t count;
    size_t cap;
} StrVec;

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) die("malloc");
    return p;
}

static char *xstrdup(const char *s) {
    size_t n = strlen(s);
    char *p = (char *)xmalloc(n + 1);
    memcpy(p, s, n + 1);
    return p;
}

static void vec_push(StrVec *v, char *s) {
    if (v->count == v->cap) {
        size_t nc = v->cap ? v->cap * 2 : 64;
        v->items = (char **)realloc(v->items, nc * sizeof(v->items[0]));
        if (!v->items) die("realloc");
        v->cap = nc;
    }
    v->items[v->count++] = s;
}

static void vec_free(StrVec *v) {
    for (size_t i = 0; i < v->count; i++) free(v->items[i]);
    free(v->items);
    v->items = NULL;
    v->count = v->cap = 0;
}

static int str_ends_with(const char *s, const char *suffix) {
    size_t n = strlen(s), m = strlen(suffix);
    return n >= m && memcmp(s + (n - m), suffix, m) == 0;
}

static int str_starts_with(const char *s, const char *pfx) {
    size_t n = strlen(pfx);
    return strncmp(s, pfx, n) == 0;
}

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

static time_t mtime_of(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return (time_t)0;
    return st.st_mtime;
}

static void mkdir_p(const char *path) {
    /* mkdir -p equivalent (silent) */
    char tmp[4096];
    size_t n = strlen(path);
    if (n >= sizeof(tmp)) {
        errno = ENAMETOOLONG;
        die("mkdir_p");
    }
    memcpy(tmp, path, n + 1);

    for (size_t i = 1; i < n; i++) {
        if (tmp[i] == '/') {
            tmp[i] = '\0';
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) die("mkdir");
            tmp[i] = '/';
        }
    }
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) die("mkdir");
}

static void dirname_of(const char *path, char *out, size_t cap) {
    const char *slash = strrchr(path, '/');
    if (!slash) {
        snprintf(out, cap, ".");
        return;
    }
    size_t n = (size_t)(slash - path);
    if (n + 1 > cap) {
        errno = ENAMETOOLONG;
        die("dirname_of");
    }
    memcpy(out, path, n);
    out[n] = '\0';
}

/* ============================================================
 * Spawn helpers (no system()/popen(); also lets us control echo)
 * ============================================================ */

static void print_cmd_argv(char *const argv[]) {
    /* Print exactly like make (raw args separated by single spaces). */
    for (int i = 0; argv[i]; i++) {
        if (i) fputc(' ', stdout);
        fputs(argv[i], stdout);
    }
    fputc('\n', stdout);
    fflush(stdout);
}

static int spawn_wait(char *const argv[], int echo_cmd) {
    if (echo_cmd) {
        print_cmd_argv(argv);
    }

    pid_t pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) die("waitpid");

    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return 1;
}

static int spawn_capture_lines(char *const argv[], StrVec *out_lines) {
    int pipefd[2];
    if (pipe(pipefd) != 0) die("pipe");

    pid_t pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) _exit(127);
        close(pipefd[1]);
        execvp(argv[0], argv);
        _exit(127);
    }

    close(pipefd[1]);

    char buf[8192];
    char line[8192];
    size_t line_len = 0;

    for (;;) {
        ssize_t r = read(pipefd[0], buf, sizeof(buf));
        if (r < 0) die("read");
        if (r == 0) break;

        for (ssize_t i = 0; i < r; i++) {
            char c = buf[i];
            if (c == '\n') {
                line[line_len] = '\0';
                /* trim CR */
                if (line_len && line[line_len - 1] == '\r') line[line_len - 1] = '\0';
                vec_push(out_lines, xstrdup(line));
                line_len = 0;
            } else if (line_len + 1 < sizeof(line)) {
                line[line_len++] = c;
            }
        }
    }

    close(pipefd[0]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) die("waitpid");

    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return 1;
}

static int spawn_to_file(char *const argv[], const char *path, int append) {
    int flags = O_CREAT | O_WRONLY | (append ? O_APPEND : O_TRUNC);
    int fd = open(path, flags, 0644);
    if (fd < 0) die("open");

    pid_t pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {
        if (dup2(fd, STDOUT_FILENO) < 0) _exit(127);
        close(fd);
        execvp(argv[0], argv);
        _exit(127);
    }

    close(fd);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) die("waitpid");
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return 1;
}

/* ============================================================
 * Source discovery (match Makefile's: find src -name '*.c' then filter-out src/samples/%)
 * ============================================================ */

static void collect_src_c_files(StrVec *out) {
    char *argv_find[] = { "find", "src", "-name", "*.c", NULL };

    StrVec lines = {0};
    int rc = spawn_capture_lines(argv_find, &lines);
    if (rc != 0) {
        vec_free(&lines);
        fprintf(stderr, "loom: find failed\n");
        exit(1);
    }

    for (size_t i = 0; i < lines.count; i++) {
        const char *p = lines.items[i];
        if (str_starts_with(p, "src/samples/")) continue; /* filter-out */
        vec_push(out, xstrdup(p));
    }

    vec_free(&lines);
}

static void src_to_obj(const char *src_path, char *out, size_t cap) {
    /* src/foo/bar.c -> build/foo/bar.o */

    if (!str_starts_with(src_path, "src/") || !str_ends_with(src_path, ".c")) {
        snprintf(out, cap, "%s", src_path);
        return;
    }

    const char *rel = src_path + 4; /* after "src/" */
    size_t reln = strlen(rel);

    /* must end with ".c" */
    if (reln < 3) {
        errno = EINVAL;
        die("src_to_obj");
    }

    /* remove ".c" */
    size_t stem_len = reln - 2; /* drop 'c' */
    if (strlen(BUILD_DIR) + 1 + stem_len + 1 >= cap) {
        errno = ENAMETOOLONG;
        die("src_to_obj");
    }

    /* build/foo/bar */
    snprintf(out, cap, "%s/%.*s", BUILD_DIR, (int)(stem_len - 1), rel);

    /* append ".o" */
    strcat(out, ".o");
}

static void obj_to_dep(const char *obj_path, char *out, size_t cap) {
    snprintf(out, cap, "%s", obj_path);
    size_t n = strlen(out);
    if (n >= 2 && out[n - 2] == '.' && out[n - 1] == 'o') {
        out[n - 1] = 'd';
    }
}

/* Parse a GCC/Clang .d file and decide if any dep is newer than obj. */
static int depfile_newer_than_obj(const char *dep_path, time_t obj_mtime) {
    FILE *f = fopen(dep_path, "r");
    if (!f) return 1; /* missing .d => be safe */

    int newer = 0;

    char tok[4096];
    int c;
    size_t len = 0;

    /* Tokenizer: split on whitespace, ignore '\' line-continuations and "target:" token. */
    int saw_colon = 0;

    while ((c = fgetc(f)) != EOF) {
        if (c == '\\') {
            /* swallow backslash-newline continuation */
            int n = fgetc(f);
            if (n == '\n') continue;
            if (n != EOF) ungetc(n, f);
        }

        if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
            if (len) {
                tok[len] = '\0';
                len = 0;

                if (!saw_colon) {
                    /* first token is "obj:". Mark once we've consumed a token containing ':' */
                    if (strchr(tok, ':')) {
                        saw_colon = 1;
                    }
                } else {
                    /* dependency path */
                    time_t mt = mtime_of(tok);
                    if (mt > obj_mtime) {
                        newer = 1;
                        break;
                    }
                }
            }
            continue;
        }

        if (len + 1 < sizeof(tok)) tok[len++] = (char)c;
    }

    if (!newer && len) {
        tok[len] = '\0';
        if (saw_colon) {
            time_t mt = mtime_of(tok);
            if (mt > obj_mtime) newer = 1;
        }
    }

    fclose(f);
    return newer;
}

static int needs_rebuild_obj(const char *src_path, const char *obj_path, const char *dep_path) {
    if (!file_exists(obj_path)) return 1;

    time_t obj_mt = mtime_of(obj_path);
    if (mtime_of(src_path) > obj_mt) return 1;

    /* Header deps (like make via -MMD -MP) */
    if (!file_exists(dep_path)) return 1;
    if (depfile_newer_than_obj(dep_path, obj_mt)) return 1;

    return 0;
}

static int needs_relink_bin(const char *bin_path, StrVec *objs) {
    if (!file_exists(bin_path)) return 1;
    time_t bin_mt = mtime_of(bin_path);

    for (size_t i = 0; i < objs->count; i++) {
        if (mtime_of(objs->items[i]) > bin_mt) return 1;
    }
    return 0;
}

/* ============================================================
 * Standardise (match make's *printed* output)
 * ============================================================ */

static void loom_check(void) {
    printf("Running standardisation checks...\n");
    fflush(stdout);

    char *argv1[] = { (char *)STANDARDISE_PATH, "src", NULL };
    char *argv2[] = { (char *)STANDARDISE_PATH, "tools", NULL };

    /* Makefile uses @ for these lines => commands not echoed, only tool output. */
    if (spawn_wait(argv1, 0) != 0) exit(1);
    if (spawn_wait(argv2, 0) != 0) exit(1);
}

static void loom_regen_standards(void) {
    printf("Regenerating CodeStyleGuide.md...\n");
    fflush(stdout);

    if (!file_exists(STYLE_GUIDE_PATH)) {
        /* bootstrap: tool writes whole base file */
        char *argv_init[] = { (char *)STANDARDISE_PATH, "--init-style-guide", NULL };
        if (spawn_to_file(argv_init, STYLE_GUIDE_PATH, 0) != 0) exit(1);
        return;
    }

    /* Replace the STANDARDISE region:
       - Copy everything, but when we see BEGIN, we emit the freshly generated section
         (tool output), then skip until END in the old file.
       This works whether --emit-style-guide prints markers or not. */
    FILE *in = fopen(STYLE_GUIDE_PATH, "r");
    if (!in) die("fopen CodeStyleGuide.md");

    char tmp_path[4096];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", STYLE_GUIDE_PATH);

    FILE *out = fopen(tmp_path, "w");
    if (!out) die("fopen CodeStyleGuide.md.tmp");

    const char *BEGIN = "<!-- STANDARDISE:BEGIN -->";
    const char *END   = "<!-- STANDARDISE:END -->";

    char line[8192];
    int skipping = 0;

    while (fgets(line, sizeof(line), in)) {
        if (!skipping && strstr(line, BEGIN)) {
            /* emit fresh section */
            char *argv_emit[] = { (char *)STANDARDISE_PATH, "--emit-style-guide", NULL };

            /* We want the tool output verbatim. We'll stream it to out. */
            int pipefd[2];
            if (pipe(pipefd) != 0) die("pipe");

            pid_t pid = fork();
            if (pid < 0) die("fork");

            if (pid == 0) {
                close(pipefd[0]);
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) _exit(127);
                close(pipefd[1]);
                execvp(argv_emit[0], argv_emit);
                _exit(127);
            }

            close(pipefd[1]);

            /* Also include the BEGIN line from the original file if the tool doesn't print it.
               We'll just write the original BEGIN line first; if the tool prints it too, it's still stable
               because your tool can choose to *not* include markers and this is the safer direction.
               (If you want strict no-dup markers, make --emit-style-guide print content only.) */
            fputs(line, out);

            char buf[8192];
            for (;;) {
                ssize_t r = read(pipefd[0], buf, sizeof(buf));
                if (r < 0) die("read");
                if (r == 0) break;
                if (fwrite(buf, 1, (size_t)r, out) != (size_t)r) die("fwrite");
            }

            close(pipefd[0]);

            int status = 0;
            if (waitpid(pid, &status, 0) < 0) die("waitpid");
            if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
                fclose(in);
                fclose(out);
                exit(1);
            }

            /* Now skip old generated content until END, but keep END line. */
            skipping = 1;
            continue;
        }

        if (skipping) {
            if (strstr(line, END)) {
                fputs(line, out);
                skipping = 0;
            }
            continue;
        }

        fputs(line, out);
    }

    fclose(in);
    fclose(out);

    if (rename(tmp_path, STYLE_GUIDE_PATH) != 0) die("rename CodeStyleGuide.md.tmp");
}

static void loom_standardise(void) {
    loom_check();
    loom_regen_standards();
}

/* ============================================================
 * Build (match make's printed output)
 * ============================================================ */

static void loom_compile_and_link(void) {
    /* Discover sources in the same conceptual way as Makefile. */
    StrVec srcs = {0};
    collect_src_c_files(&srcs);

    /* Build obj list (always, for link line order). */
    StrVec objs = {0};
    for (size_t i = 0; i < srcs.count; i++) {
        char obj[4096];
        src_to_obj(srcs.items[i], obj, sizeof(obj));
        vec_push(&objs, xstrdup(obj));
    }

    /* Compile only what needs recompiling (timestamp + .d deps like make). */
    for (size_t i = 0; i < srcs.count; i++) {
        const char *src = srcs.items[i];
        const char *obj = objs.items[i];

        char dep[4096];
        obj_to_dep(obj, dep, sizeof(dep));

        if (!needs_rebuild_obj(src, obj, dep)) continue;

        char dir[4096];
        dirname_of(obj, dir, sizeof(dir));
        mkdir_p(dir); /* silent (matches @mkdir -p) */

        /* Print exactly the cc invocation line like make does. */
        /* cc -std=c99 ... -MMD -MP -c src/... -o build/... */
        /* Build argv list. */
        char *argv[64];
        int k = 0;
        argv[k++] = (char *)CC_PATH;
        for (int j = 0; CFLAGS_ARR[j]; j++) argv[k++] = (char *)CFLAGS_ARR[j];
        argv[k++] = "-c";
        argv[k++] = (char *)src;
        argv[k++] = "-o";
        argv[k++] = (char *)obj;
        argv[k++] = NULL;

        if (spawn_wait(argv, 1) != 0) {
            vec_free(&srcs);
            vec_free(&objs);
            exit(1);
        }
    }

    /* Link only if needed (like make). */
    if (needs_relink_bin(BIN_NAME, &objs)) {
        /* Build link argv: cc <objs...> <LDFLAGS> -o liminal
           (Make prints two spaces before -o when LDFLAGS is empty; our argv printing
            will show a single space between args, which is still the canonical command line.
            If you *need* the double-space cosmetic, you can keep system(); but argv-style is cleaner.) */
        size_t max = 4 + objs.count + 4;
        char **argv = (char **)xmalloc(max * sizeof(char *));
        size_t k = 0;
        argv[k++] = (char *)CC_PATH;
        for (size_t i = 0; i < objs.count; i++) argv[k++] = objs.items[i];

        /* LDFLAGS is empty; keep the shape. If you add flags later, put them here. */
        (void)LDFLAGS_STR;

        argv[k++] = "-o";
        argv[k++] = (char *)BIN_NAME;
        argv[k++] = NULL;

        /* Print link line like make does (raw argv). */
        if (spawn_wait(argv, 1) != 0) {
            free(argv);
            vec_free(&srcs);
            vec_free(&objs);
            exit(1);
        }
        free(argv);
    }

    vec_free(&srcs);
    vec_free(&objs);
}

static void loom_build(void) {
    loom_check();
    loom_compile_and_link();
}

/* ============================================================
 * Clean (match make clean: rm -rf build liminal)
 * ============================================================ */

static void loom_clean(void) {
    /* make clean prints the rm line */
    char *argv[] = { "rm", "-rf", (char *)BUILD_DIR, (char *)BIN_NAME, NULL };
    if (spawn_wait(argv, 1) != 0) exit(1);
}

/* ============================================================
 * Samples / Tests (match make test behavior)
 * ============================================================ */

static int cmp_cstr(const void *a, const void *b) {
    const char *aa = *(const char * const *)a;
    const char *bb = *(const char * const *)b;
    return strcmp(aa, bb);
}

static void collect_sample_files(const char *dir, StrVec *out) {
    DIR *d = opendir(dir);
    if (!d) return;

    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        if (!str_ends_with(e->d_name, ".c")) continue;

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", dir, e->d_name);
        vec_push(out, xstrdup(path));
    }
    closedir(d);

    qsort(out->items, out->count, sizeof(out->items[0]), cmp_cstr);
}

static void format_test_dir(char *out, size_t cap) {
    const char *det = getenv("LOOM_DETERMINISTIC");
    const char *root = getenv("LOOM_TEST_ROOT");

    if (det && root) {
        /* Deterministic, fixed path */
        snprintf(out, cap, "%s/run", root);
        return;
    }

    /* Normal (timestamped) behavior */
    time_t t = time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);

    char stamp[64];
    strftime(stamp, sizeof(stamp), "%d-%m-%Y-%H-%M-%S", &tmv);

    snprintf(out, cap, "test/%s", stamp);
}

static int run_liminal_sample(const char *sample_path, const char *outdir) {
    /* Print exactly like your run_sample macro */
    printf("\n>>> %s\n", sample_path);
    fflush(stdout);

    mkdir_p(outdir);

    char *argv[] = {
        "./" BIN_NAME,
        "run",
        (char *)sample_path,
        "--emit-artifacts",
        "--emit-timeline",
        "--artifact-dir",
        (char *)outdir,
        "--run-id",
        "analysis",
        NULL
    };

    return spawn_wait(argv, 0); /* command itself is not echoed by make */
}

static void loom_test(void) {
    loom_clean();
    loom_build();

    char test_dir[4096];
    format_test_dir(test_dir, sizeof(test_dir));
    mkdir_p(test_dir);

    printf("== Running Liminal samples ==\n");
    printf("Artifacts -> %s\n", test_dir);
    fflush(stdout);

    StrVec basic = {0}, fail = {0}, poc = {0};
    collect_sample_files("src/samples/basic", &basic);
    collect_sample_files("src/samples/fail", &fail);
    collect_sample_files("src/samples/poc", &poc);

    /* BASIC — must succeed */
    for (size_t i = 0; i < basic.count; i++) {
        char outdir[4096];
        const char *f = basic.items[i];
        const char *base = strrchr(f, '/'); base = base ? base + 1 : f;

        char name[256];
        snprintf(name, sizeof(name), "%s", base);
        char *dot = strrchr(name, '.'); if (dot) *dot = '\0';

        snprintf(outdir, sizeof(outdir), "%s/%s", test_dir, name);

        int rc = run_liminal_sample(f, outdir);
        if (rc != 0) {
            printf("ERROR: basic sample failed\n");
            vec_free(&basic); vec_free(&fail); vec_free(&poc);
            exit(1);
        }
    }

    /* FAIL — must fail */
    for (size_t i = 0; i < fail.count; i++) {
        char outdir[4096];
        const char *f = fail.items[i];
        const char *base = strrchr(f, '/'); base = base ? base + 1 : f;

        char name[256];
        snprintf(name, sizeof(name), "%s", base);
        char *dot = strrchr(name, '.'); if (dot) *dot = '\0';

        snprintf(outdir, sizeof(outdir), "%s/%s", test_dir, name);

        int rc = run_liminal_sample(f, outdir);
        if (rc == 0) {
            printf("ERROR: expected failure but succeeded\n");
            vec_free(&basic); vec_free(&fail); vec_free(&poc);
            exit(1);
        }
        printf(">>> expected semantic denial\n");
        fflush(stdout);
    }

    /* POC — always run */
    for (size_t i = 0; i < poc.count; i++) {
        char outdir[4096];
        const char *f = poc.items[i];
        const char *base = strrchr(f, '/'); base = base ? base + 1 : f;

        char name[256];
        snprintf(name, sizeof(name), "%s", base);
        char *dot = strrchr(name, '.'); if (dot) *dot = '\0';

        snprintf(outdir, sizeof(outdir), "%s/%s", test_dir, name);

        int rc = run_liminal_sample(f, outdir);
        if (rc != 0) {
            printf(">>> NOTE: PoC semantic denial (exit %d)\n", rc);
            fflush(stdout);
        }
    }

    vec_free(&basic);
    vec_free(&fail);
    vec_free(&poc);
}

/* ============================================================
 * Flatten / flatten-min / flatten-samples (cover the Makefile)
 * ============================================================ */

static void loom_flatten(void) {
    mkdir_p("artifacts");

    FILE *out = fopen("artifacts/liminal_flat.c", "w");
    if (!out) die("fopen artifacts/liminal_flat.c");

    time_t t = time(NULL);
    fprintf(out, "/* ========================================================= */\n");
    fprintf(out, "/*  LIMINAL — SINGLE FILE FLATTENED BUILD                    */\n");
    fprintf(out, "/*  Generated: %s", ctime(&t));
    fprintf(out, "/* ========================================================= */\n");

    /* Headers then sources (sorted like make does) */
    /* We’ll shell out to find/sort in the simplest possible way? No: do a tiny collector. */
    /* For parity with your current Makefile output expectations, we keep this silent. */
    fclose(out);

    /* Re-use your exact Makefile behavior by delegating to sh pipelines without echo.
       This keeps it aligned with what you already tested. */
    char *argv1[] = { "sh", "-c",
        "find src -name '*.h' | sort | while read f; do "
        "echo \"\\n/* === $f ============================================ */\" >> artifacts/liminal_flat.c; "
        "cat \"$f\" >> artifacts/liminal_flat.c; "
        "done", NULL
    };
    if (spawn_wait(argv1, 0) != 0) exit(1);

    char *argv2[] = { "sh", "-c",
        "find src -name '*.c' | grep -v src/samples | sort | while read f; do "
        "echo \"\\n/* === $f ============================================ */\" >> artifacts/liminal_flat.c; "
        "cat \"$f\" >> artifacts/liminal_flat.c; "
        "done", NULL
    };
    if (spawn_wait(argv2, 0) != 0) exit(1);
}

static void loom_flatten_min(void) {
    /* Matches Makefile behavior/printing: building flattener prints cc line; running tool prints its line. */
    mkdir_p("build/tools");
    mkdir_p("artifacts");

    const char *flattener = "build/tools/liminal-flatten";
    int need_build = 0;

    if (!file_exists(flattener)) need_build = 1;
    else if (mtime_of("tools/flatten/flatten.c") > mtime_of(flattener)) need_build = 1;

    if (need_build) {
        char *argv_cc[] = { (char *)CC_PATH, "-std=c99", "-O2", "tools/flatten/flatten.c", "-o", (char *)flattener, NULL };
        if (spawn_wait(argv_cc, 1) != 0) exit(1);
    }

    char *argv_run[] = {
        (char *)flattener,
        "--out",
        "artifacts/liminal_flat.min.c",
        "src/common",
        "src/executor",
        "src/analyzer",
        "src/consumers",
        "src/frontends",
        "src/commands",
        "src/policy",
        "src/main.c",
        NULL
    };
    if (spawn_wait(argv_run, 1) != 0) exit(1);
}

static void loom_flatten_samples(void) {
    mkdir_p("artifacts/samples");

    StrVec basic = {0}, fail = {0}, poc = {0};
    collect_sample_files("src/samples/basic", &basic);
    collect_sample_files("src/samples/fail", &fail);
    collect_sample_files("src/samples/poc", &poc);

    /* Makefile does this silently (all @). We'll be silent too. */
    for (size_t pass = 0; pass < 3; pass++) {
        StrVec *v = (pass == 0) ? &basic : (pass == 1) ? &fail : &poc;
        for (size_t i = 0; i < v->count; i++) {
            const char *f = v->items[i];
            const char *base = strrchr(f, '/'); base = base ? base + 1 : f;

            char name[256];
            snprintf(name, sizeof(name), "%s", base);
            char *dot = strrchr(name, '.'); if (dot) *dot = '\0';

            char outp[4096];
            snprintf(outp, sizeof(outp), "artifacts/samples/%s.flat.c", name);

            FILE *o = fopen(outp, "w");
            if (!o) die("fopen sample flat");

            time_t t = time(NULL);
            fprintf(o, "/* LIMINAL SAMPLE %s */\n", name);
            fprintf(o, "/* Generated: %s", ctime(&t));
            fprintf(o, "*/\n\n");

            FILE *in = fopen(f, "r");
            if (!in) die("fopen sample");

            char buf[8192];
            size_t r;
            while ((r = fread(buf, 1, sizeof(buf), in)) > 0) {
                if (fwrite(buf, 1, r, o) != r) die("fwrite");
            }

            fclose(in);
            fclose(o);
        }
    }

    vec_free(&basic);
    vec_free(&fail);
    vec_free(&poc);
}

/* ============================================================
 * Dispatch
 * ============================================================ */

int loom_main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr,
            "usage: loom <command>\n"
            "commands:\n"
            "  build\n"
            "  test\n"
            "  check\n"
            "  regen-standards\n"
            "  standardise\n"
            "  flatten\n"
            "  flatten-min\n"
            "  flatten-samples\n"
            "  clean\n"
        );
        return 1;
    }

    if (strcmp(argv[1], "build") == 0) {
        loom_build();
        return 0;
    }
    if (strcmp(argv[1], "test") == 0) {
        loom_test();
        return 0;
    }
    if (strcmp(argv[1], "check") == 0) {
        loom_check();
        return 0;
    }
    if (strcmp(argv[1], "regen-standards") == 0) {
        loom_regen_standards();
        return 0;
    }
    if (strcmp(argv[1], "standardise") == 0) {
        loom_standardise();
        return 0;
    }
    if (strcmp(argv[1], "flatten") == 0) {
        loom_flatten();
        return 0;
    }
    if (strcmp(argv[1], "flatten-min") == 0) {
        loom_flatten_min();
        return 0;
    }
    if (strcmp(argv[1], "flatten-samples") == 0) {
        loom_flatten_samples();
        return 0;
    }
    if (strcmp(argv[1], "clean") == 0) {
        loom_clean();
        return 0;
    }

    fprintf(stderr, "loom: unknown command '%s'\n", argv[1]);
    return 1;
}

int main(int argc, char **argv) {
    return loom_main(argc, argv);
}
