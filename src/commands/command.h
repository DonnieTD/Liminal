#ifndef LIMINAL_COMMAND_H
#define LIMINAL_COMMAND_H

typedef int (*command_fn)(int argc, char **argv);

typedef struct {
    const char *name;
    int min_args;
    command_fn handler;
} CommandSpec;

/*
 * Dispatch a subcommand.
 *
 * argc/argv are expected to start at the command name.
 */
int dispatch_command(
    int argc,
    char **argv,
    const CommandSpec *commands,
    int command_count
);

#endif /* LIMINAL_COMMAND_H */
