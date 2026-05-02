# libcp

A C library containing data structures and algorithms, especially for Competitive Programming.

The legacy version was based on macros, which made it difficult to read and debug.
It's now written entirely in Vibe Coding, in runtime mode, which is fantastic.

Here are some implementation inspiration:
- https://github.com/antirez/sds
- https://github.com/tezc/sc
- https://github.com/KaisenAmin/c_std
- https://github.com/postgres/postgres/blob/master/src/backend/lib/rbtree.c

### Library list

- **sds**: A simplification of Redis SDS
- **vector**: runtime-generic dynamic array
- **deque**: runtime-generic double-ended queue
- **list**: runtime-generic doubly-linked list
- **heap**: runtime-generic binary heap, for `priority_queue`
- **hashmap**: runtime-generic open addressing hash map
- **hashset**: runtime-generic open addressing hash set
- **treemap**: runtime-generic ordered map, implemented by a red-black tree
- **treeset**: runtime-generic ordered set, implemented by a red-black tree
- **bitset**: fixed-size packed boolean array
- **unionfind**: disjoint-set union with path compression and union by size
- **bitree**: binary indexed tree for point updates and range sums
- **segtree**: runtime-generic segment tree for point updates and range queries
- **graph**: adjacency-list graph container with typed neighbor access
- **radix**: compressed radix tree set for string keys and prefix queries
- **radixmap**: compressed radix tree map for string keys and prefix queries

#### Legacy

- **vector_typed**: legacy macro-based typed dynamic array
- **deque_typed**: legacy macro-based typed double-ended queue
- **heap_typed**: legacy macro-based typed binary heap
- **dict_typed**: legacy macro-based hash map / hash set
- **rbtree_typed**: legacy macro-based red-black tree map

### Usage

```sh
# Install libcp
sudo make install
export LIBRARY_PATH=/usr/local/lib

# Use libcp
gcc your_code.c -l cp

# Generate an independent file with the suffix _combined.c
combine your_code.c
```

### SDS example

```c
#include <stdio.h>
#include "sds.h"

int main(void) {
    sds s = sdsnew("hello world");
    size_t count = 0;
    sds *parts = sdssplit(s, " ", &count);
    sds joined = sdsjoin(parts, count, "-");

    printf("%s (%zu)\n", joined, sdslen(joined));

    sdssplitfree(parts, count);
    sdsfree(joined);
    sdsfree(s);
}
```

For binary-safe payloads with embedded `'\0'`, use the length-aware APIs:
`sdsnewlen`, `sdsncat`, `sdsjoinlen`, `sdssplitlen`, and `sdsreplacelen`.
`sdsalloc`, `sdsavail`, `sdsreserve`, and `sdsclear` expose the stored
capacity and reusable buffer space.

### Vector example

```c
#include <stdio.h>
#include "vector.h"

int main(void) {
    vector *v = vector_new(sizeof(int), NULL, NULL);

    vector_push_value(v, int, 1);
    vector_push_value(v, int, 2);
    vector_set_value(v, 1, int, 42);

    for (size_t i = 0; i < vector_size(v); i++) {
        printf("%d\n", *vector_get_as(v, i, int));
    }

    vector_free(v);
}
```

### Deque example

```c
#include <stdio.h>
#include "deque.h"

int main(void) {
    deque *q = deque_new(sizeof(int), NULL, NULL);

    deque_push_back_value(q, int, 1);
    deque_push_back_value(q, int, 2);
    deque_push_front_value(q, int, 0);

    printf("front=%d back=%d size=%zu\n",
           *deque_front_as(q, int),
           *deque_back_as(q, int),
           deque_size(q));

    deque_free(q);
}
```

### List example

```c
#include <stdio.h>
#include "list.h"

int main(void) {
    list *l = list_new(sizeof(int), NULL, NULL);
    list_node *node;

    list_push_back_value(l, int, 1);
    list_push_back_value(l, int, 3);
    node = list_node_at(l, 1);
    list_insert_before_value(l, node, int, 2);

    for (node = list_front_node(l); node != NULL; node = node->next) {
        printf("%d\n", *list_node_value_as(node, int));
    }

    list_free(l);
}
```

### Heap example

