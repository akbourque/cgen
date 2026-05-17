VERSION = 0.1.1

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -DCGEN_VERSION=\"$(VERSION)\"

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
