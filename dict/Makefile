CFLAGS := -std=c11 -Wall -pedantic -O2
HEADERS := $(wildcard ./*.h)
SOURCES := $(wildcard ./*.c)

dict-test: $(SOURCES) $(HEADERS)
	$(CC) -o dict-test $(SOURCES) $(CFLAGS)
	./dict-test

clean:
	rm -f dict-test
