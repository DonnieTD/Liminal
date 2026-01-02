#!/usr/bin/env bash
# loom.sh — authoritative build & orchestration wrapper
set -euo pipefail

# ============================================================
# Paths
# ============================================================

LOOM=tools/loom/loom
LOOM_SRC=tools/loom/loom.c
LOOM_DIR=$(dirname "$LOOM")
VERIFY_SCRIPT=./test_loom.sh

# ============================================================
# Flags
# ============================================================

DETERMINISTIC=0
SHOW_HELP=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --deterministic|-d)
      DETERMINISTIC=1
      shift
      ;;
    help|-h|--help)
      SHOW_HELP=1
      shift
      ;;
    *)
      break
      ;;
  esac
done

# ============================================================
# Rebuild loom (always authoritative)
# ============================================================

mkdir -p "$LOOM_DIR"

echo ">> rebuilding loom"
cc -std=c99 -O2 -Wall -Wextra -Wpedantic \
   -Itools/loom \
   "$LOOM_SRC" \
   -o "$LOOM"

# ============================================================
# Help (shell-level, before execution)
# ============================================================

if [[ "$SHOW_HELP" -eq 1 || $# -eq 0 ]]; then
  cat <<'EOF'
loom — Liminal build & analysis toolchain

USAGE:
  ./loom.sh [options] <command>

OPTIONS:
  -d, --deterministic
        Enable deterministic execution.

        In this mode:
          • Loom test artifacts are written to:
              ./tmp/loom/deterministic/run
          • No timestamps are used
          • Output is stable and diffable

        Intended for:
          • CI verification
          • Artifact diffing
          • Make-vs-Loom comparison

COMMANDS:
  build
        Run standardisation checks and build liminal

  test
        Build and run all semantic test samples

  check
        Run code standardisation checks only

  regen-standards
        Regenerate CodeStyleGuide.md from rules

  standardise
        Run check + regen-standards

  flatten
        Generate a single-file flattened build

  flatten-min
        Generate a minified flattened build

  flatten-samples
        Flatten all sample programs individually

  clean
        Remove build artifacts and binary

  verify-make
        Verify Loom output matches Make output.

        This command:
          • Runs `make loom-test`
              → writes oracle artifacts to:
                ./tmp/loom/make/run
          • Runs `loom test` in deterministic mode
              → writes loom artifacts to:
                ./tmp/loom/deterministic/run
          • Recursively diffs both trees
          • Fails on semantic divergence

MAKE VS LOOM VERIFICATION MODEL:

  Make is treated as the reference oracle during transition.
  Loom must produce semantically identical artifacts.

  IMPORTANT:
    • `make test` writes to `test/` (human-facing)
    • `make loom-test` writes to `tmp/loom/make/run`
    • Loom deterministic mode writes to `tmp/loom/deterministic/run`
    • Verification NEVER touches `test/`

MANUAL VERIFICATION (if needed):

    make loom-test
    ./loom.sh --deterministic test
    diff -r tmp/loom/make/run tmp/loom/deterministic/run

NOTES:
  • loom is always rebuilt before execution
  • deterministic mode is opt-in
  • loom.sh is a loader — loom is the engine
  • make remains temporarily as a reference oracle

EXAMPLES:
  ./loom.sh build
  ./loom.sh test
  ./loom.sh -d test
  ./loom.sh verify-make
  ./loom.sh flatten
  ./loom.sh clean

EOF
  exit 0
fi

# ============================================================
# Deterministic environment (opt-in)
# ============================================================

if [[ "$DETERMINISTIC" -eq 1 ]]; then
  export LOOM_DETERMINISTIC=1
  export LOOM_TEST_ROOT="./tmp/loom/deterministic"

  echo ">> deterministic mode enabled"
  echo ">> test root: $LOOM_TEST_ROOT"

  rm -rf "$LOOM_TEST_ROOT"
  mkdir -p "$LOOM_TEST_ROOT"
fi

# ============================================================
# Dispatch
# ============================================================

case "$1" in
  build|test|check|regen-standards|standardise|flatten|flatten-min|flatten-samples|clean)
    exec "$LOOM" "$@"
    ;;
  verify-make)
    if [[ ! -x "$VERIFY_SCRIPT" ]]; then
      echo "ERROR: $VERIFY_SCRIPT not found or not executable"
      exit 1
    fi
    exec "$VERIFY_SCRIPT"
    ;;
  *)
    echo "loom.sh: unknown command '$1'"
    echo
    echo "Run './loom.sh help' for usage."
    exit 1
    ;;
esac
