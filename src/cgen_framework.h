#pragma once
#ifndef CGEN_FRAMEWORK_H
#define CGEN_FRAMEWORK_H
#include "libpstr.h"
#include <stdbool.h>

// =====================================================================
// V2 ENGINE: The Generic "Slot Mapper"
// =====================================================================

/**
 * @brief Represents a single key-value token replacement mapping.
 */
typedef struct {
    const char *key;         // e.g., "00B", "KEY", "VAL_U"
    libpstr_pstr_t *val;     // The exact string to inject
} cgen_token_t;

/**
 * @brief The core templating engine. Uses a single-pass linear scanner 
 * to inject tokens into the template without heap reallocation thrashing.
 * * @param sb The builder to append the rendered text into.
 * @param template_slice The raw template string to scan.
 * @param tokens Array of available token mappings.
 * @param num_tokens Length of the tokens array.
 * @return 0 on success, 1 on missing token mapping, 2 on malformed syntax.
 */
int cgen_render_block(
    libpstr_builder_t *sb, 
    libpstr_slice_t template_slice, 
    cgen_token_t *tokens, 
    size_t num_tokens
);

/**
 * @brief Writes the contents of a builder directly to disk.
 */
bool cgen_write_file(const char *path, libpstr_builder_t *sb);


// =====================================================================
// LEGACY V1 ENGINE (Kept temporarily to ensure builds don't break)
// =====================================================================

typedef struct {
    const char *subcommand_name; 
    const char *opt_spec;        
    const char *template_h;      
    const char *template_c; 
} cgen_app_def_t;

typedef struct {
    const char *subcommand_name; 
    const char *template_h;
    const char *template_c; 
} cgen_app_dual_def_t;

int cgen_app_run(const cgen_app_def_t *app, int argc, char **argv);
int cgen_app_run_dual(const cgen_app_dual_def_t *app, int argc, char **argv);
libpstr_pstr_t *cgen_generate_filename(libpstr_pstr_t *container, libpstr_pstr_t *type_name);

#endif // CGEN_FRAMEWORK_H
