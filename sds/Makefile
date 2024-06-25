CFLAGS := -std=c11 -Wall -pedantic -O2
HEADERS := $(wildcard ./*.h)
SOURCES := $(wildcard ./*.c)

sds-test: $(SOURCES) $(HEADERS)
	$(CC) -o sds-test $(SOURCES) $(CFLAGS)
	./sds-test

clean:
	rm -r sds-test
