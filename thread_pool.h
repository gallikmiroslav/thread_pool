/*
 * MIT License
 *
 * Copyright (c) 2025 Miroslav Gallik
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef THREAD_POOL_THREAD_POOL_H
#define THREAD_POOL_THREAD_POOL_H

typedef void (*tp_func_t)(void *arg);

/// Initialize new thread pool
/// \param pool_size size of thread pool
/// \param queue_size maximum tasks in queue before blocking
/// \return reference to thread pool
void* tp_init(size_t pool_size, size_t queue_size);

/// Add new task to thread pool
/// \param thread_pool reference to thread pool (return value from tp_init)
/// \param func function to call in worker thread
/// \param arg function argument
/// \param no_block zero value to block if queue is full otherwise return -1
/// \return zero on success
int tp_add_task(void *thread_pool, tp_func_t func, void *arg, int no_block);

/// End thread pool and remove all memory. all references to pool is invalid after this function
/// \param thread_pool reference to thread pool (return value from tp_init)
void tp_done(void *thread_pool);

#endif //THREAD_POOL_THREAD_POOL_H
