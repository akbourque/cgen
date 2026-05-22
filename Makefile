# ============================================================================
# CGEN MASTER MAKEFILE CONFIGURATION
# ============================================================================
VERSION = 0.1.2

CC = gcc
# Localized PATH ensures generated assets use the locally built cgen tools
export PATH := .:$(PATH)

CFLAGS = -Wall -Wextra -std=c11 -O2 -DCGEN_VERSION=\"$(VERSION)\" -Isrc -I../libpstr/src
TEST_CFLAGS = -Wall -Wextra -std=c11 -g -DENABLE_PANIC_TESTING -include mock_struct.h -Isrc -I../libpstr/src -Itest -I$(TEST_GEN_DIR)

LIBPSTR_A = ../libpstr/libpstr.a
TEST_GEN_DIR = test/generated
FRAMEWORK_SRCS = src/opt.c src/parser.c src/cgen_framework.c
FRAMEWORK_OBJS = $(FRAMEWORK_SRCS:.c=.o)

ASSETS = $(TEST_GEN_DIR)/vec_int.c $(TEST_GEN_DIR)/vec_custom_t.c \
         $(TEST_GEN_DIR)/sbovec_int.c $(TEST_GEN_DIR)/sbovec_custom_t.c \
         $(TEST_GEN_DIR)/map_int_int.c $(TEST_GEN_DIR)/map_int_custom_t.c \
         $(TEST_GEN_DIR)/map_iter_int_custom_t.c \
         $(TEST_GEN_DIR)/btree_int_int.c $(TEST_GEN_DIR)/btree_int_custom_t.c \
         $(TEST_GEN_DIR)/btree_iter_int_custom_t.c \
         $(TEST_GEN_DIR)/result_int_int.c $(TEST_GEN_DIR)/result_int_custom_t.c \
         $(TEST_GEN_DIR)/option_int.c $(TEST_GEN_DIR)/option_custom_t.c \
         $(TEST_GEN_DIR)/ring_int.c $(TEST_GEN_DIR)/ring_custom_t.c \
         $(TEST_GEN_DIR)/pqueue_int.c $(TEST_GEN_DIR)/pqueue_custom_t.c \
         $(TEST_GEN_DIR)/sample_t.c

.PHONY: all clean test

all: cgen cgen-vec cgen-sbovec cgen-map cgen-ring cgen-pqueue cgen-option cgen-result cgen-btree \
	cgen-variant cgen-map-iter cgen-btree-iter

$(LIBPSTR_A):
	$(MAKE) -C ../libpstr static

# Explicit target for the main cgen driver
cgen: src/bin/cgen.o $(LIBPSTR_A)
	$(CC) $(CFLAGS) src/bin/cgen.o $(LIBPSTR_A) -o $@

cgen-vec: src/bin/cgen-vec.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-sbovec: src/bin/cgen-sbovec.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-map: src/bin/cgen-map.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-map-iter: src/bin/cgen-map-iter.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-ring: src/bin/cgen-ring.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-pqueue: src/bin/cgen-pqueue.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-option: src/bin/cgen-option.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-result: src/bin/cgen-result.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-btree: src/bin/cgen-btree.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-btree-iter: src/bin/cgen-btree-iter.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

cgen-variant: src/bin/cgen-variant.o $(FRAMEWORK_OBJS) $(LIBPSTR_A)
	$(CC) $(CFLAGS) $^ -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

src/bin/%.o: src/bin/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_GEN_DIR)/%.c:
	@mkdir -p $(TEST_GEN_DIR)
	@TYPE_NAME=$$(basename $@ .c); \
	case "$$TYPE_NAME" in \
		sample_t) \
			./cgen-variant sample_t high:int low:int text:libpstr_builder_t; \
			mv sample_t.h sample_t.c $(TEST_GEN_DIR)/ ;; \
		map_iter_* | btree_iter_*) \
			TOOL=$$(echo $$TYPE_NAME | cut -d'_' -f1-2 | sed 's/_/-/g'); \
			KEY=$$(echo $$TYPE_NAME | cut -d'_' -f3); \
			VAL=$$(echo $$TYPE_NAME | cut -d'_' -f4-); \
			./cgen-$$TOOL -o $(TEST_GEN_DIR) $$KEY $$VAL ;; \
		map_* | btree_* | result_*) \
			TOOL=$$(echo $$TYPE_NAME | cut -d'_' -f1); \
			KEY=$$(echo $$TYPE_NAME | cut -d'_' -f2); \
			VAL=$$(echo $$TYPE_NAME | cut -d'_' -f3-); \
			./cgen-$$TOOL -o $(TEST_GEN_DIR) $$KEY $$VAL ;; \
		*) \
			TOOL=$$(echo $$TYPE_NAME | cut -d'_' -f1); \
			ARG=$$(echo $$TYPE_NAME | cut -d'_' -f2-); \
			./cgen-$$TOOL -o $(TEST_GEN_DIR) $$ARG ;; \
	esac

test: all $(ASSETS)
	gcc $(TEST_CFLAGS) \
		test/test_main.c \
		test/test_parser.c \
		test/test_vec.c \
		test/test_sbovec.c \
		test/test_map.c \
		test/test_btree.c \
		test/test_result.c \
		test/test_option.c \
		test/test_ring.c \
		test/test_pqueue.c \
		test/test_variant.c \
		$(FRAMEWORK_SRCS) \
		$(ASSETS) \
		../libpstr/src/panic.c ../libpstr/libpstr.a -o test_runner
	./test_runner
	rm -rf test_runner

clean:
	rm -f src/*.o src/bin/*.o cgen* test_runner
	rm -rf $(TEST_GEN_DIR)
