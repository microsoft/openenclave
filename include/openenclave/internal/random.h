// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_RANDOM_H
#define _OE_RANDOM_H

#include <openenclave/bits/result.h>
#include <openenclave/bits/types.h>

OE_EXTERNC_BEGIN

/**
 * Generate a sequence of random bytes.
 *
 * This function generates a sequence of random bytes.
 *
 * @param data the buffer that will be filled with random bytes
 * @param size the size of the buffer
 *
 * @return OE_OK on success
 */
oe_result_t oe_random(void* data, size_t size);

OE_EXTERNC_END

#endif /* _OE_RANDOM_H */
