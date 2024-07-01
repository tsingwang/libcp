CFLAGS := -std=c11 -Wall -pedantic -O2
HEADERS := $(wildcard ./*.h)
SOURCES := $(wildcard ./*.c)

bitset-test: $(SOURCES) $(HEADERS)
	$(CC) -o bitset-test $(SOURCES) $(CFLAGS)
	./bitset-test

clean:
	rm -f bitset-test
