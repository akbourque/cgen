#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "opt.h"
#include "parser.h"
#include "libpstr.h"

static void test_opt_validation(void) {
    libpstr_slice_t attr = { .ptr = "-nn_a-me", .len = 8 };
    libpstr_pstr_t *err = NULL;
    
    // Test valid layout
    assert(cgen_opt_check(attr, &err) == true);
    cgen_opt_t p = cgen_opt_new(attr);
    assert(cgen_opt_arg_type(p) == '-');
    assert(cgen_opt_short_name(p) == 'n');
    
    libpstr_slice_t ln = cgen_opt_long_name(p);
    assert(ln.len == 6 && memcmp(ln.ptr, "n_a-me", 6) == 0);

    // Test failure states
    assert(cgen_opt_check((libpstr_slice_t){.ptr="-", .len=1}, &err) == false);     libpstr.pstr.free(err); err = NULL;
    assert(cgen_opt_check((libpstr_slice_t){.ptr="+n", .len=2}, &err) == false);    libpstr.pstr.free(err); err = NULL;
    assert(cgen_opt_check((libpstr_slice_t){.ptr="- ", .len=2}, &err) == false);    libpstr.pstr.free(err); err = NULL;
    assert(cgen_opt_check((libpstr_slice_t){.ptr="-nx", .len=3}, &err) == false);   libpstr.pstr.free(err); err = NULL;
    assert(cgen_opt_check((libpstr_slice_t){.ptr="-n2n", .len=4}, &err) == false);  libpstr.pstr.free(err); err = NULL;
    assert(cgen_opt_check((libpstr_slice_t){.ptr="-nn%", .len=4}, &err) == false);  libpstr.pstr.free(err); err = NULL;
    
    printf(" -> test_opt_validation passed.\n");
}

static void test_parse_short_clustering(void) {
    cgen_opt_t opts[4] = {
        cgen_opt_new((libpstr_slice_t){.ptr="-e", .len=2}),
        cgen_opt_new((libpstr_slice_t){.ptr="-f", .len=2}),
        cgen_opt_new((libpstr_slice_t){.ptr="?g", .len=2}),
        cgen_opt_new((libpstr_slice_t){.ptr="=h", .len=2})
    };
    char *args[] = {"-fe", "-g", "-garg", "-harg", "-h", "arg"};
    cgen_parser_t p;
    cgen_parser_init(&p, 6, args, opts, 4);

    cgen_parse_result_t res;

    // 1. Extract clustered '-f'
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(cgen_opt_short_name(res.as.option.opt) == 'f');
    assert(res.as.option.arg == NULL);
    cgen_parse_result_free(res);

    // 2. Extract clustered '-e'
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(cgen_opt_short_name(res.as.option.opt) == 'e');
    assert(res.as.option.arg == NULL);
    cgen_parse_result_free(res);

    // 3. Optional arg missing ('-g')
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(cgen_opt_short_name(res.as.option.opt) == 'g');
    assert(res.as.option.arg == NULL);
    cgen_parse_result_free(res);

    // 4. Optional arg attached ('-garg')
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(strcmp(res.as.option.arg->buf, "arg") == 0);
    cgen_parse_result_free(res);

    // 5. Required arg attached ('-harg')
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(strcmp(res.as.option.arg->buf, "arg") == 0);
    cgen_parse_result_free(res);

    // 6. Required arg detached ('-h', 'arg')
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(strcmp(res.as.option.arg->buf, "arg") == 0);
    cgen_parse_result_free(res);

    // End of stream
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_NONE);

    cgen_parser_cleanup(&p);
    printf(" -> test_parse_short_clustering passed.\n");
}

static void test_parse_long_arguments(void) {
    cgen_opt_t opts[4] = {
        cgen_opt_new((libpstr_slice_t){.ptr="--ee", .len=4}),
        cgen_opt_new((libpstr_slice_t){.ptr="--ff", .len=4}),
        cgen_opt_new((libpstr_slice_t){.ptr="?-gg", .len=4}),
        cgen_opt_new((libpstr_slice_t){.ptr="=-hh", .len=4})
    };
    char *args[] = {"--ff", "--ee", "--gg", "--gg=\"arg\"", "--hh=arg", "--hh", "arg"};
    cgen_parser_t p;
    cgen_parser_init(&p, 7, args, opts, 4);

    cgen_parse_result_t res;

    // --ff
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(res.as.option.arg == NULL);
    cgen_parse_result_free(res);

    // --ee
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    cgen_parse_result_free(res);

    // --gg
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(res.as.option.arg == NULL);
    cgen_parse_result_free(res);

    // --gg="arg" (Stripping verification)
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(strcmp(res.as.option.arg->buf, "arg") == 0);
    cgen_parse_result_free(res);

    // --hh=arg
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(strcmp(res.as.option.arg->buf, "arg") == 0);
    cgen_parse_result_free(res);

    // --hh arg
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_OPTION);
    assert(strcmp(res.as.option.arg->buf, "arg") == 0);
    cgen_parse_result_free(res);

    cgen_parser_cleanup(&p);
    printf(" -> test_parse_long_arguments passed.\n");
}

static void test_parser_errors(void) {
    cgen_opt_t opts[4] = {
        cgen_opt_new((libpstr_slice_t){.ptr="-fflag", .len=6}),
        cgen_opt_new((libpstr_slice_t){.ptr="-ggit", .len=5}),
        cgen_opt_new((libpstr_slice_t){.ptr="?mmy-opt", .len=8}),
        cgen_opt_new((libpstr_slice_t){.ptr="=nname", .len=6})
    };
    char *args[] = {"-xg", "--unknown", "--name"};
    cgen_parser_t p;
    cgen_parser_init(&p, 3, args, opts, 4);

    cgen_parse_result_t res;

    // Unknown short flag within a cluster
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_ERR);
    assert(res.as.error.kind == CGEN_ERR_UNKNOWN_SHORT);
    cgen_parse_result_free(res);

    // Unknown explicit long option
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_ERR);
    assert(res.as.error.kind == CGEN_ERR_UNKNOWN_LONG);
    cgen_parse_result_free(res);

    // Missing required tail argument
    res = cgen_parser_next(&p);
    assert(res.kind == CGEN_PARSE_ERR);
    assert(res.as.error.kind == CGEN_ERR_MISSING_ARG);
    cgen_parse_result_free(res);

    cgen_parser_cleanup(&p);
    printf(" -> test_parser_errors passed.\n");
}

void run_parser_tests(void) {
    
    test_opt_validation();
    test_parse_short_clustering();
    test_parse_long_arguments();
    test_parser_errors();
}
