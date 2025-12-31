# Semantic Execution Engine (Liminal)

## Overview

Liminal is a semantic execution and analysis engine for C.

It is NOT:
- a compiler
- a traditional virtual machine
- an IR pipeline
- a static analyzer
- a symbolic executor

Instead, Liminal treats a program as a concrete trajectory through semantic
state over time.

A program is executed exactly once, producing a complete semantic
history. All understanding, reasoning, and visualization happens
afterward.

Execution is separated from analysis.
Analysis is separated from visualization.

---

## Core Idea

Programs are not instruction streams.
Programs are trajectories through state.

Execution is modeled as a sequence of immutable semantic states indexed
by time.

Bugs, undefined behavior, and exploits are treated as pathological
transitions in that sequence — not as crashes, warnings, or abstract
violations.

The engine answers one question:

How did semantic reality evolve over time, and where did it stop being smooth?

---

## Architecture Summary

The system is split into strictly separated stages:

1. Frontend / Parser
2. Executor
3. Analyzer
4. Constraint Engine
5. Artifact Layer
6. Visualization
7. Cross-Run Reasoning

Data flows strictly forward.

No stage mutates the output of a previous stage.

---

## Stage 1: Frontend / Parser

Responsibility:
What is the program?

The frontend parses C source code into an abstract syntax tree (AST).

The AST is:
- immutable
- execution-agnostic
- shared by all runs
- free of semantic effects

The AST represents the terrain the program will traverse.

Primary artifact:
- AST (arena-owned, stable IDs)

---

## Stage 2: Executor

Responsibility:
What actually happened in this run?

The executor:
- starts from an empty initial state
- walks the AST deterministically
- follows exactly one concrete execution path
- never speculates
- never explores alternatives

For each semantic event:
- one AST node is involved
- a new World is created
- the World is linked to the previous World
- the semantic cause is recorded as a Step

Execution produces a linear timeline:

World0 <-> World1 <-> World2 <-> ... <-> WorldN

Primary artifacts:
- World timeline
- Step stream

---

## World

A World represents the complete semantic state of the program at a single
moment in time.

Worlds are immutable once created.

A World contains:
- monotonic time index
- pointer to the active lexical scope
- call stack (future)
- memory state (future)
- pointer to the Step that caused this World
- links to previous and next Worlds

A World does NOT:
- manage history
- decide control flow
- execute logic
- speculate about alternatives

---

## Universe

The Universe owns time and history.

The Universe:
- owns the full World timeline
- knows which World is current
- advances execution by creating new Worlds
- allows backward traversal for analysis

Worlds do not know about the Universe.
The Universe knows about Worlds.

---

## Semantic Write-Ahead Log

The World timeline acts as a bidirectional semantic write-ahead log.

Forward traversal:
- execution

Backward traversal:
- analysis

Execution happens once.
Understanding happens arbitrarily many times.

---

## Scope Model

Scope is modeled as nested hashmaps arranged in a rose-tree structure.

Each scope:
- maps names to storage locations
- is immutable once created
- points to a parent scope

Primary artifacts:
- Scope graph
- Scope lifetime intervals

---

## Lifetime Model

Lifetime is a runtime fact, not a type-system concept.

A storage location is:
- born when created
- alive while reachable
- dead once unreachable

Primary artifacts:
- Storage lifetime ranges
- Scope lifetime ranges

---

## Stage 3: Analyzer

Responsibility:
What does this execution mean structurally?

The analyzer:
- consumes the World timeline
- never re-executes
- never mutates Worlds
- derives semantic facts

Primary artifacts:
- ScopeLifetime
- VariableLifetime
- ShadowReport
- UseReport
- Diagnostic

Analysis is pure, replayable, and repeatable.

Stage 3 ends at:
- structural correctness
- temporal correctness
- name and lifetime validity

No value reasoning.

---

## Stage 4: Constraint Engine (Planned)

Responsibility:
What semantic rules must hold?

This stage introduces constraints, not execution.

Examples:
- type compatibility
- definite assignment
- return-path completeness
- control-flow soundness

Primary artifacts:
- Constraint graphs
- Constraint violations
- Justification traces

---

## Stage 5: Artifact Layer (Planned)

Responsibility:
Make meaning stable.

This layer:
- defines stable schemas for all artifacts
- version-controls semantic output
- enables serialization and tooling

Primary artifacts:
- JSON schemas
- Binary snapshots
- Artifact diffs

---

## Stage 6: Visualization (Planned)

Responsibility:
Make time visible.

The visualization layer:
- consumes artifacts only
- renders timelines, graphs, and comparisons
- supports interactive exploration

Primary artifacts:
- Timelines
- Scope trees
- Lifetime bands
- Diagnostic overlays

---

## Stage 7: Cross-Run Reasoning (Planned)

Responsibility:
Compare realities.

This stage enables:
- regression detection
- exploit delta analysis
- semantic drift tracking

Primary artifacts:
- Cross-run diffs
- Temporal alignment maps
- Invariant stability reports

---

## Why This Exists

Most tools answer:
What did the machine do?

Liminal answers:
How did semantic reality evolve over time, and where did it break?

---

## Design Principles

- Execute once, analyze forever
- Prefer observation over inference
- Prefer explicit state over abstraction
- Time must always be explicit
- Artifacts over opinions

---

## Roadmap Status

### Stage 1: DONE
### Stage 2: DONE
### Stage 3: DONE (v0.3.8)
### Stage 4: PLANNED
### Stage 5: PLANNED
### Stage 6: PLANNED
### Stage 7: PLANNED

---

## Milestone: Hello World

A program is supported when:
- AST drives execution
- execution produces Worlds
- variables are declared and used
- lifetimes are derived
- violations are reported
- artifacts are emitted

---

## One-Line Summary

Programs are executed once to produce a semantic history.
Artifacts let us understand it forever.
