# Liminal

> A semantic analysis engine for C programs  
> Built to explain what code does — not just whether it compiles.

---

## Overview

**Liminal** is a semantic execution and analysis engine for C.

Unlike traditional compilers or static analyzers, Liminal does not stop at syntax or type correctness.  
It models **program behavior as a timeline**, applies **policy constraints**, and emits **semantic artifacts** that explain *why* a program is valid, invalid, or dangerous.

Liminal is designed to be:

- Deterministic
- Artifact-driven
- Policy-aware
- Auditable

This repository contains not just Liminal itself, but a complete **semantic toolchain** used to build, normalize, flatten, verify, and compare program meaning.

---

## Core Concepts

### 1. Programs as Executions (Not Just Trees)

Liminal treats a program as something that **happens over time**.

Every input program is transformed into:

- An **AST** (existence)
- An **execution timeline** (time)
- A **semantic world model** (meaning)

This allows Liminal to reason about:

- Variable lifetimes
- Scope boundaries
- Declaration vs use ordering
- Shadowing
- Redeclaration
- Policy violations

---

### 2. Semantic Artifacts (The Primary Output)

Liminal does not “print errors”.

It emits **artifacts**.

Each run produces a structured artifact directory containing:

- `meta.json` — run metadata
- `timeline.ndjson` — ordered execution events
- `diagnostics.ndjson` — semantic violations and observations

These artifacts are:
- Stable
- Diffable
- Machine-readable
- Human-auditable

They are the canonical output of the system.

---

### 3. Policy-Driven Semantics

Semantic correctness is defined by **policy**, not hard-coded rules.

Policies define constraints such as:

- Variables must be declared before use
- Redeclarations are forbidden
- Shadowing is reported (or denied)
- Execution may be conditionally allowed or denied

Policies are enforced during execution, not after the fact.

---

## Repository Structure

High-level structure (build and temp artifacts omitted):

```sh
src/ Core engine (frontend, executor, analyzer)
tools/ Auxiliary semantic tools
├── loom/ Authoritative build & orchestration tool
├── flatten/ Single-file source flattener
└── standardise/ Code-style normalization tool
artifacts/ Flattened and generated reference outputs
samples/ Semantic test programs
loom.sh Canonical entrypoint for all operations
```


## The Toolchain

This repository contains **four cooperating tools**:

### 1. `liminal` — Semantic Engine

The core binary.

Responsible for:
- Parsing C
- Executing semantic timelines
- Enforcing policy
- Emitting artifacts

Example:

```sh
  ./liminal run sample.c --emit-artifacts --emit-timeline
```

### 2. `loom` — Orchestration & Authority

Loom is the **authoritative build and execution controller**.

Responsibilities:
- Rebuild tools deterministically
- Run semantic tests
- Compare outputs against Make
- Enforce reproducibility

Loom replaces Make over time.

Example:

```sh
./loom.sh test
./loom.sh verify-make
```

### 3. `flatten` — Single-File Source Generator

Flattens the entire project into a single C file.

Used for:
- Auditing
- Review
- LLM ingestion
- Long-term archival

Outputs live in `artifacts/`.

---

### 4. `standardise` — Code Normalization

Enforces and regenerates the project’s code style rules.

Used to:
- Prevent stylistic drift
- Ensure deterministic formatting
- Generate `CodeStyleGuide.md`

---

## Determinism & Verification

Liminal is built to be **provably reproducible**.

- Deterministic execution modes
- Temporary-only test artifacts
- Semantic diffs between Make and Loom
- Non-semantic metadata normalization

Verification command:

```sh
  ./loom.sh verify-make
```


A successful run guarantees **semantic equivalence** between build systems.

---

## What Liminal Is Not

- ❌ Not a compiler
- ❌ Not a linter
- ❌ Not a type checker
- ❌ Not a formatter

Liminal answers a different question:

> *“What does this program actually do — and is that allowed?”*

---

## Status

This project is under **active, rapid development**.

The architecture is stable.  
The surface area is expanding.

Roadmap and future direction live in `RoadMap.md`.

---

## License

See `LICENSE`.

---

## Contributing

See `Contribute.md`.


---

## Tools

See `Tools.md`