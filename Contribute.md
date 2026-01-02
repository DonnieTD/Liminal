## Contributing to Liminal

This document explains the internal structure of Liminal and how its parts fit together.

Liminal is not a conventional compiler or analyzer.
It is a semantic execution system with strict stage separation and immutable artifacts.

If you are contributing, assume:

execution happens once

meaning is derived later

nothing mutates upstream artifacts

determinism is non-negotiable

## Repository Overview

At the top level:

.
├── Makefile
├── ReadMe.md
├── CONTRIBUTE.md
├── src/
├── artifacts/
├── test/
├── build/        (generated)
└── liminal       (binary)


Only src/, artifacts/, and test/ are part of the semantic system.
build/ is purely mechanical and intentionally excluded here.

## Makefile

The Makefile enforces:

strict C99

no implicit declarations

no hidden link-time behavior

full dependency tracking

There is no build magic.
If something links, it is because it was explicitly compiled.

## artifacts/

This directory contains reference material, not live code.

## liminal_flat.c

A fully flattened version of the system.

## Purpose:

- auditing

- review

- long-term archival

- external analysis (LLMs, static tools, research)

This file is not authoritative.
It is a snapshot of meaning, not a development surface.

## samples.flat.txt

Flattened semantic outputs from sample runs.

Used to:

validate determinism

visually inspect semantic stability

reason about diffs without tooling

## src/ — The System Itself

The src tree is organized by semantic role, not by convenience.

Nothing here is accidental.

## src/frontends/c/

Stage 1 — Frontend / Parser

Files:

lexer.*

parser.*

ast.*

frontend.*

ast_dump.*

## Responsibilities:

parse C into an AST

assign stable node IDs

perform no semantic reasoning

The AST:

is immutable

is execution-agnostic

is shared across all runs

If you add syntax here, you must justify the semantic necessity downstream.

## src/executor/

## Stage 2 — Execution

Key files:

executor.*

universe.*

world.*

step.*

scope.*

stack.*

memory.*

## Responsibilities:

walk the AST deterministically

produce a linear sequence of World states

record each semantic transition as a Step

Important invariants:

execution happens exactly once

no speculation

no branching

no re-execution during analysis

The executor does not validate semantics.
It only records what happened.

## src/analyzer/

## Stage 3 — Semantic Derivation

Files here derive facts from execution:

Examples:

lifetime.*

variable_lifetime.*

trace.*

use.*

validate.*

# Rules:

analyzers consume World timelines

analyzers never mutate Worlds

analyzers never enforce policy

Output:

intermediate semantic structures

raw diagnostic candidates

## src/analyzer/constraint*

## Stage 4 — Constraint Engine

## Files:

constraint_engine.*

constraint_*.{c,h}

constraint_diagnostic.*

# Purpose:

define semantic rules as constraints

separate “what is true” from “what is allowed”

Constraints are:

structural

deterministic

presentation-free

They do not decide outcomes.

## src/analyzer/diagnostic*

## Stage 5 — Stable Meaning

## Files:

diagnostic.*

diagnostic_id.*

diagnostic_project.*

diagnostic_serialize.*

artifact_emit.*

Responsibilities:

assign stable DiagnosticIds

anchor diagnostics to source structure

serialize diagnostics as NDJSON

This stage is what makes cross-run comparison possible.

Once a DiagnosticId exists, it must remain stable unless meaning changes.

## src/consumers/

This directory contains post-execution reasoning.

Nothing here:

re-executes code

inspects ASTs

mutates artifacts

Core Consumers

load_run.*
Loads a run snapshot (diagnostics + timeline)

load_diagnostics.*
Reads diagnostic artifacts

load_timeline.* / timeline_extract.*
Reads emitted execution timelines

Timeline & Diffing

timeline_emit.*
Emits human and NDJSON timelines

timeline_diff.*
Compares timelines across runs

## Stage 7 guarantee:

identical meaning → identical output

Diagnostic Diffing

diagnostic_diff.*

semantic_diff.*

semantic_diff_render.*

## Used for:

regression detection

semantic drift tracking

stability validation

No AST access.
No execution.
Artifacts only.

Root Cause & Structure

## Stage 8 — Semantic Surfaces

Files:

root_chain*

root_cause*

cause_key*

convergence_*

fix_surface_*

scope_*

These build ephemeral semantic structures such as:

causal chains per diagnostic

convergence points

minimal fix surfaces

role-annotated timelines

Important:

these are derived

they are not persisted

they are safe to rebuild arbitrarily

This is where meaning becomes actionable.

## src/commands/

CLI surface only.

## Files:

cmd_analyze.*

cmd_diff.*

cmd_policy.*

command_dispatch.*

Commands orchestrate:

loading artifacts

invoking consumers

rendering output

They do not perform semantic reasoning themselves.

## src/common/

# #Infrastructure utilities:

arena.* — deterministic allocation

hashmap.*

file.*, fs.*

shared types and helpers

No semantic logic belongs here.

## src/policy/

Authority Layer

Files:

policy.*

default_policy.*

## Policies:

consume diagnostics only

decide allow / warn / deny

never affect analysis

You can swap policies without invalidating artifacts.

## src/samples/

Executable semantic tests.

## Categories:

basic/ — must succeed

fail/ — must be denied

poc/ — allowed to vary

These are semantic fixtures, not unit tests.

## test/

Historical run artifacts.

Each directory is a frozen semantic snapshot:

meta.json

diagnostics.ndjson

timeline.ndjson

Used to:

validate determinism

test cross-run diffing

audit semantic evolution

## Coding Standards

Liminal enforces a strict, machine-checkable coding standard.

All contributors **MUST** run the `standardise` tool before submitting changes.

### Required Checks

- Header include guards
- Standalone tool purity
- Dependency boundaries

### How to Run

```sh
make check
```

This command runs the standards checker over:

- ./src

- ./tools

Submissions that fail standardisation checks will be rejected.

## Regenerating the Style Guide

The canonical style rules are rendered directly from the enforcement tool.

To regenerate the standards documentation:

```sh
    make regen-standards
```

Manual edits inside the generated section of CodeStyleGuide.md are not permitted.