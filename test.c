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

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include "thread_pool.h"
#include <assert.h>

#define POOL_SIZE   5
#define QUEUE_SIZE  5

#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

void
task_proc(void *arg, int tid, void *user_data)
{
    (void)user_data;    // suppress unused parameter warning

    printf("%d TID: %d   gettid():%lu \n", (int)arg, tid, (unsigned long)gettid());
    sleep(1);
}

int main() {
    int ret;

    // Test initialization
    void *tp = tp_init(POOL_SIZE, QUEUE_SIZE);
    assert(tp != NULL);

    // Test sequential submission
    for(int i = 0; i < 32; i++)
    {
        ret = tp_add_task(tp, task_proc, (void*)i, 0);
        assert(ret == 0);
    }
    
    // Test non-blocking submission (
    ret = tp_add_task(tp, task_proc, (void*)100, 1);
    if (ret)
        printf("Task queue is full.\n");
    else
        printf("Task added.\n");
    
    tp_done(tp);
    return 0;
}