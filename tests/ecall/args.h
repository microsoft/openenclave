// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _new_args_h
#define _new_args_h

#include <openenclave/internal/sgxtypes.h>

#define NEW_MAGIC 0x7541cc89

#define FUNC1 1

typedef struct _TestArgs
{
    void* self;
    unsigned int magic;
    uint64_t baseHeapPage;
    uint64_t num_heap_pages;
    uint64_t numPages;
    const void* base;
    oe_thread_data_t threadData;
    uint64_t threadDataAddr;
    unsigned int mm;
    unsigned int dd;
    unsigned int yyyy;
    unsigned int setjmpResult;
    unsigned int magic2;
} TestArgs;

#endif /* _new_args_h */
