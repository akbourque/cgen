#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef CGEN_VERSION
    #define CGEN_VERSION "unknown"
#endif

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: cgen <subcommand> [options] [arguments]\n");
        return 1;
    }

    char *subcommand = argv[1];
    // Intercept global version flags before executing any sub-commands
    if (strcmp(subcommand, "-V") == 0 || strcmp(subcommand, "--version") == 0) {
        printf("cgen version %s\n", CGEN_VERSION);
        return 0;
    }

    // 1. Construct the target binary name: "cgen-" + subcommand
    char binary_name[256];
    snprintf(binary_name, sizeof(binary_name), "cgen-%s", subcommand);

    // 2. Prepare the execution arguments array
    char **sub_argv = malloc(sizeof(char*) * argc);
    if (!sub_argv) {
        perror("cgen error: memory allocation failed");
        return 1;
    }

    sub_argv[0] = binary_name;
    for (int i = 2; i < argc; i++) {
        sub_argv[i - 1] = argv[i];
    }
    sub_argv[argc - 1] = NULL; // Must be NULL-terminated for exec functions

    // 3. Construct the explicit local path relative to the current directory
    char local_path[280];
    snprintf(local_path, sizeof(local_path), "./%s", binary_name);

    // 4. PRIORITY 1: Check the current working directory first
    // X_OK checks if the file exists AND has executable permissions
    if (access(local_path, X_OK) == 0) {
        // execv executes an exact path without searching the system PATH
        execv(local_path, sub_argv);
    }

    // 5. PRIORITY 2: Fallback to the global system PATH scan
    execvp(binary_name, sub_argv);

    // --- Failure Path ---
    // This only runs if BOTH local execv and global execvp failed to find the binary.
    fprintf(stderr, "Error: 'cgen %s' is not a recognized cgen command.\n", subcommand);
    fprintf(stderr, "Could not find '%s' locally or in your system PATH.\n", binary_name);
    
    free(sub_argv);
    return 1;
}
