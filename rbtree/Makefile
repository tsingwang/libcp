CFLAGS := -std=c11 -Wall -pedantic -O2
HEADERS := $(wildcard ./*.h)
SOURCES := $(wildcard ./*.c)

rbtree-test: $(SOURCES) $(HEADERS)
	$(CC) -o rbtree-test $(SOURCES) $(CFLAGS)
	./rbtree-test

clean:
	rm -f rbtree-test
