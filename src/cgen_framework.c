#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cgen_framework.h"
#include "opt.h"
#include "parser.h"

// =====================================================================
// V2 ENGINE: The Generic "Slot Mapper" Implementation
// =====================================================================

int cgen_render_block(libpstr_builder_t *sb, libpstr_slice_t template_slice, cgen_token_t *tokens, size_t num_tokens) {
    if (sb == NULL || template_slice.ptr == NULL) {
        return -1;
    }

    libpstr_slice_t remaining = template_slice;
    libpstr_slice_t left, right;

    // Scan forward sequentially for the opening brackets
    while (libpstr.slice.split_once(remaining, "{{", &left, &right) == true) {
        
        // 1. Append the normal raw text before the token
        if (left.len > 0) {
            libpstr.builder.append(sb, left.ptr, left.len);
        }
        
        // 2. Find the closing brackets
        libpstr_slice_t token_name, after_token;
        if (libpstr.slice.split_once(right, "}}", &token_name, &after_token) == true) {
            
            // 3. Lookup the token in the provided dictionary
            bool found = false;
            for (size_t i = 0; i < num_tokens; i++) {
                // Safeguard: Check exact length first to prevent "00B" from matching "00BU"
                if (strlen(tokens[i].key) == token_name.len && 
                    strncmp(token_name.ptr, tokens[i].key, token_name.len) == 0) {
                    
                    if (tokens[i].val != NULL) {
                        libpstr.builder.append_pstr(sb, tokens[i].val);
                    }
                    found = true;
                    break;
                }
            }
            
            // 🚨 FAIL-FAST: Unmapped token discovered 🚨
            if (found == false) {
                fprintf(stderr, "CGEN ERROR: Unmapped template token '{{%.*s}}' discovered.\n", 
                        (int)token_name.len, token_name.ptr);
                return 1; 
            }
            
            // 4. Advance the scanner past the "}}"
            remaining = after_token;
        } else {
            // 🚨 FAIL-FAST: Malformed template syntax 🚨
            fprintf(stderr, "CGEN ERROR: Malformed template. Found '{{' without closing '}}'.\n");
            return 2; 
        }
    }
    
    // Append any trailing text after the final token substitution
    if (remaining.len > 0) {
        libpstr.builder.append(sb, remaining.ptr, remaining.len);
    }
    
    return 0; // Success
}

bool cgen_write_file(const char *path, libpstr_builder_t *sb) {
    if (path == NULL || sb == NULL) return false;
    
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        return false;
    }
    if (sb->vec.len > 0) {
        fwrite(sb->vec.data, 1, sb->vec.len, f);
    }
    fclose(f);
    return true;
}


// =====================================================================
// LEGACY V1 ENGINE (Untouched for backward compatibility)
// =====================================================================

libpstr_pstr_t *cgen_generate_filename(libpstr_pstr_t *container, libpstr_pstr_t *type_name) {
    if (container == NULL || type_name == NULL) return NULL;

    libpstr_builder_t sb;
    libpstr.builder.init(&sb);

    libpstr.builder.append_pstr(&sb, container);
    libpstr.builder.append_cstr(&sb, "_");
    libpstr.builder.append_pstr(&sb, type_name);

    libpstr_pstr_t *generated_name = libpstr.builder.build(&sb);
    libpstr.builder.cleanup(&sb);

    if (libpstr.pstr.ends_with(type_name, "_t") == true) {
        if (libpstr.pstr.ends_with(generated_name, "_t") == false) {
            libpstr.builder.init(&sb);
            libpstr.builder.append_pstr(&sb, generated_name);
            libpstr.builder.append_cstr(&sb, "_t");
            
            libpstr.pstr.free(generated_name);
            generated_name = libpstr.builder.build(&sb);
            libpstr.builder.cleanup(&sb);
        }
    }

    libpstr.builder.init(&sb);
    libpstr.builder.append_pstr(&sb, generated_name);
    libpstr.builder.append_cstr(&sb, ".c");
    
    libpstr.pstr.free(generated_name);
    generated_name = libpstr.builder.build(&sb);
    libpstr.builder.cleanup(&sb);

    return generated_name;
}

