all: sds-test vector-test deque-test

sds-test: sds
	$(CC) -o sds-test sds/sds_test.c sds/sds.c -Wall -std=c11 -pedantic -O2

vector-test: vector
	$(CC) -o vector-test vector/vector_test.c -Wall -std=c11 -pedantic -O2

deque-test: deque
	$(CC) -o deque-test deque/deque_test.c -Wall -std=c11 -pedantic -O2

clean:
	rm -f sds-test vector-test deque-test
