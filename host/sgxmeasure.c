// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "sgxmeasure.h"
#include <openenclave/bits/raise.h>
#include <openenclave/bits/sgxtypes.h>
#include <openenclave/bits/trace.h>
#include <openenclave/host.h>

static void _MeasureZeros(oe_sha256__context_t* context, size_t size)
{
    char zeros[128] = {0};

    while (size)
    {
        if (size < sizeof(zeros))
        {
            oe_sha256__update(context, zeros, size);
            size -= size;
        }
        else
        {
            oe_sha256__update(context, zeros, sizeof(zeros));
            size -= sizeof(zeros);
        }
    }
}

static void _MeasureEExtend(
    oe_sha256__context_t* context,
    uint64_t vaddr,
    uint64_t flags,
    const void* page)
{
    uint64_t pgoff = 0;
    const uint64_t CHUNK_SIZE = 256;

    /* Write this page one chunk at a time */
    for (pgoff = 0; pgoff < OE_PAGE_SIZE; pgoff += CHUNK_SIZE)
    {
        const uint64_t moffset = vaddr + pgoff;

        oe_sha256__update(context, "EEXTEND", 8);
        oe_sha256__update(context, &moffset, sizeof(moffset));
        _MeasureZeros(context, 48);
        oe_sha256__update(context, (const uint8_t*)page + pgoff, CHUNK_SIZE);
    }
}

oe_result_t oe_sgx__measure_create_enclave(oe_sha256__context_t* context, SGX_Secs* secs)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!context || !secs)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Initialize measurement */
    oe_sha256__init(context);

    /* Measure ECREATE */
    oe_sha256__update(context, "ECREATE", 8);
    oe_sha256__update(context, &secs->ssaframesize, sizeof(uint32_t));
    oe_sha256__update(context, &secs->size, sizeof(uint64_t));
    _MeasureZeros(context, 44);

    result = OE_OK;

done:
    return result;
}

oe_result_t oe_sgx__measure_load_enclave_data(
    oe_sha256__context_t* context,
    uint64_t base,
    uint64_t addr,
    uint64_t src,
    uint64_t flags,
    bool extend)
{
    oe_result_t result = OE_UNEXPECTED;
    uint64_t vaddr = addr - base;

    if (!context || !base || !addr || !src || !flags || addr < base)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Measure EADD */
    oe_sha256__update(context, "EADD\0\0\0", 8);
    oe_sha256__update(context, &vaddr, sizeof(vaddr));
    oe_sha256__update(context, &flags, sizeof(flags));
    _MeasureZeros(context, 40);

    /* Measure EEXTEND if requested */
    if (extend)
        _MeasureEExtend(context, vaddr, flags, (void*)src);

    result = OE_OK;

done:
    return result;
}

oe_result_t oe_sgx__measure_initialize_enclave(
    oe_sha256__context_t* context,
    OE_SHA256* mrenclave)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!context || !mrenclave)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Finalize measurement like EINIT */
    oe_sha256__final(context, mrenclave);

    result = OE_OK;

done:
    return result;
}
