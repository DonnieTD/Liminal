#include "standardise.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <dirent.h>
#include <sys/stat.h>

/* ===================================================================== */
/*  Configuration                                                        */
/* ===================================================================== */

#define MAX_PATH 4096
#define MAX_LINE 1024

/* ===================================================================== */
/*  Rule metadata                                                        */
/* ===================================================================== */

typedef enum {
    APPLY_HEADERS = 1 << 0,
    APPLY_TOOLS   = 1 << 1
} RuleApplicability;

typedef struct {
    int id;
    const char *title;
    const char *applies_to;
    const char *description;
    const char *rationale;
    int applicability;
} RuleSpec;

static const RuleSpec RULES[] = {
    {
        1,
        "Header Inclusion Policy",
        ".h files",
        "All header files MUST use classic include guards. "
        "`#pragma once` is forbidden.",
        "Headers define semantic identity boundaries. Classic include guards "
        "are explicit, analyzable, deterministic, and stable under source "
        "flattening and artifact reconstruction.",
        APPLY_HEADERS
    },
    {
        2,
        "Standalone Tool Purity",
        "/tools/*",
        "Standalone tools must be dependency-free and limited in scope.",
        "Standalone tools must remain portable, auditable, deterministic, and "
        "bootstrappable. Dependencies obscure behavior and introduce hidden "
        "coupling.",
        APPLY_TOOLS
    }
};

static const size_t RULE_COUNT = sizeof(RULES) / sizeof(RULES[0]);

/* ===================================================================== */
/*  Utilities                                                            */
/* ===================================================================== */

static int had_failure = 0;

static int is_directory(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int has_suffix(const char *s, const char *suffix)
{
    size_t n = strlen(s), m = strlen(suffix);
    return n >= m && strcmp(s + n - m, suffix) == 0;
}

static int is_tools_path(const char *path)
{
    return strstr(path, "/tools/") != NULL;
}

static void report(const char *file, int line, int rule, const char *msg)
{
    if (!had_failure) {
        printf("FAIL\n\n");
        had_failure = 1;
    }

    printf("%s:%d\n", file, line);
    printf("  Rule #%d:\n", rule);
    printf("    %s\n\n", msg);
}

/* ===================================================================== */
/*  Rule checks                                                          */
/* ===================================================================== */

static void check_header(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[MAX_LINE];
    int lineno = 0;
    int saw_ifndef = 0, saw_define = 0, saw_endif = 0;

    while (fgets(line, sizeof(line), f)) {
        lineno++;

        if (strstr(line, "#pragma once")) {
            report(path, lineno, 1, "forbidden directive: #pragma once");
            break;
        }

        if (strstr(line, "#ifndef")) saw_ifndef = 1;
        if (strstr(line, "#define")) saw_define = 1;
        if (strstr(line, "#endif"))  saw_endif  = 1;
    }

    if (!saw_ifndef || !saw_define || !saw_endif) {
        report(path, 1, 1, "missing or malformed include guard");
    }

    fclose(f);
}

static void check_standalone_tool_file(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[MAX_LINE];
    int lineno = 0;

    while (fgets(line, sizeof(line), f)) {
        lineno++;

        if (strstr(line, "#include \"")) {
            report(path, lineno, 2,
                   "standalone tool includes non-stdlib headers");
            break;
        }

        if (strstr(line, "system(") || strstr(line, "popen(")) {
            report(path, lineno, 2,
                   "standalone tool invokes external utilities");
            break;
        }
    }

    fclose(f);
}

/* ===================================================================== */
/*  Directory walk                                                       */
/* ===================================================================== */

static void walk(const char *root)
{
    DIR *d = opendir(root);
    if (!d) return;

    struct dirent *e;
    while ((e = readdir(d))) {
        if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
            continue;

        char path[MAX_PATH];
        snprintf(path, sizeof(path), "%s/%s", root, e->d_name);

        if (is_directory(path)) {
            if (!strcmp(e->d_name, "build") ||
                !strcmp(e->d_name, "test"))
                continue;
            walk(path);
            continue;
        }

        if (has_suffix(path, ".h")) {
            check_header(path);
        }

        if (has_suffix(path, ".c") && is_tools_path(path)) {
            check_standalone_tool_file(path);
        }
    }

    closedir(d);
}

/* ===================================================================== */
/*  Markdown emission                                                    */
/* ===================================================================== */

static void emit_rules(void)
{
    for (size_t i = 0; i < RULE_COUNT; i++) {
        const RuleSpec *r = &RULES[i];

        printf("## Rule #%d — %s\n\n", r->id, r->title);
        printf("**Applies to:** %s\n\n", r->applies_to);
        printf("**Description**\n\n%s\n\n", r->description);
        printf("**Rationale**\n\n%s\n\n", r->rationale);
        printf("---\n\n");
    }
}

static void emit_full_style_guide(void)
{
    printf(
        "# Code Style Guide\n\n"
        "## 1️⃣ Purpose of CodeStyleGuide.md\n\n"
        "This file is not about aesthetics.  \n"
        "It exists to encode constraints that protect architecture.\n\n"
        "> Every rule must prevent a future mistake you can already imagine.\n\n"
        "This document is a **normative contract** between contributors and the codebase.\n\n"
        "Some sections are **machine-generated** and **must not be edited manually**.\n"
        "Those sections are rendered directly from the `standardise` tool to guarantee\n"
        "that documentation and enforcement never diverge.\n\n"
        "---\n\n"
        "<!-- STANDARDISE:BEGIN -->\n\n"
    );

    emit_rules();

    printf(
        "<!-- STANDARDISE:END -->\n\n"
        "## Notes\n\n"
        "- Rules inside the generated section above are **authoritative**.\n"
        "- Manual edits inside the generated region are not permitted.\n"
        "- To update rules, modify `/tools/standardise` and regenerate this document.\n\n"
        "To regenerate the standards documentation:\n\n"
        "```sh\n"
        "make regen-standards\n"
        "```\n\n"
        "To run enforcement checks:\n\n"
        "```sh\n"
        "make check\n"
        "```\n"
    );
}

/* ===================================================================== */
/*  Entry point                                                          */
/* ===================================================================== */

StandardiseResult standardise_run(const char *root_dir)
{
    had_failure = 0;
    walk(root_dir);

    if (!had_failure) {
        printf("PASS\n");
        return STANDARDISE_OK;
    }

    return STANDARDISE_FAIL;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--emit-style-guide") == 0) {
        emit_rules();
        return 0;
    }

    if (argc == 2 && strcmp(argv[1], "--init-style-guide") == 0) {
        emit_full_style_guide();
        return 0;
    }

    if (argc != 2) {
        fprintf(stderr,
            "usage:\n"
            "  standardise <dir>\n"
            "  standardise --emit-style-guide\n"
            "  standardise --init-style-guide\n");
        return 1;
    }

    return standardise_run(argv[1]);
}
