// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_ENCLAVE_EC_H
#define _OE_ENCLAVE_EC_H

#include <mbedtls/pk.h>
#include <openenclave/internal/ec.h>

OE_INLINE bool oe_is_ec_key(const mbedtls_pk_context* pk)
{
    return (pk->pk_info == mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
}

oe_result_t oe_ec_public_key_init(
    oe_ec_public_key_t* publicKey,
    const mbedtls_pk_context* pk);

#endif /* _OE_ENCLAVE_EC_H */
