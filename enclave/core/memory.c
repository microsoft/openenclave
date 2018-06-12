// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/globals.h>

bool oe_is_within_enclave(const void* p, size_t n)
{
    uint64_t rangeStart = (uint64_t)p;
    uint64_t rangeEnd = rangeStart + (n == 0 ? 1 : n);
    uint64_t enclaveStart = (uint64_t)__oe_get_enclave_base();
    uint64_t enclaveEnd = enclaveStart + __oe_get_enclave_size();

    // Disallow nullptr and check that arithmetic operations do not wrap
    // Check that block lies completely within the enclave
    if ((rangeStart > 0) && (rangeEnd > rangeStart) &&
        (enclaveEnd > enclaveStart) &&
        ((rangeStart >= enclaveStart) && (rangeEnd <= enclaveEnd)))
    {
        return true;
    }

    return false;
}

bool oe_is_outside_enclave(const void* p, size_t n)
{
    uint64_t rangeStart = (uint64_t)p;
    uint64_t rangeEnd = rangeStart + (n == 0 ? 1 : n);
    uint64_t enclaveStart = (uint64_t)__oe_get_enclave_base();
    uint64_t enclaveEnd = enclaveStart + __oe_get_enclave_size();

    // Disallow nullptr and check that arithmetic operations do not wrap
    // Check that block lies completely outside the enclave
    if ((rangeStart > 0) && (rangeEnd > rangeStart) &&
        (enclaveEnd > enclaveStart) &&
        ((rangeEnd <= enclaveStart) || (rangeStart >= enclaveEnd)))
    {
        return true;
    }

    return false;
}
