#include "opt.h"
#include <ctype.h>

bool cgen_opt_check(libpstr_slice_t attr, libpstr_pstr_t **out_err_msg) {
    if (attr.ptr == NULL || attr.len < 2) {
        if (out_err_msg != NULL) *out_err_msg = libpstr.pstr.from_cstr("Opt must have at least 2 chars");
        return false;
    }

    // 1. Check Arg Type
    char type = attr.ptr[0];
    if (type != CGEN_NO_ARG && type != CGEN_OPTIONAL_ARG && type != CGEN_REQUIRED_ARG) {
        if (out_err_msg != NULL) *out_err_msg = libpstr.pstr.from_cstr("first char must be '-', '?' or '='");
        return false;
    }

    // 2. Check Short Name (Graphic char constraint)
    unsigned char short_name = (unsigned char)attr.ptr[1];
    if (short_name < 0x21 || short_name > 0x7E) {
        if (out_err_msg != NULL) *out_err_msg = libpstr.pstr.from_cstr("short name must be a graphic char (0x21-0x7E)");
        return false;
    }

    if (attr.len == 2) return true;
    if (attr.len == 3) {
        if (out_err_msg != NULL) *out_err_msg = libpstr.pstr.from_cstr("long name must have 2 or more chars");
        return false;
    }

    // 3. Check Long Name Constraints
    if (isdigit((unsigned char)attr.ptr[2])) {
        if (out_err_msg != NULL) *out_err_msg = libpstr.pstr.from_cstr("first char in long name cannot be digit");
        return false;
    }

    for (size_t i = 2; i < attr.len; i++) {
        char ch = attr.ptr[i];
        bool valid = isalnum((unsigned char)ch) || ch == '_' || ch == '-';
        if (valid == false) {
            if (out_err_msg != NULL) *out_err_msg = libpstr.pstr.from_cstr("only alphanumeric, '_' or '-' chars in long name");
            return false;
        }
    }

    return true;
}

cgen_opt_t cgen_opt_new(libpstr_slice_t attr) {
    libpstr_pstr_t *err = NULL;
    if (cgen_opt_check(attr, &err) == false) {
        PANIC(err != NULL ? err->buf : "Invalid Opt specification format");
    }
    return (cgen_opt_t){ .raw = attr };
}

char cgen_opt_arg_type(cgen_opt_t opt) {
    return opt.raw.ptr[0];
}

char cgen_opt_short_name(cgen_opt_t opt) {
    return opt.raw.ptr[1];
}

libpstr_slice_t cgen_opt_long_name(cgen_opt_t opt) {
    if (opt.raw.len <= 2) return (libpstr_slice_t){ .ptr = NULL, .len = 0 };
    return (libpstr_slice_t){ .ptr = opt.raw.ptr + 2, .len = opt.raw.len - 2 };
}
