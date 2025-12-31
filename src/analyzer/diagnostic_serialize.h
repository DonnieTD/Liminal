#ifndef LIMINAL_DIAGNOSTIC_SERIALIZE_H
#define LIMINAL_DIAGNOSTIC_SERIALIZE_H

#include <stdio.h>
#include "analyzer/diagnostic.h"

/*
 * Serialize diagnostics to NDJSON.
 */
void diagnostic_serialize_ndjson(
    FILE *out,
    const DiagnosticArtifact *a
);

/*
 * Deserialize a single diagnostic from an NDJSON stream.
 *
 * Returns:
 *   1 on success
 *   0 on EOF or parse failure
 *
 * Does NOT allocate.
 */
int diagnostic_deserialize_line(
    FILE *in,
    Diagnostic *out
);

#endif /* LIMINAL_DIAGNOSTIC_SERIALIZE_H */
