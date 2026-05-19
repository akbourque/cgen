#include "parser.h"

void cgen_parser_init(cgen_parser_t *p, int argc, char **argv, cgen_opt_t *opts, size_t opts_len) {
    pstr.list.init(&p->args);
    // Populate slice storage with incoming system argument array contexts
    for (int i = 0; i < argc; i++) {
        pstr_slice_t slice = { .ptr = argv[i], .len = strlen(argv[i]) };
        pstr.list.push(&p->args, slice);
    }
    p->arg_idx = 0;
    p->short_idx = 0;
    p->current_arg = NULL;
    p->opts = opts;
    p->opts_len = opts_len;
}

void cgen_parser_cleanup(cgen_parser_t *p) {
    pstr.list.free(&p->args);
    if (p->current_arg) {
        pstr.free(p->current_arg);
        p->current_arg = NULL;
    }
}

void cgen_parse_result_free(cgen_parse_result_t res) {
    if (res.kind == CGEN_PARSE_NON_OPTION_ARG && res.as.non_option_arg) {
        pstr.free(res.as.non_option_arg);
    } else if (res.kind == CGEN_PARSE_OPTION && res.as.option.arg) {
        pstr.free(res.as.option.arg);
    } else if (res.kind == CGEN_PARSE_ERR && res.as.error.msg) {
        pstr.free(res.as.error.msg);
    }
}

static pstr_t* remove_quotes(pstr_t *arg) {
    if (!arg || arg->len < 2) return arg;
    if (arg->buf[0] == '"' && arg->buf[arg->len - 1] == '"') {
        pstr_slice_t slice = { .ptr = arg->buf + 1, .len = arg->len - 2 };
        pstr_t *clean = pstr.from_slice(slice);
        pstr.free(arg);
        return clean;
    }
    return arg;
}

static bool get_by_short(cgen_parser_t *p, char name, cgen_opt_t *out_opt) {
    for (size_t i = 0; i < p->opts_len; i++) {
        if (cgen_opt_short_name(p->opts[i]) == name) {
            *out_opt = p->opts[i];
            return true;
        }
    }
    return false;
}

static bool get_by_long(cgen_parser_t *p, pstr_slice_t name, cgen_opt_t *out_opt) {
    for (size_t i = 0; i < p->opts_len; i++) {
        pstr_slice_t long_name = cgen_opt_long_name(p->opts[i]);
        if (long_name.len == 0) continue;

        if (name.len >= long_name.len && memcmp(name.ptr, long_name.ptr, long_name.len) == 0) {
            if (name.len == long_name.len) {
                *out_opt = p->opts[i];
                return true;
            }
            if (name.ptr[long_name.len] == '=') {
                *out_opt = p->opts[i];
                return true;
            }
        }
    }
    return false;
}

static cgen_parse_result_t parse_long(cgen_parser_t *p, pstr_t *arg_pstr) {
    cgen_parse_result_t res = {0};
    pstr_slice_t long_part = { .ptr = arg_pstr->buf + 2, .len = arg_pstr->len - 2 };
    cgen_opt_t opt;

    if (get_by_long(p, long_part, &opt)) {
        char type = cgen_opt_arg_type(opt);
        if (type == CGEN_NO_ARG) {
            pstr.free(arg_pstr);
            res.kind = CGEN_PARSE_OPTION;
            res.as.option.opt = opt;
            res.as.option.arg = NULL;
            return res;
        }

        pstr_slice_t long_name = cgen_opt_long_name(opt);
        size_t eq_idx = long_name.len + 2;
        
        // Attached long argument configuration check (e.g., --param=value)
        if (eq_idx < arg_pstr->len && arg_pstr->buf[eq_idx] == '=') {
            pstr_slice_t val_slice = { .ptr = arg_pstr->buf + eq_idx + 1, .len = arg_pstr->len - eq_idx - 1 };
            pstr_t *val = pstr.from_slice(val_slice);
            pstr.free(arg_pstr);

            res.kind = CGEN_PARSE_OPTION;
            res.as.option.opt = opt;
            res.as.option.arg = remove_quotes(val);
            return res;
        }

        if (type == CGEN_OPTIONAL_ARG) {
            pstr.free(arg_pstr);
            res.kind = CGEN_PARSE_OPTION;
            res.as.option.opt = opt;
            res.as.option.arg = NULL;
            return res;
        }

        if (type == CGEN_REQUIRED_ARG) {
            size_t total_args = pstr_list_len(&p->args);
            if (p->arg_idx < total_args) {
                pstr_slice_t next_slice = pstr.list.get(&p->args, p->arg_idx++);
                pstr_t *next_val = pstr.from_slice(next_slice);
                pstr.free(arg_pstr);

                res.kind = CGEN_PARSE_OPTION;
                res.as.option.opt = opt;
                res.as.option.arg = remove_quotes(next_val);
                return res;
            } else {
                res.kind = CGEN_PARSE_ERR;
                res.as.error.kind = CGEN_ERR_MISSING_ARG;
                res.as.error.msg = pstr_format("option '%s' requires an argument", arg_pstr->buf);
                pstr.free(arg_pstr);
                return res;
            }
        }
    }

    res.kind = CGEN_PARSE_ERR;
    res.as.error.kind = CGEN_ERR_UNKNOWN_LONG;
    res.as.error.msg = pstr_format("unknown long option '%s'", arg_pstr->buf);
    pstr.free(arg_pstr);
    return res;
}

