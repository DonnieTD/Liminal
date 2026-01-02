# Code Style Guide

## 1️⃣ Purpose of CodeStyleGuide.md

This file is not about aesthetics.  
It exists to encode constraints that protect architecture.

> Every rule must prevent a future mistake you can already imagine.

This document is a **normative contract** between contributors and the codebase.

Some sections are **machine-generated** and **must not be edited manually**.
Those sections are rendered directly from the `standardise` tool to guarantee
that documentation and enforcement never diverge.

---

<!-- STANDARDISE:BEGIN -->

## Rule #1 — Header Inclusion Policy

**Applies to:** .h files

**Description**

All header files MUST use classic include guards. `#pragma once` is forbidden.

**Rationale**

Headers define semantic identity boundaries. Classic include guards are explicit, analyzable, deterministic, and stable under source flattening and artifact reconstruction.

---

## Rule #2 — Standalone Tool Purity

**Applies to:** /tools/*

**Description**

Standalone tools must be dependency-free and limited in scope.

**Rationale**

Standalone tools must remain portable, auditable, deterministic, and bootstrappable. Dependencies obscure behavior and introduce hidden coupling.

---

<!-- STANDARDISE:END -->

## Notes

- Rules inside the generated section above are **authoritative**.
- Manual edits inside the generated region are not permitted.
- To update rules, modify `/tools/standardise` and regenerate this document.

To regenerate the standards documentation:

```sh
make regen-standards
```

To run enforcement checks:

```sh
make check
```
