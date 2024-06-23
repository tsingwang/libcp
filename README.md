# libcp

A C library containing data structures and algorithms, especially for Competitive Programming.

Here are some implementation inspiration:
- https://github.com/redis/redis
- https://github.com/tezc/sc

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
- **vector**: dynamic array
- **deque**: double-ended queue
- **heap**: binary heap, for priority_queue

### Failed attempt

- **list**: doubly-linked list, it can be replaced by deque.
