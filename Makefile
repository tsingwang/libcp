BUILD_DIR := build
SRC_DIR := src
TEST_DIR := tests

CFLAGS := -std=c11 -Wall -pedantic -O2
TEST_TARGETS := \
	test-sds test-vector test-vector-typed \
	test-deque test-list test-deque-typed test-heap \
	test-heap-typed test-hashmap test-hashset \
	test-treemap test-treeset test-dict-typed \
	test-rbtree-typed test-bitset test-unionfind test-bitree \
	test-segtree test-graph test-radix test-radixmap

.PHONY: test $(TEST_TARGETS) clean install

test: $(TEST_TARGETS)

test-sds:
	$(MAKE) -C $(TEST_DIR) test-sds CC="$(CC)" CFLAGS="$(CFLAGS)"

test-vector:
	$(MAKE) -C $(TEST_DIR) test-vector CC="$(CC)" CFLAGS="$(CFLAGS)"

test-vector-typed:
	$(MAKE) -C $(TEST_DIR) test-vector-typed CC="$(CC)" CFLAGS="$(CFLAGS)"

test-deque:
	$(MAKE) -C $(TEST_DIR) test-deque CC="$(CC)" CFLAGS="$(CFLAGS)"

test-list:
	$(MAKE) -C $(TEST_DIR) test-list CC="$(CC)" CFLAGS="$(CFLAGS)"

test-deque-typed:
	$(MAKE) -C $(TEST_DIR) test-deque-typed CC="$(CC)" CFLAGS="$(CFLAGS)"

test-heap:
	$(MAKE) -C $(TEST_DIR) test-heap CC="$(CC)" CFLAGS="$(CFLAGS)"

test-heap-typed:
	$(MAKE) -C $(TEST_DIR) test-heap-typed CC="$(CC)" CFLAGS="$(CFLAGS)"

test-hashmap:
	$(MAKE) -C $(TEST_DIR) test-hashmap CC="$(CC)" CFLAGS="$(CFLAGS)"

test-hashset:
	$(MAKE) -C $(TEST_DIR) test-hashset CC="$(CC)" CFLAGS="$(CFLAGS)"

test-treemap:
	$(MAKE) -C $(TEST_DIR) test-treemap CC="$(CC)" CFLAGS="$(CFLAGS)"

test-treeset:
	$(MAKE) -C $(TEST_DIR) test-treeset CC="$(CC)" CFLAGS="$(CFLAGS)"

test-dict-typed:
	$(MAKE) -C $(TEST_DIR) test-dict-typed CC="$(CC)" CFLAGS="$(CFLAGS)"

test-rbtree-typed:
	$(MAKE) -C $(TEST_DIR) test-rbtree-typed CC="$(CC)" CFLAGS="$(CFLAGS)"

test-bitset:
	$(MAKE) -C $(TEST_DIR) test-bitset CC="$(CC)" CFLAGS="$(CFLAGS)"

test-unionfind:
	$(MAKE) -C $(TEST_DIR) test-unionfind CC="$(CC)" CFLAGS="$(CFLAGS)"

test-bitree:
	$(MAKE) -C $(TEST_DIR) test-bitree CC="$(CC)" CFLAGS="$(CFLAGS)"

test-segtree:
	$(MAKE) -C $(TEST_DIR) test-segtree CC="$(CC)" CFLAGS="$(CFLAGS)"

test-graph:
	$(MAKE) -C $(TEST_DIR) test-graph CC="$(CC)" CFLAGS="$(CFLAGS)"

test-radix:
	$(MAKE) -C $(TEST_DIR) test-radix CC="$(CC)" CFLAGS="$(CFLAGS)"

test-radixmap:
	$(MAKE) -C $(TEST_DIR) test-radixmap CC="$(CC)" CFLAGS="$(CFLAGS)"

clean:
	$(MAKE) clean -C $(TEST_DIR)
	rm -rf $(BUILD_DIR)


