// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/**
 * @file properties.h
 *
 * This file defines the SGX properties for an enclave.
 *
 * The enclave properties should only be defined once for all code compiled
 * into an enclave binary using the OE_SET_ENCLAVE_SGX macro.
 * These properties can be overwritten at sign time by the oesign tool.
 */

#ifndef _OE_BITS_PROPERTIES_H
#define _OE_BITS_PROPERTIES_H

#include "defs.h"
#include "types.h"

OE_EXTERNC_BEGIN

/**
 * @cond DEV
 */
/* Injected by OE_SET_ENCLAVE_SGX macro and by the signing tool (oesign) */
#define OE_INFO_SECTION_NAME ".oeinfo"

/* Max number of threads in an enclave supported */
#define OE_SGX_MAX_TCS 32

typedef struct _oe_enclave_size_settings
{
    uint64_t num_heap_pages;
    uint64_t num_stack_pages;
    uint64_t num_tcs;
} oe_enclave_size_settings_t;

/* Base type for enclave properties */
typedef struct _oe_enclave_properties_header
{
    uint32_t size; /**< (0) Size of the extended structure */

    oe_enclave_type_t enclave_type; /**< (4) Enclave type */

    oe_enclave_size_settings_t size_settings; /**< (8) Enclave settings */
} oe_enclave_properties_header_t;

// oe_sgx_enclave_properties_t SGX enclave properties derived type
#define OE_SGX_FLAGS_DEBUG 0x0000000000000002ULL
#define OE_SGX_FLAGS_MODE64BIT 0x0000000000000004ULL
#define OE_SGX_SIGSTRUCT_SIZE 1808

typedef struct oe_sgx_enclave_config_t
{
    uint16_t product_id;
    uint16_t security_version;

    /* Padding to make packed and unpacked size the same */
    uint32_t padding;

    /* (OE_SGX_FLAGS_DEBUG | OE_SGX_FLAGS_MODE64BIT) */
    uint64_t attributes;
} oe_sgx_enclave_config_t;

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

#define OE_INFO_SECTION_BEGIN __attribute__((section(".oeinfo")))
#define OE_INFO_SECTION_END

#define OE_MAKE_ATTRIBUTES(_AllowDebug_) \
    (OE_SGX_FLAGS_MODE64BIT | (_AllowDebug_ ? OE_SGX_FLAGS_DEBUG : 0))

/**
 * @endcond
 */

// This macro initializes and injects an oe_sgx_enclave_properties_t struct
// into the .oeinfo section.

/**
 * Defines the SGX properties for an enclave.
 *
 * The enclave properties should only be defined once for all code compiled into
 * an enclave binary. These properties can be overwritten at sign time by
 * the oesign tool.
 *
 * @param \_ProductID\_ ISV assigned Product ID (ISVPRODID) to use in the
 * enclave signature
 * @param \_SecurityVersion\_ ISV assigned Security Version number (ISVSVN)
 * to use in the enclave signature
 * @param \_AllowDebug\_ If true, allows the enclave to be created with
 * OE_ENCLAVE_FLAG_DEBUG and debugged at runtime
 * @param \_HeapPageCount\_ Number of heap pages to allocate in the enclave
 * @param \_StackPageCount\_ Number of stack pages per thread to reserve in
 * the enclave
 * @param \_TcsCount\_ Number of concurrent threads in an enclave to support
 */
// Note: disable clang-format since it badly misformats this macro
// clang-format off

#define OE_SET_ENCLAVE_SGX(                                               \
    _ProductID_,                                                          \
    _SecurityVersion_,                                                    \
    _AllowDebug_,                                                         \
    _HeapPageCount_,                                                      \
    _StackPageCount_,                                                     \
    _TcsCount_)                                                           \
    OE_INFO_SECTION_BEGIN                                                 \
    OE_EXPORT const oe_sgx_enclave_properties_t oe_enclavePropertiesSGX = \
    {                                                                     \
        .header =                                                         \
        {                                                                 \
            .size = sizeof(oe_sgx_enclave_properties_t),                  \
            .enclave_type = OE_ENCLAVE_TYPE_SGX,                          \
            .size_settings =                                              \
            {                                                             \
                .num_heap_pages = _HeapPageCount_,                        \
                .num_stack_pages = _StackPageCount_,                      \
                .num_tcs = _TcsCount_                                     \
            }                                                             \
        },                                                                \
        .config =                                                         \
        {                                                                 \
            .product_id = _ProductID_,                                    \
            .security_version = _SecurityVersion_,                        \
            .padding = 0,                                                 \
            .attributes = OE_MAKE_ATTRIBUTES(_AllowDebug_)                \
        },                                                                \
        .sigstruct =                                                      \
        {                                                                 \
            0                                                             \
        }                                                                 \
    };                                                                    \
    OE_INFO_SECTION_END

// clang-format on

OE_EXTERNC_END

#endif /* _OE_BITS_PROPERTIES_H */
