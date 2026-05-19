# ============================================================================
# CGEN MASTER MAKEFILE CONFIGURATION
# ============================================================================
VERSION = 0.1.1

CC = gcc
# Clean CFLAGS: Strict compiler tuning flags and include paths only!
CFLAGS = -Wall -Wextra -std=c11 -O2 -DCGEN_VERSION=\"$(VERSION)\" -Isrc -I../libpstr/src

TARGET = cgen

# Path to our peer library static archive dependency
LIBPSTR_A = ../libpstr/libpstr.a

# Framework sources relocated into the src/ folder
FRAMEWORK_SRCS = src/opt.c src/parser.c src/cgen_framework.c
FRAMEWORK_OBJS = $(FRAMEWORK_SRCS:.c=.o)

.PHONY: all clean test clean_test

all: cgen cgen-vec cgen-sbovec cgen-map cgen-ring cgen-pqueue cgen-option cgen-result cgen-btree \
	cgen-variant cgen-map-iter cgen-btree-iter

# Automated trigger rule to build the peer library archive if it doesn't exist
$(LIBPSTR_A):
	$(MAKE) -C ../libpstr static

# Central driver orchestrator
cgen: src/main.o $(LIBPSTR_A)
	$(CC) $(CFLAGS) src/main.o $(LIBPSTR_A) -o $@

# Standard dynamic array generator
cgen-vec: src/cgen-vec.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Small-buffer optimized array generator
cgen-sbovec: src/cgen-sbovec.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# SWAR-accelerated SwissTable hash map generator
cgen-map: src/cgen-map.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# SWAR-accelerated SwissTable hash map iterator generator
cgen-map-iter: src/cgen-map-iter.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Circular Ring Buffer generator
cgen-ring: src/cgen-ring.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Priority Queue generator
cgen-pqueue: src/cgen-pqueue.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Type-safe Option/Maybe monad generator
cgen-option: src/cgen-option.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Type-safe Error/Result monad generator
cgen-result: src/cgen-result.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Self-balancing B-Tree index generator
cgen-btree: src/cgen-btree.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Self-balancing B-Tree index generator
cgen-btree-iter: src/cgen-btree-iter.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Custom type-safe Variant/Sum-Type generator
cgen-variant: src/cgen-variant.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

# Pattern rule for local src compilation
src/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o cgen cgen-vec cgen-sbovec cgen-map cgen-ring cgen-pqueue cgen-option cgen-result \
		cgen-btree cgen-variant cgen-map-iter cgen-btree-iter

# ============================================================================
# CGEN AUTOMATED INTEGRATION TESTING HARNESS
# ============================================================================

TEST_GEN_DIR = test/generated
# Added -DENABLE_PANIC_TESTING explicitly to the testing configuration flags
TEST_CFLAGS = -Wall -Wextra -std=c11 -g -DENABLE_PANIC_TESTING -I./$(TEST_GEN_DIR) -I./test -I../libpstr/src -include mock_struct.h

test: all $(LIBPSTR_A)
	@echo "🚀 Initializing cgen integration test framework..."
	@mkdir -p $(TEST_GEN_DIR)
	
	@echo "📦 Generating container test fixtures..."
	./cgen-vec -o $(TEST_GEN_DIR) int
	./cgen-vec -o $(TEST_GEN_DIR) custom_t
	./cgen-sbovec -o $(TEST_GEN_DIR) int
	./cgen-sbovec -o $(TEST_GEN_DIR) custom_t
	./cgen-map -o $(TEST_GEN_DIR) int int
	./cgen-map -o $(TEST_GEN_DIR) int custom_t
	./cgen-map-iter -o $(TEST_GEN_DIR) int custom_t
	./cgen-btree -o $(TEST_GEN_DIR) int int
	./cgen-btree -o $(TEST_GEN_DIR) int custom_t
	./cgen-btree-iter -o $(TEST_GEN_DIR) int custom_t
	./cgen-result -o $(TEST_GEN_DIR) int int
	./cgen-result -o $(TEST_GEN_DIR) int custom_t
	./cgen-option -o $(TEST_GEN_DIR) int
	./cgen-option -o $(TEST_GEN_DIR) custom_t
	./cgen-ring -o $(TEST_GEN_DIR) int
	./cgen-ring -o $(TEST_GEN_DIR) custom_t
	./cgen-pqueue -o $(TEST_GEN_DIR) int
	./cgen-pqueue -o $(TEST_GEN_DIR) custom_t
	
	@echo "🎨 Generating variant test fixtures..."
	./cgen-variant sample_t high:int low:int text:pstr_builder_t
	@mv sample_t.h sample_t.c $(TEST_GEN_DIR)/
	
	@echo "🔨 Compiling test runner with generated assets..."
	$(CC) $(TEST_CFLAGS) \
		test/test_main.c \
		test/test_vec.c \
		test/test_sbovec.c\
		test/test_map.c \
		test/generated/map_iter_int_custom.c \
		test/test_btree.c \
		test/generated/btree_iter_int_custom.c \
		test/test_option.c \
		test/test_ring.c \
		test/test_pqueue.c \
		test/test_variant.c \
		test/test_result.c \
		$(TEST_GEN_DIR)/vec_int.c \
		$(TEST_GEN_DIR)/vec_custom.c \
		$(TEST_GEN_DIR)/sbovec_int.c \
		$(TEST_GEN_DIR)/sbovec_custom.c \
		$(TEST_GEN_DIR)/map_int_int.c \
		$(TEST_GEN_DIR)/map_int_custom.c \
		$(TEST_GEN_DIR)/btree_int_int.c \
		$(TEST_GEN_DIR)/btree_int_custom.c \
		$(TEST_GEN_DIR)/result_int_int.c \
		$(TEST_GEN_DIR)/result_int_custom.c \
		$(TEST_GEN_DIR)/option_int.c \
		$(TEST_GEN_DIR)/option_custom.c \
		$(TEST_GEN_DIR)/ring_int.c \
		$(TEST_GEN_DIR)/ring_custom.c \
		$(TEST_GEN_DIR)/pqueue_int.c \
		$(TEST_GEN_DIR)/pqueue_custom.c \
		$(TEST_GEN_DIR)/sample_t.c \
		../libpstr/src/panic.c \
		$(LIBPSTR_A) \
		-o test_runner
	
	@echo "🧪 Running automated assertions..."
	./test_runner
	@echo "✨ Success! All cgen generated containers passed verification tests flawlessly."
	@$(MAKE) clean_test

clean_test:
	rm -rf $(TEST_GEN_DIR) test_runner
