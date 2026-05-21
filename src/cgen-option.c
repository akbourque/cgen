#include <string.h>
#include "cgen_framework.h"

// ============================================================================
// 1. HEADER ASSET TEMPLATE
// ============================================================================
const char *OPTION_TEMPLATE_H =
    "#pragma once\n"
    "#ifndef OPTION_{{00BU}}_H\n"
    "#define OPTION_{{00BU}}_H\n\n"
    "#include <stdbool.h>\n"
    "#include <stddef.h>\n\n"
    "/**\n"
    " * @brief Discriminator tag indicating the presence or absence of an Option value.\n"
    " */\n"
    "typedef enum {\n"
    "    OPTION_{{00BU}}_NONE = 0, /**< Indicates the option contains no data. */\n"
    "    OPTION_{{00BU}}_SOME = 1  /**< Indicates the option contains a valid payload. */\n"
    "} option_{{00B}}_tag_t;\n\n"
    
    "/**\n"
    " * @brief A type-safe variant structure representing an optional value.\n"
    " */\n"
    "typedef struct {\n"
    "    option_{{00B}}_tag_t tag; /**< Active state discriminator. */\n"
    "    union {\n"
    "        {{00}} value;         /**< Target payload data. Valid only if tag is SOME. */\n"
    "    } as;\n"
    "} option_{{00B}}_t;\n\n"
    
    "/**\n"
    " * @brief Constructs an Option containing a valid payload item.\n"
    " */\n"
    "option_{{00B}}_t option_{{00B}}_some({{00}} val);\n\n"
    
    "/**\n"
    " * @brief Constructs an empty Option containing no payload.\n"
    " */\n"
    "option_{{00B}}_t option_{{00B}}_none(void);\n\n"
    
    "/**\n"
    " * @brief Returns true if the option contains a valid payload item.\n"
    " */\n"
    "bool option_{{00B}}_is_some(const option_{{00B}}_t *opt);\n\n"
    
    "/**\n"
    " * @brief Returns true if the option contains no payload item.\n"
    " */\n"
    "bool option_{{00B}}_is_none(const option_{{00B}}_t *opt);\n\n"
    
    "/**\n"
    " * @brief Extracts the internal payload value.\n"
    " *\n"
    " * @warning If the instance is NONE, this function will print a panic message\n"
    " * to stderr and abort execution immediately.\n"
    " *\n"
    " * @param opt The target option structure.\n"
    " * @return The raw underlying payload value.\n"
    " */\n"
    "{{00}} option_{{00B}}_unwrap(option_{{00B}}_t opt);\n\n"
    "#endif\n";

// ============================================================================
// 2. SOURCE ASSET TEMPLATE
// ============================================================================
const char *OPTION_TEMPLATE_C =
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include \"option_{{00}}.h\"\n\n"
    
    "option_{{00B}}_t option_{{00B}}_some({{00}} val) {\n"
    "    option_{{00B}}_t opt;\n"
    "    opt.tag = OPTION_{{00BU}}_SOME;\n"
    "    opt.as.value = val;\n"
    "    return opt;\n"
    "}\n\n"
    
    "option_{{00B}}_t option_{{00B}}_none(void) {\n"
    "    option_{{00B}}_t opt;\n"
    "    opt.tag = OPTION_{{00BU}}_NONE;\n"
    "    return opt;\n"
    "}\n\n"
    
    "bool option_{{00B}}_is_some(const option_{{00B}}_t *opt) {\n"
    "    if (opt == NULL) return false;\n"
    "    return opt->tag == OPTION_{{00BU}}_SOME;\n"
    "}\n\n"
    
    "bool option_{{00B}}_is_none(const option_{{00B}}_t *opt) {\n"
    "    if (opt == NULL) return true;\n"
    "    return opt->tag == OPTION_{{00BU}}_NONE;\n"
    "}\n\n"
    
    "{{00}} option_{{00B}}_unwrap(option_{{00B}}_t opt) {\n"
    "    if (opt.tag == OPTION_{{00BU}}_NONE) {\n"
    "        fprintf(stderr, \"panic: called option_{{00B}}_unwrap() on a NONE value\\n\");\n"
    "        abort();\n"
    "    }\n"
    "    return opt.as.value;\n"
    "}\n";


// ============================================================================
// 3. APPLICATION ENTRYPOINT
// ============================================================================
int main(int argc, char **argv) {
    cgen_app_def_t app = {
        .subcommand_name = "option",
        .opt_spec        = "=ttypename",
        .template_h      = OPTION_TEMPLATE_H,
        .template_c      = OPTION_TEMPLATE_C,
    };
    return cgen_app_run(&app, argc - 1, argv + 1);
}
