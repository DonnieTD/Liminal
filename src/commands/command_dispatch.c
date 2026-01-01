#include <stdio.h>
#include <string.h>

#include "command.h"

int dispatch_command(
    int argc,
    char **argv,
    const CommandSpec *commands,
    int command_count
) {
    if (argc < 1) {
        fprintf(stderr, "error: missing command\n");
        return 1;
    }

    const char *cmd = argv[0];

    for (int i = 0; i < command_count; i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            if (argc - 1 < commands[i].min_args) {
                fprintf(stderr, "error: missing arguments for '%s'\n", cmd);
                return 1;
            }
            return commands[i].handler(argc - 1, argv + 1);
        }
    }

    fprintf(stderr, "error: unknown command '%s'\n", cmd);
    return 1;
}
