// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _ocall_args_h
#define _ocall_args_h

#include <openenclave/internal/calls.h>
#include <openenclave/internal/sgxtypes.h>

#define TEST1_MAGIC 0xec9a613e

typedef struct _test1_args
{
    void* self;

    /* --- Input arguments --- */

    int64_t in_num;

    const char* in_str;

    /* --- Output arguments --- */

    int64_t op;
    unsigned int magic;

    char* str;

    volatile void* sp1;
    volatile void* sp2;
    volatile void* sp3;

    unsigned int* mem;

    void* func;

    int ret;
} Test1Args;

typedef struct _test2_args
{
    int64_t in;
    int64_t out;
} Test2Args;

typedef struct _test_allocator_args
{
    int ret;
} TestAllocatorArgs;

typedef struct _func1_args
{
    char buf[128];
} Func1Args;

typedef struct _set_tsd_args
{
    void* value;
    int ret;
} SetTSDArgs;

typedef struct _get_tsd_args
{
    void* value;
    int ret;
} GetTSDArgs;

typedef struct _test_my_ocall_args
{
    uint64_t result;
} TestMyOCallArgs;

typedef struct _my_ocall_args
{
    uint64_t in;
    uint64_t out;
} my_ocall_args_t;

typedef struct _was_destructor_called_args
{
    bool called;
} was_destructor_called_args_t;

typedef struct _test_callback_args
{
    void (*callback)(void* arg, oe_enclave_t* enclave);
    uint64_t in;
    uint64_t out;
} test_callback_args_t;

#endif /* _ocall_args_h */
