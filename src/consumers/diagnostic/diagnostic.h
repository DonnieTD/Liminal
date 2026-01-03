#ifndef LIMINAL_CONSUMER_DIAGNOSTIC_H
#define LIMINAL_CONSUMER_DIAGNOSTIC_H

#include "../../analyzer/analyzer.h"

#include "./anchor/anchor.h"
#include "./cause/cause.h"
#include "./diff/diff.h"
#include "./render/render.h"
#include "./anchor/anchor.h"
#include "./stats/stats.h"
#include "./timeline/timeline.h"
#include "./validate/validate.h"

int load_diagnostics(const char *path, DiagnosticArtifact *out);

#endif /* LIMINAL_CONSUMER_DIAGNOSTIC_H */
