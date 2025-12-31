# Semantic Execution Engine

## Overview

This project is a semantic execution and analysis engine for C.

It is NOT:
- a compiler
- a traditional VM
- an IR pipeline
- a static analyzer
- a symbolic executor

Instead, it treats a program as a concrete trajectory through semantic
state over time.

The program is executed exactly once, producing a complete semantic
history. All understanding, reasoning, and visualization happens
afterward.

Execution is separated from analysis.
Analysis is separated from visualization.

---

## Core Idea

Programs are not instruction streams.
Programs are trajectories through state.

Execution is modeled as a sequence of state transitions indexed by time.

Bugs, undefined behavior, and exploits are treated as pathological
transitions in that sequence, not as crashes, warnings, or abstract
violations.

The engine answers one question:

How did semantic reality evolve over time, and where did it stop
being smooth?

---

## Architecture Summary

The system is split into four strictly separated stages:

1. Parser / Frontend
2. Executor
3. Analyzer
4. Visualization

Data flows strictly forward.

No stage mutates the output of a previous stage.

---

## Stage 1: Parser / Frontend

Responsibility:
What is the program?

The frontend parses C source code into an abstract syntax tree (AST).

The AST is:
- immutable
- shared by all executions
- free of execution semantics

The AST represents the terrain the program will traverse.

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

Execution produces a linear sequence of Worlds:

World0 <-> World1 <-> World2 <-> ... <-> WorldN

This sequence is the primary output of execution.

---

## World

A World represents the complete semantic state of the program at a single
moment in time.

Worlds are immutable once created.

A World contains:
- monotonic time index
- pointer to the active scope
- call stack (future)
- memory state (future)
- pointer to the Step that caused this World
- links to previous and next Worlds

A World does NOT:
- manage history
- decide control flow
- execute logic
- speculate about alternatives

A World answers only:

What is the state right now, and how did we get here?

---

## Universe

The Universe owns time and history.

The Universe:
- owns the sequence of Worlds
- knows which World is current
- advances execution by creating new Worlds
- allows backward traversal for analysis

Worlds do not know about the Universe.
The Universe knows about Worlds.

This keeps time explicit and prevents recursive corruption.

---

## Semantic Write-Ahead Log

The World sequence acts as a bidirectional semantic write-ahead log.

Forward traversal:
  execution

Backward traversal:
  analysis

Unlike a traditional WAL:
- entries are semantic, not bytes
- Worlds are materialized views of history
- backward traversal is analytical, not destructive

Execution happens once.
Understanding happens arbitrarily many times.

---

## Control Flow

Control flow is not stored as intention.

In a concrete execution:
- there is always exactly one next step
- branches resolve immediately
- loops iterate deterministically

The path taken is recorded implicitly by:
- World ordering
- AST node pointers stored in Steps

The executor does not manage possible paths.

---

## Scope Model

Scope is modeled as nested hashmaps arranged in a rose-tree structure.

ASCII model:

Scope 0 (file)
  |
  +-- Scope 1 (function)
        |
        +-- Scope 2 (block)

Each scope:
- maps names -> storage locations
- is immutable once created
- points to a parent scope

File scope, function scope, and block scope are the same mechanism.
They differ only in creation and destruction time.

Scope lookup:
- if name exists in current scope: in scope
- else: lookup parent
- else: error or undefined behavior

No recovery or guessing.

---

## Lifetime Model

Lifetime is a runtime fact, not a type-system concept.

A storage location is:
- born when created
- alive while reachable in the current World
- dead once no longer reachable

Lifetime is derived from:
- scope existence
- call stack frames
- memory object lifetime

The only question asked:

Is this storage alive now?

---

## Stage 3: Analyzer

Responsibility:
What does this execution mean?

The analyzer:
- consumes the World sequence
- never re-executes the program
- never mutates Worlds
- derives artifacts such as:
  - scope lifetimes
  - variable lifetimes
  - variable use validation
  - semantic invariant violations
  - execution summaries

Analysis is pure, replayable, and repeatable.

---

## Stage 4: Visualization

Responsibility:
How do humans understand this?

The visualization layer:
- ingests analysis artifacts
- renders timelines, graphs, and comparisons
- provides interactive exploration

It performs no execution and no semantic inference.

---

## Why This Exists

Most tools answer:
What did the machine do?

This engine answers:
How did semantic reality evolve over time, and where did it break?

Bugs become structural distortions in time, not mysterious failures.

---

## Non-Goals

This project does NOT aim to:
- replace production compilers
- be fast
- fully model all of C immediately
- statically prevent bugs
- infer programmer intent

It optimizes for clarity, observability, and semantic truth.

---

## Design Principles

- Execute once, analyze forever
- Prefer observation over inference
- Prefer explicit state over abstraction
- Time must always be explicit
- If a structure does not explain semantic drift, it does not belong

---

## Roadmap / TODO

### DONE

Core execution model:
- [x] World structure (immutable semantic state)
- [x] Universe owning time and history
- [x] Bidirectional World timeline
- [x] Monotonic time advancement per semantic event
- [x] Arena allocation for Worlds, Steps, Scopes, Storage
- [x] Explicit Step model (semantic causality)

Scopes:
- [x] Immutable scope model (parent-linked)
- [x] Scope enter / exit Steps
- [x] Scope lifetime derivation
- [x] Scope invariant validation

Variables and storage:
- [x] Variable declaration Step (STEP_DECLARE)
- [x] Storage objects with unique IDs
- [x] Name binding via persistent hashmaps
- [x] Scope -> storage association

Use semantics:
- [x] Variable use Step (STEP_USE)
- [x] Analyzer validation of STEP_USE
- [x] Use-before-declare detection
- [x] Use-after-scope-exit detection

Analysis infrastructure:
- [x] Trace iterator (forward and backward)
- [x] Analyzer / executor separation
- [x] Pure analysis passes

CLI:
- [x] Deterministic execution scaffold
- [x] Raw timeline output
- [x] Trace output
- [x] Scope lifetime reporting
- [x] Use validation reporting

---

### IN PROGRESS

- [ ] Precise variable lifetime intervals
- [ ] Shadowing detection
- [ ] Multiple declaration validation
- [ ] Structured diagnostic output

---

### NEXT (HELLO WORLD MILESTONE)

Frontend integration:
- [ ] Minimal real frontend AST for:
      - function
      - block
      - variable declaration
      - variable use
      - return
- [ ] Remove dummy AST scaffolding
- [ ] AST-driven execution

Execution semantics:
- [ ] Variable declaration driven by AST
- [ ] Variable use driven by AST
- [ ] Return semantics
- [ ] End-of-function teardown

Artifacts:
- [ ] Variable lifetime ranges
- [ ] Stable artifact schema
- [ ] Artifact serialization (JSON or similar)

---

### LATER

- [ ] Call stack frames
- [ ] Function calls and returns
- [ ] Heap allocation modeling
- [ ] Use-after-free detection
- [ ] Memory lifetime analysis
- [ ] Invariant drift detection
- [ ] Visualization frontend
- [ ] Cross-run comparisons

---

## Milestone Definition: Hello World

A program is considered supported when:
- source is parsed into an AST
- AST drives execution
- a variable is declared and used
- lifetime is derived correctly
- violations are reported if present
- artifacts are emitted

---

## One-Line Summary

Programs are executed once to produce a semantic history.
Bugs are singularities in that history.
