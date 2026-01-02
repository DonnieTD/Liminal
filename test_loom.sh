#!/usr/bin/env bash
# test_loom.sh — Make vs Loom semantic equivalence verifier
set -euo pipefail

MAKE_ROOT="tmp/loom/make/run"
LOOM_ROOT="tmp/loom/deterministic/run"

echo "== Running Make oracle tests =="
make loom-test

echo
echo "== Running Loom deterministic tests =="
./loom.sh --deterministic test

# ------------------------------------------------------------
# Normalize non-semantic metadata (PORTABLE)
# ------------------------------------------------------------

echo
echo "== Normalizing non-semantic metadata =="

normalize_meta() {
  find "$1" -name meta.json -print0 | while IFS= read -r -d '' f; do
    perl -0777 -i -pe 's/"started_at"\s*:\s*\d+/"started_at": 0/g' "$f"
  done
}

normalize_meta "$MAKE_ROOT"
normalize_meta "$LOOM_ROOT"

# ------------------------------------------------------------
# Diff semantic artifacts
# ------------------------------------------------------------

echo
echo "== Diffing semantic artifacts =="

if diff -r "$MAKE_ROOT" "$LOOM_ROOT"; then
  echo
  echo "✅ Loom matches Make oracle (semantic equivalence)"
  exit 0
else
  echo
  echo "❌ Loom output diverges from Make oracle"
  exit 1
fi
