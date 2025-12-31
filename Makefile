CC      := cc
CFLAGS  := -std=c99 -Wall -Wextra -Wpedantic -g -Isrc
LDFLAGS :=

BUILD   := build
BIN     := liminal

# ------------------------------------------------------------
# Tool source files ONLY (no samples)
# ------------------------------------------------------------

SRC := $(filter-out src/samples/%,$(shell find src -name '*.c'))
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)
DEP := $(OBJ:.o=.d)

# ------------------------------------------------------------
# Build rules
# ------------------------------------------------------------

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD) $(BIN)

-include $(DEP)

# ------------------------------------------------------------
# Test artifacts layout
# ------------------------------------------------------------

TEST_ROOT := test
TEST_RUN  := $(shell date +"%d-%m-%Y-%H-%M-%S")
TEST_DIR  := $(TEST_ROOT)/$(TEST_RUN)

# ------------------------------------------------------------
# Samples
# ------------------------------------------------------------

SAMPLES := $(sort $(wildcard src/samples/[0-9]*.c))

.PHONY: test

test: clean liminal
	@mkdir -p $(TEST_DIR)
	@echo "== Running Liminal samples =="
	@echo "Artifacts -> $(TEST_DIR)"
	@set -e; \
	for f in $(SAMPLES); do \
		name=$$(basename $$f .c); \
		outdir="$(TEST_DIR)/$$name"; \
		echo ""; \
		echo ">>> $$f"; \
		mkdir -p $$outdir; \
		./liminal run $$f \
			--emit-artifacts \
			--emit-timeline \
			--artifact-dir $$outdir \
			--run-id analysis; \
	done

# ------------------------------------------------------------
# Flattened single-file build
# ------------------------------------------------------------

FLATTEN := artifacts/liminal_flat.c

flatten: $(FLATTEN)

$(FLATTEN):
	@mkdir -p artifacts
	@echo "/* ========================================================= */" >  $@
	@echo "/*  LIMINAL — SINGLE FILE FLATTENED BUILD                    */" >> $@
	@echo "/*  Generated: $$(date)                                      */" >> $@
	@echo "/* ========================================================= */" >> $@
	@find src -name '*.h' | sort | while read f; do \
		echo "\n/* === $$f ============================================ */" >> $@; \
		cat $$f >> $@; \
	done
	@find src -name '*.c' | grep -v src/samples | sort | while read f; do \
		echo "\n/* === $$f ============================================ */" >> $@; \
		cat $$f >> $@; \
	done

# -------------------------------------------------------------
# Minified flattened single-file build (repo-correct)
# -------------------------------------------------------------

FLATTEN_MIN := artifacts/liminal_flat.min.c

.PHONY: flatten-min

flatten-min: $(FLATTEN_MIN)

$(FLATTEN_MIN):
	@mkdir -p artifacts
	@echo "/* LIMINAL_FLAT_MIN $$(date -u +%Y%m%dT%H%M%SZ) */" > $@

	@echo "/* -------- HEADERS -------- */" >> $@
	@for dir in common executor analyzer consumers frontends; do \
		find src/$$dir -name '*.h' -type f | sort | while read f; do \
			echo "//@header $$f" >> $@; \
			sed '/^[[:space:]]*$$/d' $$f >> $@; \
		done; \
	done

	@echo "/* -------- SOURCES -------- */" >> $@
	@for dir in common executor analyzer consumers frontends; do \
		find src/$$dir -name '*.c' -type f | sort | while read f; do \
			echo "//@source $$f" >> $@; \
			sed '/^[[:space:]]*$$/d' $$f >> $@; \
		done; \
	done

	@echo "//@source src/main.c" >> $@
	@sed '/^[[:space:]]*$$/d' src/main.c >> $@
