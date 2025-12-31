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


# -------------------------------------------------------------
# Flattened single-file build
# -------------------------------------------------------------
FLATTEN := artifacts/liminal_flat.c

flatten: $(FLATTEN)

$(FLATTEN):
	@echo "/* ========================================================= */" >  $@
	@echo "/*  LIMINAL — SINGLE FILE FLATTENED BUILD                    */" >> $@
	@echo "/*  Generated: $$(date)                                      */" >> $@
	@echo "/* ========================================================= */\n" >> $@

	@echo "/* --------- Headers ---------------------------------------- */\n" >> $@
	@find src -name '*.h' | sort | while read f; do \
		echo "\n/* === $$f ============================================ */" >> $@; \
		cat $$f >> $@; \
	done

	@echo "\n/* --------- Sources ---------------------------------------- */\n" >> $@

	@for f in $(SRC); do \
		echo "\n/* === $$f ============================================ */" >> $@; \
		cat $$f >> $@; \
	done

	@echo "\n/* ======================= END ============================== */" >> $@


# -------------------------------------------------------------
# Test target samples
# -------------------------------------------------------------
SAMPLES := $(sort $(wildcard src/samples/[0-9]*.c))

.PHONY: run-samples

run-samples: liminal
	@echo "== Running Liminal samples =="
	@set -e; \
	for f in $(SAMPLES); do \
		echo ""; \
		echo ">>> $$f"; \
		./liminal run $$f; \
	done

.PHONY: test

test: clean liminal run-samples


# -------------------------------------------------------------
# Flatten Samples
# -------------------------------------------------------------
SAMPLES      := $(sort $(wildcard src/samples/[0-9]*.c))
FLAT_SAMPLES := artifacts/samples.flat.txt


.PHONY: flatten-samples

flatten-samples: liminal
	@mkdir -p artifacts
	@echo "== Flattening Liminal samples ==" > $(FLAT_SAMPLES)
	@set -e; \
	for f in $(SAMPLES); do \
		echo "" >> $(FLAT_SAMPLES); \
		echo "==================================================" >> $(FLAT_SAMPLES); \
		echo "-- SAMPLE: $$f" >> $(FLAT_SAMPLES); \
		echo "==================================================" >> $(FLAT_SAMPLES); \
		echo "" >> $(FLAT_SAMPLES); \
		echo "-- SOURCE --" >> $(FLAT_SAMPLES); \
		cat $$f >> $(FLAT_SAMPLES); \
		echo "" >> $(FLAT_SAMPLES); \
		echo "" >> $(FLAT_SAMPLES); \
		echo "-- AST + EXECUTION --" >> $(FLAT_SAMPLES); \
		./liminal run $$f >> $(FLAT_SAMPLES); \
	done
	@echo ""
	@echo "Flattened samples written to $(FLAT_SAMPLES)"


# -------------------------------------------------------------
# Minified flattened single-file build (LLM-friendly)
# -------------------------------------------------------------
FLATTEN_MIN := artifacts/liminal_flat.min.c

flatten-min: $(FLATTEN_MIN)

$(FLATTEN_MIN):
	@mkdir -p artifacts
	@echo "/* LIMINAL_FLAT_MIN $(shell date -u +%Y%m%dT%H%M%SZ) */" > $@

	@find src -name '*.h' | sort | while read f; do \
		echo "//@header $$f" >> $@; \
		sed '/^[[:space:]]*$$/d' $$f >> $@; \
	done

	@find src -name '*.c' | grep -v 'src/samples/' | sort | while read f; do \
		echo "//@source $$f" >> $@; \
		sed '/^[[:space:]]*$$/d' $$f >> $@; \
	done


# -------------------------------------------------------------
# Minified flattened samples (LLM-friendly)
# -------------------------------------------------------------
FLAT_SAMPLES_MIN := artifacts/samples.flat.min.txt

.PHONY: flatten-samples-min

flatten-samples-min: liminal
	@mkdir -p artifacts
	@echo "@liminal_samples $(shell date -u +%Y%m%dT%H%M%SZ)" > $(FLAT_SAMPLES_MIN)
	@set -e; \
	for f in $(SAMPLES); do \
		echo "@sample $$f" >> $(FLAT_SAMPLES_MIN); \
		echo "@src" >> $(FLAT_SAMPLES_MIN); \
		sed '/^[[:space:]]*$$/d' $$f | tr '\n' ' ' >> $(FLAT_SAMPLES_MIN); \
		echo "" >> $(FLAT_SAMPLES_MIN); \
		echo "@ast" >> $(FLAT_SAMPLES_MIN); \
		./liminal run $$f | sed -n '/-- AST ARTIFACT --/,$$p' \
			| sed '/-- EXECUTION ARTIFACT --/,$$d' \
			| sed '/^[[:space:]]*$$/d' >> $(FLAT_SAMPLES_MIN); \
		echo "@exec" >> $(FLAT_SAMPLES_MIN); \
		./liminal run $$f | sed -n '/-- EXECUTION ARTIFACT --/,$$p' \
			| sed '/^[[:space:]]*$$/d' >> $(FLAT_SAMPLES_MIN); \
	done
