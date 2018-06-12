// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_MALLOC_H
#define _OE_MALLOC_H

#include <openenclave/bits/defs.h>
#include <openenclave/bits/result.h>
#include <openenclave/bits/types.h>

OE_EXTERNC_BEGIN

typedef void (*OE_AllocationFailureCallback)(
    const char* file,
    size_t line,
    const char* func,
    size_t size);

void OE_SetAllocationFailureCallback(OE_AllocationFailureCallback function);

typedef struct _OE_MallocStats
{
    uint64_t peakSystemBytes;
    uint64_t systemBytes;
    uint64_t inUseBytes;
} OE_MallocStats;

/**
 * Obtains enclave malloc statistics.
 *
 * This function obtains malloc statistics for the calling enclave. The enclave
 * must link the **oelibc** library, where this function is defined. These
 * statistics include:
 *
 *     - the peak system bytes allocated
 *     - the current system bytes allocated
 *     - the number of bytes in use
 *
 * @param stats[output] the malloc statistics
 *
 * @return 0 success
 * @return -1 failure
 */
OE_Result OE_GetMallocStats(OE_MallocStats* stats);

OE_EXTERNC_END

#endif /* _OE_MALLOC_H */
