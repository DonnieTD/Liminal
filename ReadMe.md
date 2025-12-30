# Semantic Execution Engine

## Overview

This project is a semantic execution and analysis engine for C.

It is not a compiler, not a traditional VM, not an IR pipeline, not a static analyzer, and not a symbolic executor.

Instead, it treats a program as a concrete trajectory through semantic state over time. The program is executed once, producing a complete semantic history. All understanding, reasoning, and visualization happens afterward.

Execution is separated from analysis. Analysis is separated from visualization.

---

## Core Idea

Programs are not instruction streams.  
Programs are trajectories through state.

Execution is modeled as a sequence of state transitions indexed by time. Bugs, undefined behavior, and exploits are treated as pathological transitions in that sequence, not as crashes, warnings, or abstract violations.

The engine answers the question:

How did semantic reality evolve over time, and where did it stop being smooth?

---

## Architecture Summary

The system is split into four strictly separated stages:

1. Parser / Frontend
2. Executor
3. Analyzer
4. Visualization

Each stage has exactly one responsibility and data only flows forward.

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

This sequence is the primary output of execution.

Execution is deliberately minimal and explicit.

---

## World

A World represents the complete semantic state of the program at a single moment in time.

Worlds are immutable once created.

A World contains:

- a pointer to the active scope
- the call stack
- a pointer to the AST node that produced this state
- links to the previous and next World

A World does not:

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

This prevents recursive state corruption and keeps time explicit.

---

## Semantic Write-Ahead Log

The sequence of Worlds functions as a bidirectional semantic write-ahead log.

Forward traversal corresponds to execution.
Backward traversal corresponds to analysis.

Unlike a traditional WAL:

- entries are semantic, not bytes
- Worlds are materialized views of history
- backward traversal is analytical, not destructive

Execution happens once.
Understanding can happen arbitrarily many times.

---

## Control Flow

Control flow is not stored as state.

In a concrete execution:

- there is always exactly one next step
- branches resolve immediately
- loops iterate deterministically

Control flow is treated as history, not intention.

The path taken is recorded implicitly by:

- the ordering of Worlds
- the AST node pointers stored in each World

The executor does not manage possible paths.

---

## Scope Model

Scope is modeled as nested hashmaps arranged in a rose-tree structure.

Each scope:

- is a hashmap mapping names to storage locations
- is immutable once created
- has a parent scope (except the root)

File scope, function scope, and block scope are all the same mechanism. They differ only in when they are created and destroyed.

At runtime, execution walks exactly one branch of the scope tree.

Scope resolution is a hashmap lookup:

- if the name exists, it is in scope
- if the name does not exist, that is an error or undefined behavior

No recovery or guessing is performed.

---

## Lifetime Model

Lifetime is treated as a runtime fact, not a type-system concept.

A storage location is:

- born when it is created
- alive while it exists in the current World
- dead once it no longer exists

Lifetime is determined by:

- scope existence
- call stack frames
- memory object lifetime

Lifetime is simply the question:
Is this storage alive now?

---

## Stage 3: Analyzer

Responsibility: What does this execution mean?

The analyzer:

- consumes the World sequence
- never re-executes the program
- never mutates Worlds
- derives artifacts such as:
  - memory usage over time
  - call stack evolution
  - lifetime violations
  - invariant drift
  - exploit motifs
  - summaries and metrics

Analysis is pure, replayable, and repeatable.

---

## Stage 4: Visualization

Responsibility: How do humans understand this?

The visualization layer:

- ingests analysis artifacts
- renders timelines, graphs, and comparisons
- provides interactive exploration

It performs no execution, no inference, and no semantic reasoning.

---

## Why This Exists

Most tools answer:
What did the machine do?

This engine answers:
How did semantic reality evolve over time, and where did it break?

By making time and state explicit, bugs and exploits become observable as structural distortions rather than mysterious failures.

---

## Non-Goals

This project does not aim to:

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
- If a structure does not help explain semantic drift, it does not belong in the executor
- Abstraction must be derived from concrete behavior, not imposed upfront

---

## One-Line Summary

Programs are executed once to produce a semantic history.  
Bugs are singularities in that history.
