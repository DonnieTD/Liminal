# ============================================================
# Compiler / flags
# ============================================================

CC      := cc
CFLAGS  := -std=c99 -Wall -Wextra -Wpedantic -g -Isrc
LDFLAGS :=

BUILD   := build
BIN     := liminal

# ============================================================
# Tool source files ONLY (no samples)
# ============================================================

SRC := $(filter-out src/samples/%,$(shell find src -name '*.c'))
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)
DEP := $(OBJ:.o=.d)

# ============================================================
# Build rules
# ============================================================

all: check $(BIN)

$(BIN): check $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD) $(BIN)

-include $(DEP)

# ============================================================
# Test artifacts layout
# ============================================================

TEST_ROOT := test
TEST_RUN  := $(shell date +"%d-%m-%Y-%H-%M-%S")
TEST_DIR  := $(TEST_ROOT)/$(TEST_RUN)

# ============================================================
# Samples (semantic classes)
# ============================================================

SAMPLES_DIR := src/samples

SAMPLES_BASIC := $(sort $(wildcard $(SAMPLES_DIR)/basic/*.c))
SAMPLES_FAIL  := $(sort $(wildcard $(SAMPLES_DIR)/fail/*.c))
SAMPLES_POC   := $(sort $(wildcard $(SAMPLES_DIR)/poc/*.c))

# ============================================================
# Helper macro (NO trailing semicolon)
# ============================================================

define run_sample
	name=$$(basename $(1) .c); \
	outdir="$(TEST_DIR)/$$name"; \
	echo ""; \
	echo ">>> $(1)"; \
	mkdir -p $$outdir; \
	./liminal run $(1) \
		--emit-artifacts \
		--emit-timeline \
		--artifact-dir $$outdir \
		--run-id analysis; \
	status=$$?
endef

# ============================================================
# Test runner (semantic-aware)
# ============================================================

.PHONY: test

test: clean liminal
	@mkdir -p $(TEST_DIR)
	@echo "== Running Liminal samples =="
	@echo "Artifacts -> $(TEST_DIR)"
	@set +e; \

	# -------------------------------
	# BASIC — must succeed
	# -------------------------------
	for f in $(SAMPLES_BASIC); do \
		$(call run_sample,$$f); \
		if [ $$status -ne 0 ]; then \
			echo "ERROR: basic sample failed"; \
			exit 1; \
		fi; \
	done; \

	# -------------------------------
	# FAIL — must fail
	# -------------------------------
	for f in $(SAMPLES_FAIL); do \
		$(call run_sample,$$f); \
		if [ $$status -eq 0 ]; then \
			echo "ERROR: expected failure but succeeded"; \
			exit 1; \
		fi; \
		echo ">>> expected semantic denial"; \
	done; \

	# -------------------------------
	# POC — always run
	# -------------------------------
	for f in $(SAMPLES_POC); do \
		$(call run_sample,$$f); \
		if [ $$status -ne 0 ]; then \
			echo ">>> NOTE: PoC semantic denial (exit $$status)"; \
		fi; \
	done

# ============================================================
# Flattened single-file build
# ============================================================

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

# ============================================================
# Minified flattened single-file build (repo-correct)
# ============================================================
TOOLS_DIR := build/tools
FLATTENER := $(TOOLS_DIR)/liminal-flatten

FLATTEN_MIN := artifacts/liminal_flat.min.c
SRC_DIRS := src/common src/executor src/analyzer src/consumers \
            src/frontends src/commands src/policy src/main.c

$(FLATTENER): tools/flatten/flatten.c
	@mkdir -p $(TOOLS_DIR)
	$(CC) -std=c99 -O2 $< -o $@

.PHONY: flatten-min
flatten-min: $(FLATTENER)
	@mkdir -p artifacts
	$(FLATTENER) --out $(FLATTEN_MIN) $(SRC_DIRS)

# ============================================================
# Flattened samples (single-file per sample)
# ============================================================

SAMPLE_FLAT_DIR := artifacts/samples

.PHONY: flatten-samples

flatten-samples:
	@mkdir -p $(SAMPLE_FLAT_DIR)
	@for f in $(SAMPLES_BASIC) $(SAMPLES_FAIL) $(SAMPLES_POC); do \
		name=$$(basename $$f .c); \
		out="$(SAMPLE_FLAT_DIR)/$$name.flat.c"; \
		echo "/* LIMINAL SAMPLE $$name */" > $$out; \
		echo "/* Generated: $$(date) */" >> $$out; \
		echo "" >> $$out; \
		cat $$f >> $$out; \
	done


# ============================================================
# Standardise tool
# ============================================================

STANDARDISE := tools/standardise/standardise
STYLE_GUIDE := CodeStyleGuide.md

.PHONY: check regen-standards standardise

check: $(STANDARDISE)
	@echo "Running standardisation checks..."
	@$(STANDARDISE) src
	@$(STANDARDISE) tools

regen-standards: $(STANDARDISE)
	@echo "Regenerating CodeStyleGuide.md..."
	@if [ ! -f $(STYLE_GUIDE) ]; then \
		echo "Bootstrapping CodeStyleGuide.md..."; \
		$(STANDARDISE) --init-style-guide > $(STYLE_GUIDE); \
	else \
		awk '\
			/<!-- STANDARDISE:BEGIN -->/ { \
				print; \
				system("$(STANDARDISE) --emit-style-guide"); \
				skip=1; \
				next \
			} \
			/<!-- STANDARDISE:END -->/ { \
				skip=0; \
				print; \
				next \
			} \
			!skip { print } \
		' $(STYLE_GUIDE) > $(STYLE_GUIDE).tmp && \
		mv $(STYLE_GUIDE).tmp $(STYLE_GUIDE); \
	fi

standardise: check regen-standards


# ============================================================
# Loom verification test (deterministic, temp-only)
# ============================================================

LOOM_TEST_ROOT := tmp/loom/make
LOOM_TEST_DIR  := $(LOOM_TEST_ROOT)/run

.PHONY: loom-test

loom-test:
	@$(MAKE) test \
		TEST_ROOT=tmp/loom/make \
		TEST_RUN=run \
		TEST_DIR=tmp/loom/make/run
