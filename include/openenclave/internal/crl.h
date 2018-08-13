// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_CRL_H
#define _OE_CRL_H

#include <openenclave/bits/result.h>
#include <openenclave/bits/types.h>
#include <openenclave/internal/date.h>

OE_EXTERNC_BEGIN

typedef struct _oe_crl
{
    /* Internal private implementation */
    uint64_t impl[4];
} oe_crl_t;

typedef struct _oe_crl oe_crl_t;

/**
 * Read a certificate revocation list (CRL) from DER format.
 *
 * The caller is responsible for releasing the certificate by passing it to
 * oe_crl_free().
 *
 * @param derData zero-terminated DER data.
 * @param derSize size of the DER data
 * @param crl initialized certificate handle upon return
 *
 * @return OE_OK load was successful
 */
oe_result_t oe_crl_read_der(
    oe_crl_t* crl,
    const uint8_t* der_data,
    size_t der_size);

/**
 * Releases a certificate revocation list (CRL)
 *
 * @param crl handle of the CRL being released
 *
 * @return OE_OK the CRL was successfully released
 */
oe_result_t oe_crl_free(oe_crl_t* crl);

oe_result_t oe_crl_get_update_dates(
    const oe_crl_t* crl,
    oe_date_t* last,
    oe_date_t* next);

OE_EXTERNC_END

#endif /* _OE_CRL_H */
