# LOOM — Authoritative Build & Orchestration

Loom is the **control plane** of the Liminal project.

It is not a helper script.
It is not a convenience wrapper.
It is the **authority** that decides *what the system is*, *how it is built*, and *how its meaning is verified*.

If Liminal is the semantic engine, **Loom is the law**.

---

## What Loom Is

Loom is a small C program (`tools/loom/loom.c`) driven by a shell loader (`loom.sh`).

Together they form a **self-hosting orchestration tool** that:

- Rebuilds itself on every invocation
- Controls all build, test, and artifact flows
- Enforces deterministic execution when required
- Verifies semantic equivalence against Make
- Owns temporary state and cleans up after itself

Loom exists because **Make cannot express semantic intent**.

---

## Why Loom Exists

Make answers:

> “Are these files newer than those files?”

Loom answers:

> “Does this execution preserve the meaning of the system?”

Once Liminal started producing:
- timelines
- diagnostics
- semantic diffs
- policy-driven artifacts

…the build system itself became part of the correctness surface.

At that point, Make stopped being sufficient.

---

## Architecture

```sh
    loom.sh ← authoritative entry point
    │
    ├─ rebuilds tools/loom/loom
    │
    ├─ sets invariants (determinism, temp roots)
    │
    └─ dispatches to loom (C binary)
    │
    ├─ build
    ├─ test
    ├─ flatten
    ├─ verify-make
    └─ clean
```

### Key Principle

> **The shell never does work.  
> The shell only establishes law.**

All real behavior lives in `loom` (the C binary).

---

## The Self-Rebuild Rule

Every invocation of `./loom.sh` does this first:

```sh
rebuilding loom
```

This is non-negotiable.

### Why?

- Loom must always reflect the current source
- No stale control plane is allowed
- The build tool cannot lie about itself

This mirrors how compilers like `zig` and `nix` treat their drivers.

---

## Commands

### `build`

```sh
    ./loom.sh build
```

- Runs standardisation checks
- Compiles `liminal`
- Produces the main binary

This replaces

```sh 
make 
```

### `test`

```sh
    ./loom.sh test
```

- Builds the system
- Runs all semantic samples
- Emits structured artifacts

By default, outputs go to:

```sh
    ./test/<timestamp>/
```

This is **developer-facing output**.

---

### Deterministic Test Mode

```sh
    ./loom.sh --deterministic test
```


This changes one critical invariant:

- Outputs are written to a **fixed location**

```sh
    ./tmp/loom/deterministic/run/
```

This mode exists for:
- CI
- Artifact diffing
- Regression testing
- Oracle comparison

No timestamps.
No randomness.
No ambiguity.

---

### `verify-make`

```sh
    ./loom.sh verify-make
```


This is Loom’s most important command.

It performs a **semantic equivalence proof**:

1. Runs Make-based tests into a temp directory
2. Runs Loom tests in deterministic mode
3. Normalizes non-semantic metadata
4. Diffs resulting artifacts
5. Fails on divergence

Make acts as a **reference oracle**, not a peer.

This command is how Loom earns authority.

---

### `check`

```sh
    ./loom.sh check
```


- Runs `standardise` against `src/` and `tools/`
- No build
- No artifacts

Used for fast validation.

---

### `flatten`, `flatten-min`, `flatten-samples`

```sh
    ./loom.sh flatten
    ./loom.sh flatten-min
    ./loom.sh flatten-samples
```


Delegates to the Flatten tool.

Loom owns *when* flattening happens and *where* outputs go.

---

### `clean`

```sh
    ./loom.sh clean
```


Removes:
- build artifacts
- binaries
- temporary outputs

Does **not** remove:
- test artifacts
- flattened sources

Those are intentional records.

---

## Temporary State Discipline

Loom enforces strict separation:

| Path            | Purpose |
|-----------------|---------|
| `test/`         | Developer-visible test artifacts |
| `tmp/loom/`     | Ephemeral, machine-owned output |
| `artifacts/`    | Durable, inspectable results |

Loom **never pollutes** the working tree.

If it creates state, it owns it.
If it owns it, it cleans it.

---

## Determinism Contract

When deterministic mode is enabled, Loom guarantees:

- Stable directory names
- Stable ordering
- Stable paths
- Stable contents (excluding explicitly marked metadata)

Any divergence is treated as a **bug**.

This is how semantic diffing becomes possible.

---

## Loom vs Make

| Aspect            | Make                | Loom |
|-------------------|---------------------|------|
| Dependency graph  | File timestamps     | Semantic intent |
| Authority         | Filesystem          | Toolchain |
| Determinism       | Accidental          | Explicit |
| Verification      | None                | Built-in |
| Artifact awareness| No                  | First-class |

Make is still present — but only as a **reference oracle** during transition.

The long-term goal is **Make-free operation**.

---

## Design Rules

Loom follows strict rules:

1. **Loom must be boring**
2. **Loom must be small**
3. **Loom must be correct**
4. **Loom must be rebuildable**
5. **Loom must never guess**

If a behavior cannot be proven correct, it does not belong in Loom.

---

## Mental Model

Think of Loom as:

- A judge, not a worker
- A contract, not a script
- A law, not a tool

Liminal computes meaning.  
Flatten exposes meaning.  
Standardise stabilizes meaning.  

**Loom ensures meaning survives time.**
