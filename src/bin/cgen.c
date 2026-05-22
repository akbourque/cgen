#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#ifndef CGEN_VERSION
    #define CGEN_VERSION "unknown"
#endif

static bool is_safe_subcommand(const char *str) {
    if (str == NULL || *str == '\0') return false;
    for (size_t i = 0; str[i] != '\0'; i++) {
        char ch = str[i];
        if (isalnum((unsigned char)ch) == false && ch != '-') {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: cgen <subcommand> [options] [arguments]\n");
        return 1;
    }

    char *subcommand = argv[1];

    if (strcmp(subcommand, "-V") == 0 || strcmp(subcommand, "--version") == 0) {
        printf("cgen version %s\n", CGEN_VERSION);
        return 0;
    }

    if (is_safe_subcommand(subcommand) == false) {
        fprintf(stderr, "Error: Invalid subcommand name '%s'. Only alphanumeric characters and '-' are allowed.\n", subcommand);
        return 1;
    }

    char binary_name[256];
    if (snprintf(binary_name, sizeof(binary_name), "cgen-%s", subcommand) >= (int)sizeof(binary_name)) {
        fprintf(stderr, "Error: Subcommand name is too long.\n");
        return 1;
    }

    char **sub_argv = malloc(sizeof(char*) * argc);
    if (sub_argv == NULL) {
        perror("cgen error: memory allocation failed");
        return 1;
    }

    sub_argv[0] = binary_name;
    for (int i = 2; i < argc; i++) {
        sub_argv[i - 1] = argv[i];
    }
    sub_argv[argc - 1] = NULL; 

    execvp(binary_name, sub_argv);

    fprintf(stderr, "Error: 'cgen %s' is not a recognized cgen command.\n", subcommand);
    fprintf(stderr, "Could not find '%s' in your system PATH.\n", binary_name);
    
    free(sub_argv);
    return 1;
}
