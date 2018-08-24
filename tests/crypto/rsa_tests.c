// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#if defined(OE_BUILD_ENCLAVE)
#include <openenclave/enclave.h>
#endif

#include <openenclave/internal/cert.h>
#include <openenclave/internal/rsa.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "tests.h"

/* Generated with: openssl genrsa -out private.pem -3 3072 */
static const char _PRIVATE_KEY[] =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "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\n"
    "-----END RSA PRIVATE KEY-----\n";

/* Generated with: openssl rsa -in private.pem -pubout -out public.pem */
static const char _PUBLIC_KEY[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MIIBoDANBgkqhkiG9w0BAQEFAAOCAY0AMIIBiAKCAYEAyXKod6+poFfDaMIT6dKe\n"
    "zZG/OeYSSvo2Nw7ufOtBCUrNaXOc5jIi1swpSASOx9nm3aYNeUGCPjDPads+YkSs\n"
    "o8Bmm4HzAR51/4rSt5/fjD5PVMpM4GozwbXSCsK8TjgVcQ3pthk16GgVpL0k1+G4\n"
    "abe+RYZKrS3KFwAT4Uk1ic+NcafKPwRo5uEs2YjX3aj0DiA/O3n95zMQoxsqwgbZ\n"
    "7dzhLfw0gkljr+Bj07Z+w/RUTYFo0OJ/QwW8A5iT5DxOaVvuf2hjbf8462xivQyv\n"
    "Ph9RH15ARvDYrMzdbDwW6Xpz7c+g61+AFtl64wYJDvD6nIkimyjYEcJRLtLeEdds\n"
    "ySqxPVv1Ki9KMlLFwKNnm8J8Pw5KeZ29YucZP95tPOXcPv6tx9/Sx8W21FNPzKrg\n"
    "oxlRJE9M8TDwh+qIxiwy9ComvBat1/px0fFwcSM+3MYG5s0TCsLp56mkn7aiv09e\n"
    "sqkFL5inQfbToVbB8FDxu0o9VUVax6A3V+xJh0yGeD/RAgED\n"
    "-----END PUBLIC KEY-----\n";

/* Signature of ALPHABET_HASH using _PRIVATE_KEY */
static const uint8_t _SIGNATURE[] = {
    0x65, 0x98, 0x25, 0xBA, 0xE8, 0x08, 0xAF, 0x10, 0xC0, 0xAC, 0xDF, 0xAB,
    0x83, 0x65, 0x8D, 0x6E, 0xE8, 0x36, 0x42, 0xAC, 0x01, 0x4B, 0x5D, 0x9E,
    0xF0, 0x26, 0xB3, 0x25, 0x46, 0x4D, 0xD3, 0xC7, 0xB6, 0x4D, 0xA3, 0xCB,
    0xC6, 0x51, 0x12, 0xD5, 0xC9, 0x14, 0x35, 0xB1, 0x7A, 0x3C, 0xD6, 0x19,
    0xF2, 0x7F, 0xD4, 0x4B, 0x1C, 0x66, 0x56, 0xC4, 0xFE, 0x06, 0xDB, 0x86,
    0xDF, 0x69, 0x87, 0x8D, 0x47, 0x1F, 0x69, 0x74, 0xCA, 0xF6, 0x16, 0x3F,
    0x81, 0xF7, 0xD5, 0x34, 0xBB, 0xEE, 0xE1, 0xA9, 0x7E, 0x06, 0xCF, 0x21,
    0x44, 0x76, 0x83, 0xA1, 0xF1, 0x56, 0xC6, 0x06, 0x07, 0xFE, 0xA1, 0x19,
    0x66, 0xF1, 0xAB, 0x4E, 0xFC, 0xFD, 0xC9, 0x6B, 0x2D, 0x29, 0x6E, 0xE4,
    0xBA, 0x56, 0xD6, 0x65, 0x88, 0xA4, 0x8B, 0x38, 0x41, 0xC6, 0x50, 0x38,
    0xAB, 0x1D, 0x61, 0xB9, 0x12, 0xCE, 0x00, 0x45, 0x5B, 0x52, 0xC7, 0xE2,
    0x91, 0x3A, 0xAE, 0xC3, 0xAC, 0xC9, 0x6F, 0xBD, 0xB2, 0x0D, 0xCC, 0xAF,
    0xD8, 0x58, 0xC8, 0x38, 0x02, 0xEA, 0xA5, 0xD4, 0x9C, 0x80, 0x9F, 0xA3,
    0x8C, 0xCE, 0x78, 0xA4, 0xE5, 0xFB, 0x0D, 0xF0, 0x1B, 0xF6, 0x58, 0xBB,
    0xF0, 0xC8, 0xEB, 0x2E, 0x97, 0x47, 0x31, 0x5E, 0xDA, 0xDD, 0x2F, 0x1E,
    0x14, 0x07, 0x91, 0xB0, 0x01, 0xB8, 0x00, 0xF3, 0xD9, 0xD1, 0x94, 0xAC,
    0x46, 0x84, 0x45, 0x99, 0x70, 0x59, 0x4B, 0x01, 0x32, 0x20, 0x11, 0x23,
    0x01, 0x6E, 0xC7, 0xD8, 0x3E, 0x37, 0xC5, 0xB1, 0x03, 0xA6, 0xE6, 0xC4,
    0x0D, 0x36, 0x8C, 0x0A, 0xCA, 0xC1, 0xF1, 0x9D, 0xF7, 0xF5, 0xCD, 0x92,
    0x60, 0x5B, 0x31, 0x94, 0x3B, 0xA4, 0xBD, 0xEE, 0xD3, 0xE4, 0x2F, 0xF5,
    0x8F, 0x61, 0x10, 0x13, 0xE5, 0xE3, 0x5A, 0x58, 0xC7, 0x36, 0x4D, 0xBD,
    0x00, 0x85, 0x54, 0x34, 0x34, 0x7E, 0x05, 0x01, 0xFE, 0xE8, 0x76, 0xB9,
    0x20, 0x09, 0x5F, 0x37, 0xF7, 0x38, 0x57, 0xE0, 0x35, 0xB6, 0x62, 0xD2,
    0x1B, 0x23, 0xE0, 0xCC, 0xE1, 0x1A, 0x29, 0x3D, 0x76, 0x18, 0xDB, 0x6A,
    0x7A, 0x9A, 0x08, 0xD4, 0x2E, 0x38, 0x6C, 0x0F, 0xE2, 0x93, 0x2E, 0x5B,
    0x4B, 0xE4, 0x88, 0x72, 0xA8, 0x0D, 0x3E, 0xD3, 0x4A, 0xCA, 0x1E, 0xDB,
    0x5C, 0x39, 0x88, 0x57, 0xBD, 0x53, 0xCB, 0x1C, 0x60, 0xC2, 0xAD, 0x8B,
    0x59, 0x1F, 0x3E, 0xC0, 0xC1, 0x6C, 0x3C, 0x1A, 0x5A, 0x72, 0xE1, 0x86,
    0xAB, 0xA3, 0xB3, 0x56, 0xA8, 0x0D, 0xB1, 0xAA, 0x1F, 0xB8, 0x15, 0x86,
    0x52, 0xBF, 0x24, 0x8B, 0xD4, 0x52, 0x26, 0x92, 0x2C, 0xCC, 0x90, 0x86,
    0x62, 0x9D, 0x1C, 0x09, 0x85, 0xEB, 0x2C, 0x11, 0xCD, 0xDF, 0xD1, 0xDA,
    0x68, 0xA8, 0x08, 0xE8, 0xF1, 0x10, 0xFB, 0x78, 0xDA, 0x7E, 0xB3, 0x56,
};

static const size_t _SIGNATURE_SIZE = sizeof(_SIGNATURE);

// Test RSA signing operation over an ASCII alphabet string.
static void _TestSign()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_rsa_private_key_t key = {0};
    uint8_t* signature = NULL;
    size_t signatureSize = 0;

    r = oe_rsa_private_key_read_pem(
        &key, (const uint8_t*)_PRIVATE_KEY, sizeof(_PRIVATE_KEY));
    OE_TEST(r == OE_OK);

    r = oe_rsa_private_key_sign(
        &key,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        &signatureSize);
    OE_TEST(r == OE_BUFFER_TOO_SMALL);

    OE_TEST(signature = (uint8_t*)malloc(signatureSize));

    r = oe_rsa_private_key_sign(
        &key,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        &signatureSize);
    OE_TEST(r == OE_OK);

    OE_TEST(signatureSize == _SIGNATURE_SIZE);
    OE_TEST(memcmp(signature, &_SIGNATURE, _SIGNATURE_SIZE) == 0);

    oe_rsa_private_key_free(&key);
    free(signature);

    printf("=== passed %s()\n", __FUNCTION__);
}