`heap` is a **max-heap** by default if your comparator returns `> 0` when the
first element should stay above the second one.

- `int_cmp(a, b) = *(int *)a - *(int *)b` -> max-heap, larger values come out first
- `int_cmp(a, b) = *(int *)b - *(int *)a` -> min-heap, smaller values come out first

```c
#include <stdio.h>
#include "heap.h"

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

int main(void) {
    heap *h = heap_new(sizeof(int), int_cmp, NULL, NULL);

    heap_push_value(h, int, 3);
    heap_push_value(h, int, 1);
    heap_push_value(h, int, 5);

    while (!heap_empty(h)) {
        printf("%d\n", *heap_top_as(h, int));
        heap_pop(h);
    }

    heap_free(h);
}
```

### Hashmap example

`hashmap` keys are intentionally limited to:

- scalars such as `int`, `long long`, enums, or pointer values
- `char *`, compared by string contents and copied internally

```c
#include <stdio.h>
#include "hashmap.h"

int main(void) {
    hashmap *map = hashmap_new(HASHMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                               NULL, NULL);

    hashmap_set_value(map, int, 1, int, 100);
    hashmap_set_value(map, int, 2, int, 200);

    printf("%d\n", *hashmap_get_as(map, int, 2, int));

    hashmap_free(map);
}
```

`HASHMAP_KEY_CSTR` compares by string contents and copies keys internally:

```c
#include <stdio.h>
#include "hashmap.h"

int main(void) {
    hashmap *map = hashmap_new(HASHMAP_KEY_CSTR, sizeof(char *), sizeof(int),
                               NULL, NULL);

    hashmap_set_value(map, char *, "alice", int, 10);
    hashmap_set_value(map, char *, "bob", int, 20);

    printf("%d\n", *hashmap_get_as(map, char *, "bob", int));

    hashmap_free(map);
}
```

To iterate all entries, walk occupied buckets with `hashmap_first(...)` /
`hashmap_next(...)`, then read each key and value with
`hashmap_entry_key(...)` and `hashmap_entry_value(...)`.

### Hashset example

```c
#include <stdio.h>
#include "hashset.h"

int main(void) {
    hashset *set = hashset_new(HASHMAP_KEY_SCALAR, sizeof(int));

    hashset_add_value(set, int, 3);
    hashset_add_value(set, int, 5);

    printf("%d\n", hashset_contains_value(set, int, 5));

    hashset_free(set);
}
```

### Treemap example

```c
#include <stdio.h>
#include "treemap.h"

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

int main(void) {
    treemap *map = treemap_new(TREEMAP_KEY_SCALAR, sizeof(int), sizeof(int),
                               int_cmp, NULL, NULL);
    treemap_node *node;

    treemap_set_value(map, int, 3, int, 30);
    treemap_set_value(map, int, 1, int, 10);
    treemap_set_value(map, int, 2, int, 20);

    for (node = treemap_first(map); node != NULL; node = treemap_next(node)) {
        printf("%d -> %d\n",
               *treemap_node_key_as(map, node, int),
               *treemap_node_value_as(map, node, int));
    }

    node = treemap_lower_bound(map, &(int){ 2 });
    printf("lower_bound(2) = %d\n", *treemap_node_key_as(map, node, int));

    treemap_free(map);
}
```

### Treeset example

```c
#include <stdio.h>
#include "treeset.h"

static int int_cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

int main(void) {
    treeset *set = treeset_new(TREEMAP_KEY_SCALAR, sizeof(int), int_cmp);
    treemap_node *node;

    treeset_add_value(set, int, 3);
    treeset_add_value(set, int, 1);
    treeset_add_value(set, int, 2);

    for (node = treeset_first(set); node != NULL; node = treeset_next(node)) {
        printf("%d\n", *treeset_node_key_as(set, node, int));
    }

    treeset_free(set);
}
```

```c
#include <stdio.h>
#include "hashset.h"

int main(void) {
    hashset *set = hashset_new(HASHMAP_KEY_CSTR, sizeof(char *));

    hashset_add_value(set, char *, "alice");
    hashset_add_value(set, char *, "bob");

    printf("%d\n", hashset_contains_value(set, char *, "bob"));

    hashset_free(set);
}
```

