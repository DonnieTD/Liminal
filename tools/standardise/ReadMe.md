# STANDARDISE — Determinism Enforcement

Standardise is the **discipline layer** of the Liminal toolchain.

It exists to eliminate *incidental differences* so that only **meaningful differences remain**.

Standardise does not interpret code.
Standardise does not analyze semantics.

Standardise **forces sameness** where sameness is required.

---

## What Standardise Is

Standardise is a standalone C tool:

```sh
    tools/standardise/standardise.c
```

It is used by **Loom**, not by humans directly.

Standardise:
- Normalizes non-semantic metadata
- Removes time-based variance
- Canonicalizes JSON output
- Enforces deterministic formatting

If two runs differ *after* standardisation, the difference is **real**.

---

## Why Standardise Exists

Without Standardise, this happens:

- Timestamps differ
- File order differs
- Formatting differs
- Hashes differ
- Diff tools lie

That makes **semantic equivalence impossible to prove**.

Standardise exists so that:

> “If something changed, it changed *for a reason*.”

---

## What Standardise Touches

Standardise operates only on **artifacts**, never on source.

Specifically:
- `meta.json`
- `diagnostics.ndjson`
- `timeline.ndjson`

It does **not**:
- rewrite ASTs
- modify execution steps
- alter analyzer output

Only *representation* is normalized.

---

## Canonicalization Rules

### 1. Timestamp Neutralization

Fields like:

```json
"started_at": 1767354368
```

Are replaced with deterministic values:

```json
"started_at": 0
```

This ensures:
- time does not leak into meaning
- builds are comparable across machines

### Stable Key Ordering

JSON objects are rewritten so that:

- keys appear in fixed order
- output is byte-for-byte stable

This enables:

- clean diffs
- reproducible hashes
- artifact signing

### 3. Line-Based Stability

NDJSON output is normalized so that:

- ordering is stable
- no transient whitespace exists
- line endings are consistent


## Determinism Contract

After Standardise:

- Two equivalent runs must produce identical bytes
- Any diff is actionable
- CI systems can trust results

This is what enables Loom to say:

```sh
✅ Loom matches Make oracle (semantic equivalence)
```

Without Standardise, that statement is meaningless.

## What Standardise Refuses to Do

Standardise will not:
- hide semantic differences
- collapse errors
- “make tests pass”

If something fails after standardisation, the system is wrong.

This tool is unforgiving by design.


## Relationship to Loom

Standardise is always invoked inside Loom:

```sh
loom verify-make
```

Flow:

- Run Liminal
- Collect artifacts
- Standardise artifacts
- Diff results

Humans do not intervene.

## Relationship to Flatten

Flatten reveals the system.
Standardise stabilizes its output.

They operate at different layers:

| Tool        | Operates On | Purpose      |
| ----------- | ----------- | ------------ |
| Flatten     | Source      | Transparency |
| Standardise | Artifacts   | Determinism  |

| Tool        | Operates On | Purpose      |
| ----------- | ----------- | ------------ |
| Flatten     | Source      | Transparency |
| Standardise | Artifacts   | Determinism  |
