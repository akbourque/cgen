# ============================================================================
# CGEN MASTER MAKEFILE CONFIGURATION
# ============================================================================
VERSION = 0.1.1

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -DCGEN_VERSION=\"$(VERSION)\"
TARGET = cgen

# 1. Framework sources used by cgen-vec and cgen-sbovec
FRAMEWORK_SRCS = opt.c parser.c cgen_framework.c
FRAMEWORK_OBJS = $(FRAMEWORK_SRCS:.c=.o)

# 2. Vendored libpstr sources required by ALL generation engines
VENDOR_SRCS = vendor/libpstr.c vendor/panic.c vendor/pstr_utf8.c vendor/pstr_vec.c
VENDOR_OBJS = $(VENDOR_SRCS:.c=.o)

.PHONY: all clean

all: cgen cgen-vec cgen-sbovec cgen-map cgen-ring cgen-pqueue cgen-option cgen-result cgen-btree

# Central driver orchestrator
cgen: main.o
	$(CC) $(CFLAGS) $^ -o $@

# Standard dynamic array generator
cgen-vec: cgen-vec.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Small-buffer optimized array generator
cgen-sbovec: cgen-sbovec.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# SWAR-accelerated SwissTable hash map generator
cgen-map: cgen-map.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# 
cgen-ring: cgen-ring.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

cgen-pqueue: cgen-pqueue.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

cgen-option: cgen-option.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

cgen-result: cgen-result.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

cgen-btree: cgen-btree.o $(FRAMEWORK_OBJS) $(VENDOR_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Pattern rule for local and nested vendor compilation
%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o vendor/*.o cgen cgen-vec cgen-sbovec cgen-map cgen-ring cgen-pqueue cgen-option cgen-result
	rm -f cgen-btree

# ============================================================================
# CGEN AUTOMATED INTEGRATION TESTING HARNESS
# ============================================================================

TEST_GEN_DIR = test/generated
TEST_CFLAGS = -Wall -Wextra -std=c11 -g -I./$(TEST_GEN_DIR) -I./test -include mock_struct.h

.PHONY: test clean_test

test: all
	@echo "🚀 Initializing cgen integration test framework..."
	@mkdir -p $(TEST_GEN_DIR)
	
	@echo "📦 Generating container test fixtures..."
	./cgen-vec -o $(TEST_GEN_DIR) int
	./cgen-vec -o $(TEST_GEN_DIR) custom_t 
	./cgen-sbovec -o $(TEST_GEN_DIR) int
	./cgen-sbovec -o $(TEST_GEN_DIR) custom_t
	./cgen-map -o $(TEST_GEN_DIR) int int
	./cgen-map -o $(TEST_GEN_DIR) int custom_t
	./cgen-btree -o $(TEST_GEN_DIR) int int
	./cgen-btree -o $(TEST_GEN_DIR) int custom_t
	./cgen-result -o $(TEST_GEN_DIR) int int
	./cgen-result -o $(TEST_GEN_DIR) int custom_t
	./cgen-option -o $(TEST_GEN_DIR) int
	./cgen-option -o $(TEST_GEN_DIR) custom_t
	./cgen-ring -o $(TEST_GEN_DIR) int
	./cgen-ring -o $(TEST_GEN_DIR) custom_t
	./cgen-pqueue -o $(TEST_GEN_DIR) int
	./cgen-pqueue -o $(TEST_GEN_DIR) custom_t
	# Note: As you or contributors add maps, ring buffers, or priority queues,
	# simply add their generation commands right here!
	
	@echo "🔨 Compiling test runner with generated assets..."
	$(CC) $(TEST_CFLAGS) \
		test/test_main.c \
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
		-o test_runner
	
	@echo "🧪 Running automated assertions..."
	./test_runner
	@echo "✨ Success! All cgen generated containers passed verification tests flawlessly."
	@$(MAKE) clean_test

clean_test:
	rm -rf $(TEST_GEN_DIR) test_runner