// Test RSA verify operation over an ASCII alphabet string.
static void _TestVerify()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_rsa_public_key_t key = {0};

    r = oe_rsa_public_key_read_pem(
        &key, (const uint8_t*)_PUBLIC_KEY, sizeof(_PUBLIC_KEY));
    OE_TEST(r == OE_OK);

    r = oe_rsa_public_key_verify(
        &key,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        _SIGNATURE,
        _SIGNATURE_SIZE);
    OE_TEST(r == OE_OK);

    oe_rsa_public_key_free(&key);

    printf("=== passed %s()\n", __FUNCTION__);
}

/* Certificate signed by CHAIN1 */
static const char _CERT1[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDMzCCAhsCAhABMA0GCSqGSIb3DQEBCwUAMGMxGjAYBgNVBAMMEVRlc3QgSW50\n"
    "ZXJtZWRpYXRlMQ4wDAYDVQQIDAVUZXhhczELMAkGA1UEBhMCVVMxEjAQBgNVBAoM\n"
    "CU1pY3Jvc29mdDEUMBIGA1UECwwLT3BlbkVuY2xhdmUwHhcNMTgwMjEzMTc1MjUz\n"
    "WhcNMTkwMjEzMTc1MjUzWjBbMRIwEAYDVQQDDAlUZXN0IExlYWYxDjAMBgNVBAgM\n"
    "BVRleGFzMQswCQYDVQQGEwJVUzESMBAGA1UECgwJTWljcm9zb2Z0MRQwEgYDVQQL\n"
    "DAtPcGVuRW5jbGF2ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAOjL\n"
    "A0tUP/Sw+L9KowKL94PJe2Bk9u0YeeRa0z1PyIoLVE3KCeOLQueo7gQwah0s/ZA1\n"
    "53lggkyt3VjMOUC5FBS5hy79VcoInrrS9DG8PtZBk3AobDcUBNipWIJ5lofijppi\n"
    "uRFfr4HtMN9TYJhfWnau7puep5X/HeW0k3/Hox8+R6Gdu74QkTILVrDh6EcXzLUv\n"
    "XXFu0bi/pDhoBeW+HGxK8ot+wjKt/NjnYc3KlrNQVDzBDEpXx5enWFbow37O6Rab\n"
    "+iHCkvOYvJe1tgJTpI65Qi688Xc3/NFzZ3lA3PET+xKjjzBS1wHrumCu9L3ugJJ3\n"
    "ZVHwHlDQ9u9qTRHlGYcCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAlP9O97ydoazt\n"
    "w4oGluwo3Wef9O2Nx6OhNqY+lrCx/KkdBHVqGLaveo6UDlkRQLydyx55ekrMdatG\n"
    "UyzFm6JTAh29R7ocTWdERmNLQNR1yQFCr0JJ1yPHucikY7ubD0iIxlAliPKPsH/S\n"
    "t4pff8GRRrv5+jCON6zT2lX+ZVOCwyolu5oZWFI6iWy6JldYdaHhmiy3gP/F2abr\n"
    "NASwM79RRO+JGskwgswboXp8Tg83jzdbSe6DL6LfK0UgpeEr3QtNhDMkw7KY1oXs\n"
    "7WxpjlnJCyCkAW0c5+Hh2WgZLwYXcfRXer6WuugAz6WPayLDsHf0ZqiuiVjkbS1l\n"
    "ln6O0i8HeQ==\n"
    "-----END CERTIFICATE-----\n";

/* RSA modulus of _CERT1 */
static const char _CERT1_RSA_MODULUS[] = {
    0xE8, 0xCB, 0x03, 0x4B, 0x54, 0x3F, 0xF4, 0xB0, 0xF8, 0xBF, 0x4A, 0xA3,
    0x02, 0x8B, 0xF7, 0x83, 0xC9, 0x7B, 0x60, 0x64, 0xF6, 0xED, 0x18, 0x79,
    0xE4, 0x5A, 0xD3, 0x3D, 0x4F, 0xC8, 0x8A, 0x0B, 0x54, 0x4D, 0xCA, 0x09,
    0xE3, 0x8B, 0x42, 0xE7, 0xA8, 0xEE, 0x04, 0x30, 0x6A, 0x1D, 0x2C, 0xFD,
    0x90, 0x35, 0xE7, 0x79, 0x60, 0x82, 0x4C, 0xAD, 0xDD, 0x58, 0xCC, 0x39,
    0x40, 0xB9, 0x14, 0x14, 0xB9, 0x87, 0x2E, 0xFD, 0x55, 0xCA, 0x08, 0x9E,
    0xBA, 0xD2, 0xF4, 0x31, 0xBC, 0x3E, 0xD6, 0x41, 0x93, 0x70, 0x28, 0x6C,
    0x37, 0x14, 0x04, 0xD8, 0xA9, 0x58, 0x82, 0x79, 0x96, 0x87, 0xE2, 0x8E,
    0x9A, 0x62, 0xB9, 0x11, 0x5F, 0xAF, 0x81, 0xED, 0x30, 0xDF, 0x53, 0x60,
    0x98, 0x5F, 0x5A, 0x76, 0xAE, 0xEE, 0x9B, 0x9E, 0xA7, 0x95, 0xFF, 0x1D,
    0xE5, 0xB4, 0x93, 0x7F, 0xC7, 0xA3, 0x1F, 0x3E, 0x47, 0xA1, 0x9D, 0xBB,
    0xBE, 0x10, 0x91, 0x32, 0x0B, 0x56, 0xB0, 0xE1, 0xE8, 0x47, 0x17, 0xCC,
    0xB5, 0x2F, 0x5D, 0x71, 0x6E, 0xD1, 0xB8, 0xBF, 0xA4, 0x38, 0x68, 0x05,
    0xE5, 0xBE, 0x1C, 0x6C, 0x4A, 0xF2, 0x8B, 0x7E, 0xC2, 0x32, 0xAD, 0xFC,
    0xD8, 0xE7, 0x61, 0xCD, 0xCA, 0x96, 0xB3, 0x50, 0x54, 0x3C, 0xC1, 0x0C,
    0x4A, 0x57, 0xC7, 0x97, 0xA7, 0x58, 0x56, 0xE8, 0xC3, 0x7E, 0xCE, 0xE9,
    0x16, 0x9B, 0xFA, 0x21, 0xC2, 0x92, 0xF3, 0x98, 0xBC, 0x97, 0xB5, 0xB6,
    0x02, 0x53, 0xA4, 0x8E, 0xB9, 0x42, 0x2E, 0xBC, 0xF1, 0x77, 0x37, 0xFC,
    0xD1, 0x73, 0x67, 0x79, 0x40, 0xDC, 0xF1, 0x13, 0xFB, 0x12, 0xA3, 0x8F,
    0x30, 0x52, 0xD7, 0x01, 0xEB, 0xBA, 0x60, 0xAE, 0xF4, 0xBD, 0xEE, 0x80,
    0x92, 0x77, 0x65, 0x51, 0xF0, 0x1E, 0x50, 0xD0, 0xF6, 0xEF, 0x6A, 0x4D,
    0x11, 0xE5, 0x19, 0x87,
};

/* RSA exponent of CERT */
static const char _CERT_RSA_EXPONENT[] = {0x01, 0x00, 0x01};

#define CHAIN1                                                           \
    "-----BEGIN CERTIFICATE-----\n"                                      \
    "MIIDQjCCAioCAhAAMA0GCSqGSIb3DQEBCwUAMGoxCzAJBgNVBAYTAlVTMQ4wDAYD\n" \
    "VQQIDAVUZXhhczEPMA0GA1UEBwwGQXVzdGluMRIwEAYDVQQKDAlNaWNyb3NvZnQx\n" \
    "FDASBgNVBAsMC09wZW5FbmNsYXZlMRAwDgYDVQQDDAdUZXN0IENBMB4XDTE4MDIx\n" \
    "MzE3NTI1M1oXDTE5MDIxMzE3NTI1M1owYzEaMBgGA1UEAwwRVGVzdCBJbnRlcm1l\n" \
    "ZGlhdGUxDjAMBgNVBAgMBVRleGFzMQswCQYDVQQGEwJVUzESMBAGA1UECgwJTWlj\n" \
    "cm9zb2Z0MRQwEgYDVQQLDAtPcGVuRW5jbGF2ZTCCASIwDQYJKoZIhvcNAQEBBQAD\n" \
    "ggEPADCCAQoCggEBALtrsdnSA0135a5Ry0BlAoiCuvMS4ttmKrG5aNqq1QtXT0rr\n" \
    "eeVs7uCu43DeuBjoPDCe113LIse68o0VfpyGoEB4k5IEftwNuqt/s9YPnpCjIPw0\n" \
    "tJIhf/0iNo2HXDK7KVkHnjPTcS1KVlWrnVABP7+Q80nBj7S9yxv7LMuKEzU/NSgG\n" \
    "TZZWbIDh8DnTvG2YCz9XAtPjBjWAICDOh4Hlkujt2Z/9D3/At3yL577VBBngZG0A\n" \
    "wCJYE+SpibP8d/f0tuFa6vhNzsDxwqYmBLU1CL1G+3brlCTgfSUCzxC/RSu5lJGo\n" \
    "fZ7E/0s1+kY07p7iiLCgMOJCBIUE+NP2aX8WSRsCAwEAATANBgkqhkiG9w0BAQsF\n" \
    "AAOCAQEAWHmQ3uAggLQY/G0Idzk+HhxRArgLXvd4nAlELlArngTGviuZBdcjQhIA\n" \
    "Do/5GUcoBHLI3Q7lspbT9kbn+2664Gm5jh7A9OU/kpFpMQT+e6aTUDPTlLuHygpx\n" \
    "bP4X6hn+D5QEBct/befUKcAMZGya46N+m+qWJbH+fplkA2zuZ4NdXav21BsqXRer\n" \
    "JN6pIxewAEhYGt8nd5mjRFJnMfb6IWs49TRxvk7SntVrWktW36TxPTTWsusuCd8v\n" \
    "RGTsgD4AIHqFRVi+e+y32K9xxUL4f27s6wSu+f2z2oAQsrfuhHyFMUZT4NLs4KAk\n" \
    "QbVRkhj9vYJrIP8rRJ+XV9j/IvfdjQ==\n"                                 \
    "-----END CERTIFICATE-----\n"                                        \
    "-----BEGIN CERTIFICATE-----\n"                                      \
    "MIIDUDCCAjgCCQDNkOt+e8l1aDANBgkqhkiG9w0BAQsFADBqMQswCQYDVQQGEwJV\n" \
    "UzEOMAwGA1UECAwFVGV4YXMxDzANBgNVBAcMBkF1c3RpbjESMBAGA1UECgwJTWlj\n" \
    "cm9zb2Z0MRQwEgYDVQQLDAtPcGVuRW5jbGF2ZTEQMA4GA1UEAwwHVGVzdCBDQTAe\n" \
    "Fw0xODAyMTMxNzUyNTJaFw0yMzAyMTMxNzUyNTJaMGoxCzAJBgNVBAYTAlVTMQ4w\n" \
    "DAYDVQQIDAVUZXhhczEPMA0GA1UEBwwGQXVzdGluMRIwEAYDVQQKDAlNaWNyb3Nv\n" \
    "ZnQxFDASBgNVBAsMC09wZW5FbmNsYXZlMRAwDgYDVQQDDAdUZXN0IENBMIIBIjAN\n" \
    "BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlrzZNtYmicG/Z0ZK0qYzmTyur/n4\n" \
    "94fkDBklgsHb9inbe455MgUsIcrlwE/1qXbcO0SnQmUjmopE2iYJhDVDDnobgDlP\n" \
    "iO1o8E7tIW/jzxRStDyfrO6uOrc8mUAvnsHc800d2Dt6vAHhdbiBeIi8CkIgy5/h\n" \
    "XXQphjk9H0jrVFydJ7KYl53KvwZ95NAodPZpiuCD6WXDEvXOLvQaup2nG9+ha5QB\n" \
    "h0pfdNRhHtsfIcg5ExghoOYXcOhCtEqkVYcllmYr2tf3tn/dRVXO7KYbfqr/mQo+\n" \
    "oPfhziAyyKHxx7a6bfDngV8ORI7q9b774VgZd3dTDDYechYTIH5V4pX61QIDAQAB\n" \
    "MA0GCSqGSIb3DQEBCwUAA4IBAQBbCNHoUIVpUnLhrugyooDCygYQeTebVILY1DHG\n" \
    "Kj7GEpMK70suXQlJ7/hbuL8jTA/kHMtHARy+9DAQDjiWRfRNOpE4eEbUdEiwei4L\n" \
    "2tPke58FBxkq6GcpldPBin16ux379zM43vYwhEf9yuY6KwBfaABg6Eeftrpcuvt3\n" \
    "1Ibbn4oM2MZixhZXTkKsB3O1OlaZ/kw6qPt7LklLhij1SmvtrrlkWGWoAg8JWYh+\n" \
    "+Wd1yIgPZbbio9b4rgQ6j0mpWaRVPm7cKAt3nfhnMgr1QV+RQnPDTlsO+sieCLZm\n" \
    "Z/7gtSYO0BbJnQu3dXzP0OBD1SQvQOpjWXwC71ioBn1rbqDl\n"                 \
    "-----END CERTIFICATE-----\n"

#define CHAIN2                                                           \
    "-----BEGIN CERTIFICATE-----\n"                                      \
    "MIIDQjCCAioCAhAAMA0GCSqGSIb3DQEBCwUAMGoxCzAJBgNVBAYTAlVTMQ4wDAYD\n" \
    "VQQIDAVUZXhhczEPMA0GA1UEBwwGQXVzdGluMRIwEAYDVQQKDAlNaWNyb3NvZnQx\n" \
    "FDASBgNVBAsMC09wZW5FbmNsYXZlMRAwDgYDVQQDDAdUZXN0IENBMB4XDTE4MDIx\n" \
    "MzAxNDAwOFoXDTE5MDIxMzAxNDAwOFowYzEaMBgGA1UEAwwRVGVzdCBJbnRlcm1l\n" \
    "ZGlhdGUxDjAMBgNVBAgMBVRleGFzMQswCQYDVQQGEwJVUzESMBAGA1UECgwJTWlj\n" \
    "cm9zb2Z0MRQwEgYDVQQLDAtPcGVuRW5jbGF2ZTCCASIwDQYJKoZIhvcNAQEBBQAD\n" \
    "ggEPADCCAQoCggEBALlie2zjCfdy1fckjuM7kXf7CkyVp+hqhkJMUU80ETP8AM2R\n" \
    "JsJ2Xkvdf8GrIl93lQU9f9O+HRl45O2M/zz2ZTucDF6dWvJ4yCDUWOwE2ro8eh3d\n" \
    "0BRJvzjs/wgt10v5KJaHnMu9wYVn/8gLWOS/qLwVnPxzqQ507GwOdNaOSHiWXYc9\n" \
    "fl062HLXirLaKMPx5ZD2QlXDPYQta+lbTHWDReelDNHT3G8FtiKtzQ0uT2EWLw7z\n" \
    "dOu61+EGf2PdqZc+2MVTPca/qO+cCSwzdzGzOQgUYmtn6YUN1y/GJv73AbnPazvY\n" \
    "fVysQanSa1g+LD9WFjF4qInlTnCvjvKQEeqGiTsCAwEAATANBgkqhkiG9w0BAQsF\n" \
    "AAOCAQEAmABx/BVWs4w8bfU0ce5Yj9RYWziMilhrEZJgCmSQzNlNO7DNcPthBbNi\n" \
    "OAgd8y+lSpcHk5KJ9tm9rD6G+0RbL+8M1TyagrI0M++7Ex2gZV9DSEtvBqnl8XgS\n" \
    "tlb2xW0x2jol9MdQrsaCcORbrEnogP8YGzICMoJQ8OiJed99PS7q+eQ0lW9A7e+o\n" \
    "XJ9PjI7n93Wou0xC4kYOszGPGIZ9X5mEPAKqqCcXsxGfruDrgmulB526hb/lHC/5\n" \
    "1gRic7SCYsyWgxde7R5D+IxxcJNnIWNnt3TUIT8I9fbwonddxA3Qln9tr784dDiB\n" \
    "/c5qUrfE7k0DrKr4OZvt/xbV9oKMyg==\n"                                 \
    "-----END CERTIFICATE-----\n"                                        \
    "-----BEGIN CERTIFICATE-----\n"                                      \
    "MIIDUDCCAjgCCQCCncn7BEtWSzANBgkqhkiG9w0BAQsFADBqMQswCQYDVQQGEwJV\n" \
    "UzEOMAwGA1UECAwFVGV4YXMxDzANBgNVBAcMBkF1c3RpbjESMBAGA1UECgwJTWlj\n" \
    "cm9zb2Z0MRQwEgYDVQQLDAtPcGVuRW5jbGF2ZTEQMA4GA1UEAwwHVGVzdCBDQTAe\n" \
    "Fw0xODAyMTMwMTQwMDhaFw0yMzAyMTMwMTQwMDhaMGoxCzAJBgNVBAYTAlVTMQ4w\n" \
    "DAYDVQQIDAVUZXhhczEPMA0GA1UEBwwGQXVzdGluMRIwEAYDVQQKDAlNaWNyb3Nv\n" \
    "ZnQxFDASBgNVBAsMC09wZW5FbmNsYXZlMRAwDgYDVQQDDAdUZXN0IENBMIIBIjAN\n" \
    "BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAwSnztYAKE2Mq5XL2dCZsFGKsMjsr\n" \
    "pIK37PoBEPJsphL8jrpbRtYklp5HpIpcakT0poL34p90xaW+bIthzbU/MOC/2A/0\n" \
    "63WS4v4n2h33JaMoGVQ4qt+4sV6sZwG70ifbG6linQQAfhdzt+7hxizLr0sh+gys\n" \
    "/E5qkik87pwb2NxDc4tO5vybq50AKf+UVBC5/f//YD0LrXYafUVEruwsCj9fAjso\n" \
    "vAhPqn4nVylOwuNBCjXGkLNF/Rxsi25Cb8rX0rMS1/evjTaTveMRtVYJP4OnDuWL\n" \
    "SzksoGwC4D4hb2SNB+QEMyGv1SnrrmfEXdN/RKf3pCtFJigskcxy/3jgnwIDAQAB\n" \
    "MA0GCSqGSIb3DQEBCwUAA4IBAQABen8yc7HPFjqi+xuiwh+3YcVOYOd5R+gVdsPm\n" \
    "1VLcetXOVx1aRjHF9jwkF3GzQ5UbKex9MSiScjDjLV9ukmJD+HjMwAm3W/Rex+rY\n" \
    "Y7bM6uvKN5zxs5SeuO7odkYP2jHlSnozMJt7jMENr2sJUscIrn073Z3b0gLcv/Cb\n" \
    "QKQY5OSmyQYYu7ib14SxcpIjdZi1T/PH8hlKaKsdt+OFTzA3t8VdN69jZSubwP+A\n" \
    "gBmiW1o/q+r+pN3woaClk5c0/Dh9t/xJcNth4NSKV/YrLHQLZQ76SvICtKexz1RZ\n" \
    "wZSQ03dKw87xW3t3f5GS3O75zDNeT4TMeYd4RMRiK6wDt2WE\n"                 \
    "-----END CERTIFICATE-----\n"

/* This chain is a concatenation of two unrelated chains: CHAIN1 and CHAIN2 */
#define MIXED_CHAIN CHAIN1 CHAIN2

static void _TestCertVerifyGood()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_verify_cert_error_t error = {0};
    oe_cert_t cert = {0};
    oe_cert_chain_t chain = {0};
    oe_crl_t* crl = NULL;

    r = oe_cert_read_pem(&cert, _CERT1, sizeof(_CERT1));
    OE_TEST(r == OE_OK);

    r = oe_cert_chain_read_pem(&chain, CHAIN1, sizeof(CHAIN1));
    OE_TEST(r == OE_OK);

    r = oe_cert_verify(&cert, &chain, crl, &error);
    OE_TEST(r == OE_OK);

    oe_cert_free(&cert);
    oe_cert_chain_free(&chain);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _TestCertVerifyBad()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_verify_cert_error_t error = {0};
    oe_cert_t cert = {0};
    oe_cert_chain_t chain = {0};
    oe_crl_t* crl = NULL;

    r = oe_cert_read_pem(&cert, _CERT1, sizeof(_CERT1));
    OE_TEST(r == OE_OK);

    /* Chain does not contain a root for this certificate */
    r = oe_cert_chain_read_pem(&chain, CHAIN2, sizeof(CHAIN2));
    OE_TEST(r == OE_OK);

    r = oe_cert_verify(&cert, &chain, crl, &error);
    OE_TEST(r == OE_VERIFY_FAILED);

    oe_cert_free(&cert);
    oe_cert_chain_free(&chain);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _TestMixedChain()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_cert_t cert = {0};
    oe_cert_chain_t chain = {0};

    r = oe_cert_read_pem(&cert, _CERT1, sizeof(_CERT1));
    OE_TEST(r == OE_OK);

    /* Chain does not contain a root for this certificate */
    r = oe_cert_chain_read_pem(&chain, MIXED_CHAIN, sizeof(MIXED_CHAIN));
    OE_TEST(r == OE_FAILURE);

    oe_cert_free(&cert);
    oe_cert_chain_free(&chain);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _TestGenerate()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_rsa_private_key_t privateKey = {0};
    oe_rsa_public_key_t publicKey = {0};
    uint8_t* signature = NULL;
    size_t signatureSize = 0;

    r = oe_rsa_generate_key_pair(1024, 3, &privateKey, &publicKey);
    OE_TEST(r == OE_OK);

    r = oe_rsa_private_key_sign(
        &privateKey,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        &signatureSize);
    OE_TEST(r == OE_BUFFER_TOO_SMALL);

    OE_TEST(signature = (uint8_t*)malloc(signatureSize));

    r = oe_rsa_private_key_sign(
        &privateKey,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        &signatureSize);
    OE_TEST(r == OE_OK);

    r = oe_rsa_public_key_verify(
        &publicKey,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        signatureSize);
    OE_TEST(r == OE_OK);

    free(signature);
    oe_rsa_private_key_free(&privateKey);
    oe_rsa_public_key_free(&publicKey);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _TestWritePrivate()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_rsa_private_key_t key = {0};
    void* pemData = NULL;
    size_t pemSize = 0;

    r = oe_rsa_private_key_read_pem(
        &key, (const uint8_t*)_PRIVATE_KEY, sizeof(_PRIVATE_KEY));
    OE_TEST(r == OE_OK);

    r = oe_rsa_private_key_write_pem(&key, pemData, &pemSize);
    OE_TEST(r == OE_BUFFER_TOO_SMALL);

    OE_TEST(pemData = (uint8_t*)malloc(pemSize));

    r = oe_rsa_private_key_write_pem(&key, pemData, &pemSize);
    OE_TEST(r == OE_OK);

    OE_TEST(sizeof(_PRIVATE_KEY) == pemSize);
    OE_TEST(memcmp(_PRIVATE_KEY, pemData, pemSize) == 0);

    free(pemData);
    oe_rsa_private_key_free(&key);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _TestWritePublic()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_rsa_public_key_t key = {0};
    void* pemData = NULL;
    size_t pemSize = 0;

    r = oe_rsa_public_key_read_pem(
        &key, (const uint8_t*)_PUBLIC_KEY, sizeof(_PUBLIC_KEY));
    OE_TEST(r == OE_OK);

    r = oe_rsa_public_key_write_pem(&key, pemData, &pemSize);
    OE_TEST(r == OE_BUFFER_TOO_SMALL);

    OE_TEST(pemData = (uint8_t*)malloc(pemSize));

    r = oe_rsa_public_key_write_pem(&key, pemData, &pemSize);
    OE_TEST(r == OE_OK);

    OE_TEST(sizeof(_PUBLIC_KEY) == pemSize);
    OE_TEST(memcmp(_PUBLIC_KEY, pemData, pemSize) == 0);

    free(pemData);
    oe_rsa_public_key_free(&key);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _TestCertMethods()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;

    /* Test oe_cert_get_rsa_public_key() */
    {
        oe_cert_t cert = {0};

        r = oe_cert_read_pem(&cert, _CERT1, sizeof(_CERT1));
        OE_TEST(r == OE_OK);

        oe_rsa_public_key_t key = {0};
        r = oe_cert_get_rsa_public_key(&cert, &key);
        OE_TEST(r == OE_OK);

        /* Test oe_rsa_public_key_get_modulus() */
        {
            uint8_t* data;
            size_t size = 0;

            /* Determine required buffer size */
            r = oe_rsa_public_key_get_modulus(&key, NULL, &size);
            OE_TEST(r == OE_BUFFER_TOO_SMALL);
            OE_TEST(size == sizeof(_CERT1_RSA_MODULUS));

            /* Fetch the key bytes */
            OE_TEST(data = (uint8_t*)malloc(size));
            r = oe_rsa_public_key_get_modulus(&key, data, &size);
            OE_TEST(r == OE_OK);

            /* Does it match expected modulus? */
            OE_TEST(size == sizeof(_CERT1_RSA_MODULUS));
            OE_TEST(memcmp(data, _CERT1_RSA_MODULUS, size) == 0);
            free(data);
        }

        /* Test oe_rsa_public_key_get_exponent() */
        {
            uint8_t* data;
            size_t size = 0;

            /* Determine required buffer size */
            r = oe_rsa_public_key_get_exponent(&key, NULL, &size);
            OE_TEST(r == OE_BUFFER_TOO_SMALL);
            OE_TEST(size == sizeof(_CERT_RSA_EXPONENT));

            /* Fetch the key bytes */
            OE_TEST(data = (uint8_t*)malloc(size));
            r = oe_rsa_public_key_get_exponent(&key, data, &size);
            OE_TEST(r == OE_OK);

            /* Does it match expected exponent */
            OE_TEST(size == sizeof(_CERT_RSA_EXPONENT));
            OE_TEST(memcmp(data, _CERT_RSA_EXPONENT, size) == 0);
            free(data);
        }

        /* Test oe_rsa_public_key_equal() */
        {
            bool equal;
            OE_TEST(oe_rsa_public_key_equal(&key, &key, &equal) == OE_OK);
            OE_TEST(equal == true);
        }

        oe_rsa_public_key_free(&key);
        oe_cert_free(&cert);
    }

    /* Test oe_cert_chain_get_cert() */
    {
        oe_cert_chain_t chain = {0};

        /* Load the chain from PEM format */
        r = oe_cert_chain_read_pem(&chain, CHAIN1, sizeof(CHAIN1));
        OE_TEST(r == OE_OK);

        /* Get the length of the chain */
        size_t length;
        r = oe_cert_chain_get_length(&chain, &length);
        OE_TEST(r == OE_OK);
        OE_TEST(length == 2);

        /* Get each certificate in the chain */
        for (size_t i = 0; i < length; i++)
        {
            oe_cert_t cert = {0};
            r = oe_cert_chain_get_cert(&chain, i, &cert);
            OE_TEST(r == OE_OK);
            oe_cert_free(&cert);
        }

        /* Test out of bounds */
        {
            oe_cert_t cert = {0};
            r = oe_cert_chain_get_cert(&chain, length + 1, &cert);
            OE_TEST(r == OE_OUT_OF_BOUNDS);
            oe_cert_free(&cert);
        }

        oe_cert_chain_free(&chain);
    }

    /* Test oe_cert_chain_get_root_cert() and oe_cert_chain_get_leaf_cert() */
    {
        oe_cert_chain_t chain = {0};
        oe_cert_t root = {0};
        oe_cert_t leaf = {0};

        /* Load the chain from PEM format */
        r = oe_cert_chain_read_pem(&chain, CHAIN1, sizeof(CHAIN1));
        OE_TEST(r == OE_OK);

        /* Get the root certificate */
        r = oe_cert_chain_get_root_cert(&chain, &root);
        OE_TEST(r == OE_OK);

        /* Get the leaf certificate */
        r = oe_cert_chain_get_leaf_cert(&chain, &leaf);
        OE_TEST(r == OE_OK);

        /* Check that the keys are identical for top and root certificate */
        {
            oe_rsa_public_key_t rootKey = {0};
            oe_rsa_public_key_t certKey = {0};

            OE_TEST(oe_cert_get_rsa_public_key(&root, &rootKey) == OE_OK);

            oe_rsa_public_key_free(&rootKey);
            oe_rsa_public_key_free(&certKey);
        }

        /* Check that the keys are not identical for leaf and root */
        {
            oe_rsa_public_key_t rootKey = {0};
            oe_rsa_public_key_t leafKey = {0};
            bool equal;

            OE_TEST(oe_cert_get_rsa_public_key(&root, &rootKey) == OE_OK);
            OE_TEST(oe_cert_get_rsa_public_key(&leaf, &leafKey) == OE_OK);

            OE_TEST(
                oe_rsa_public_key_equal(&rootKey, &leafKey, &equal) == OE_OK);
            OE_TEST(equal == false);

            oe_rsa_public_key_free(&rootKey);
            oe_rsa_public_key_free(&leafKey);
        }

        oe_cert_free(&root);
        oe_cert_free(&leaf);
        oe_cert_chain_free(&chain);
    }

    printf("=== passed %s()\n", __FUNCTION__);
}

void TestRSA(void)
{
    _TestCertMethods();
    _TestCertVerifyGood();
    _TestCertVerifyBad();
    _TestMixedChain();
    _TestGenerate();
    _TestSign();
    _TestVerify();
    _TestWritePrivate();
    _TestWritePublic();
}
