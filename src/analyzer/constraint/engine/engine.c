#include "analyzer/constraint/engine/engine.h"
#include "analyzer/constraint/variable/variable.h"
#include "analyzer/constraint/decleration/declaration.h"
#include <stdlib.h>
#include <string.h>

ConstraintArtifact analyze_constraints(struct World *head)
{
    ConstraintArtifact a = analyze_variable_constraints(head);
    ConstraintArtifact b = analyze_declaration_constraints(head);

    /* Temporary merge (Stage 4 discipline) */
    size_t total = a.count + b.count;
    Constraint *buf = calloc(total, sizeof(Constraint));

    if (!buf)
        return a;

    memcpy(buf, a.items, a.count * sizeof(Constraint));
    memcpy(buf + a.count, b.items, b.count * sizeof(Constraint));

    free(a.items);
    free(b.items);

    return (ConstraintArtifact){
        .items = buf,
        .count = total
    };
}
