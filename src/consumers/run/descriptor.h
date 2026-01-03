#ifndef LIMINAL_RUN_DESCRIPTOR_H
#define LIMINAL_RUN_DESCRIPTOR_H

/*
 * RunDescriptor
 *
 * Pure structural identity for a run directory.
 * No IO, no validation, no semantics.
 */
typedef struct RunDescriptor {
    const char *root_dir;
    const char *run_id;

    const char *meta_path;
    const char *diagnostics_path;
    const char *timeline_path;
} RunDescriptor;

#endif /* LIMINAL_RUN_DESCRIPTOR_H */
