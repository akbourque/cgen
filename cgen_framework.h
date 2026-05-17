#pragma once
#ifndef CGEN_FRAMEWORK_H
#define CGEN_FRAMEWORK_H

typedef struct {
    const char *subcommand_name; // e.g., "vec"
    const char *opt_spec;        // Custom subcommand option string, e.g., "=ttypename"
    const char *template_h;      // The raw header text string template
    const char *template_c;      // The raw source text string template
} cgen_app_def_t;

// NEW: Application definition for two-type containers
typedef struct {
    const char *subcommand_name; // e.g., "result", "map", "btree"
    const char *template_h;
    const char *template_c;
} cgen_app_dual_def_t;

// Shuts down boilerplate loops, runs token swaps, and clobbers files to disk
int cgen_app_run(const cgen_app_def_t *app, int argc, char **argv);

// NEW: Dual-type orchestration engine execution entrypoint
int cgen_app_run_dual(const cgen_app_dual_def_t *app, int argc, char **argv);

#endif // CGEN_FRAMEWORK_H
