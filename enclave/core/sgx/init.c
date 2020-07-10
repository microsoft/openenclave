// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include "init.h"
#include <openenclave/advanced/allocator.h>
#include <openenclave/bits/eeid.h>
#include <openenclave/bits/sgx/sgxtypes.h>
#include <openenclave/corelibc/string.h>
#include <openenclave/enclave.h>
#include <openenclave/internal/calls.h>
#include <openenclave/internal/eeid.h>
#include <openenclave/internal/fault.h>
#include <openenclave/internal/globals.h>
#include <openenclave/internal/jump.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/thread.h>
#include "asmdefs.h"
#include "td.h"

/*
**==============================================================================
**
** _check_memory_boundaries()
**
**     Check that the variables in globals.h are actually within the enclave.
**
**==============================================================================
*/

static void _check_memory_boundaries(void)
{
    /* This is a tautology! */
    if (!oe_is_within_enclave(__oe_get_enclave_base(), __oe_get_enclave_size()))
        oe_abort();

    if (!oe_is_within_enclave(__oe_get_reloc_base(), __oe_get_reloc_size()))
        oe_abort();

    if (!oe_is_within_enclave(__oe_get_heap_base(), __oe_get_heap_size()))
        oe_abort();
}

#ifdef OE_WITH_EXPERIMENTAL_EEID
extern volatile const oe_sgx_enclave_properties_t oe_enclave_properties_sgx;
extern oe_eeid_t* oe_eeid;

static void _find_eeid()
{
    /* Get the last page before the heap. */
    uint8_t* heap_base = (uint8_t*)__oe_get_heap_base();
    oe_eeid_page_t* eeid_page = (oe_eeid_page_t*)(heap_base - OE_PAGE_SIZE);
    if (!oe_is_within_enclave(eeid_page, OE_PAGE_SIZE))
        oe_abort();
    /* Without EEID, eeid_page page is all zero. */
    if (eeid_page->eeid.version != 0)
    {
        if (eeid_page->eeid.version != OE_EEID_VERSION)
            oe_abort();
        oe_eeid = &eeid_page->eeid;
    }
}
#endif

static void _initialize_enclave_image()
{
    /* Relocate symbols */
    if (!oe_apply_relocations())
    {
        oe_abort();
    }

#ifdef OE_WITH_EXPERIMENTAL_EEID
    _find_eeid();
#endif

    /* Check that memory boundaries are within enclave */
    _check_memory_boundaries();
}

static oe_once_t _enclave_initialize_once;

static void _initialize_enclave_imp(void)
{
    _initialize_enclave_image();
}

/*
**==============================================================================
**
** oe_initialize_enclave()
**
**     This function is called the first time the enclave is entered. It
**     performs any necessary enclave initialization, such as applying
**     relocations, initializing exception etc.
**
**==============================================================================
*/
void oe_initialize_enclave()
{
    oe_once(&_enclave_initialize_once, _initialize_enclave_imp);
}
