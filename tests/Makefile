SRC_DIR := ../src
LEGACY_DIR := $(SRC_DIR)/legacy
CPPFLAGS := -I$(SRC_DIR) -I$(LEGACY_DIR) -I.
CFLAGS ?= -std=c11 -Wall -pedantic -O2

test: test-sds test-vector test-vector-typed test-deque \
	test-list \
	test-deque-typed test-heap test-heap-typed test-hashmap \
	test-hashset test-treemap test-treeset test-dict-typed \
	test-rbtree-typed test-bitset test-unionfind test-bitree \
	test-segtree \
	test-graph \
	test-radix \
	test-radixmap

test-sds: sds-test

sds-test: $(SRC_DIR)/sds.c $(SRC_DIR)/sds.h sds_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/sds.c sds_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-vector: runtime-vector-test

test-vector-typed: typed-vector-test

runtime-vector-test: $(SRC_DIR)/vector.c $(SRC_DIR)/vector.h vector_test.c testhelp.h
	$(CC) -o vector-test $(SRC_DIR)/vector.c vector_test.c $(CPPFLAGS) $(CFLAGS)
	./vector-test

typed-vector-test: $(LEGACY_DIR)/vector_typed.h vector_typed_test.c testhelp.h
	$(CC) -o vector-typed-test vector_typed_test.c $(CPPFLAGS) $(CFLAGS)
	./vector-typed-test

test-deque: runtime-deque-test

test-list: list-test

list-test: $(SRC_DIR)/list.c $(SRC_DIR)/list.h list_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/list.c list_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-deque-typed: typed-deque-test

runtime-deque-test: $(SRC_DIR)/deque.c $(SRC_DIR)/deque.h deque_test.c testhelp.h
	$(CC) -o deque-test $(SRC_DIR)/deque.c deque_test.c $(CPPFLAGS) $(CFLAGS)
	./deque-test

typed-deque-test: $(LEGACY_DIR)/deque_typed.h deque_typed_test.c testhelp.h
	$(CC) -o deque-typed-test deque_typed_test.c $(CPPFLAGS) $(CFLAGS)
	./deque-typed-test

test-heap: runtime-heap-test

test-heap-typed: typed-heap-test

runtime-heap-test: $(SRC_DIR)/heap.c $(SRC_DIR)/heap.h heap_test.c testhelp.h
	$(CC) -o heap-test $(SRC_DIR)/heap.c heap_test.c $(CPPFLAGS) $(CFLAGS)
	./heap-test

typed-heap-test: $(LEGACY_DIR)/heap_typed.h heap_typed_test.c testhelp.h
	$(CC) -o heap-typed-test heap_typed_test.c $(CPPFLAGS) $(CFLAGS)
	./heap-typed-test

test-hashmap: hashmap-test

hashmap-test: $(SRC_DIR)/hashmap.c $(SRC_DIR)/hashmap.h hashmap_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/hashmap.c hashmap_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-hashset: hashset-test

hashset-test: $(SRC_DIR)/hashmap.c $(SRC_DIR)/hashmap.h $(SRC_DIR)/hashset.c $(SRC_DIR)/hashset.h hashset_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/hashmap.c $(SRC_DIR)/hashset.c hashset_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-treemap: treemap-test

treemap-test: $(SRC_DIR)/treemap.c $(SRC_DIR)/treemap.h treemap_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/treemap.c treemap_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-treeset: treeset-test

treeset-test: $(SRC_DIR)/treemap.c $(SRC_DIR)/treemap.h $(SRC_DIR)/treeset.c $(SRC_DIR)/treeset.h treeset_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/treemap.c $(SRC_DIR)/treeset.c treeset_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-dict-typed: dict-typed-test

dict-typed-test: $(LEGACY_DIR)/dict_typed.h dict_typed_test.c testhelp.h
	$(CC) -o dict-typed-test dict_typed_test.c $(CPPFLAGS) $(CFLAGS)
	./dict-typed-test

test-rbtree-typed: rbtree-typed-test

rbtree-typed-test: $(LEGACY_DIR)/rbtree_typed.h rbtree_typed_test.c testhelp.h
	$(CC) -o rbtree-typed-test rbtree_typed_test.c $(CPPFLAGS) $(CFLAGS)
	./rbtree-typed-test

test-bitset: bitset-test

bitset-test: $(SRC_DIR)/bitset.c $(SRC_DIR)/bitset.h bitset_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/bitset.c bitset_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-unionfind: unionfind-test

unionfind-test: $(SRC_DIR)/unionfind.c $(SRC_DIR)/unionfind.h unionfind_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/unionfind.c unionfind_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-bitree: bitree-test

bitree-test: $(SRC_DIR)/bitree.c $(SRC_DIR)/bitree.h bitree_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/bitree.c bitree_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-segtree: segtree-test

segtree-test: $(SRC_DIR)/segtree.c $(SRC_DIR)/segtree.h segtree_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/segtree.c segtree_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-radix: radix-test

radix-test: $(SRC_DIR)/radix.c $(SRC_DIR)/radix.h $(SRC_DIR)/radixmap.c $(SRC_DIR)/radixmap.h radix_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/radix.c $(SRC_DIR)/radixmap.c radix_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-graph: graph-test

graph-test: $(SRC_DIR)/graph.c $(SRC_DIR)/graph.h $(SRC_DIR)/vector.c $(SRC_DIR)/vector.h $(SRC_DIR)/heap.c $(SRC_DIR)/heap.h graph_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/graph.c $(SRC_DIR)/vector.c $(SRC_DIR)/heap.c graph_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

test-radixmap: radixmap-test

radixmap-test: $(SRC_DIR)/radixmap.c $(SRC_DIR)/radixmap.h radixmap_test.c testhelp.h
	$(CC) -o $@ $(SRC_DIR)/radixmap.c radixmap_test.c $(CPPFLAGS) $(CFLAGS)
	./$@

clean:
	rm -f *-test
