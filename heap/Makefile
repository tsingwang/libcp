CFLAGS := -std=c11 -Wall -pedantic -O2
HEADERS := $(wildcard ./*.h)
SOURCES := $(wildcard ./*.c)

heap-test: $(SOURCES) $(HEADERS)
	$(CC) -o heap-test $(SOURCES) $(CFLAGS)
	./heap-test

clean:
	rm -f heap-test
