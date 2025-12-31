CC      := cc
CFLAGS  := -std=c99 -Wall -Wextra -Wpedantic -g -Isrc
LDFLAGS :=

BUILD   := build
BIN     := liminal

# ------------------------------------------------------------
# Tool source files ONLY (no samples)
# ------------------------------------------------------------

SRC := \
    src/main.c \
    src/common/arena.c \
    src/common/file.c \
    src/common/hashmap.c \
    src/frontends/c/ast.c \
    src/frontends/c/ast_dump.c \
    src/frontends/c/frontend.c \
    src/frontends/c/lexer.c \
    src/frontends/c/parser.c \
    src/executor/world.c \
    src/executor/universe.c \
    src/executor/scope.c \
    src/executor/step.c \
    src/executor/stack.c \
    src/executor/memory.c \
    src/analyzer/trace.c \
    src/analyzer/validate.c \
    src/analyzer/lifetime.c \
    src/analyzer/use_validate.c \
    src/analyzer/variable_lifetime.c

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
