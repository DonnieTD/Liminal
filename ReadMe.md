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

A program is executed exactly once, producing a complete semantic history.
All understanding, reasoning, validation, and visualization happens
after execution.

Execution is separated from analysis.
Analysis is separated from presentation.
Artifacts are separated from policy.

---

## Core Idea

Programs are not instruction streams.

Programs are trajectories through semantic reality.

Execution is modeled as a sequence of immutable semantic states indexed by
time.

Bugs, undefined behavior, and exploits are treated as pathological
transitions in that sequence — not as crashes, warnings, or abstract rule
violations.

The engine answers one question:

**How did semantic reality evolve over time, and where did it stop being smooth?**

---

## Architecture Summary

The system is split into strictly ordered stages:

1. Frontend / Parser  
2. Executor  
3. Analyzer  
4. Constraint Engine  
5. Artifact Layer  
6. Visualization  
7. Cross-Run Reasoning  
8. Semantic Surface Construction  

Data flows strictly forward.
No stage mutates the output of a previous stage.

---

## Stage 1: Frontend / Parser

**Responsibility:** What exists?

The frontend parses C source code into an abstract syntax tree (AST).

The AST is:
- immutable
- execution-agnostic
- shared by all runs
- free of semantic effects

The AST represents the terrain the program will traverse.

**Primary artifact**
- `ASTProgram` (arena-owned, stable node IDs)

---

## Stage 2: Executor

**Responsibility:** In what order did existence occur?

The executor:
- starts from an empty initial state
- walks the AST deterministically
- follows exactly one concrete execution path
- never speculates
- never explores alternatives

For each semantic event:
- one AST node is involved
- a new `World` is created
- the `World` is linked to the previous `World`
- the semantic cause is recorded as a `Step`

Execution produces a linear timeline:

World₀ → World₁ → World₂ → … → Worldₙ


**Primary artifacts**
- World timeline
- Step stream

---

## World

A `World` represents the complete semantic state of the program at a single
moment in time.

Worlds are immutable once created.

A World contains:
- monotonic time index
- active lexical scope
- the Step that caused this World
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
- advances execution by creating new Worlds
- allows backward traversal for analysis

Worlds do not know about the Universe.
The Universe knows about Worlds.

---

## Semantic Write-Ahead Log

The World timeline acts as a bidirectional semantic write-ahead log.

- Forward traversal → execution
- Backward traversal → analysis

Execution happens once.
Understanding happens arbitrarily many times.

---

## Policy Enforcement

Liminal separates **diagnostics** from **authority**.

Diagnostics describe semantic facts.
Policies decide whether those facts are acceptable.

Policies:
- consume diagnostic artifacts only
- are deterministic
- may allow, warn, or deny execution

Policy decisions are explicit and visible.

---

## Stage 3: Analyzer

**Responsibility:** What structural facts can be derived from this execution?

The analyzer:
- consumes the World timeline
- never re-executes
- never mutates Worlds
- derives semantic facts

**Primary artifacts**
- Scope lifetime
- Variable lifetime
- Use reports
- Structural violations

---

## Stage 4: Constraint Engine

**Responsibility:** What semantic rules must hold?

This stage introduces **constraints**, not diagnostics.

Constraints express facts about semantic reality, independent of UX or
presentation.

Examples:
- a variable use must have a declaration
- a declaration may not redeclare in the same scope
- a declaration may not shadow a parent binding
- a use may not occur after scope exit

**Primary artifacts**
- Constraint sets
- Constraint kinds

---

## Stage 5: Artifact Layer — DONE

**Responsibility:** Make meaning stable.

This layer:
- assigns stable identity to semantic facts
- anchors diagnostics to source structure
- defines serializable, versioned artifacts
- enables cross-run comparison

**Primary artifacts**
- `DiagnosticArtifact`
- `DiagnosticId` (stable across runs)
- NDJSON diagnostic streams

---

## Stage 6: Visualization / Timeline Emission — DONE

**Responsibility:** Make time visible.

Capabilities:
- linear timeline emission
- step-by-step temporal traces
- AST ↔ Step ↔ Time correlation
- policy-gated execution outcomes

All output is deterministic and derived solely from execution history.

---

## Stage 7: Cross-Run Reasoning — DONE

**Responsibility:** Compare semantic realities across executions.

Stage 7 operates exclusively on emitted artifacts.
It never:
- re-executes code
- inspects ASTs
- depends on execution state

Concrete guarantees achieved:
- identical runs produce empty diffs
- single semantic changes produce stable `DiagnosticId` deltas
- timeline divergence is localized and deterministic
- repeated diffs are byte-for-byte identical

Stage 7 establishes **semantic comparability** as a first-class property of
the system.

---

## Stage 8: Semantic Surface Construction — DONE

**Responsibility:** Turn semantic facts into actionable structure.

Stage 8 derives higher-order semantic objects from diagnostics and timelines,
without mutation or persistence.

This includes:
- Root chains (causal paths per diagnostic)
- Role assignment along chains
- Convergence mapping between diagnostics
- Minimal fix surfaces (smallest semantic change sets)

All Stage 8 artifacts are:
- derived ephemerally
- arena-allocated
- analysis-only
- non-persistent

This stage is where meaning becomes actionable.

---

## Post-Stage-8 Development Phases

Further work is semantic expansion, not architectural repair.

### Phase A: Cross-Run Calibration
Validate that semantic diffs change **only** when meaning changes.

### Phase B: Semantic Expansion
- B1: Memory semantics (lifetime, aliasing)
- B2: Control-flow shape (time, not logic)

### Phase C: Syntax Expansion
Grammar grows only when corresponding semantics exist.

### Phase D: Exploit-Class Coverage
Measure exploit surface deltas across runs.

---

## Roadmap Status

- Stage 1: DONE  
- Stage 2: DONE  
- Stage 3: DONE  
- Stage 4: DONE  
- Stage 5: DONE  
- Stage 6: DONE  
- Stage 7: DONE  
- Stage 8: DONE  

Next: **Phase A — Cross-Run Calibration**

---

## One-Line Summary

Programs are executed once to produce semantic history.  
Stages 7 and 8 make that history comparable, explainable, and actionable.
