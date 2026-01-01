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

Execution is modeled as a sequence of immutable semantic states indexed
by time.

Bugs, undefined behavior, and exploits are treated as pathological
transitions in that sequence, not as crashes, warnings, or abstract
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
8. Semantic Surface Construction

Data flows strictly forward.
No stage mutates the output of a previous stage.

---

## Stage 1: Frontend / Parser

Responsibility:
What exists?

The frontend parses C source code into an abstract syntax tree (AST).

The AST is:
- immutable
- execution-agnostic
- shared by all runs
- free of semantic effects

Primary artifact:
- ASTProgram (arena-owned, stable node IDs)

---

## Stage 2: Executor

Responsibility:
In what order did existence occur?

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

Primary artifacts:
- World timeline
- Step stream

---

## World & Universe

A World represents the complete semantic state at a single moment in time.

Worlds are immutable once created.

The Universe:
- owns the full World timeline
- advances execution by creating Worlds
- enables backward traversal for analysis

The World timeline functions as a bidirectional semantic write-ahead log:
- forward traversal: execution
- backward traversal: analysis

---

## Stage 3: Analyzer

Responsibility:
What structural facts can be derived from this execution?

Consumes the World timeline and derives semantic facts without mutation.

Primary artifacts:
- ScopeLifetime
- VariableLifetime
- Use reports
- Structural violations

---

## Stage 4: Constraint Engine

Responsibility:
What semantic rules must hold?

Constraints express invariant facts about semantic reality.

Examples:
- a variable use must have a declaration
- a declaration may not redeclare in the same scope
- a declaration may not shadow a parent binding
- a use may not occur after scope exit

Primary artifacts:
- ConstraintArtifact
- ConstraintKind
- Constraint sets

---

## Stage 5: Artifact Layer (DONE)

Responsibility:
Make meaning stable.

This layer:
- assigns stable identity to semantic facts
- anchors diagnostics to source structure
- defines serializable, replayable artifacts

Primary artifacts:
- DiagnosticArtifact
- DiagnosticId (stable across runs)
- NDJSON streams (diagnostics, timelines)

Artifacts are deterministic, reproducible, and consumer-agnostic.

---

## Stage 6: Visualization / Timeline Emission (DONE)

Responsibility:
Make time explicit.

Capabilities:
- linear timeline emission
- step-by-step temporal traces
- AST ↔ Step ↔ Time correlation
- policy-gated execution outcomes

All visualization is derived solely from artifacts.

---

## Stage 7: Cross-Run Reasoning (DONE)

Responsibility:
Compare semantic realities across executions.

Stage 7 operates exclusively on emitted artifacts.
It never:
- re-executes code
- parses source
- inspects ASTs

Concrete guarantees achieved:

- Two runs with no semantic change produce an empty diff
- A single semantic change produces:
  - one stable DiagnosticId delta
  - one localized timeline divergence
- Running diff twice yields identical output byte-for-byte
- No consumer requires AST access
- No consumer requires execution

Stage 7 establishes **semantic comparability** as a hard invariant.

---

## Stage 8: Semantic Surface Construction (DONE)

Responsibility:
Turn semantic facts into actionable structures.

Stage 8 derives higher-order semantic objects ephemerally from artifacts:

- Root Chains (causal paths per diagnostic)
- Role assignment along chains
- Convergence mapping between diagnostics
- Minimal Fix Surfaces (smallest semantic change sets)

Properties:
- no mutation
- no persistence
- arena-allocated
- analysis-only

This stage does not alter semantics.
It **interprets** them.

---

## Post-Stage 8 Development Phases

Progress now follows semantic expansion, not feature accretion.

### Phase A: Cross-Run Calibration
Validate stability across controlled change sets.

### Phase B: Semantic Expansion
- Memory semantics (existence, not bytes)
- Control-flow shape (time, not logic)

### Phase C: Syntax Expansion
Grammar grows only when semantics exist.

### Phase D: Exploit-Class Coverage
Track exploit surface deltas over time.

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
- Phase A: NEXT
- Phase B: PLANNED
- Phase C: PLANNED
- Phase D: PLANNED

---

## One-Line Summary

Programs are executed once to produce semantic history.
Liminal turns that history into stable, comparable, and actionable meaning.
