// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/internal/enclavelibc.h>
#include <stdlib.h>

void* malloc(size_t size)
{
    return oe_malloc(size);
}

void free(void* ptr)
{
    return oe_free(ptr);
}

void* calloc(size_t nmemb, size_t size)
{
    return oe_calloc(nmemb, size);
}

void* realloc(void* ptr, size_t size)
{
    return oe_realloc(ptr, size);
}

int posix_memalign(void** memptr, size_t alignment, size_t size)
{
    return oe_posix_memalign(memptr, alignment, size);
}

void* memalign(size_t alignment, size_t size)
{
    return oe_memalign(alignment, size);
}
