#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cgen_framework.h"
#include "opt.h"
#include "parser.h"

static void framework_replace_tokens(pstr_builder_t *sb, pstr_t *replacement) {
    // Determine the base length by stripping a trailing '_t' or '_T' suffix
    size_t base_len = replacement->len;
    if (replacement->len > 2 && 
        replacement->buf[replacement->len - 2] == '_' && 
        (replacement->buf[replacement->len - 1] == 't' || replacement->buf[replacement->len - 1] == 'T')) {
        base_len = replacement->len - 2;
    }

    // Allocate exact and screaming uppercase full variants
    pstr_t *full_exact = pstr_format("%s", replacement->buf);
    pstr_t *full_upper = pstr_format("%s", replacement->buf);
    for (size_t i = 0; i < full_upper->len; i++) {
        full_upper->buf[i] = toupper((unsigned char)full_upper->buf[i]);
    }

    // Allocate exact and screaming uppercase base variants
    pstr_t *base_exact = pstr_format("%.*s", (int)base_len, replacement->buf);
    pstr_t *base_upper = pstr_format("%.*s", (int)base_len, replacement->buf);
    for (size_t i = 0; i < base_upper->len; i++) {
        base_upper->buf[i] = toupper((unsigned char)base_upper->buf[i]);
    }

    // Scan and replace tokens in order of narrowing specificity
    while (1) {
        pstr_slice_t match = pstr.builder.find_cstr(sb, "{{00BU}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        pstr.builder.replace_range(sb, offset, match.len, base_upper->buf, base_upper->len);
    }

    while (1) {
        pstr_slice_t match = pstr.builder.find_cstr(sb, "{{00B}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        pstr.builder.replace_range(sb, offset, match.len, base_exact->buf, base_exact->len);
    }

    while (1) {
        pstr_slice_t match = pstr.builder.find_cstr(sb, "{{00U}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        pstr.builder.replace_range(sb, offset, match.len, full_upper->buf, full_upper->len);
    }

    while (1) {
        pstr_slice_t match = pstr.builder.find_cstr(sb, "{{00}}");
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        pstr.builder.replace_range(sb, offset, match.len, full_exact->buf, full_exact->len);
    }

    pstr.free(full_exact);
    pstr.free(full_upper);
    pstr.free(base_exact);
    pstr.free(base_upper);
}

static bool write_file_clobber(const char *path, pstr_builder_t *sb) {
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

int cgen_app_run(const cgen_app_def_t *app, int argc, char **argv) {
    cgen_opt_t schema[3];
    schema[0] = cgen_opt_new((pstr_slice_t){.ptr = (char*)app->opt_spec, .len = strlen(app->opt_spec)});
    schema[1] = cgen_opt_new((pstr_slice_t){.ptr = "=oout-dir", .len = 9});
    schema[2] = cgen_opt_new((pstr_slice_t){.ptr = "-hhelp", .len = 6}); // Registered global help context

    cgen_parser_t parser;
    cgen_parser_init(&parser, argc, argv, schema, 3);

    pstr_t *parsed_option_0 = NULL;
    pstr_t *parsed_argument_0 = NULL;
    pstr_t *out_dir = NULL;

    cgen_parse_result_t res;
    while ((res = cgen_parser_next(&parser)).kind != CGEN_PARSE_NONE) {
        if (res.kind == CGEN_PARSE_ERR) {
            fprintf(stderr, "Error: %s\n", res.as.error.msg->buf);
            cgen_parse_result_free(res);
            cgen_parser_cleanup(&parser);
            if (parsed_option_0 == NULL) pstr.free(parsed_option_0);
            if (parsed_argument_0 == NULL) pstr.free(parsed_argument_0);
            if (out_dir == NULL) pstr.free(out_dir);
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
                if (parsed_option_0 != NULL) pstr.free(parsed_option_0);
                if (parsed_argument_0 != NULL) pstr.free(parsed_argument_0);
                if (out_dir != NULL) pstr.free(out_dir);
                return 0;
            }
            else if (cgen_opt_short_name(res.as.option.opt) == 'o') {
                if (out_dir != NULL) pstr.free(out_dir);
                out_dir = res.as.option.arg;
                res.as.option.arg = NULL;
            } else {
                if (parsed_option_0 != NULL) pstr.free(parsed_option_0);
                parsed_option_0 = res.as.option.arg;
                res.as.option.arg = NULL;
            }
        } 
        else if (res.kind == CGEN_PARSE_NON_OPTION_ARG) {
            if (parsed_argument_0 == NULL) {
                parsed_argument_0 = res.as.non_option_arg;
                res.as.non_option_arg = NULL;
            }
        }
        cgen_parse_result_free(res);
    }

    pstr_t *replacement = parsed_option_0;
    if (replacement == NULL || replacement->len == 0) {
        replacement = parsed_argument_0;
    }

    if (replacement == NULL || replacement->len == 0) {
        fprintf(stderr, "Error: Missing target type name specification.\n");
        cgen_parser_cleanup(&parser);
        if (out_dir != NULL) pstr.free(out_dir);
        return 1;
    }

    pstr_t *base_path = NULL;
    if (out_dir != NULL && out_dir->len > 0) {
        if (out_dir->buf[out_dir->len - 1] == '/') {
            base_path = pstr_format("%s", out_dir->buf);
        } else {
            base_path = pstr_format("%s/", out_dir->buf);
        }
    } else {
        base_path = pstr_from_cstr("");
    }

    size_t file_base_len = replacement->len;
    if (replacement->len > 2 && 
        replacement->buf[replacement->len - 2] == '_' && 
        (replacement->buf[replacement->len - 1] == 't' || replacement->buf[replacement->len - 1] == 'T')) {
        file_base_len = replacement->len - 2;
    }

    // Generate Header asset using base naming structures
    pstr_builder_t sb_h;
    pstr.builder.init(&sb_h);
    pstr.builder.append_cstr(&sb_h, app->template_h);
    framework_replace_tokens(&sb_h, replacement);
    pstr_t *path_h = pstr_format("%s%s_%.*s.h", base_path->buf, app->subcommand_name, (int)file_base_len, replacement->buf);

    // Generate Source Implementation asset using base naming structures
    pstr_builder_t sb_c;
    pstr.builder.init(&sb_c);
    pstr.builder.append_cstr(&sb_c, app->template_c);
    framework_replace_tokens(&sb_c, replacement);
    pstr_t *path_c = pstr_format("%s%s_%.*s.c", base_path->buf, app->subcommand_name, (int)file_base_len, replacement->buf);

    if (write_file_clobber(path_h->buf, &sb_h) == false) {
        fprintf(stderr, "Error: Failed to write file to disk at destination: %s\n", path_h->buf);
    }
    if (write_file_clobber(path_c->buf, &sb_c) == false) {
        fprintf(stderr, "Error: Failed to write file to disk at destination: %s\n", path_c->buf);
    }

    pstr.builder.cleanup(&sb_h);
    pstr.builder.cleanup(&sb_c);
    pstr.free(path_h);
    pstr.free(path_c);
    pstr.free(base_path);
    if (parsed_option_0 != NULL) pstr.free(parsed_option_0);
    if (parsed_argument_0 != NULL) pstr.free(parsed_argument_0);
    if (out_dir != NULL) pstr.free(out_dir);
    cgen_parser_cleanup(&parser);

    return 0;
}

static void framework_replace_string(pstr_builder_t *sb, const char *token, const char *rep, size_t rep_len) {
    while (1) {
        pstr_slice_t match = pstr.builder.find_cstr(sb, token);
        if (match.ptr == NULL) break;
        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        pstr.builder.replace_range(sb, offset, match.len, rep, rep_len);
    }
}

static void framework_expand_dual_tokens(pstr_builder_t *sb, const char *type_name, const char *t_full, const char *t_full_u, const char *t_base, const char *t_base_u) {
    size_t len = strlen(type_name);
    size_t base_len = len;
    if (len > 2 && type_name[len - 2] == '_' && (type_name[len - 1] == 't' || type_name[len - 1] == 'T')) {
        base_len = len - 2;
    }

    pstr_t *full_exact = pstr_format("%s", type_name);
    pstr_t *full_upper = pstr_format("%s", type_name);
    for (size_t i = 0; i < full_upper->len; i++) full_upper->buf[i] = toupper((unsigned char)full_upper->buf[i]);

    pstr_t *base_exact = pstr_format("%.*s", (int)base_len, type_name);
    pstr_t *base_upper = pstr_format("%.*s", (int)base_len, type_name);
    for (size_t i = 0; i < base_upper->len; i++) base_upper->buf[i] = toupper((unsigned char)base_upper->buf[i]);

    framework_replace_string(sb, t_base_u, base_upper->buf, base_upper->len);
    framework_replace_string(sb, t_base, base_exact->buf, base_exact->len);
    framework_replace_string(sb, t_full_u, full_upper->buf, full_upper->len);
    framework_replace_string(sb, t_full, full_exact->buf, full_exact->len);

    pstr.free(full_exact); pstr.free(full_upper);
    pstr.free(base_exact); pstr.free(base_upper);
}

int cgen_app_run_dual(const cgen_app_dual_def_t *app, int argc, char **argv) {
    if (argc < 3 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        printf("Usage: cgen %s <key_type> <value_type>\n", app->subcommand_name);
        return 0;
    }

    const char *key_type = argv[1];
    const char *val_type = argv[2];

    size_t k_len = strlen(key_type);
    if (k_len > 2 && key_type[k_len - 2] == '_' && (key_type[k_len - 1] == 't' || key_type[k_len - 1] == 'T')) k_len -= 2;
    size_t v_len = strlen(val_type);
    if (v_len > 2 && val_type[v_len - 2] == '_' && (val_type[v_len - 1] == 't' || val_type[v_len - 1] == 'T')) v_len -= 2;

    pstr_t *path_h = pstr_format("%s_%.*s_%.*s.h", app->subcommand_name, (int)k_len, key_type, (int)v_len, val_type);
    pstr_t *path_c = pstr_format("%s_%.*s_%.*s.c", app->subcommand_name, (int)k_len, key_type, (int)v_len, val_type);

    // Generate Header output structures
    pstr_builder_t sb_h; pstr.builder.init(&sb_h);
    pstr.builder.append_cstr(&sb_h, app->template_h);
    framework_expand_dual_tokens(&sb_h, key_type, "{{KEY}}", "{{KEY_U}}", "{{KEY_B}}", "{{KEY_BU}}");
    framework_expand_dual_tokens(&sb_h, val_type, "{{VAL}}", "{{VAL_U}}", "{{VAL_B}}", "{{VAL_BU}}");

    // Generate Source output structures
    pstr_builder_t sb_c; pstr.builder.init(&sb_c);
    pstr.builder.append_cstr(&sb_c, app->template_c);
    framework_expand_dual_tokens(&sb_c, key_type, "{{KEY}}", "{{KEY_U}}", "{{KEY_B}}", "{{KEY_BU}}");
    framework_expand_dual_tokens(&sb_c, val_type, "{{VAL}}", "{{VAL_U}}", "{{VAL_B}}", "{{VAL_BU}}");

    if (write_file_clobber(path_h->buf, &sb_h) == false) {
        fprintf(stderr, "Error: Failed writing %s\n", path_h->buf);
    }
    if (write_file_clobber(path_c->buf, &sb_c) == false) {
        fprintf(stderr, "Error: Failed writing %s\n", path_c->buf);
    }

    pstr.builder.cleanup(&sb_h); pstr.builder.cleanup(&sb_c);
    pstr.free(path_h); pstr.free(path_c);
    return 0;
}
