#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "libpstr.h"
#include "../../libpstr/src/panic.h"

// Example auto-generated variant
#include "sample_t.h"

void run_variant_tests(void) {
    printf("  [Variant] Running sample_t lifecycle tests...\n");

    // --- 1. Test the 'high' variant (int) ---
    sample_t v_high = sample_new_high(42);
    assert(sample_is_high(&v_high) == true);
    assert(sample_is_low(&v_high) == false);
    assert(sample_is_text(&v_high) == false);
    assert(sample_unwrap_high(&v_high) == 42);
    // Verify panic on invalid unwrap
    ASSERT_PANIC({sample_unwrap_low(&v_high);});

    // --- 2. Test the 'low' variant (int) ---
    sample_t v_low = sample_new_low(-999);
    assert(sample_is_low(&v_low) == true);
    assert(sample_is_high(&v_low) == false);
    assert(sample_is_text(&v_low) == false);
    assert(sample_unwrap_low(&v_low) == -999);

    // --- 3. Test the 'text' variant (libpstr_builder_t) ---
    libpstr_builder_t sb;
    libpstr.builder.init(&sb);
    libpstr.builder.append_cstr(&sb, "Variant Payload");

    // Move the builder into the variant
    sample_t v_text = sample_new_text(sb);
    assert(sample_is_text(&v_text) == true);
    assert(sample_is_high(&v_text) == false);
    assert(sample_is_low(&v_text) == false);

    // Extract it and verify the contents
    libpstr_builder_t extracted = sample_unwrap_text(&v_text);
    
    // We can verify it worked by checking the length of the internal vector
    assert(extracted.vec.len > 0); 
    
    // Because this is C, we must manually clean up the builder we moved into the variant!
    libpstr.builder.cleanup(&extracted);

    printf("  [Variant] All tests passed!\n");
}
