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

The AST represents the terrain the program will traverse.

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

Execution produces a linear timeline:

World0 -> World1 -> World2 -> ... -> WorldN

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
- active lexical scope
- call stack (future)
- memory state (future)
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
- tracks the current World
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

## Policy Enforcement

Liminal separates *diagnostics* from *authority*.

Diagnostics describe semantic facts.
Policies decide whether those facts are acceptable.

Policies:
- consume DiagnosticArtifacts only
- are deterministic
- may allow, warn, or deny execution

This enables:
- compiler-style gating
- audit-only modes
- strict or permissive semantic regimes

Policy decisions are explicit and visible.

--- 

## Stage 3: Analyzer

Responsibility:
What structural facts can be derived from this execution?

The analyzer:
- consumes the World timeline
- never re-executes
- never mutates Worlds
- derives semantic facts

Primary artifacts:
- ScopeLifetime
- VariableLifetime
- UseReport
- Structural violations

Analysis is pure, replayable, and repeatable.

---

## Stage 4: Constraint Engine

Responsibility:
What semantic rules must hold?

This stage introduces constraints, not diagnostics.

Constraints express facts about semantic reality, independent of
presentation or UX.

Examples:
- a variable use must have a declaration
- a declaration may not redeclare in the same scope
- a declaration may not shadow a parent binding
- a use may not occur after scope exit

Constraints are:
- derived from the World timeline
- independent of reporting
- stable across tooling

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
- defines versioned, serializable artifacts
- enables cross-run comparison

This is where analysis becomes tooling-grade.

Primary artifacts:
- DiagnosticArtifact
- DiagnosticId (stable across runs)
- NDJSON diagnostic streams
- Artifact directories with metadata

### Artifact Directory Contract

.liminal/
  <run-id>/
    meta.json
    diagnostics.ndjson

Artifacts are:
- deterministic
- serializable
- replayable
- consumer-agnostic

---

## Diagnostic Consumers

Stage 5 introduces first-class consumers:

- Render: terminal diagnostics
- Validate: structural gating (duplicate IDs, monotonic time)
- Diff: cross-run comparison
- Stats: aggregated diagnostic metrics

Consumers:
- never mutate artifacts
- never allocate persistent state
- are deterministic and composable

---

## CLI Model

Liminal provides two primary commands:

liminal run <file> [--emit-artifacts]
liminal analyze <artifact-dir>

- run executes and optionally emits artifacts
- analyze consumes artifacts only (no execution)

---

## Stage 6: Visualization / Timeline Emission (PARTIAL)

Responsibility:
Make time visible.

Stage 6 introduces first-class temporal consumers over execution history.

The visualization layer:
- consumes World timelines only
- never re-executes
- never mutates artifacts
- renders semantic time explicitly

Current capabilities:
- linear timeline emission
- step-by-step temporal traces
- AST ↔ Step ↔ Time correlation
- policy-gated execution outcomes

Timeline output is deterministic and derived solely from the World sequence.

Future work:
- graph visualization
- interactive inspection
- cross-run temporal diffing

---

## Stage 7: Cross-Run Reasoning (Planned)

Responsibility:
Compare semantic realities across executions.

Stage 7 operates exclusively on emitted artifacts.
It never re-executes code, parses source, or inspects ASTs.

This stage enables:
- semantic diffing between runs
- regression detection
- semantic drift tracking
- exploit delta analysis
- time-based behavior comparison

Cross-run reasoning treats each execution as a completed reality and asks:

What changed between these realities, and when did meaning diverge?

Primary inputs:
- meta.json
- diagnostics.ndjson
- timeline.ndjson

Primary outputs:
- diagnostic diffs
- regression reports
- timeline divergence summaries
- exploit surface deltas

Stage 7 establishes the foundation for security, regression, and exploit research
without speculation or symbolic execution.

---

## Post-Stage 7 Development Phases

Once Stage 7 exists, further progress follows semantic expansion, not feature accretion.

The roadmap deliberately avoids premature lexer or language completeness.

### Phase A: Cross-Run Calibration (Current)

Goal:
Validate that semantic comparison is stable and meaningful.

Activities:
- build a small curated corpus of example programs
- run repeated executions with controlled changes
- validate diagnostic identity stability
- validate timeline diff behavior
- ensure regressions and drift are detectable without heuristics

No new semantics are added in this phase.

Exit condition:
Cross-run diffs reliably detect semantic change when meaning changes,
and remain stable when it does not.

---

### Phase B: Semantic Expansion (Before Syntax Expansion)

#### Phase B1: Memory Semantics (Existence, Not Bytes)

Goal:
Enable lifetime-based reasoning without implementing concrete memory models.

Scope:
- allocation events
- deallocation events
- alias creation
- lifetime invalidation

Explicitly out of scope:
- pointer arithmetic
- concrete addresses
- memory layouts
- byte-level modeling

This phase enables detection of:
- use-after-free
- double-free
- stale alias use
- lifetime leaks

Exit condition:
Cross-run reasoning can detect changes in memory lifetime semantics.

---

#### Phase B2: Control-Flow Shape (Time, Not Logic)

Goal:
Make execution shape and temporal structure observable.

Scope:
- conditional entry and exit
- loop iteration
- early return
- structured control regions

Explicitly out of scope:
- condition evaluation
- symbolic branching
- speculative execution

This phase enables detection of:
- dead code activation
- unexpected control flow
- temporal regressions
- time-of-check/time-of-use gaps

Exit condition:
Timeline diffs can identify where and when execution diverged.

---

### Phase C: Lexer and Parser Expansion (Driven by Semantics)

Goal:
Support real-world C examples without premature language completeness.

Rules:
- syntax is added only when corresponding semantics exist
- every new AST node must map to a semantic event
- grammar growth is corpus-driven

Exit condition:
Realistic vulnerable C snippets can be represented faithfully,
even if incompletely.

---

### Phase D: Exploit-Class Coverage

Goal:
Transition from bug detection to exploit research.

Activities:
- curate exploit-focused C corpora
- map exploit classes to semantic patterns
- track exploit surface changes across runs

Exploit classes include:
- use-after-free
- double-free
- shadowing-based authority confusion
- temporal safety violations
- control-flow misuse

Exit condition:
Stage 7 can answer:

Did this change increase, decrease, or alter the semantic attack surface?

At this point, Liminal operates as an exploit research engine,
not a static analyzer.

---

## Roadmap Clarification

Language completeness is not a goal.

Semantic observability is the primary axis of progress.

Every new feature must:
- produce observable semantic events
- survive cross-run comparison
- strengthen time-based reasoning

---

## Summary

Stages 1 through 6 establish semantic execution and time visibility.

Stage 7 enables comparison between semantic realities.

Phases A through D define the path from execution engine
to exploit research platform.

---

## Design Principles

- Execute once, analyze forever
- Prefer observation over inference
- Prefer explicit state over abstraction
- Time must always be explicit
- Artifacts over opinions

---

## Roadmap Status

- Stage 1: DONE
- Stage 2: DONE
- Stage 3: DONE
- Stage 4: DONE
- Stage 5: DONE
- Stage 6: DONE
- Stage 7: PLANNED
- Stage A: PLANNED
- Stage B: PLANNED
- Stage C: PLANNED
- Stage D: PLANNED

---

## One-Line Summary

Programs are executed once to produce a semantic history.
Artifacts let us understand it forever.
