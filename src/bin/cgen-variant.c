#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "libpstr.h"
#include "cgen_framework.h"

#define MAX_CHOICES 64

// =========================================================================
// THE TEMPLATE BLOCKS
// =========================================================================

const char *TMPL_H_TOP = 
    "#pragma once\n"
    "#include <stddef.h>\n"
    "#include <stdbool.h>\n"
    "#include \"libpstr.h\"\n"
    "#include \"panic.h\"\n\n";

const char *TMPL_H_TYPEDEF = "typedef {{TYPE}} {{PREFIX}}_{{FIELD}}_t;\n";

const char *TMPL_H_MID = 
    "\ntypedef enum {\n"
    "    {{PREFIX_U}}_EMPTY,\n";

const char *TMPL_H_ENUM   = "    {{PREFIX_U}}_{{FIELD_U}},\n";

const char *TMPL_H_STRUCT_TOP = 
    "} {{PREFIX}}_tag_t;\n\n"
    "typedef struct {\n"
    "    {{PREFIX}}_tag_t tag;\n"
    "    union {\n";

const char *TMPL_H_STRUCT = "        {{PREFIX}}_{{FIELD}}_t as_{{FIELD}};\n";

const char *TMPL_H_BOT    = "    } data;\n} {{VARIANT}};\n\n";

// --- Header Methods (With Doxygen & Typedefs) ---
const char *TMPL_H_CONSTRUCTOR = 
    "/**\n"
    " * @brief Constructs a new {{VARIANT}} containing a {{FIELD}} value.\n"
    " * @param val The {{PREFIX}}_{{FIELD}}_t value to store in the variant.\n"
    " * @return A new {{VARIANT}} instance initialized with the provided value.\n"
    " */\n"
    "{{VARIANT}} {{PREFIX}}_new_{{FIELD}}({{PREFIX}}_{{FIELD}}_t val);\n\n";

const char *TMPL_H_INSPECTOR = 
    "/**\n"
    " * @brief Inspects the {{VARIANT}} to determine if it currently holds a {{FIELD}}.\n"
    " * @param v Pointer to the {{VARIANT}} to inspect.\n"
    " * @return true if the variant holds a {{FIELD}}, false otherwise.\n"
    " */\n"
    "bool {{PREFIX}}_is_{{FIELD}}(const {{VARIANT}}* v);\n\n";

const char *TMPL_H_EXTRACTOR = 
    "/**\n"
    " * @brief Extracts the {{FIELD}} value from the {{VARIANT}}.\n"
    " * @note This function will trigger a PANIC if the variant does not currently hold a {{FIELD}}.\n"
    " * @param v Pointer to the {{VARIANT}} to unwrap.\n"
    " * @return The underlying {{PREFIX}}_{{FIELD}}_t value.\n"
    " */\n"
    "{{PREFIX}}_{{FIELD}}_t {{PREFIX}}_unwrap_{{FIELD}}(const {{VARIANT}}* v);\n\n";

// --- Source Methods (With Typedefs) ---
const char *TMPL_C_TOP = "#include \"{{VARIANT}}.h\"\n#include <string.h>\n\n";

const char *TMPL_C_CONSTRUCTOR = 
    "{{VARIANT}} {{PREFIX}}_new_{{FIELD}}({{PREFIX}}_{{FIELD}}_t val) {\n"
    "    {{VARIANT}} v;\n"
    "    v.tag = {{PREFIX_U}}_{{FIELD_U}};\n"
    "    v.data.as_{{FIELD}} = val;\n"
    "    return v;\n"
    "}\n\n";

const char *TMPL_C_INSPECTOR = 
    "bool {{PREFIX}}_is_{{FIELD}}(const {{VARIANT}}* v) {\n"
    "    return v->tag == {{PREFIX_U}}_{{FIELD_U}};\n"
    "}\n\n";

const char *TMPL_C_EXTRACTOR = 
    "{{PREFIX}}_{{FIELD}}_t {{PREFIX}}_unwrap_{{FIELD}}(const {{VARIANT}}* v) {\n"
    "    if (v->tag != {{PREFIX_U}}_{{FIELD_U}}) {\n"
    "        PANIC(\"Catastrophic: Variant unwrapping type mismatch violation!\");\n"
    "    }\n"
    "    return v->data.as_{{FIELD}};\n"
    "}\n\n";


// =========================================================================
// GENERATOR LOGIC
// =========================================================================

