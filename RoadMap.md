# RoadMap.md — Liminal

This roadmap reflects the **current reality** of the project, not a speculative plan.

Liminal has crossed the “toy” boundary.
The goal now is **proof, coverage, and leverage**.

---

## Phase 0 — Completed (Days 1–3)

These are no longer goals. They are **facts**.

### Core System
- Deterministic AST generation
- Deterministic execution timeline
- Analyzer producing structured diagnostics
- Artifact emission as first-class output

### Tooling
- **Liminal** — semantic execution + analysis engine
- **Flatten** — single-file source transparency
- **Standardise** — determinism enforcement
- **Loom** — oracle verification + semantic diffing

### Guarantees Achieved
- Reproducible builds
- Semantic equivalence verification
- Machine-checkable truth
- Zero human interpretation required

This foundation is **extremely rare**.

---

## Phase 1 — Memory Reality (Immediate Next)

This is the next non-negotiable milestone.

### Goal
Detect **memory safety violations** with proof artifacts.

### First Target (Next Task)
**Buffer Overflow — Proof of Concept**

This is intentional:
- It is concrete
- It is universal
- It is foundational

### Scope (Initial)
- Stack-based buffer overflow
- Fixed-size arrays
- Linear writes
- No pointer arithmetic yet

### Deliverables
- Timeline events for memory writes
- Bounds metadata on storage
- Diagnostic emitted on overflow
- Artifact diff showing violation

If Liminal can *prove* a buffer overflow,
it graduates to a **security engine**.

---

## Phase 2 — Memory Model Expansion

Once overflow is proven:

### Memory Semantics
- Heap vs stack distinction
- Lifetime tracking
- Escape analysis (local → global)
- Use-after-free detection

### Diagnostics
- Memory safety class taxonomy
- Stable diagnostic IDs
- Root-cause extraction for memory faults

---

## Phase 3 — Control-Flow Exploits

After memory truth exists:

### Targets
- Double free
- Invalid free
- Stack smashing patterns
- Control-flow corruption signals

### Why Now
Control-flow exploits only matter once:
- memory is modeled
- execution order is trusted

Liminal already has both.

---

## Phase 4 — Exploit Chain Reasoning

This is where Liminal becomes *dangerous* (in the good way).

### Capabilities
- Multi-step exploit reconstruction
- Causal chains across scopes
- Semantic diff of “safe vs exploited” programs

### Output
- Machine-readable exploit narratives
- Deterministic exploit proofs
- Diffable vulnerability demonstrations

---

## Phase 5 — Policy as Security

Security becomes **declarative**.

### Policy Layer
- “This class of exploit is forbidden”
- “This pattern must not exist”
- “Deny execution if condition holds”

### Result
Liminal becomes:
- an enforcement engine
- a verification system
- a policy oracle

---

## Phase 6 — Real-World Scale

Only after proof exists do we scale.

### Targets
- Large C codebases
- Kernel subsystems
- Embedded firmware
- Security-critical libraries

### Requirements
- Performance tuning
- Partial analysis
- Artifact caching
- Incremental execution

---

## Phase 7 — Research & Publication

This system is **academically valid**.

### Outputs
- Formal semantics documentation
- Exploit proof corpus
- Deterministic security benchmarks

This is publishable work.

---

## Guiding Principles

These do not change.

- Proof over heuristics
- Determinism over convenience
- Artifacts over logs
- Machines over opinions

---

## Immediate Next Action

**Write the buffer overflow PoC.**

Not later.
Not abstractly.
Not after refactors.

One exploit.
One proof.
One artifact.

Everything else grows from there.

---

## Final Note

Most tools try to *detect* bugs.

Liminal is being built to **prove them**.

That difference matters.