static void framework_replace_tokens(libpstr_builder_t *sb, libpstr_pstr_t *replacement) {
    libpstr_slice_t rep_slice = { .ptr = replacement->buf, .len = replacement->len };
    libpstr_slice_t base_slice = rep_slice;
    
    if (libpstr.slice.ends_with(rep_slice, "_t") == true || libpstr.slice.ends_with(rep_slice, "_T") == true) {
        base_slice.len -= 2;
    }

    libpstr_pstr_t *full_exact = libpstr.pstr.from_slice(rep_slice);
    libpstr_pstr_t *full_upper = libpstr.pstr.from_slice(rep_slice);
    libpstr.pstr.to_uppercase(full_upper);

    libpstr_pstr_t *base_exact = libpstr.pstr.from_slice(base_slice);
    libpstr_pstr_t *base_upper = libpstr.pstr.from_slice(base_slice);
    libpstr.pstr.to_uppercase(base_upper);

    while (1) {
        libpstr_slice_t match = libpstr.builder.find_cstr(sb, "{{00BU}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        libpstr.builder.replace_range(sb, offset, match.len, base_upper->buf, base_upper->len);
    }

    while (1) {
        libpstr_slice_t match = libpstr.builder.find_cstr(sb, "{{00B}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        libpstr.builder.replace_range(sb, offset, match.len, base_exact->buf, base_exact->len);
    }

    while (1) {
        libpstr_slice_t match = libpstr.builder.find_cstr(sb, "{{00U}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        libpstr.builder.replace_range(sb, offset, match.len, full_upper->buf, full_upper->len);
    }

    while (1) {
        libpstr_slice_t match = libpstr.builder.find_cstr(sb, "{{00}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        libpstr.builder.replace_range(sb, offset, match.len, full_exact->buf, full_exact->len);
    }

    libpstr.pstr.free(full_exact);
    libpstr.pstr.free(full_upper);
    libpstr.pstr.free(base_exact);
    libpstr.pstr.free(base_upper);
}

static bool write_file_clobber(const char *path, libpstr_builder_t *sb) {
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        return false;
    }
    if (sb->vec.len > 0) {
        fwrite(sb->vec.data, 1, sb->vec.len, f);
    }
    fclose(f);
    return true;
}

// static bool is_native_type_name(const libpstr_pstr_t* type) {
//     static const char* builtin[] = {"char", "short", "int", "long", "float", "double", "unsigned", "signed", NULL};
//     if (type == NULL) {
//         return false;
//     }
//
//     libpstr_slice_t trimmed = (libpstr_slice_t){ .ptr = type->buf, .len = type->len };
//     trimmed = libpstr.slice.trim(trimmed);
//
//     if (libpstr.slice.starts_with(trimmed, "unsigned ") == true) {
//         trimmed.ptr += 9;
//         trimmed.len -= 9;
//     } else if (libpstr.slice.starts_with(trimmed, "signed ") == true) {
//         trimmed.ptr += 7;
//         trimmed.len -= 7;
//     }
//
//     assert(trimmed.len > 0); 
//
//     for(size_t n = 0; builtin[n] != NULL; ++n) {
//         libpstr_slice_t slice = libpstr.slice.find_cstr(type, builtin[n]);
//         if (slice.ptr != NULL && slice.len == type->len) {
//             return true;
//         }
//     }
//
//     return false;
// }

static libpstr_pstr_t* normalize_type_name(const libpstr_pstr_t *type) {
    if (type == NULL || type->len == 0) return NULL;

    const char *raw = type->buf;
    if (strcmp(raw, "unsigned int") == 0)   return libpstr.pstr.from_cstr("uint");
    if (strcmp(raw, "unsigned long") == 0)  return libpstr.pstr.from_cstr("ulong");
    if (strcmp(raw, "unsigned char") == 0)  return libpstr.pstr.from_cstr("uchar");
    if (strcmp(raw, "unsigned short") == 0) return libpstr.pstr.from_cstr("ushort");
    if (strcmp(raw, "signed char") == 0)    return libpstr.pstr.from_cstr("schar");
    if (strcmp(raw, "long long") == 0)      return libpstr.pstr.from_cstr("llong");
    if (strcmp(raw, "long double") == 0)    return libpstr.pstr.from_cstr("ldouble");

    libpstr_pstr_t *safe_name = libpstr.pstr.from_cstr(raw);
    for (size_t i = 0; i < safe_name->len; i++) {
        char c = safe_name->buf[i];
        if (c == ' ') {
            safe_name->buf[i] = '_';
        } else if (c == '*') {
            safe_name->buf[i] = 'p'; 
        }
    }
    return safe_name;
}

int cgen_app_run(const cgen_app_def_t *app, int argc, char **argv) {
    cgen_opt_t schema[3];
    schema[0] = cgen_opt_new((libpstr_slice_t){.ptr = app->opt_spec, .len = strlen(app->opt_spec)});
    schema[1] = cgen_opt_new((libpstr_slice_t){.ptr = "=oout-dir", .len = 9});
    schema[2] = cgen_opt_new((libpstr_slice_t){.ptr = "-hhelp", .len = 6});

    cgen_parser_t parser;
    cgen_parser_init(&parser, argc, argv, schema, 3);

    libpstr_pstr_t *parsed_option_0 = NULL;
    libpstr_pstr_t *parsed_argument_0 = NULL;
    libpstr_pstr_t *out_dir = NULL;

    cgen_parse_result_t res;
    while ((res = cgen_parser_next(&parser)).kind != CGEN_PARSE_NONE) {
        if (res.kind == CGEN_PARSE_ERR) {
            fprintf(stderr, "Error: %s\n", res.as.error.msg->buf);
            cgen_parse_result_free(res);
            cgen_parser_cleanup(&parser);
            if (parsed_option_0 != NULL) libpstr.pstr.free(parsed_option_0);
            if (parsed_argument_0 != NULL) libpstr.pstr.free(parsed_argument_0);
            if (out_dir != NULL) libpstr.pstr.free(out_dir);
            return 1;
        }

        if (res.kind == CGEN_PARSE_OPTION) {
            if (cgen_opt_short_name(res.as.option.opt) == 'h') {
                printf("Usage: cgen %s [options] <typename>\n\n", app->subcommand_name);
                printf("Options:\n");
                printf("  -h, --help               Show this help menu context\n");
                printf("  -o, --out-dir <dir>      Specify target output directory prefix\n");
                printf("  -t, --typename <name>    Explicitly specify the target type name\n");

                cgen_parse_result_free(res);
                cgen_parser_cleanup(&parser);
                if (parsed_option_0 != NULL) libpstr.pstr.free(parsed_option_0);
                if (parsed_argument_0 != NULL) libpstr.pstr.free(parsed_argument_0);
                if (out_dir != NULL) libpstr.pstr.free(out_dir);
                return 0;
            }
            else if (cgen_opt_short_name(res.as.option.opt) == 'o') {
                if (res.as.option.arg == NULL) {
                    fprintf(stderr, "Error: Option -o requires an argument.\n");
                    exit(EXIT_FAILURE);
                }
                
                if (res.as.option.arg->len == 0) {
                    fprintf(stderr, "Error: Output directory cannot be empty.\n");
                    exit(EXIT_FAILURE);
                }

                if (out_dir != NULL) libpstr.pstr.free(out_dir);
                out_dir = res.as.option.arg;
                res.as.option.arg = NULL;
            } 
        } else if (res.kind == CGEN_PARSE_NON_OPTION_ARG) {
            if (parsed_argument_0 == NULL) {
                parsed_argument_0 = res.as.non_option_arg;
                res.as.non_option_arg = NULL;
            }
        }
        cgen_parse_result_free(res);
    }

    libpstr_pstr_t *replacement = parsed_option_0;
    if (replacement == NULL || replacement->len == 0) {
        replacement = parsed_argument_0;
    }

    if (replacement == NULL || replacement->len == 0) {
        fprintf(stderr, "Error: Missing target type name specification.\n");
        cgen_parser_cleanup(&parser);
        if (out_dir != NULL) libpstr.pstr.free(out_dir);
        return 1;
    }

    //bool native_type = is_native_type_name(replacement);
    libpstr_pstr_t *safe_replacement = normalize_type_name(replacement);
    libpstr_pstr_t *base_path = NULL;

    if (out_dir != NULL && out_dir->len > 0) {
        if (out_dir->buf[out_dir->len - 1] == '/') {
            base_path = libpstr.pstr.format("%s", out_dir->buf);
        } else {
            base_path = libpstr.pstr.format("%s/", out_dir->buf);
        }
    } else {
        base_path = libpstr.pstr.from_cstr("");
    }

    // 1. Generate the 4 token variants just like before
    libpstr_slice_t rep_slice = { .ptr = safe_replacement->buf, .len = safe_replacement->len };
    libpstr_slice_t base_slice = rep_slice;
    
    if (libpstr.slice.ends_with(rep_slice, "_t") == true || libpstr.slice.ends_with(rep_slice, "_T") == true) {
        base_slice.len -= 2;
    }

    libpstr_pstr_t *full_exact = libpstr.pstr.from_slice(rep_slice);
    libpstr_pstr_t *full_upper = libpstr.pstr.from_slice(rep_slice);
    libpstr.pstr.to_uppercase(full_upper);

    libpstr_pstr_t *base_exact = libpstr.pstr.from_slice(base_slice);
    libpstr_pstr_t *base_upper = libpstr.pstr.from_slice(base_slice);
    libpstr.pstr.to_uppercase(base_upper);

    // 2. ⚡ DEFINE THE TOKEN MAP FOR SINGLE-TYPE TOOLS ⚡
    cgen_token_t tokens[4] = {
        { "00",   full_exact },
        { "00U",  full_upper },
        { "00B",  base_exact },
        { "00BU", base_upper }
    };

    // 3. Render the Header
    libpstr_builder_t sb_h;
    libpstr.builder.init(&sb_h);
    libpstr_slice_t tmpl_h = { .ptr = app->template_h, .len = strlen(app->template_h) };
    
    if (cgen_render_block(&sb_h, tmpl_h, tokens, 4) != 0) {
        fprintf(stderr, "Fatal error rendering header template.\n");
        exit(EXIT_FAILURE);
    }
    
    libpstr_pstr_t *path_h = libpstr.pstr.format("%s%s_%s.h", base_path->buf, app->subcommand_name, safe_replacement->buf);

    // 4. Render the Source
    libpstr_builder_t sb_c;
    libpstr.builder.init(&sb_c);
    
    if (app->template_c == NULL) {
        fprintf(stderr, "Critical: Missing template_c definition.\n");
        exit(EXIT_FAILURE);
    }
    
    libpstr_slice_t tmpl_c = { .ptr = app->template_c, .len = strlen(app->template_c) };
    if (cgen_render_block(&sb_c, tmpl_c, tokens, 4) != 0) {
        fprintf(stderr, "Fatal error rendering source template.\n");
        exit(EXIT_FAILURE);
    }
    libpstr_pstr_t *path_c = libpstr.pstr.format("%s%s_%s.c", base_path->buf, app->subcommand_name, safe_replacement->buf);

    if (write_file_clobber(path_h->buf, &sb_h) == false) {
        fprintf(stderr, "Error: Failed to write file to disk at destination: %s\n", path_h->buf);
    }
    if (write_file_clobber(path_c->buf, &sb_c) == false) {
        fprintf(stderr, "Error: Failed to write file to disk at destination: %s\n", path_c->buf);
    }

    libpstr.pstr.free(full_exact);
    libpstr.pstr.free(full_upper);
    libpstr.pstr.free(base_exact);
    libpstr.pstr.free(base_upper);
    libpstr.builder.cleanup(&sb_h);
    libpstr.builder.cleanup(&sb_c);
    libpstr.pstr.free(path_h);
    libpstr.pstr.free(path_c);
    libpstr.pstr.free(base_path);
    
    libpstr.pstr.free(safe_replacement); 
    
    if (parsed_option_0 != NULL) libpstr.pstr.free(parsed_option_0);
    if (parsed_argument_0 != NULL) libpstr.pstr.free(parsed_argument_0);
    if (out_dir != NULL) libpstr.pstr.free(out_dir);
    cgen_parser_cleanup(&parser);

    return 0;
}

static void framework_replace_string(libpstr_builder_t *sb, const char *token, const char *rep, size_t rep_len) {
    while (1) {
        libpstr_slice_t match = libpstr.builder.find_cstr(sb, token);
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        libpstr.builder.replace_range(sb, offset, match.len, rep, rep_len);
    }
}

static void framework_expand_dual_tokens(libpstr_builder_t *sb, const char *type_name, const char *t_full, const char *t_full_u, const char *t_base, const char *t_base_u) {
    libpstr_slice_t rep_slice = { .ptr = type_name, .len = strlen(type_name) };
    libpstr_slice_t base_slice = rep_slice;
    
    if (libpstr.slice.ends_with(rep_slice, "_t") == true || libpstr.slice.ends_with(rep_slice, "_T") == true) {
        base_slice.len -= 2;
    }

    libpstr_pstr_t *full_exact = libpstr.pstr.from_slice(rep_slice);
    libpstr_pstr_t *full_upper = libpstr.pstr.from_slice(rep_slice);
    libpstr.pstr.to_uppercase(full_upper);

    libpstr_pstr_t *base_exact = libpstr.pstr.from_slice(base_slice);
    libpstr_pstr_t *base_upper = libpstr.pstr.from_slice(base_slice);
    libpstr.pstr.to_uppercase(base_upper);

    framework_replace_string(sb, t_base_u, base_upper->buf, base_upper->len);
    framework_replace_string(sb, t_base, base_exact->buf, base_exact->len);
    framework_replace_string(sb, t_full_u, full_upper->buf, full_upper->len);
    framework_replace_string(sb, t_full, full_exact->buf, full_exact->len);

    libpstr.pstr.free(full_exact); 
    libpstr.pstr.free(full_upper);
    libpstr.pstr.free(base_exact); 
    libpstr.pstr.free(base_upper);
}

int cgen_app_run_dual(const cgen_app_dual_def_t *app, int argc, char **argv) {
    cgen_opt_t schema[2];
    schema[0] = cgen_opt_new((libpstr_slice_t){.ptr = "=oout-dir", .len = 9});
    schema[1] = cgen_opt_new((libpstr_slice_t){.ptr = "-hhelp", .len = 6});

    cgen_parser_t parser;
    cgen_parser_init(&parser, argc, argv, schema, 2);

    libpstr_pstr_t *out_dir = NULL;
    libpstr_pstr_t *parsed_key = NULL;
    libpstr_pstr_t *parsed_val = NULL;

    cgen_parse_result_t res;
    while ((res = cgen_parser_next(&parser)).kind != CGEN_PARSE_NONE) {
        if (res.kind == CGEN_PARSE_ERR) {
            fprintf(stderr, "Error: %s\n", res.as.error.msg->buf);
            cgen_parse_result_free(res);
            cgen_parser_cleanup(&parser);
            if (out_dir != NULL) libpstr.pstr.free(out_dir);
            if (parsed_key != NULL) libpstr.pstr.free(parsed_key);
            if (parsed_val != NULL) libpstr.pstr.free(parsed_val);
            return 1;
        }
        if (res.kind == CGEN_PARSE_OPTION) {
            if (cgen_opt_short_name(res.as.option.opt) == 'h') {
                printf("Usage: cgen %s [options] <key_type> <value_type>\n\n", app->subcommand_name);
                printf("Options:\n");
                printf("  -h, --help               Show this help menu context\n");
                printf("  -o, --out-dir <dir>      Specify target output directory prefix\n");

                cgen_parse_result_free(res);
                cgen_parser_cleanup(&parser);
                if (out_dir != NULL) libpstr.pstr.free(out_dir);
                if (parsed_key != NULL) libpstr.pstr.free(parsed_key);
                if (parsed_val != NULL) libpstr.pstr.free(parsed_val);
                return 0;
            } else if (cgen_opt_short_name(res.as.option.opt) == 'o') {
                if (res.as.option.arg == NULL) {
                    fprintf(stderr, "Error: Option -o requires an argument.\n");
                    exit(EXIT_FAILURE);
                }
                if (res.as.option.arg->len == 0) {
                    fprintf(stderr, "Error: Output directory cannot be empty.\n");
                    exit(EXIT_FAILURE);
                }

                if (out_dir != NULL) libpstr.pstr.free(out_dir);
                out_dir = res.as.option.arg;
                res.as.option.arg = NULL;
            } 
        } else if (res.kind == CGEN_PARSE_NON_OPTION_ARG) {
            if (parsed_key == NULL) {
                parsed_key = res.as.non_option_arg;
                res.as.non_option_arg = NULL;
            } else if (parsed_val == NULL) {
                parsed_val = res.as.non_option_arg;
                res.as.non_option_arg = NULL;
            }
        }
        cgen_parse_result_free(res);
    }

    if (parsed_key == NULL || parsed_key->len == 0 || parsed_val == NULL || parsed_val->len == 0) {
        fprintf(stderr, "Error: Missing target <key_type> or <value_type> specification.\n");
        cgen_parser_cleanup(&parser);
        if (out_dir != NULL) libpstr.pstr.free(out_dir);
        if (parsed_key != NULL) libpstr.pstr.free(parsed_key);
        if (parsed_val != NULL) libpstr.pstr.free(parsed_val);
        return 1;
    }

    libpstr_pstr_t *base_path = NULL;
    if (out_dir != NULL && out_dir->len > 0) {
        if (out_dir->buf[out_dir->len - 1] == '/') {
            base_path = libpstr.pstr.format("%s", out_dir->buf);
        } else {
            base_path = libpstr.pstr.format("%s/", out_dir->buf);
        }
    } else {
        base_path = libpstr.pstr.from_cstr("");
    }
    
    libpstr_pstr_t *safe_key = normalize_type_name(parsed_key);
    libpstr_pstr_t *safe_val = normalize_type_name(parsed_val);

    // =========================================================================
    // V2 ENGINE: Token Map Generation
    // =========================================================================

    // 1. Generate KEY variants
    libpstr_slice_t k_rep_slice = { .ptr = safe_key->buf, .len = safe_key->len };
    libpstr_slice_t k_base_slice = k_rep_slice;
    if (libpstr.slice.ends_with(k_rep_slice, "_t") == true || libpstr.slice.ends_with(k_rep_slice, "_T") == true) {
        k_base_slice.len -= 2;
    }
    libpstr_pstr_t *k_full_exact = libpstr.pstr.from_slice(k_rep_slice);
    libpstr_pstr_t *k_full_upper = libpstr.pstr.from_slice(k_rep_slice);
    libpstr.pstr.to_uppercase(k_full_upper);
    libpstr_pstr_t *k_base_exact = libpstr.pstr.from_slice(k_base_slice);
    libpstr_pstr_t *k_base_upper = libpstr.pstr.from_slice(k_base_slice);
    libpstr.pstr.to_uppercase(k_base_upper);

    // 2. Generate VAL variants
    libpstr_slice_t v_rep_slice = { .ptr = safe_val->buf, .len = safe_val->len };
    libpstr_slice_t v_base_slice = v_rep_slice;
    if (libpstr.slice.ends_with(v_rep_slice, "_t") == true || libpstr.slice.ends_with(v_rep_slice, "_T") == true) {
        v_base_slice.len -= 2;
    }
    libpstr_pstr_t *v_full_exact = libpstr.pstr.from_slice(v_rep_slice);
    libpstr_pstr_t *v_full_upper = libpstr.pstr.from_slice(v_rep_slice);
    libpstr.pstr.to_uppercase(v_full_upper);
    libpstr_pstr_t *v_base_exact = libpstr.pstr.from_slice(v_base_slice);
    libpstr_pstr_t *v_base_upper = libpstr.pstr.from_slice(v_base_slice);
    libpstr.pstr.to_uppercase(v_base_upper);

    // 3. Map the Tokens
    cgen_token_t tokens[8] = {
        { "KEY",    k_full_exact },
        { "KEY_U",  k_full_upper },
        { "KEY_B",  k_base_exact },
        { "KEY_BU", k_base_upper },
        { "VAL",    v_full_exact },
        { "VAL_U",  v_full_upper },
        { "VAL_B",  v_base_exact },
        { "VAL_BU", v_base_upper }
    };

    // =========================================================================
    // V2 ENGINE: Rendering
    // =========================================================================

    libpstr_pstr_t *path_h = libpstr.pstr.format("%s%s_%s_%s.h", base_path->buf, app->subcommand_name, safe_key->buf, safe_val->buf);
    libpstr_pstr_t *path_c = libpstr.pstr.format("%s%s_%s_%s.c", base_path->buf, app->subcommand_name, safe_key->buf, safe_val->buf);

    // Render Header
    libpstr_builder_t sb_h; 
    libpstr.builder.init(&sb_h);
    libpstr_slice_t tmpl_h = { .ptr = app->template_h, .len = strlen(app->template_h) };
    if (cgen_render_block(&sb_h, tmpl_h, tokens, 8) != 0) {
        fprintf(stderr, "Fatal error rendering header template for dual-type container.\n");
        exit(EXIT_FAILURE);
    }

    // Render Source
    libpstr_builder_t sb_c; 
    libpstr.builder.init(&sb_c);
    if (app->template_c == NULL) {
        fprintf(stderr, "Critical: Missing template_c definition for dual-type container.\n");
        exit(EXIT_FAILURE);
    }
    libpstr_slice_t tmpl_c = { .ptr = app->template_c, .len = strlen(app->template_c) };
    if (cgen_render_block(&sb_c, tmpl_c, tokens, 8) != 0) {
        fprintf(stderr, "Fatal error rendering source template for dual-type container.\n");
        exit(EXIT_FAILURE);
    }

    // Write to Disk
    if (cgen_write_file(path_h->buf, &sb_h) == false) {
        fprintf(stderr, "Error: Failed writing file to destination: %s\n", path_h->buf);
    }
    if (cgen_write_file(path_c->buf, &sb_c) == false) {
        fprintf(stderr, "Error: Failed writing file to destination: %s\n", path_c->buf);
    }

    // =========================================================================
    // Manual Garbage Collection
    // =========================================================================

    libpstr.builder.cleanup(&sb_h); 
    libpstr.builder.cleanup(&sb_c);
    libpstr.pstr.free(path_h); 
    libpstr.pstr.free(path_c);
    libpstr.pstr.free(base_path);
    libpstr.pstr.free(safe_key);
    libpstr.pstr.free(safe_val);
    libpstr.pstr.free(parsed_key);
    libpstr.pstr.free(parsed_val);
    if (out_dir != NULL) libpstr.pstr.free(out_dir);

    // Free the 8 token mapped strings!
    libpstr.pstr.free(k_full_exact);
    libpstr.pstr.free(k_full_upper);
    libpstr.pstr.free(k_base_exact);
    libpstr.pstr.free(k_base_upper);
    libpstr.pstr.free(v_full_exact);
    libpstr.pstr.free(v_full_upper);
    libpstr.pstr.free(v_base_exact);
    libpstr.pstr.free(v_base_upper);

    cgen_parser_cleanup(&parser);

    return 0;
}
