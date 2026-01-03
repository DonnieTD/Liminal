#ifndef LIMINAL_USE_REPORT_H
#define LIMINAL_USE_REPORT_H

#include <stdint.h>

typedef enum UseKind {
    USE_OK = 0,
    USE_BEFORE_DECLARE,
    USE_AFTER_SCOPE
} UseKind;

typedef struct UseReport {
    uint64_t time;
    uint64_t storage_id;   /* UINT64_MAX if unresolved */
    uint64_t scope_id;
    UseKind  kind;
} UseReport;

#endif
