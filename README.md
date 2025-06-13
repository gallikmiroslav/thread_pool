# Thread Pool Implementation in C

A lightweight, efficient thread pool implementation in C using POSIX threads (pthreads) and semaphores. This implementation provides a fixed-size thread pool with a bounded task queue, supporting both blocking and non-blocking task submission.

## Features

- Fixed-size thread pool with configurable number of worker threads
- Bounded task queue with configurable size
- Support for both blocking and non-blocking task submission
- Single allocation design for better memory efficiency
- Thread-safe implementation using POSIX synchronization primitives
- Clean shutdown with proper resource cleanup
- MIT Licensed

## Implementation Details
The thread pool uses:
- Semaphores for queue synchronization
- Mutexes for thread-safe queue access
- Single memory allocation for all structures
- Circular buffer for task queue

## Performance Considerations
- Task submission is O(1)
- Memory usage is fixed after initialization
- No dynamic allocations during operation
- Efficient task distribution among threads
- Minimal context switching overhead

## Thread Safety
All public functions are thread-safe and can be called from multiple threads simultaneously.

## API

### Thread Pool Initialization
```c
void* tp_init(size_t pool_size, size_t queue_size);
```

- Number of worker threads to create `pool_size`
- Maximum number of pending tasks `queue_size`
- Returns: Thread pool handle or NULL on failure

### Task Submission
``` c
int tp_add_task(void *thread_pool, tp_func_t func, void *arg, int no_block);
```
- Thread pool handle from tp_init `thread_pool`
- Function to execute `func`
- Argument to pass to the function `arg`
- If non-zero, return immediately when queue is full `no_block`
- Returns: 0 on success, non-zero on failure

### Cleanup
``` c
void tp_done(void *thread_pool);
```
- Thread pool handle to clean up `thread_pool`
- Waits for all tasks to complete
- Frees all resources

## Example Usage
``` c
#include "thread_pool.h"

void task_function(void *arg) {
    printf("Task %d executing\n", (int)arg);
}

int main() {
    // Create pool with 4 threads and queue size of 8
    void *pool = tp_init(4, 8);
    if (!pool) return 1;

    // Add tasks (blocking mode)
    for(int i = 0; i < 10; i++) {
        tp_add_task(pool, task_function, (void*)i, 0);
    }

    // Cleanup
    tp_done(pool);
    return 0;
}
```
## Building

### Build Commands
``` bash
mkdir build
cd build
cmake ..
make
```
## Testing
The project includes a test program that demonstrates basic functionality:
``` bash
./test
```

