BUILD_DIR := build

CFLAGS := -std=c11 -Wall -pedantic -O2

test: test-sds test-vector test-deque test-heap test-dict test-bitset

test-sds: sds
	$(MAKE) -C sds

test-vector: vector
	$(MAKE) -C vector

test-deque: deque
	$(MAKE) -C deque

test-heap: heap
	$(MAKE) -C heap

test-dict: dict
	$(MAKE) -C dict

test-bitset: bitset
	$(MAKE) -C bitset

clean:
	$(MAKE) clean -C sds
	$(MAKE) clean -C vector
	$(MAKE) clean -C deque
	$(MAKE) clean -C heap
	$(MAKE) clean -C dict
	$(MAKE) clean -C bitset
	rm -r $(BUILD_DIR)


PREFIX := /usr/local
HEADERS := $(shell find . -name '*.h')
SOURCES := sds/sds.c bitset/bitset.c

$(BUILD_DIR)/sds.o: sds/sds.c sds/sds.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c sds/sds.c $(CFLAGS)

$(BUILD_DIR)/bitset.o: bitset/bitset.c bitset/bitset.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ -c bitset/bitset.c $(CFLAGS)

install: $(BUILD_DIR)/sds.o $(BUILD_DIR)/bitset.o
	ar rcs $(BUILD_DIR)/libcp.a $^
	cp $(BUILD_DIR)/libcp.a $(PREFIX)/lib/
	cp $(HEADERS) $(PREFIX)/include/
	cp $(SOURCES) $(PREFIX)/include/
	cp combine.py $(PREFIX)/bin/combine
