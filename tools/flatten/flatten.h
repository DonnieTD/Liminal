#ifndef LIMINAL_FLATTEN_H
#define LIMINAL_FLATTEN_H

/*
 * liminal-flatten
 *
 * Deterministically flattens a set of C source trees into a single file.
 *
 * Guarantees:
 *  - Stable traversal order
 *  - System includes preserved
 *  - Local includes removed
 *  - Header guards stripped
 *  - "//" comments stripped
 *  - block comments stripped
 *  - Source boundaries marked
 *  - UTC timestamped banner
 *
 * Intended for:
 *  - LLM ingestion
 *  - Semantic diffing
 *  - Artifact hashing
 *  - Reproducible builds
 */

/* CLI entrypoint */
int flatten_main(int argc, char **argv);

#endif /* LIMINAL_FLATTEN_H */