### Bitset example

```c
#include <stdio.h>
#include "bitset.h"

int main(void) {
    char buf[9];
    bitset *bs = bitset_new(8);

    bitset_from_string(bs, "00101001");
    bitset_flip(bs, 7);

    printf("%s (%zu set bits)\n", bitset_to_string(bs, buf, sizeof(buf)),
           bitset_count(bs));
    printf("first set bit = %zu\n", bitset_find_first_set(bs));

    bitset_free(bs);
}
```

### Union-find example

```c
#include <stdio.h>
#include "unionfind.h"

int main(void) {
    unionfind *uf = unionfind_new(5);

    unionfind_union(uf, 0, 1);
    unionfind_union(uf, 1, 2);

    printf("same=%d size=%zu set_count=%zu\n",
           unionfind_same(uf, 0, 2),
           unionfind_set_size(uf, 0),
           unionfind_set_count(uf));

    unionfind_free(uf);
}
```

### Bitree example

```c
#include <inttypes.h>
#include <stdio.h>
#include "bitree.h"

int main(void) {
    bitree *bt = bitree_new(5);

    bitree_add(bt, 0, 3);
    bitree_add(bt, 2, 5);
    int64_t range_sum = bitree_range(bt, 0, 3);

    printf("%" PRId64 "\n", range_sum);

    bitree_free(bt);
}
```

### Segment tree example

```c
#include <inttypes.h>
#include <stdio.h>
#include "segtree.h"

static int sum_merge(void *dst, const void *lhs, const void *rhs) {
    *(int64_t *)dst = *(const int64_t *)lhs + *(const int64_t *)rhs;
    return 0;
}

int main(void) {
    int64_t zero = 0;
    int64_t values[] = { 1, 2, 3, 4, 5 };
    int64_t range_sum;
    segtree *st = segtree_new(5, sizeof(int64_t), &zero, sum_merge);

    segtree_build(st, values);
    segtree_set_value(st, 2, int64_t, 10);
    segtree_query(st, 1, 4, &range_sum);

    printf("%" PRId64 "\n", range_sum);

    segtree_free(st);
}
```

`segtree` stores one copied identity value and uses your merge function to
answer half-open `[begin, end)` range queries in `O(log n)`. This makes it a
good fit for sums, minima, maxima, gcd, or small custom structs.

### Graph example

```c
#include <stdio.h>
#include "graph.h"

int main(void) {
    graph *g = graph_new(GRAPH_UNDIRECTED, 4);
    const graph_edge *neighbors;
    size_t degree;

    graph_add_edge(g, 0, 1, 2);
    graph_add_edge(g, 1, 2, 3);
    graph_add_edge(g, 2, 3, 4);

    degree = graph_degree(g, 1);
    neighbors = graph_neighbors(g, 1);
    printf("degree=%zu first_to=%zu\n", degree, neighbors[0].to);

    graph_free(g);
}
```

Traversal is intentionally left to the caller: use `graph_degree(...)` and
`graph_neighbors(...)` to write the exact BFS/DFS variant your problem needs.
If you want direct adjacency-list access, `graph_adj_list(...)` returns the
underlying `vector<graph_edge>` for one vertex, and `graph.adj_lists` is now
explicitly typed as `vector *`.
`graph_delete_edge(...)` removes one exact `(from, to, weight)` logical edge.

### Radix example

```c
#include <stdio.h>
#include "radix.h"

int main(void) {
    radix *set = radix_new();

    radix_add(set, "foo");
    radix_add(set, "foobar");

    printf("contains=%d prefix(fo)=%d\n",
           radix_contains(set, "foo"),
           radix_has_prefix(set, "fo"));

    radix_free(set);
}
```

### Radix map example

```c
#include <stdio.h>
#include "radixmap.h"

int main(void) {
    radixmap *tree = radixmap_new(sizeof(int), NULL, NULL);

    radixmap_set_value(tree, "foo", int, 1);
    radixmap_set_value(tree, "foobar", int, 2);

    printf("foo=%d prefix(fo)=%d\n",
           *radixmap_get_as(tree, "foo", int),
           radixmap_has_prefix(tree, "fo"));

    radixmap_free(tree);
}
```
