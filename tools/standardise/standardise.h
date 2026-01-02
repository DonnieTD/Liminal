#ifndef LIMINAL_TOOLS_STANDARDISE_H
#define LIMINAL_TOOLS_STANDARDISE_H

typedef enum {
    STANDARDISE_OK   = 0,
    STANDARDISE_FAIL = 1
} StandardiseResult;

/* Run all enforceable style rules on a directory tree */
StandardiseResult standardise_run(const char *root_dir);

/* Emit canonical CodeStyleGuide.md content (rules section only) */
void standardise_emit_style_guide(void);

#endif /* LIMINAL_TOOLS_STANDARDISE_H */
