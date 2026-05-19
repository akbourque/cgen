#include <assert.h>
#include <stdio.h>
#include "libpstr.h" // defines pstr_builder_t
#include "panic.h"   // defines ASSERT_PANIC
#include "sample_t.h"// deInes sample_t

void test_variant_happy_path(void) {
    // 1. Test basic primitive initialization & inspection
    sample_t v_high = sample_new_high(100);
    assert(sample_is_high(&v_high));
    assert(!sample_is_low(&v_high));
    assert(sample_unwrap_high(&v_high) == 100);

    // 2. Test custom struct (pstr_builder_t) tracking
    // Assumes pstr_builder_t has a default init helper or mock instance
    pstr_builder_t builder; 
    memset(&builder, 0, sizeof(pstr_builder_t)); 
    
    sample_t v_text = sample_new_text(builder);
    assert(sample_is_text(&v_text));
    assert(!sample_is_high(&v_text));
    
    printf("  ✅ Variant happy paths passed successfully.\n");
}

void test_variant_panic_boundaries(void) {
    sample_t v_high = sample_new_high(42);

    // Verify that attempting to extract a 'low' from a 'high' container
    // safely invokes your library's panic logic instead of giving raw bits
    ASSERT_PANIC({
        sample_unwrap_low(&v_high);
    });

    // Verify that attempting to extract 'text' from a 'high' container
    // also triggers a controlled panic violation
    ASSERT_PANIC({
        sample_unwrap_text(&v_high);
    });

    printf("  ✅ Variant exceptional panic boundaries verified safely.\n");
}

void run_variant_tests(void) {
    printf("🧪 Running cgen-variant Test Suite...\n");
    
    test_variant_happy_path();
    test_variant_panic_boundaries();
    
    printf("✨ All variant integration tests passed cleanly!\n");
}
