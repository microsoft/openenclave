// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <mbedtls/cmac.h>
#include <mbedtls/config.h>
#include <openenclave/enclave.h>

#include <openenclave/internal/cmac.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/sgxtypes.h>
#include <openenclave/internal/utils.h>

OE_Result OE_AESCMACSign(
    const uint8_t* key,
    uint32_t keySize,
    const uint8_t* message,
    uint32_t messageLength,
    OE_AESCMAC* aesCMAC)
{
    OE_Result result = OE_UNEXPECTED;
    const mbedtls_cipher_info_t* info = NULL;
    uint32_t keySizeBits = keySize * 8;

    if (aesCMAC == NULL)
        OE_RAISE(OE_INVALID_PARAMETER);

    if (keySizeBits != 128)
        OE_RAISE(OE_UNSUPPORTED);

    info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB);
    if (info == NULL)
        OE_RAISE(OE_FAILURE);

    OE_SecureZeroFill(aesCMAC->impl, sizeof(*aesCMAC));

    if (mbedtls_cipher_cmac(
            info,
            key,
            keySizeBits,
            message,
            messageLength,
            (uint8_t*)aesCMAC->impl) != 0)
        OE_RAISE(OE_FAILURE);

    result = OE_OK;

done:
    return result;
}
