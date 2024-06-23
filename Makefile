PREFIX = /usr/local
BUILD_DIR = build

CFLAGS = -std=c11 -Wall -pedantic -O2

HEADERS = $(shell find . -name '*.h')
SOURCES = sds/sds.c

test: test-sds test-vector test-deque test-heap

test-sds: sds
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/sds-test sds/sds_test.c sds/sds.c $(CFLAGS)
	./$(BUILD_DIR)/sds-test

test-vector: vector
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/vector-test vector/vector_test.c $(CFLAGS)
	./$(BUILD_DIR)/vector-test

test-deque: deque
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/deque-test deque/deque_test.c $(CFLAGS)
	./$(BUILD_DIR)/deque-test

test-heap: heap
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $(BUILD_DIR)/heap-test heap/heap_test.c $(CFLAGS)
	./$(BUILD_DIR)/heap-test

$(BUILD_DIR)/sds.o: sds/sds.c sds/sds.h
	@mkdir $(BUILD_DIR)
	$(CC) -o $@ -c sds/sds.c $(CFLAGS)

install: $(BUILD_DIR)/sds.o
	ar rcs $(BUILD_DIR)/libcp.a $^
	cp $(BUILD_DIR)/libcp.a $(PREFIX)/lib/
	cp $(HEADERS) $(PREFIX)/include/
	cp $(SOURCES) $(PREFIX)/include/
	cp combine.py $(PREFIX)/bin/combine

clean:
	rm -r build