PREFIX := /usr/local
LEGACY_DIR := $(SRC_DIR)/legacy
HEADERS := \
	$(wildcard $(SRC_DIR)/*.h) \
	$(wildcard $(LEGACY_DIR)/*.h)
SOURCES := $(wildcard $(SRC_DIR)/*.c)
LIB_OBJECTS := \
	$(BUILD_DIR)/sds.o \
	$(BUILD_DIR)/vector.o \
	$(BUILD_DIR)/deque.o \
	$(BUILD_DIR)/list.o \
	$(BUILD_DIR)/heap.o \
	$(BUILD_DIR)/hashmap.o \
	$(BUILD_DIR)/hashset.o \
	$(BUILD_DIR)/treemap.o \
	$(BUILD_DIR)/treeset.o \
	$(BUILD_DIR)/bitset.o \
	$(BUILD_DIR)/unionfind.o \
	$(BUILD_DIR)/bitree.o \
	$(BUILD_DIR)/segtree.o \
	$(BUILD_DIR)/graph.o \
	$(BUILD_DIR)/radix.o \
	$(BUILD_DIR)/radixmap.o

$(BUILD_DIR)/sds.o: $(SRC_DIR)/sds.c $(SRC_DIR)/sds.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/sds.c $(CFLAGS)

$(BUILD_DIR)/vector.o: $(SRC_DIR)/vector.c $(SRC_DIR)/vector.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/vector.c $(CFLAGS)

$(BUILD_DIR)/deque.o: $(SRC_DIR)/deque.c $(SRC_DIR)/deque.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/deque.c $(CFLAGS)

$(BUILD_DIR)/list.o: $(SRC_DIR)/list.c $(SRC_DIR)/list.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/list.c $(CFLAGS)

$(BUILD_DIR)/heap.o: $(SRC_DIR)/heap.c $(SRC_DIR)/heap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/heap.c $(CFLAGS)

$(BUILD_DIR)/hashmap.o: $(SRC_DIR)/hashmap.c $(SRC_DIR)/hashmap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/hashmap.c $(CFLAGS)

$(BUILD_DIR)/hashset.o: \
	$(SRC_DIR)/hashset.c $(SRC_DIR)/hashset.h $(SRC_DIR)/hashmap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/hashset.c $(CFLAGS)

$(BUILD_DIR)/treemap.o: $(SRC_DIR)/treemap.c $(SRC_DIR)/treemap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/treemap.c $(CFLAGS)

$(BUILD_DIR)/treeset.o: \
	$(SRC_DIR)/treeset.c $(SRC_DIR)/treeset.h $(SRC_DIR)/treemap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/treeset.c $(CFLAGS)

$(BUILD_DIR)/bitset.o: $(SRC_DIR)/bitset.c $(SRC_DIR)/bitset.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/bitset.c $(CFLAGS)

$(BUILD_DIR)/unionfind.o: $(SRC_DIR)/unionfind.c $(SRC_DIR)/unionfind.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/unionfind.c $(CFLAGS)

$(BUILD_DIR)/bitree.o: $(SRC_DIR)/bitree.c $(SRC_DIR)/bitree.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/bitree.c $(CFLAGS)

$(BUILD_DIR)/segtree.o: $(SRC_DIR)/segtree.c $(SRC_DIR)/segtree.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/segtree.c $(CFLAGS)

$(BUILD_DIR)/graph.o: $(SRC_DIR)/graph.c $(SRC_DIR)/graph.h $(SRC_DIR)/vector.h $(SRC_DIR)/heap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/graph.c $(CFLAGS)

$(BUILD_DIR)/radix.o: $(SRC_DIR)/radix.c $(SRC_DIR)/radix.h $(SRC_DIR)/radixmap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/radix.c $(CFLAGS)

$(BUILD_DIR)/radixmap.o: $(SRC_DIR)/radixmap.c $(SRC_DIR)/radixmap.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c $(SRC_DIR)/radixmap.c $(CFLAGS)

install: $(LIB_OBJECTS)
	ar rcs $(BUILD_DIR)/libcp.a $^
	cp $(BUILD_DIR)/libcp.a $(PREFIX)/lib/
	cp $(HEADERS) $(PREFIX)/include/
	cp $(SOURCES) $(PREFIX)/include/
	cp combine.py $(PREFIX)/bin/combine
