// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#if defined(OE_BUILD_ENCLAVE)
#include <openenclave/enclave.h>
#endif

#include <openenclave/internal/cert.h>
#include <openenclave/internal/ec.h>
#include <openenclave/internal/hexdump.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ec.h"
#include "ec_tests.h"
#include "hash.h"

/* Certificate with an EC key */
static const char _CERT[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDRDCCAuqgAwIBAgIVAO34O//eez2nCYF6dX5lnmDXUhoHMAoGCCqGSM49BAMC\n"
    "MHExIzAhBgNVBAMMGkludGVsIFNHWCBQQ0sgUHJvY2Vzc29yIENBMRowGAYDVQQK\n"
    "DBFJbnRlbCBDb3Jwb3JhdGlvbjEUMBIGA1UEBwwLU2FudGEgQ2xhcmExCzAJBgNV\n"
    "BAgMAkNBMQswCQYDVQQGEwJVUzAeFw0xODAyMjcxNDI5MTBaFw0yNTAyMjcxNDI5\n"
    "MTBaMHAxIjAgBgNVBAMMGUludGVsIFNHWCBQQ0sgQ2VydGlmaWNhdGUxGjAYBgNV\n"
    "BAoMEUludGVsIENvcnBvcmF0aW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkG\n"
    "A1UECAwCQ0ExCzAJBgNVBAYTAlVTMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE\n"
    "ebMENxGyc7Ns/OV4bt6OOZy9KxUzFtPOwuGq9chDuJkgz1M/4iLtm0STR8mIENnJ\n"
    "vwS0E6STuxsCtNGIzNscOKOCAV4wggFaMB8GA1UdIwQYMBaAFJ8Gl+9TIUTU+kx+\n"
    "6LqNs9Ml5JKQMFgGA1UdHwRRME8wTaBLoEmGR2h0dHBzOi8vY2VydGlmaWNhdGVz\n"
    "LnRydXN0ZWRzZXJ2aWNlcy5pbnRlbC5jb20vSW50ZWxTR1hQQ0tQcm9jZXNzb3Iu\n"
    "Y3JsMB0GA1UdDgQWBBQUdCfHZzHpiEurA+45dylYXpVvDjAOBgNVHQ8BAf8EBAMC\n"
    "BsAwDAYDVR0TAQH/BAIwADCBnwYJKoZIhvhNAQ0BAQH/BIGOMIGLMB4GCiqGSIb4\n"
    "TQENAQEEEAusByQ8F/2YbRVLVQlDPxUwHgYKKoZIhvhNAQ0BAgQQAAAAAAEBAAAA\n"
    "AAAAAAAAADAQBgoqhkiG+E0BDQEDBAIAADAQBgoqhkiG+E0BDQEEBAIAADAUBgoq\n"
    "hkiG+E0BDQEFBAYgkG6hAAAwDwYKKoZIhvhNAQ0BBgoBADAKBggqhkjOPQQDAgNI\n"
    "ADBFAiEAhY2Bdn5aQJH2Fj1YZriJ7DpmQCbqRyVxU65bd8v0O/4CIA2IWOarGysj\n"
    "RvR+bMRtTbhiRXkV9JD2FJA24tP32pw+\n"
    "-----END CERTIFICATE-----\n";

/* Certificate chain organized from leaf-to-root */
static const char _CHAIN[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIEeTCCBB+gAwIBAgIUCLZOGlkHY7MDrT4wyodHxMlDpSowCgYIKoZIzj0EAwIw\n"
    "cTEjMCEGA1UEAwwaSW50ZWwgU0dYIFBDSyBQcm9jZXNzb3IgQ0ExGjAYBgNVBAoM\n"
    "EUludGVsIENvcnBvcmF0aW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkGA1UE\n"
    "CAwCQ0ExCzAJBgNVBAYTAlVTMB4XDTE4MDUzMDExMzMwNVoXDTI1MDUzMDExMzMw\n"
    "NVowcDEiMCAGA1UEAwwZSW50ZWwgU0dYIFBDSyBDZXJ0aWZpY2F0ZTEaMBgGA1UE\n"
    "CgwRSW50ZWwgQ29ycG9yYXRpb24xFDASBgNVBAcMC1NhbnRhIENsYXJhMQswCQYD\n"
    "VQQIDAJDQTELMAkGA1UEBhMCVVMwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAATW\n"
    "xmGk363GUJ+JrxRwGC9lRLyIarJntKUP5lqgQjFSj6u+zK8w+JXLhSmOctWqG29T\n"
    "3eItOqjX9zeLbIJ1bhbwo4IClDCCApAwHwYDVR0jBBgwFoAU5btSj4D54zOuGaz6\n"
    "Y0Z4EfNhu6QwWAYDVR0fBFEwTzBNoEugSYZHaHR0cHM6Ly9jZXJ0aWZpY2F0ZXMu\n"
    "dHJ1c3RlZHNlcnZpY2VzLmludGVsLmNvbS9JbnRlbFNHWFBDS1Byb2Nlc3Nvci5j\n"
    "cmwwHQYDVR0OBBYEFKGnZhYL+u0LdXb3Q4gVmfhWgirlMA4GA1UdDwEB/wQEAwIG\n"
    "wDAMBgNVHRMBAf8EAjAAMIIB1AYJKoZIhvhNAQ0BBIIBxTCCAcEwHgYKKoZIhvhN\n"
    "AQ0BAQQQoOhb4fIWDhGRy2yRyuyGgzCCAWQGCiqGSIb4TQENAQIwggFUMBAGCyqG\n"
    "SIb4TQENAQIBAgEEMBAGCyqGSIb4TQENAQICAgEEMBAGCyqGSIb4TQENAQIDAgEC\n"
    "MBAGCyqGSIb4TQENAQIEAgEEMBAGCyqGSIb4TQENAQIFAgEBMBEGCyqGSIb4TQEN\n"
    "AQIGAgIAgDAQBgsqhkiG+E0BDQECBwIBADAQBgsqhkiG+E0BDQECCAIBADAQBgsq\n"
    "hkiG+E0BDQECCQIBADAQBgsqhkiG+E0BDQECCgIBADAQBgsqhkiG+E0BDQECCwIB\n"
    "ADAQBgsqhkiG+E0BDQECDAIBADAQBgsqhkiG+E0BDQECDQIBADAQBgsqhkiG+E0B\n"
    "DQECDgIBADAQBgsqhkiG+E0BDQECDwIBADAQBgsqhkiG+E0BDQECEAIBADAQBgsq\n"
    "hkiG+E0BDQECEQIBBTAfBgsqhkiG+E0BDQECEgQQBAQCBAGAAAAAAAAAAAAAADAQ\n"
    "BgoqhkiG+E0BDQEDBAIAADAUBgoqhkiG+E0BDQEEBAYAkG6hAAAwDwYKKoZIhvhN\n"
    "AQ0BBQoBADAKBggqhkjOPQQDAgNIADBFAiEA9im3EzMQDdJrWbQoML/pTFuhjUuk\n"
    "8yainoRd1tJ/tgUCIGgo6SHqeEw0h2lRw4sZGWjEHLosoPIme3t+Gw9QosI5\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIICmDCCAj6gAwIBAgIVAOW7Uo+A+eMzrhms+mNGeBHzYbukMAoGCCqGSM49BAMC\n"
    "MGgxGjAYBgNVBAMMEUludGVsIFNHWCBSb290IENBMRowGAYDVQQKDBFJbnRlbCBD\n"
    "b3Jwb3JhdGlvbjEUMBIGA1UEBwwLU2FudGEgQ2xhcmExCzAJBgNVBAgMAkNBMQsw\n"
    "CQYDVQQGEwJVUzAeFw0xODA1MjUxMzQzNDFaFw0zMzA1MjUxMzQzNDFaMHExIzAh\n"
    "BgNVBAMMGkludGVsIFNHWCBQQ0sgUHJvY2Vzc29yIENBMRowGAYDVQQKDBFJbnRl\n"
    "bCBDb3Jwb3JhdGlvbjEUMBIGA1UEBwwLU2FudGEgQ2xhcmExCzAJBgNVBAgMAkNB\n"
    "MQswCQYDVQQGEwJVUzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABMB0yW2PyWpf\n"
    "6odNPzGnE503t30mxdRm6zKRy86UoBpGMSHUEat/8/V3bIN+QYR21tpLUtzuTx2m\n"
    "HSLi7MCO6byjgbswgbgwHwYDVR0jBBgwFoAUImUM1lqdNInzg7SVUr9QGzknBqww\n"
    "UgYDVR0fBEswSTBHoEWgQ4ZBaHR0cHM6Ly9jZXJ0aWZpY2F0ZXMudHJ1c3RlZHNl\n"
    "cnZpY2VzLmludGVsLmNvbS9JbnRlbFNHWFJvb3RDQS5jcmwwHQYDVR0OBBYEFOW7\n"
    "Uo+A+eMzrhms+mNGeBHzYbukMA4GA1UdDwEB/wQEAwIBBjASBgNVHRMBAf8ECDAG\n"
    "AQH/AgEAMAoGCCqGSM49BAMCA0gAMEUCIQDkybHzpTP7oBIm3iBwO28eAlsyJuQn\n"
    "ayD1LxMurMKCuQIgQkgfZl8ElCe+H2nzmG/pKlcox3jHyJwj8w8CH9w7pIE=\n"
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    "MIICjzCCAjSgAwIBAgIUImUM1lqdNInzg7SVUr9QGzknBqwwCgYIKoZIzj0EAwIw\n"
    "aDEaMBgGA1UEAwwRSW50ZWwgU0dYIFJvb3QgQ0ExGjAYBgNVBAoMEUludGVsIENv\n"
    "cnBvcmF0aW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkGA1UECAwCQ0ExCzAJ\n"
    "BgNVBAYTAlVTMB4XDTE4MDUyMTEwNDExMVoXDTMzMDUyMTEwNDExMFowaDEaMBgG\n"
    "A1UEAwwRSW50ZWwgU0dYIFJvb3QgQ0ExGjAYBgNVBAoMEUludGVsIENvcnBvcmF0\n"
    "aW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkGA1UECAwCQ0ExCzAJBgNVBAYT\n"
    "AlVTMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEC6nEwMDIYZOj/iPWsCzaEKi7\n"
    "1OiOSLRFhWGjbnBVJfVnkY4u3IjkDYYL0MxO4mqsyYjlBalTVYxFP2sJBK5zlKOB\n"
    "uzCBuDAfBgNVHSMEGDAWgBQiZQzWWp00ifODtJVSv1AbOScGrDBSBgNVHR8ESzBJ\n"
    "MEegRaBDhkFodHRwczovL2NlcnRpZmljYXRlcy50cnVzdGVkc2VydmljZXMuaW50\n"
    "ZWwuY29tL0ludGVsU0dYUm9vdENBLmNybDAdBgNVHQ4EFgQUImUM1lqdNInzg7SV\n"
    "Ur9QGzknBqwwDgYDVR0PAQH/BAQDAgEGMBIGA1UdEwEB/wQIMAYBAf8CAQAwCgYI\n"
    "KoZIzj0EAwIDSQAwRgIhAIpQ/KlO1XE4hH8cw5Ol/E0yzs8PToJe9Pclt+bhfLUg\n"
    "AiEAss0qf7FlMmAMet+gbpLD97ldYy/wqjjmwN7yHRVr2AM=\n"
    "-----END CERTIFICATE-----\n";

/* X-coordinate of key contained in _CERT */
static const uint8_t _CERT_KEY_X[] = {
    0x79, 0xB3, 0x04, 0x37, 0x11, 0xB2, 0x73, 0xB3, 0x6C, 0xFC, 0xE5,
    0x78, 0x6E, 0xDE, 0x8E, 0x39, 0x9C, 0xBD, 0x2B, 0x15, 0x33, 0x16,
    0xD3, 0xCE, 0xC2, 0xE1, 0xAA, 0xF5, 0xC8, 0x43, 0xB8, 0x99,
};

/* Y-coordinate of key contained in _CERT */
static const uint8_t _CERT_KEY_Y[] = {
    0x20, 0xCF, 0x53, 0x3F, 0xE2, 0x22, 0xED, 0x9B, 0x44, 0x93, 0x47,
    0xC9, 0x88, 0x10, 0xD9, 0xC9, 0xBF, 0x04, 0xB4, 0x13, 0xA4, 0x93,
    0xBB, 0x1B, 0x02, 0xB4, 0xD1, 0x88, 0xCC, 0xDB, 0x1C, 0x38,
};

static const char _PRIVATE_KEY[] =
    "-----BEGIN EC PRIVATE KEY-----\n"
    "MHQCAQEEIKVVUe1F/MxIp6jmrZ24/8iI6WTj1QDamxZLHQ8ZbL4woAcGBSuBBAAK\n"
    "oUQDQgAEmAxYbaM1rpk+d1KX5pHn0GuuaL5wgEA8xoLzHqVcX1dCOyN1rnZP9axj\n"
    "h8t36IjqPhnxNvCPruzBq/KRbbpIZA==\n"
    "-----END EC PRIVATE KEY-----\n";

static const char _PUBLIC_KEY[] =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFYwEAYHKoZIzj0CAQYFK4EEAAoDQgAEmAxYbaM1rpk+d1KX5pHn0GuuaL5wgEA8\n"
    "xoLzHqVcX1dCOyN1rnZP9axjh8t36IjqPhnxNvCPruzBq/KRbbpIZA==\n"
    "-----END PUBLIC KEY-----\n";

static const uint8_t _SIGNATURE[] = {
    0x30, 0x45, 0x02, 0x21, 0x00, 0x89, 0x3a, 0xf7, 0xe5, 0xf2, 0x21, 0xe1,
    0xf9, 0xdc, 0xe0, 0x92, 0x82, 0xe6, 0xe4, 0xec, 0xcc, 0x68, 0x6d, 0x00,
    0x5d, 0x0e, 0x9c, 0xd5, 0x08, 0x48, 0x8b, 0x09, 0x5f, 0x20, 0xee, 0xbe,
    0x95, 0x02, 0x20, 0x6e, 0xaa, 0xd2, 0x15, 0xf9, 0xf3, 0xaa, 0xc2, 0x19,
    0xc5, 0x4c, 0x44, 0x0b, 0xa7, 0x2c, 0x3e, 0xe9, 0xc3, 0xb6, 0xf3, 0xb4,
    0x04, 0x51, 0xc6, 0xe9, 0xf1, 0x69, 0x46, 0xb0, 0x3e, 0x22, 0xe6,
};

static size_t _SIGNATURE_SIZE = sizeof(_SIGNATURE);

// Test EC signing operation over an ASCII alphabet string. Note that two
// signatures over the same data produce different hex sequences, although
// signature verification will still succeed.
static void _test_sign_and_verify()
{
    printf("=== begin %s()\n", __FUNCTION__);

    uint8_t* signature = NULL;
    size_t signature_size = 0;
    oe_result_t r;

    {
        oe_ec_private_key_t key = {0};

        r = oe_ec_private_key_read_pem(
            (const uint8_t*)_PRIVATE_KEY, sizeof(_PRIVATE_KEY), &key);
        OE_TEST(r == OE_OK);

        r = oe_ec_private_key_sign(
            &key,
            OE_HASH_TYPE_SHA256,
            &ALPHABET_HASH,
            sizeof(ALPHABET_HASH),
            signature,
            &signature_size);
        OE_TEST(r == OE_BUFFER_TOO_SMALL);

        OE_TEST(signature = (uint8_t*)malloc(signature_size));

        r = oe_ec_private_key_sign(
            &key,
            OE_HASH_TYPE_SHA256,
            &ALPHABET_HASH,
            sizeof(ALPHABET_HASH),
            signature,
            &signature_size);
        OE_TEST(r == OE_OK);

        OE_TEST(signature != NULL);
        OE_TEST(signature_size != 0);

        oe_ec_private_key_free(&key);
    }

    {
        oe_ec_public_key_t key = {0};

        r = oe_ec_public_key_read_pem(
            (const uint8_t*)_PUBLIC_KEY, sizeof(_PUBLIC_KEY), &key);
        OE_TEST(r == OE_OK);

        r = oe_ec_public_key_verify(
            &key,
            OE_HASH_TYPE_SHA256,
            &ALPHABET_HASH,
            sizeof(ALPHABET_HASH),
            signature,
            signature_size);
        OE_TEST(r == OE_OK);

        r = oe_ec_public_key_verify(
            &key,
            OE_HASH_TYPE_SHA256,
            &ALPHABET_HASH,
            sizeof(ALPHABET_HASH),
            _SIGNATURE,
            _SIGNATURE_SIZE);
        OE_TEST(r == OE_OK);

        oe_ec_public_key_free(&key);
    }

    /* Convert signature to raw form and then back to ASN.1 */
    {
        uint8_t r_data[1024];
        size_t r_size = sizeof(r_data);
        uint8_t s_data[1024];
        size_t s_size = sizeof(s_data);

        r = oe_ecdsa_signature_read_der(
            signature, signature_size, r_data, &r_size, s_data, &s_size);
        OE_TEST(r == OE_OK);
        OE_TEST(r_size == 32);
        OE_TEST(s_size == 32);

        uint8_t data[signature_size];
        size_t size = sizeof(data);
        r = oe_ecdsa_signature_write_der(
            data, &size, r_data, r_size, s_data, s_size);
        OE_TEST(r == OE_OK);
        OE_TEST(signature_size == size);
        OE_TEST(memcmp(signature, data, signature_size) == 0);
    }

    /* Convert a known signature to raw form and back */
    {
        const uint8_t SIG[] = {
            0x30, 0x45, 0x02, 0x20, 0x6A, 0xCD, 0x74, 0xB9, 0x8B, 0x1A, 0xDD,
            0xA3, 0x3D, 0x84, 0x42, 0x44, 0x1F, 0x9B, 0x62, 0x5E, 0x9E, 0xB7,
            0x3F, 0x3C, 0x89, 0xFD, 0xFA, 0xFE, 0x2B, 0x25, 0x7C, 0x43, 0x29,
            0xE3, 0x3D, 0x43, 0x02, 0x21, 0x00, 0xDE, 0xEB, 0x54, 0xF8, 0x6C,
            0x7D, 0xCD, 0xA2, 0x0D, 0x8B, 0x10, 0xCB, 0x4D, 0x7D, 0x8B, 0x14,
            0xDC, 0x54, 0x83, 0x87, 0xD3, 0x35, 0x5A, 0x48, 0xD1, 0x67, 0xD1,
            0xF0, 0xA8, 0x4B, 0x31, 0xBE,
        };
        const uint8_t R[] = {
            0x6A, 0xCD, 0x74, 0xB9, 0x8B, 0x1A, 0xDD, 0xA3, 0x3D, 0x84, 0x42,
            0x44, 0x1F, 0x9B, 0x62, 0x5E, 0x9E, 0xB7, 0x3F, 0x3C, 0x89, 0xFD,
            0xFA, 0xFE, 0x2B, 0x25, 0x7C, 0x43, 0x29, 0xE3, 0x3D, 0x43,
        };

        const uint8_t S[] = {
            0xDE, 0xEB, 0x54, 0xF8, 0x6C, 0x7D, 0xCD, 0xA2, 0x0D, 0x8B, 0x10,
            0xCB, 0x4D, 0x7D, 0x8B, 0x14, 0xDC, 0x54, 0x83, 0x87, 0xD3, 0x35,
            0x5A, 0x48, 0xD1, 0x67, 0xD1, 0xF0, 0xA8, 0x4B, 0x31, 0xBE,

        };
        uint8_t r_data[1024];
        size_t r_size = sizeof(r_data);
        uint8_t s_data[1024];
        size_t s_size = sizeof(s_data);

        r = oe_ecdsa_signature_read_der(
            SIG, sizeof(SIG), r_data, &r_size, s_data, &s_size);
        OE_TEST(r == OE_OK);
        OE_TEST(r_size == sizeof(R));
        OE_TEST(s_size == sizeof(S));
        OE_TEST(memcmp(r_data, R, r_size) == 0);
        OE_TEST(memcmp(s_data, S, s_size) == 0);

        uint8_t data[sizeof(SIG)];
        size_t size = sizeof(data);
        r = oe_ecdsa_signature_write_der(
            data, &size, r_data, r_size, s_data, s_size);
        OE_TEST(r == OE_OK);
        OE_TEST(sizeof(SIG) == size);
        OE_TEST(memcmp(SIG, data, sizeof(SIG)) == 0);
    }

    free(signature);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _test_generate()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_ec_private_key_t private_key = {0};
    oe_ec_public_key_t public_key = {0};
    uint8_t* signature = NULL;
    size_t signature_size = 0;

    r = oe_ec_generate_key_pair(OE_EC_TYPE_SECP256R1, &private_key, &public_key);
    OE_TEST(r == OE_OK);

    r = oe_ec_private_key_sign(
        &private_key,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        &signature_size);
    OE_TEST(r == OE_BUFFER_TOO_SMALL);

    OE_TEST(signature = (uint8_t*)malloc(signature_size));

    r = oe_ec_private_key_sign(
        &private_key,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        &signature_size);
    OE_TEST(r == OE_OK);

    r = oe_ec_public_key_verify(
        &public_key,
        OE_HASH_TYPE_SHA256,
        &ALPHABET_HASH,
        sizeof(ALPHABET_HASH),
        signature,
        signature_size);
    OE_TEST(r == OE_OK);

    free(signature);
    oe_ec_private_key_free(&private_key);
    oe_ec_public_key_free(&public_key);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _test_write_private()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_ec_public_key_t public_key = {0};
    oe_ec_private_key_t key1 = {0};
    oe_ec_private_key_t key2 = {0};
    uint8_t* pem_data1 = NULL;
    size_t pem_size1 = 0;
    uint8_t* pem_data2 = NULL;
    size_t pem_size2 = 0;

    r = oe_ec_generate_key_pair(OE_EC_TYPE_SECP256R1, &key1, &public_key);
    OE_TEST(r == OE_OK);

    {
        r = oe_ec_private_key_write_pem(&key1, pem_data1, &pem_size1);
        OE_TEST(r == OE_BUFFER_TOO_SMALL);

        OE_TEST(pem_data1 = (uint8_t*)malloc(pem_size1));

        r = oe_ec_private_key_write_pem(&key1, pem_data1, &pem_size1);
        OE_TEST(r == OE_OK);
    }

    OE_TEST(pem_size1 != 0);
    OE_TEST(pem_data1[pem_size1 - 1] == '\0');
    OE_TEST(strlen((char*)pem_data1) == pem_size1 - 1);

    r = oe_ec_private_key_read_pem(pem_data1, pem_size1, &key2);
    OE_TEST(r == OE_OK);

    {
        r = oe_ec_private_key_write_pem(&key2, pem_data2, &pem_size2);
        OE_TEST(r == OE_BUFFER_TOO_SMALL);

        OE_TEST(pem_data2 = (uint8_t*)malloc(pem_size2));

        r = oe_ec_private_key_write_pem(&key2, pem_data2, &pem_size2);
        OE_TEST(r == OE_OK);
    }

    OE_TEST(pem_size1 == pem_size2);
    OE_TEST(memcmp(pem_data1, pem_data2, pem_size1) == 0);

    free(pem_data1);
    free(pem_data2);
    oe_ec_public_key_free(&public_key);
    oe_ec_private_key_free(&key1);
    oe_ec_private_key_free(&key2);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _test_write_public()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_ec_public_key_t key = {0};
    void* pem_data = NULL;
    size_t pem_size = 0;

    r = oe_ec_public_key_read_pem(
        (const uint8_t*)_PUBLIC_KEY, sizeof(_PUBLIC_KEY), &key);
    OE_TEST(r == OE_OK);

    {
        r = oe_ec_public_key_write_pem(&key, pem_data, &pem_size);
        OE_TEST(r == OE_BUFFER_TOO_SMALL);

        OE_TEST(pem_data = (uint8_t*)malloc(pem_size));

        r = oe_ec_public_key_write_pem(&key, pem_data, &pem_size);
        OE_TEST(r == OE_OK);
    }

    OE_TEST(sizeof(_PUBLIC_KEY) == pem_size);
    OE_TEST(memcmp(_PUBLIC_KEY, pem_data, pem_size) == 0);

    free(pem_data);
    oe_ec_public_key_free(&key);

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _test_cert_methods()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;

    /* Test oe_cert_get_ec_public_key() */
    {
        oe_cert_t cert = {0};
        oe_ec_public_key_t key = {0};

        r = oe_cert_read_pem(_CERT, sizeof(_CERT), &cert);
        OE_TEST(r == OE_OK);

        r = oe_cert_get_ec_public_key(&cert, &key);
        OE_TEST(r == OE_OK);

        /* Test oe_ec_public_key_to_coordinates() */
        {
            uint8_t* x_data = NULL;
            size_t x_size = 0;
            uint8_t* y_data = NULL;
            size_t y_size = 0;

            /* Determine the required size of the buffer */
            r = oe_ec_public_key_to_coordinates(
                &key, NULL, &x_size, NULL, &y_size);
            OE_TEST(r == OE_BUFFER_TOO_SMALL);
            OE_TEST(x_size == sizeof(_CERT_KEY_X));
            OE_TEST(y_size == sizeof(_CERT_KEY_Y));

            /* Fetch the key bytes */
            OE_TEST(x_data = (uint8_t*)calloc(1, x_size));
            OE_TEST(y_data = (uint8_t*)calloc(1, y_size));
            r = oe_ec_public_key_to_coordinates(
                &key, x_data, &x_size, y_data, &y_size);
            OE_TEST(r == OE_OK);

            /* Does it match expected key? */
            OE_TEST(x_size == sizeof(_CERT_KEY_X));
            OE_TEST(y_size == sizeof(_CERT_KEY_Y));
            OE_TEST(memcmp(_CERT_KEY_X, x_data, sizeof(_CERT_KEY_X)) == 0);
            OE_TEST(memcmp(_CERT_KEY_Y, y_data, sizeof(_CERT_KEY_Y)) == 0);
            free(x_data);
            free(y_data);
        }

        /* Test oe_ec_public_key_equal() */
        {
            bool equal;
            OE_TEST(oe_ec_public_key_equal(&key, &key, &equal) == OE_OK);
            OE_TEST(equal == true);
        }

        oe_ec_public_key_free(&key);
        oe_cert_free(&cert);
    }

    /* Test oe_cert_chain_get_cert() */
    {
        oe_cert_chain_t chain;

        /* Load the chain from PEM format */
        r = oe_cert_chain_read_pem(_CHAIN, sizeof(_CHAIN), &chain);
        OE_TEST(r == OE_OK);

        /* Get the length of the chain */
        size_t length;
        r = oe_cert_chain_get_length(&chain, &length);
        OE_TEST(r == OE_OK);
        OE_TEST(length == 3);

        /* Get each certificate in the chain */
        for (size_t i = 0; i < length; i++)
        {
            oe_cert_t cert;
            r = oe_cert_chain_get_cert(&chain, i, &cert);
            OE_TEST(r == OE_OK);
            oe_cert_free(&cert);
        }

        /* Test out of bounds */
        {
            oe_cert_t cert;
            r = oe_cert_chain_get_cert(&chain, length + 1, &cert);
            OE_TEST(r == OE_OUT_OF_BOUNDS);
            oe_cert_free(&cert);
        }

        oe_cert_chain_free(&chain);
    }

    /* Test oe_cert_chain_get_root_cert() and oe_cert_chain_get_leaf_cert() */
    {
        oe_cert_chain_t chain;
        oe_cert_t root;
        oe_cert_t leaf;

        /* Load the chain from PEM format */
        r = oe_cert_chain_read_pem(_CHAIN, sizeof(_CHAIN), &chain);
        OE_TEST(r == OE_OK);

        /* Get the root certificate */
        r = oe_cert_chain_get_root_cert(&chain, &root);
        OE_TEST(r == OE_OK);

        /* Get the leaf certificate */
        r = oe_cert_chain_get_leaf_cert(&chain, &leaf);
        OE_TEST(r == OE_OK);

        /* Check that the keys are identical for top and root certificate */
        {
            oe_ec_public_key_t root_key;
            oe_ec_public_key_t cert_key;

            OE_TEST(oe_cert_get_ec_public_key(&root, &root_key) == OE_OK);

            oe_ec_public_key_free(&root_key);
            oe_ec_public_key_free(&cert_key);
        }

        /* Check that the keys are not identical for leaf and root */
        {
            oe_ec_public_key_t root_key;
            oe_ec_public_key_t leaf_key;
            bool equal;

            OE_TEST(oe_cert_get_ec_public_key(&root, &root_key) == OE_OK);
            OE_TEST(oe_cert_get_ec_public_key(&leaf, &leaf_key) == OE_OK);

            OE_TEST(
                oe_ec_public_key_equal(&root_key, &leaf_key, &equal) == OE_OK);
            OE_TEST(equal == false);

            oe_ec_public_key_free(&root_key);
            oe_ec_public_key_free(&leaf_key);
        }

        oe_cert_free(&root);
        oe_cert_free(&leaf);
        oe_cert_chain_free(&chain);
    }

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _test_key_from_bytes()
{
    printf("=== begin %s()()\n", __FUNCTION__);

    oe_result_t r;
    oe_ec_type_t ec_type = OE_EC_TYPE_SECP256R1;

    /* Create a public EC key and get its bytes */
    {
        oe_ec_private_key_t private_key = {0};
        oe_ec_public_key_t public_key = {0};
        r = oe_ec_generate_key_pair(ec_type, &private_key, &public_key);
        OE_TEST(r == OE_OK);

        uint8_t x_data[1024];
        size_t x_size = sizeof(x_data);
        uint8_t y_data[1024];
        size_t y_size = sizeof(y_data);

        r = oe_ec_public_key_to_coordinates(
            &public_key, x_data, &x_size, y_data, &y_size);
        OE_TEST(r == OE_OK);

        oe_ec_public_key_t key = {0};
        r = oe_ec_public_key_from_coordinates(
            &key, ec_type, x_data, x_size, y_data, y_size);
        OE_TEST(r == OE_OK);

        oe_ec_private_key_free(&private_key);
        oe_ec_public_key_free(&public_key);
        oe_ec_public_key_free(&key);
    }

    /* Test creating an EC key from bytes */
    {
        oe_ec_public_key_t key = {0};
        const uint8_t x_bytes[32] = {
            0xB5, 0x5D, 0x06, 0xD6, 0xE5, 0xA2, 0xC7, 0x2D, 0x5D, 0xA0, 0xAE,
            0xD5, 0x83, 0x61, 0x4C, 0x51, 0x60, 0xD6, 0xFE, 0x90, 0x8A, 0xC2,
            0x67, 0xF7, 0x31, 0x56, 0x2A, 0x6B, 0xBC, 0xB0, 0x8D, 0xD0,
        };
        const uint8_t y_bytes[32] = {
            0xC6, 0xBD, 0x1F, 0xCB, 0xAF, 0xE1, 0x84, 0xE6, 0x2E, 0x9E, 0xAE,
            0xE0, 0x04, 0x4C, 0xC5, 0x59, 0x44, 0x39, 0x52, 0x62, 0x3B, 0x08,
            0xC5, 0xED, 0xBB, 0xC2, 0xD6, 0x50, 0xE7, 0x7B, 0x38, 0xDA,
        };

        r = oe_ec_public_key_from_coordinates(
            &key, ec_type, x_bytes, sizeof(x_bytes), y_bytes, sizeof(y_bytes));
        OE_TEST(r == OE_OK);

        uint8_t x_data[1024];
        size_t x_size = sizeof(x_data);
        uint8_t y_data[1024];
        size_t y_size = sizeof(y_data);
        r = oe_ec_public_key_to_coordinates(&key, x_data, &x_size, y_data, &y_size);
        OE_TEST(r == OE_OK);

        OE_TEST(sizeof(x_bytes) == x_size);
        OE_TEST(sizeof(y_bytes) == y_size);
        OE_TEST(memcmp(x_data, x_bytes, x_size) == 0);
        OE_TEST(memcmp(y_data, y_bytes, y_size) == 0);
        oe_ec_public_key_free(&key);
    }

    /* Test generating a key and then re-creating it from its bytes */
    {
        oe_ec_private_key_t private_key = {0};
        oe_ec_public_key_t public_key = {0};
        oe_ec_public_key_t public_key2 = {0};
        uint8_t signature[1024];
        size_t signature_size = sizeof(signature);

        /* Generate a key pair */
        r = oe_ec_generate_key_pair(ec_type, &private_key, &public_key);
        OE_TEST(r == OE_OK);

        /* Get the bytes from the public key */
        uint8_t x_data[1024];
        size_t x_size = sizeof(x_data);
        uint8_t y_data[1024];
        size_t y_size = sizeof(y_data);
        r = oe_ec_public_key_to_coordinates(
            &public_key, x_data, &x_size, y_data, &y_size);
        OE_TEST(r == OE_OK);

        /* Create a second public key from the key bytes */
        r = oe_ec_public_key_from_coordinates(
            &public_key2, ec_type, x_data, x_size, y_data, y_size);
        OE_TEST(r == OE_OK);

        /* Sign data with private key */
        r = oe_ec_private_key_sign(
            &private_key,
            OE_HASH_TYPE_SHA256,
            &ALPHABET_HASH,
            sizeof(ALPHABET_HASH),
            signature,
            &signature_size);
        OE_TEST(r == OE_OK);

        /* Verify data with key created from bytes of original public key */
        r = oe_ec_public_key_verify(
            &public_key2,
            OE_HASH_TYPE_SHA256,
            &ALPHABET_HASH,
            sizeof(ALPHABET_HASH),
            signature,
            signature_size);
        OE_TEST(r == OE_OK);

        oe_ec_private_key_free(&private_key);
        oe_ec_public_key_free(&public_key);
        oe_ec_public_key_free(&public_key2);
    }

    printf("=== passed %s()\n", __FUNCTION__);
}

static void _test_cert_chain_read()
{
    printf("=== begin %s()\n", __FUNCTION__);

    oe_result_t r;
    oe_cert_chain_t chain;

    r = oe_cert_chain_read_pem(_CHAIN, sizeof(_CHAIN), &chain);
    OE_TEST(r == OE_OK);

    oe_cert_chain_free(&chain);

    printf("=== passed %s()\n", __FUNCTION__);
}

void TestEC()
{
    _test_sign_and_verify();
    _test_generate();
    _test_write_private();
    _test_write_public();
    _test_cert_methods();
    _test_key_from_bytes();
    _test_cert_chain_read();
}
