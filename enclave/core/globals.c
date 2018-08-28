// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/globals.h>

/* Note: The variables below are initialized during enclave loading */

/*
**==============================================================================
**
** Enclave boundaries:
**
**==============================================================================
*/

OE_EXPORT uint64_t __oe_numPages;
OE_EXPORT uint64_t __oe_virtualBaseAddr;

const void* __oe_get_enclave_base()
{
    /*
     * Note: The reference to &__oe_virtualBaseAddr will be compiled
     * IP-relative by the C-compiler on x86_64, and hence does not have a
     * relocation entry. Thus it works both pre- and post-relocation.
     */
    return (uint8_t*)&__oe_virtualBaseAddr - __oe_virtualBaseAddr;
}

size_t __oe_get_enclave_size()
{
    return __oe_numPages * OE_PAGE_SIZE;
}

/*
**==============================================================================
**
** Reloc boundaries:
**
**==============================================================================
*/

OE_EXPORT uint64_t __oe_baseRelocPage;
OE_EXPORT uint64_t __oe_numRelocPages;

const void* __oe_get_reloc_base()
{
    const unsigned char* base = __oe_get_enclave_base();

    return base + (__oe_baseRelocPage * OE_PAGE_SIZE);
}

const void* __oe_get_reloc_end()
{
    return (const uint8_t*)__oe_get_reloc_base() + __oe_get_reloc_size();
}

const size_t __oe_get_reloc_size()
{
    return __oe_numRelocPages * OE_PAGE_SIZE;
}

/*
**==============================================================================
**
** ECall boundaries:
**
**==============================================================================
*/

OE_EXPORT uint64_t __oe_baseECallPage;
OE_EXPORT uint64_t __oe_numECallPages;

const void* __oe_get_ecall_base()
{
    const unsigned char* base = __oe_get_enclave_base();

    return base + (__oe_baseECallPage * OE_PAGE_SIZE);
}

const void* __oe_get_ecall_end()
{
    return (const uint8_t*)__oe_get_ecall_base() + __oe_get_ecall_size();
}

const size_t __oe_get_ecall_size()
{
    return __oe_numECallPages * OE_PAGE_SIZE;
}

/*
**==============================================================================
**
** Heap boundaries:
**
**==============================================================================
*/

OE_EXPORT uint64_t __oe_baseHeapPage;
OE_EXPORT uint64_t __oe_numHeapPages;

const void* __oe_get_heap_base()
{
    const unsigned char* base = __oe_get_enclave_base();

    return base + (__oe_baseHeapPage * OE_PAGE_SIZE);
}

const size_t __oe_get_heap_size()
{
    return __oe_numHeapPages * OE_PAGE_SIZE;
}

const void* __oe_get_heap_end()
{
    return (const uint8_t*)__oe_get_heap_base() + __oe_get_heap_size();
}

/*
**==============================================================================
**
** oe_enclave:
**
**     The enclave handle obtained with oe_create_enclave() and passed
**     to the enclave during initialization (via OE_ECALL_INIT_ENCLAVE).
**
**==============================================================================
*/

oe_enclave_t* oe_enclave;

oe_enclave_t* oe_get_enclave(void)
{
    return oe_enclave;
}