static cgen_parse_result_t parse_short(cgen_parser_t *p) {
    cgen_parse_result_t res = {0};
    size_t inx = p->short_idx;
    p->short_idx++;

    if (p->short_idx == p->current_arg->len) {
        p->short_idx = 0;
    }

    char ch = p->current_arg->buf[inx];
    cgen_opt_t opt;
    if (get_by_short(p, ch, &opt)) {
        if (cgen_opt_arg_type(opt) == CGEN_NO_ARG) {
            res.kind = CGEN_PARSE_OPTION;
            res.as.option.opt = opt;
            res.as.option.arg = NULL;
            if (p->short_idx == 0) {
                pstr.free(p->current_arg);
                p->current_arg = NULL;
            }
            return res;
        }

        pstr_t *optarg = NULL;
        // Attached short argument design extraction (e.g., -hvalue)
        if (inx + 1 < p->current_arg->len) {
            pstr_slice_t val_slice = { .ptr = p->current_arg->buf + inx + 1, .len = p->current_arg->len - (inx + 1) };
            optarg = pstr.from_slice(val_slice);
            p->short_idx = 0;
        } else if (cgen_opt_arg_type(opt) == CGEN_REQUIRED_ARG) {
            size_t total_args = pstr_list_len(&p->args);
            if (p->arg_idx < total_args) {
                pstr_slice_t next_slice = pstr.list.get(&p->args, p->arg_idx++);
                optarg = pstr.from_slice(next_slice);
                p->short_idx = 0;
            } else {
                res.kind = CGEN_PARSE_ERR;
                res.as.error.kind = CGEN_ERR_MISSING_ARG;
                res.as.error.msg = pstr_format("option '-%c' requires an argument", ch);
                pstr.free(p->current_arg);
                p->current_arg = NULL;
                return res;
            }
        }

        res.kind = CGEN_PARSE_OPTION;
        res.as.option.opt = opt;
        res.as.option.arg = optarg ? remove_quotes(optarg) : NULL;

        pstr.free(p->current_arg);
        p->current_arg = NULL;
        return res;
    }

    res.kind = CGEN_PARSE_ERR;
    res.as.error.kind = CGEN_ERR_UNKNOWN_SHORT;
    res.as.error.msg = pstr_format("unknown short option '-%c' in cluster '%s'", ch, p->current_arg->buf);
    pstr.free(p->current_arg);
    p->current_arg = NULL;
    p->short_idx = 0;
    return res;
}

cgen_parse_result_t cgen_parser_next(cgen_parser_t *p) {
    cgen_parse_result_t res = {0};
    if (p->short_idx > 0) {
        return parse_short(p);
    }

    size_t total_args = pstr_list_len(&p->args);
    if (p->arg_idx < total_args) {
        pstr_slice_t slice = pstr.list.get(&p->args, p->arg_idx++);
        pstr_t *arg_pstr = pstr.from_slice(slice);

        if (pstr_starts_with(arg_pstr, "--") && arg_pstr->len > 2) {
            return parse_long(p, arg_pstr);
        } else if (pstr_starts_with(arg_pstr, "-") && arg_pstr->len > 1) {
            p->current_arg = arg_pstr;
            p->short_idx = 1;
            return parse_short(p);
        } else {
            res.kind = CGEN_PARSE_NON_OPTION_ARG;
            res.as.non_option_arg = arg_pstr;
            return res;
        }
    }

    res.kind = CGEN_PARSE_NONE;
    return res;
}
