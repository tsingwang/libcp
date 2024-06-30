# libcp

A C library containing data structures and algorithms, especially for Competitive Programming.

Here are some implementation inspiration:
- https://github.com/redis/redis
- https://github.com/fragglet/c-algorithms
- https://github.com/tezc/sc
- https://github.com/KaisenAmin/c_std

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
- **bitset**

### No implementation

- **list**: doubly-linked list, it can be replaced by deque
- **rbtree**: red-black tree, powerful performance, but implementation is a bit complex
