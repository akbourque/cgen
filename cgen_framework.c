#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cgen_framework.h"
#include "opt.h"
#include "parser.h"
#include "vendor/libpstr.h"

static void framework_replace_tokens(pstr_builder_t *sb, pstr_t *replacement) {
    pstr_t *upper_rep = pstr_format("%s", replacement->buf);
    for (size_t i = 0; i < upper_rep->len; i++) {
        upper_rep->buf[i] = toupper((unsigned char)upper_rep->buf[i]);
    }

    // Scan and replace the ultra-short uppercase tokens
    while (1) {
        pstr_slice_t match = pstr.builder.find_cstr(sb, "{{00U}}"); // Changed here!
        if (match.ptr == NULL) break;

        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        pstr.builder.replace_range(sb, offset, match.len, upper_rep->buf, upper_rep->len);
    }

    // Scan and replace exact-case tokens
    while (1) {
        pstr_slice_t match = pstr.builder.find_cstr(sb, "{{00}}");
        if (match.ptr == NULL) break;

        size_t offset = (size_t)(match.ptr - (char *)sb->vec.data);
        pstr.builder.replace_range(sb, offset, match.len, replacement->buf, replacement->len);
    }

    pstr.free(upper_rep);
}

static bool write_file_clobber(const char *path, pstr_builder_t *sb) {
    // "w" aggressively truncates any existing file automatically (The Linux Way)
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
    cgen_opt_t schema[2];
    schema[0] = cgen_opt_new((pstr_slice_t){.ptr = (char*)app->opt_spec, .len = strlen(app->opt_spec)});
    schema[1] = cgen_opt_new((pstr_slice_t){.ptr = "?oout-dir", .len = 9}); // Global directory override flag

    cgen_parser_t parser;
    cgen_parser_init(&parser, argc, argv, schema, 2);

    pstr_t *parsed_option_0 = NULL;
    pstr_t *parsed_argument_0 = NULL;
    pstr_t *out_dir = NULL;

    cgen_parse_result_t res;
    while ((res = cgen_parser_next(&parser)).kind != CGEN_PARSE_NONE) {
        if (res.kind == CGEN_PARSE_ERR) {
            fprintf(stderr, "Error: %s\n", res.as.error.msg->buf);
            cgen_parse_result_free(res);
            cgen_parser_cleanup(&parser);
            if (parsed_option_0) pstr.free(parsed_option_0);
            if (parsed_argument_0) pstr.free(parsed_argument_0);
            if (out_dir) pstr.free(out_dir);
            return 1;
        }

        if (res.kind == CGEN_PARSE_OPTION) {
            if (cgen_opt_short_name(res.as.option.opt) == 'o') {
                if (out_dir) pstr.free(out_dir);
                out_dir = res.as.option.arg;
                res.as.option.arg = NULL;
            } else {
                if (parsed_option_0) pstr.free(parsed_option_0);
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

    // Enforce target type name existence check
    pstr_t *replacement = parsed_option_0;
    if (replacement == NULL || replacement->len == 0) {
        replacement = parsed_argument_0;
    }

    if (replacement == NULL || replacement->len == 0) {
        fprintf(stderr, "Error: Missing target type name specification.\n");
        cgen_parser_cleanup(&parser);
        if (out_dir) pstr.free(out_dir);
        return 1;
    }

    // Standardize the output path base directory prefix string using pstr_format
    pstr_t *base_path = NULL;
    if (out_dir != NULL && out_dir->len > 0) {
        // If the user forgot the trailing slash, add it natively via formatting
        if (out_dir->buf[out_dir->len - 1] == '/') {
            base_path = pstr_format("%s", out_dir->buf); //
        } else {
            base_path = pstr_format("%s/", out_dir->buf); //
        }
    } else {
        base_path = pstr_from_cstr("");
    }

    // Generate Header asset
    pstr_builder_t sb_h;
    pstr.builder.init(&sb_h);
    pstr.builder.append_cstr(&sb_h, app->template_h);
    framework_replace_tokens(&sb_h, replacement);
    pstr_t *path_h = pstr_format("%s%s_%s.h", base_path->buf, app->subcommand_name, replacement->buf);

    // Generate Source Implementation asset
    pstr_builder_t sb_c;
    pstr.builder.init(&sb_c);
    pstr.builder.append_cstr(&sb_c, app->template_c);
    framework_replace_tokens(&sb_c, replacement);
    pstr_t *path_c = pstr_format("%s%s_%s.c", base_path->buf, app->subcommand_name, replacement->buf);

    // Clobber files to disk quietly
    if (write_file_clobber(path_h->buf, &sb_h) == false) {
        fprintf(stderr, "Error: Failed to write file to disk at target destination path: %s\n", path_h->buf);
    }
    if (write_file_clobber(path_c->buf, &sb_c) == false) {
        fprintf(stderr, "Error: Failed to write file to disk at target destination path: %s\n", path_c->buf);
    }

    // Absolute memory deallocation sanity loop
    pstr.builder.cleanup(&sb_h);
    pstr.builder.cleanup(&sb_c);
    pstr.free(path_h);
    pstr.free(path_c);
    pstr.free(base_path);
    if (parsed_option_0) pstr.free(parsed_option_0);
    if (parsed_argument_0) pstr.free(parsed_argument_0);
    if (out_dir) pstr.free(out_dir);
    cgen_parser_cleanup(&parser);

    return 0;
}
