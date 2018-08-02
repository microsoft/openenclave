// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_COMMON_REVOCATION_H
#define _OE_COMMON_REVOCATION_H

#include <openenclave/bits/defs.h>
#include <openenclave/bits/result.h>
#include <openenclave/bits/types.h>
#include <openenclave/internal/cert.h>

OE_EXTERNC_BEGIN

#ifdef OE_USE_LIBSGX

oe_result_t oe_enforce_revocation(
    oe_cert_t* intermediate_cert,
    oe_cert_t* leaf_cert);

#endif

OE_EXTERNC_END

#endif // _OE_COMMON_REVOCATION_H
