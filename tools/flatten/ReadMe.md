# FLATTEN — Whole-System Transparency

Flatten is the **introspection engine** of the Liminal project.

It exists to answer one question, perfectly:

> “What is the system, really?”

Not what the filesystem *suggests*.
Not what the build tool *implies*.
Not what the compiler *infers*.

Flatten produces a **single, concrete, inspectable truth**.

---

## What Flatten Is

Flatten is a standalone C tool located at:

```sh
  tools/flatten/flatten.c
```


It is invoked **only through Loom**.

Flatten:
- Reads the real source tree
- Preserves file ordering
- Preserves comment structure
- Emits deterministic output
- Produces artifacts suitable for:
  - auditing
  - review
  - LLM ingestion
  - long-term archival

Flatten does **not** build.
Flatten does **not** analyze.
Flatten does **not** infer.

Flatten only **reveals**.

---

## Why Flatten Exists

Large systems fail in darkness.

Flatten exists to answer:

> “What does the system *really* contain?”

With Flatten, you can:
- Paste the entire system into a single buffer
- Inspect ordering guarantees
- Verify no hidden code paths exist
- Share the *actual system* with humans or machines

This is especially important for:
- security review
- semantic tooling
- AI-assisted analysis
- historical snapshots

---

## Types of Flattening

### 1. Full Flatten (`flatten`)

```sh
    ./loom.sh flatten
```

Produces:

```sh
artifacts/liminal_flat.c
```


Characteristics:
- Includes all headers
- Includes all source files
- Preserves directory ordering
- Preserves comments
- Human-readable

This is the **canonical transparency artifact**.

---

### 2. Minified Flatten (`flatten-min`)

```sh
./loom.sh flatten-min
```

Produces:

```sh
artifacts/liminal_flat.min.c
```


Characteristics:
- Uses the Flatten tool directly
- Strips redundant whitespace
- Maintains correct include order
- Optimized for:
  - tooling
  - ingestion
  - automated analysis

This is **machine-first output**.

---

### 3. Sample Flattening (`flatten-samples`)

```sh
    ./loom.sh flatten-samples
```

Produces:

```sh
artifacts/samples/*.flat.c
```

