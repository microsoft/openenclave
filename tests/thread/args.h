// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _stdc_args_h
#define _stdc_args_h

#include <openenclave/bits/types.h>

typedef struct _test_mutex_args
{
    size_t count1;
    size_t count2;
} TestMutexArgs;

typedef struct _wait_args
{
    /* The number of threads that will call wait */
    size_t num_threads;
} WaitArgs;

typedef struct _test_rwlock_args
{
    // Number of simultaneously active readers
    size_t readers;

    // Number of simultaneously active writers
    size_t writers;

    // Maximum number of simultaneously active readers
    size_t max_readers;

    // Maximum number of simultaneously active writers
    size_t max_writers;

    // Readers and writers active at same time
    bool readers_and_writers;
} TestRWLockArgs;

#endif /* _stdc_args_h */