typedef struct {
    libpstr_pstr_t *field_name;
    libpstr_pstr_t *underlying_type;
    libpstr_pstr_t *upper_name;
} variant_choice_t;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <variant_name> <field:type> [field2:type2 ...]\n", argv[0]);
        return 1;
    }

    // Safety guard to catch shifted arguments
    if (strchr(argv[1], ':') != NULL) {
        fprintf(stderr, "CGEN ERROR: Variant name '%s' contains a colon.\n", argv[1]);
        fprintf(stderr, "Did you forget the <variant_name> argument?\n");
        return 1;
    }

    // 1. Parse Variant Names
    libpstr_pstr_t *variant_name = libpstr.pstr.from_cstr(argv[1]);
    libpstr_slice_t pre_slice = { .ptr = variant_name->buf, .len = variant_name->len };
    if (libpstr.slice.ends_with(pre_slice, "_t") == true || libpstr.slice.ends_with(pre_slice, "_T") == true) {
        pre_slice.len -= 2;
    }
    
    libpstr_pstr_t *prefix = libpstr.pstr.from_slice(pre_slice);
    libpstr_pstr_t *prefix_upper = libpstr.pstr.from_slice(pre_slice);
    libpstr.pstr.to_uppercase(prefix_upper);

    // 2. Parse Field Choices
    variant_choice_t choices[MAX_CHOICES];
    int choice_count = 0;

    for (int i = 2; i < argc && choice_count < MAX_CHOICES; i++) {
        libpstr_slice_t arg_slice = { .ptr = argv[i], .len = strlen(argv[i]) };
        libpstr_slice_t left, right;
        libpstr_slice_t field_slice;

        // Split field:type if a colon exists
        if (libpstr.slice.split_once(arg_slice, ":", &left, &right) == true) {
            field_slice = left;
            choices[choice_count].underlying_type = libpstr.pstr.from_slice(right);
        } else {
            // No colon: Field name and underlying type are the same
            field_slice = arg_slice;
            choices[choice_count].underlying_type = libpstr.pstr.from_slice(arg_slice);
        }
        
        // --- THE FIX: Strip _t / _T from the field name slice ---
        if (libpstr.slice.ends_with(field_slice, "_t") == true || libpstr.slice.ends_with(field_slice, "_T") == true) {
            field_slice.len -= 2;
        }

        // Save the cleaned field name
        choices[choice_count].field_name = libpstr.pstr.from_slice(field_slice);
        
        // Generate the uppercase version for macros/enums
        choices[choice_count].upper_name = libpstr.pstr.from_cstr(choices[choice_count].field_name->buf);
        libpstr.pstr.to_uppercase(choices[choice_count].upper_name);
        
        choice_count++;
    }

    if (choice_count == 0) {
        fprintf(stderr, "Error: No variant fields specified.\n");
        return 1;
    }

    // 3. Set up the Token Maps
    cgen_token_t base_tokens[3] = {
        { "VARIANT",  variant_name },
        { "PREFIX",   prefix },
        { "PREFIX_U", prefix_upper }
    };

    cgen_token_t choice_tokens[6] = {
        { "VARIANT",  variant_name },
        { "PREFIX",   prefix },
        { "PREFIX_U", prefix_upper },
        { "FIELD",    NULL }, 
        { "FIELD_U",  NULL }, 
        { "TYPE",     NULL }  
    };

    // 4. Initialize Builders
    libpstr_builder_t sb_h; libpstr.builder.init(&sb_h);
    libpstr_builder_t sb_c; libpstr.builder.init(&sb_c);

    // --- RENDER HEADER STRUCTURE ---
    cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_TOP, strlen(TMPL_H_TOP)}, base_tokens, 3);
    
    // Render Typedefs
    libpstr.builder.append_cstr(&sb_h, "/* --- Auto-Generated Typedef Aliases --- */\n");
    for (int i = 0; i < choice_count; i++) {
        choice_tokens[3].val = choices[i].field_name;
        choice_tokens[4].val = choices[i].upper_name;
        choice_tokens[5].val = choices[i].underlying_type;
        cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_TYPEDEF, strlen(TMPL_H_TYPEDEF)}, choice_tokens, 6);
    }

    // Render Enum
    cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_MID, strlen(TMPL_H_MID)}, base_tokens, 3);
    for (int i = 0; i < choice_count; i++) {
        choice_tokens[3].val = choices[i].field_name;
        choice_tokens[4].val = choices[i].upper_name;
        choice_tokens[5].val = choices[i].underlying_type;
        cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_ENUM, strlen(TMPL_H_ENUM)}, choice_tokens, 6);
    }
    
    // Render Struct
    cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_STRUCT_TOP, strlen(TMPL_H_STRUCT_TOP)}, base_tokens, 3);
    for (int i = 0; i < choice_count; i++) {
        choice_tokens[3].val = choices[i].field_name;
        choice_tokens[4].val = choices[i].upper_name;
        choice_tokens[5].val = choices[i].underlying_type;
        cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_STRUCT, strlen(TMPL_H_STRUCT)}, choice_tokens, 6);
    }
    cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_BOT, strlen(TMPL_H_BOT)}, base_tokens, 3);

    // --- RENDER METHODS (Header + Source) ---
    cgen_render_block(&sb_c, (libpstr_slice_t){TMPL_C_TOP, strlen(TMPL_C_TOP)}, base_tokens, 3);

    // Group 1: Constructors
    libpstr.builder.append_cstr(&sb_h, "/* --- Safe Constructors --- */\n");
    libpstr.builder.append_cstr(&sb_c, "/* --- Constructors Implementation --- */\n");
    for (int i = 0; i < choice_count; i++) {
        choice_tokens[3].val = choices[i].field_name;
        choice_tokens[4].val = choices[i].upper_name;
        choice_tokens[5].val = choices[i].underlying_type;
        
        cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_CONSTRUCTOR, strlen(TMPL_H_CONSTRUCTOR)}, choice_tokens, 6);
        cgen_render_block(&sb_c, (libpstr_slice_t){TMPL_C_CONSTRUCTOR, strlen(TMPL_C_CONSTRUCTOR)}, choice_tokens, 6);
    }

    // Group 2: Inspectors
    libpstr.builder.append_cstr(&sb_h, "/* --- Type Inspectors --- */\n");
    libpstr.builder.append_cstr(&sb_c, "/* --- Inspectors Implementation --- */\n");
    for (int i = 0; i < choice_count; i++) {
        choice_tokens[3].val = choices[i].field_name;
        choice_tokens[4].val = choices[i].upper_name;
        choice_tokens[5].val = choices[i].underlying_type;
        
        cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_INSPECTOR, strlen(TMPL_H_INSPECTOR)}, choice_tokens, 6);
        cgen_render_block(&sb_c, (libpstr_slice_t){TMPL_C_INSPECTOR, strlen(TMPL_C_INSPECTOR)}, choice_tokens, 6);
    }

    // Group 3: Extractors
    libpstr.builder.append_cstr(&sb_h, "/* --- Value Extractors --- */\n");
    libpstr.builder.append_cstr(&sb_c, "/* --- Extractors Implementation --- */\n");
    for (int i = 0; i < choice_count; i++) {
        choice_tokens[3].val = choices[i].field_name;
        choice_tokens[4].val = choices[i].upper_name;
        choice_tokens[5].val = choices[i].underlying_type;
        
        cgen_render_block(&sb_h, (libpstr_slice_t){TMPL_H_EXTRACTOR, strlen(TMPL_H_EXTRACTOR)}, choice_tokens, 6);
        cgen_render_block(&sb_c, (libpstr_slice_t){TMPL_C_EXTRACTOR, strlen(TMPL_C_EXTRACTOR)}, choice_tokens, 6);
    }

    // 5. Write to Disk
    libpstr_pstr_t *h_filename = libpstr.pstr.format("%s.h", variant_name->buf);
    libpstr_pstr_t *c_filename = libpstr.pstr.format("%s.c", variant_name->buf);

    if (cgen_write_file(h_filename->buf, &sb_h) == false) fprintf(stderr, "Failed to write %s\n", h_filename->buf);
    if (cgen_write_file(c_filename->buf, &sb_c) == false) fprintf(stderr, "Failed to write %s\n", c_filename->buf);

    // 6. Be a good Garbage Collector
    libpstr.builder.cleanup(&sb_h);
    libpstr.builder.cleanup(&sb_c);
    libpstr.pstr.free(h_filename);
    libpstr.pstr.free(c_filename);
    
    libpstr.pstr.free(variant_name);
    libpstr.pstr.free(prefix);
    libpstr.pstr.free(prefix_upper);
    
    for (int i = 0; i < choice_count; i++) {
        libpstr.pstr.free(choices[i].field_name);
        libpstr.pstr.free(choices[i].underlying_type);
        libpstr.pstr.free(choices[i].upper_name);
    }

    return 0;
}
