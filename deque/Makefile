CFLAGS := -std=c11 -Wall -pedantic -O2
HEADERS := $(wildcard ./*.h)
SOURCES := $(wildcard ./*.c)

deque-test: $(SOURCES) $(HEADERS)
	$(CC) -o deque-test $(SOURCES) $(CFLAGS)
	./deque-test

clean:
	rm -r deque-test
