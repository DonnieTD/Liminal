# Semantic Execution Engine

## Overview

This project is a semantic execution and analysis engine for C.

It is **not** a compiler, not a traditional VM, not an IR pipeline, not a static analyzer, and not a symbolic executor.

Instead, it treats a program as a **concrete trajectory through semantic state over time**.  
The program is executed once, producing a complete semantic history. All understanding, reasoning, and visualization happens afterward.

Execution is separated from analysis.  
Analysis is separated from visualization.

---

## Core Idea

Programs are not instruction streams.  
Programs are trajectories through state.

Execution is modeled as a sequence of state transitions indexed by time. Bugs, undefined behavior, and exploits are treated as **pathological transitions in that sequence**, not as crashes, warnings, or abstract violations.

The engine answers the question:

How did semantic reality evolve over time, and where did it stop being smooth?

---

## Architecture Summary

The system is split into four strictly separated stages:

1. Parser / Frontend
2. Executor
3. Analyzer
4. Visualization

Data only flows forward.

---

## Stage 1: Parser / Frontend

Responsibility: What is the program?

The frontend parses C source code into an abstract syntax tree (AST).

The AST is:

- immutable
- shared by all executions
- free of execution semantics

The AST represents the terrain the program will traverse.

---

## Stage 2: Executor

Responsibility: What actually happened in this run?

The executor:

- starts from an empty initial state
- walks the AST deterministically
- follows exactly one concrete execution path
- never speculates
- never explores alternatives

For each semantic step:

- one AST node is executed
- a new World is produced
- the new World is linked to the previous one
- the AST node that caused the transition is recorded

Execution produces a linear sequence of Worlds:

World0 <-> World1 <-> World2 <-> ... <-> WorldN

This sequence is the **primary output** of execution.

---

## World

A World represents the complete semantic state of the program at a single moment in time.

Worlds are immutable once created.

A World contains:

- monotonic time index
- pointer to the active scope
- call stack
- memory state
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

Responsibility: What does this execution mean?

The analyzer:

- consumes the World sequence
- never re-executes the program
- never mutates Worlds
- derives artifacts such as:
  - scope lifetimes
  - variable lifetimes
  - memory usage over time
  - call stack evolution
  - invariant drift
  - semantic violations

Analysis is pure, replayable, and repeatable.

---

## Stage 4: Visualization

Responsibility: How do humans understand this?

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

- [x] World structure (immutable semantic state)
- [x] Universe owning time and history
- [x] Bidirectional World timeline
- [x] Step model (semantic causality)
- [x] Arena allocation for Worlds and Steps
- [x] Scope model (immutable, parent-linked)
- [x] Scope enter / exit Steps
- [x] Time advances per semantic event
- [x] Trace iterator (forward + backward)
- [x] Scope lifetime derivation
- [x] Scope invariant validation
- [x] Analyzer / executor separation
- [x] CLI scaffold (run / analyze)

### IN PROGRESS

- [ ] Variable declaration Steps
- [ ] Variable lifetime analysis
- [ ] Name binding to storage locations
- [ ] Scope-to-variable relationship tracking

### NEXT (HELLO WORLD MILESTONE)

- [ ] Minimal frontend AST for:
      - function
      - block
      - variable declaration
      - return
- [ ] AST-driven execution (no more dummy AST)
- [ ] Variable declaration -> storage creation
- [ ] Variable lifetime artifacts
- [ ] Analyzer pass producing variable lifetime ranges
- [ ] Artifact serialization (JSON or similar)

### LATER

- [ ] Memory object modeling
- [ ] Call stack frames
- [ ] Function calls / returns
- [ ] Heap allocation
- [ ] Use-after-free detection
- [ ] Invariant drift detection
- [ ] Visualization frontend
- [ ] Cross-run comparisons

---

## One-Line Summary

Programs are executed once to produce a semantic history.  
Bugs are singularities in that history.
