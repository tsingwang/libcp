# libcp

A C library containing data structures and algorithms, especially for Competitive Programming.

Here are some implementation inspiration:
- https://github.com/antirez/sds
- https://github.com/tezc/sc
- https://github.com/KaisenAmin/c_std
- https://github.com/postgres/postgres/blob/master/src/backend/lib/rbtree.c

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

### Library list

- **sds**: A simplification of Redis SDS
- **vector**: dynamic array, can be used as `stack`
- **deque**: double-ended queue, can be used as `queque`
- **heap**: binary heap, for `priority_queue`
- **dict**: open addressing hash table, for `unordered_set` and `unordered_map`
- **rbtree**: red-black tree, for `map`
- **bitset**

### No implementation

- **list**: doubly-linked list, fast insertion in the middle, but slow positioning
