# Liminal Toolchain

This repository contains multiple cooperating tools.  
They are **not helpers** — they are part of the system.

Each tool exists to preserve one invariant:

- Meaning
- Determinism
- Auditability
- Reproducibility

This document explains the **three auxiliary tools** that extend the Liminal semantic engine.

---

## Tool Overview

| Tool         | Purpose                                  | Role |
|--------------|-------------------------------------------|------|
| **loom**     | Build & orchestration authority            | Control |
| **flatten**  | Single-file source generation              | Transparency |
| **standardise** | Code normalization & style enforcement | Stability |

These tools are designed to work together but can be invoked independently.

---

## 1. Loom — Authoritative Orchestration

**Loom is the control plane of the project.**

It exists to answer one question:

> *“How do we know the system we just ran is the system we intended?”*

### Responsibilities

- Rebuilds itself on every invocation
- Owns build, test, flatten, and verification flows
- Provides deterministic execution modes
- Acts as a semantic replacement for Make
- Verifies Loom output against Make as a reference oracle

### Why Loom Exists

Make answers:
> “Are these files up to date?”

Loom answers:
> “Is the *meaning* of the system preserved?”

This distinction is critical once artifacts, policies, and timelines exist.

### Entry Point

All Loom interaction happens via:

```sh
    ./loom.sh
```

This script:
- Rebuilds `tools/loom/loom`
- Sets execution invariants
- Dispatches commands safely

### Common Commands

```sh
    ./loom.sh build
    ./loom.sh test
    ./loom.sh check
    ./loom.sh flatten
    ./loom.sh verify-make
```

### Deterministic Mode

Loom supports an opt-in deterministic execution mode:

```sh
    ./loom.sh --deterministic test
```


This forces:
- Fixed output directories
- Stable artifact paths
- Diff-friendly outputs

Used for:
- CI
- Regression testing
- Make vs Loom verification

---

## 2. Flatten — Single-File Source Generator

**Flatten exists for auditability and transport.**

It converts the entire Liminal codebase into a **single valid C file**.

### Why Flatten Exists

Flattening enables:

- Whole-program review
- Archival snapshots
- LLM ingestion
- External auditing
- Compiler archaeology
- “Show me the whole system” moments

The flattened file is not optimized for compilation speed — it is optimized for **truth**.

### Outputs

Flattened files live in:

```sh
    artifacts/
    ├── liminal_flat.c
    ├── liminal_flat.min.c
    └── samples.flat.txt
```


### Variants

- **Full flatten** — preserves comments, headers, structure
- **Minified flatten** — compact, repo-correct ordering
- **Sample flattening** — one-file-per-sample

### Usage

Via Loom:

```sh
    ./loom.sh flatten
    ./loom.sh flatten-min
    ./loom.sh flatten-samples
```


Or directly via the tool if needed.

---

## 3. Standardise — Code Normalization

**Standardise enforces consistency over time.**

It ensures that code style does not drift as the system evolves.

### Responsibilities

- Enforces formatting and structure rules
- Validates source trees
- Regenerates the canonical style guide
- Acts as a semantic hygiene layer

### Why Standardise Exists

Style drift introduces:
- Accidental complexity
- Review friction
- False diffs
- Cognitive overhead

Standardise ensures that diffs reflect **meaningful change only**.

### Code Style Guide

The canonical rules live in:

```sh
    CodeStyleGuide.md
```


This file is **generated**, not hand-edited.

### Usage

```sh
    ./loom.sh check
    ./loom.sh standardise
    ./loom.sh regen-standards
```


---

## Toolchain Philosophy

These tools follow strict principles:

### 1. No Silent State

- Every output is explicit
- Every artifact is intentional
- Temp data lives in `tmp/`

### 2. Determinism Is Optional but Enforceable

- Fast iteration by default
- Exact reproduction when needed

### 3. Make Is a Reference, Not an Authority

- Make remains temporarily
- Loom is the future authority
- Semantic equivalence is enforced via diffing

### 4. Artifacts > Logs

- Outputs are structured
- Logs are secondary
- Files are the truth

---

## How the Tools Fit Together

```sh
    Source Code
    ↓
    Standardise → Style Stability
    ↓
    Liminal → Semantic Artifacts
    ↓
    Flatten → Whole-System Transparency
    ↓
    Loom → Reproducibility & Verification
```


---

## Status

These tools were built in rapid succession but are **intentional**, not experimental.

They form the foundation for:

- Security analysis
- Semantic diffing
- Policy-driven compilation
- Long-term system archaeology

Each tool exists because the system **outgrew the previous abstraction**.

---
