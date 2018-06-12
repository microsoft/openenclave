// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/**
 * \file properties.h
 *
 * This file defines enclave-property structures which are injected into
 * the following sections of the enclave image.
 *
 *     .oeinfo - injected by OE_SET_ENCLAVE_SGX (contains
 *               enclave properties with empty sigstructs)
 *
 */

#ifndef _OE_BITS_PROPERTIES_H
#define _OE_BITS_PROPERTIES_H

#include "defs.h"
#include "types.h"

OE_EXTERNC_BEGIN

/* Injected by OE_SET_ENCLAVE_SGX macro and by the signing tool (oesign) */
#define OE_INFO_SECTION_NAME ".oeinfo"

/*
**==============================================================================
**
** oe_enclave_properties_header_t - generic enclave properties base type
**
**==============================================================================
*/

/* Max number of threads in an enclave supported */
#define OE_SGX_MAX_TCS 32

typedef struct _oe_enclave_size_settings
{
    uint64_t numHeapPages;
    uint64_t numStackPages;
    uint64_t numTCS;
} oe_enclave_size_settings_t;

OE_CHECK_SIZE(sizeof(oe_enclave_size_settings_t), 24);

/* Base type for enclave properties */
typedef struct _oe_enclave_properties_header
{
    /* (0) Size of the extended structure */
    uint32_t size;

    /* (4) Enclave type */
    oe_enclave_type_t enclaveType;

    /* (8) Enclave settings */
    oe_enclave_size_settings_t sizeSettings;
} oe_enclave_properties_header_t;

OE_STATIC_ASSERT(sizeof(oe_enclave_type_t) == sizeof(uint32_t));
OE_STATIC_ASSERT(OE_OFFSETOF(oe_enclave_properties_header_t, size) == 0);
OE_STATIC_ASSERT(OE_OFFSETOF(oe_enclave_properties_header_t, enclaveType) == 4);
OE_STATIC_ASSERT(OE_OFFSETOF(oe_enclave_properties_header_t, sizeSettings) == 8);
OE_CHECK_SIZE(sizeof(oe_enclave_properties_header_t), 32);

/*
**==============================================================================
**
** oe_sgx_enclave_properties_t - SGX enclave properties derived type
**
**==============================================================================
*/

#define OE_SGX_FLAGS_DEBUG 0x0000000000000002ULL
#define OE_SGX_FLAGS_MODE64BIT 0x0000000000000004ULL
#define OE_SGX_SIGSTRUCT_SIZE 1808

typedef struct oe_sgx_enclave_config_t
{
    uint16_t productID;
    uint16_t securityVersion;

    /* Padding to make packed and unpacked size the same */
    uint32_t padding;

    /* (OE_SGX_FLAGS_DEBUG | OE_SGX_FLAGS_MODE64BIT) */
    uint64_t attributes;
} oe_sgx_enclave_config_t;

OE_CHECK_SIZE(sizeof(oe_sgx_enclave_config_t), 16);

/* Extends oe_enclave_properties_header_t base type */
typedef struct oe_sgx_enclave_properties_t
{
    /* (0) */
    oe_enclave_properties_header_t header;

    /* (32) */
    oe_sgx_enclave_config_t config;

    /* (48) */
    uint8_t sigstruct[OE_SGX_SIGSTRUCT_SIZE];
} oe_sgx_enclave_properties_t;

OE_CHECK_SIZE(sizeof(oe_sgx_enclave_properties_t), 1856);

/*
**==============================================================================
**
** OE_SET_ENCLAVE_SGX:
**     This macro initializes and injects an oe_sgx_enclave_properties_t struct
**     into the .oeinfo section.
**
**==============================================================================
*/

#define OE_INFO_SECTION_BEGIN __attribute__((section(".oeinfo,\"\",@note#")))
#define OE_INFO_SECTION_END

#define OE_MAKE_ATTRIBUTES(_AllowDebug_) \
    (OE_SGX_FLAGS_MODE64BIT | (_AllowDebug_ ? OE_SGX_FLAGS_DEBUG : 0))

// Note: disable clang-format since it badly misformats this macro
// clang-format off

#define OE_SET_ENCLAVE_SGX(                                             \
    _ProductID_,                                                        \
    _SecurityVersion_,                                                  \
    _AllowDebug_,                                                       \
    _HeapPageCount_,                                                    \
    _StackPageCount_,                                                   \
    _TcsCount_)                                                         \
    OE_INFO_SECTION_BEGIN                                               \
    OE_EXPORT const oe_sgx_enclave_properties_t oe_enclavePropertiesSGX =  \
    {                                                                   \
        .header =                                                       \
        {                                                               \
            .size = sizeof(oe_sgx_enclave_properties_t),                   \
            .enclaveType = OE_ENCLAVE_TYPE_SGX,                         \
            .sizeSettings =                                             \
            {                                                           \
                .numHeapPages = _HeapPageCount_,                        \
                .numStackPages = _StackPageCount_,                      \
                .numTCS = _TcsCount_                                    \
            }                                                           \
        },                                                              \
        .config =                                                       \
        {                                                               \
            .productID = _ProductID_,                                   \
            .securityVersion = _SecurityVersion_,                       \
            .padding = 0,                                               \
            .attributes = OE_MAKE_ATTRIBUTES(_AllowDebug_)              \
        },                                                              \
        .sigstruct =                                                    \
        {                                                               \
            0                                                           \
        }                                                               \
    };                                                                  \
    OE_INFO_SECTION_END

// clang-format on

OE_EXTERNC_END

#endif /* _OE_BITS_PROPERTIES_H */
