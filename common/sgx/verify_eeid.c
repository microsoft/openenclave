// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openenclave/bits/report.h>
#include <openenclave/bits/types.h>
#include <openenclave/internal/defs.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/safecrt.h>
#include <openenclave/internal/sgxtypes.h>
#include <openenclave/internal/utils.h>

#include "../../host/sgx/sgxmeasure.h"
#include "../common/sgx/quote.h"

#ifdef OE_BUILD_ENCLAVE
#include <openenclave/enclave.h>
#include "../../enclave/crypto/key.h"
#include "../../enclave/crypto/rsa.h"
#else
#include <openenclave/host.h>
#include <openssl/rsa.h>
#include "../../host/crypto/openssl/key.h"
#include "../../host/crypto/openssl/rsa.h"
#endif

#include "verify_eeid.h"

static bool is_zero(const uint8_t* buf, size_t sz)
{
    while (sz != 0)
        if (buf[--sz] != 0)
            return false;
    return true;
}

oe_result_t verify_eeid(oe_report_t* report, const oe_eeid_t* eeid)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!eeid || !report)
        OE_RAISE(OE_INVALID_PARAMETER);

    if (oe_get_current_logging_level() >= OE_LOG_LEVEL_WARNING)
    {
        char buf[2 * (sizeof(oe_eeid_t) + eeid->data_size) + 8];
        OE_CHECK(oe_serialize_eeid(eeid, buf, sizeof(buf)));
        printf("EEID:\n%s", buf);
    }

    // Computed mrenclave
    OE_SHA256 cpt_mrenclave;
    oe_replay_eeid_pages(eeid, &cpt_mrenclave);

    // Extract reported mrenclave
    OE_SHA256 reported_mrenclave;
    uint8_t reported_mrsigner[OE_SIGNER_ID_SIZE];

    memcpy(reported_mrenclave.buf, report->identity.unique_id, OE_SHA256_SIZE);
    memcpy(reported_mrsigner, report->identity.signer_id, OE_SIGNER_ID_SIZE);

    // Check recomputed mrenclave against reported mrenclave
    if (memcmp(cpt_mrenclave.buf, reported_mrenclave.buf, OE_SHA256_SIZE) != 0)
        OE_RAISE(OE_VERIFY_FAILED);

    static const uint8_t debug_public_key[] = {
        0xca, 0x9a, 0xd7, 0x33, 0x14, 0x48, 0x98, 0x0a, 0xa2, 0x88, 0x90,
        0xce, 0x73, 0xe4, 0x33, 0x63, 0x83, 0x77, 0xf1, 0x79, 0xab, 0x44,
        0x56, 0xb2, 0xfe, 0x23, 0x71, 0x93, 0x19, 0x3a, 0x8d, 0xa};

    if (memcmp(debug_public_key, reported_mrsigner, OE_SIGNER_ID_SIZE) != 0)
        OE_RAISE(OE_VERIFY_FAILED);

    const sgx_sigstruct_t* sigstruct = (const sgx_sigstruct_t*)&eeid->sigstruct;

    uint16_t ppid = (uint16_t)(report->identity.product_id[1] << 8) +
                    (uint16_t)report->identity.product_id[0];

    bool sigstruct_debug = sigstruct->attributes.flags & SGX_FLAGS_DEBUG;
    bool reported_debug =
        report->identity.attributes & OE_REPORT_ATTRIBUTES_DEBUG;

    if (sigstruct_debug != reported_debug || sigstruct->isvprodid != ppid ||
        sigstruct->isvsvn != report->identity.security_version)
        OE_RAISE(OE_VERIFY_FAILED);

    // Check old signature (new signature has been checked above)
    if (sigstruct_debug && is_zero(sigstruct->signature, OE_KEY_SIZE))
        return OE_OK; // Unsigned debug image is ok?
    else
    {
        unsigned char buf[sizeof(sgx_sigstruct_t)];
        size_t n = 0;

        OE_CHECK(oe_memcpy_s(
            buf,
            sizeof(buf),
            sgx_sigstruct_header(sigstruct),
            sgx_sigstruct_header_size()));
        n += sgx_sigstruct_header_size();
        OE_CHECK(oe_memcpy_s(
            &buf[n],
            sizeof(buf) - n,
            sgx_sigstruct_body(sigstruct),
            sgx_sigstruct_body_size()));
        n += sgx_sigstruct_body_size();

        OE_SHA256 msg_hsh;
        oe_sha256_context_t context;

        oe_sha256_init(&context);
        oe_sha256_update(&context, buf, n);
        oe_sha256_final(&context, &msg_hsh);

        uint8_t reversed_modulus[OE_KEY_SIZE];
        for (size_t i = 0; i < OE_KEY_SIZE; i++)
            reversed_modulus[i] = sigstruct->modulus[OE_KEY_SIZE - 1 - i];

        uint8_t reversed_exponent[OE_KEY_SIZE];
        for (size_t i = 0; i < OE_EXPONENT_SIZE; i++)
            reversed_exponent[i] =
                sigstruct->exponent[OE_EXPONENT_SIZE - 1 - i];

        uint8_t reversed_signature[OE_KEY_SIZE];
        for (size_t i = 0; i < OE_KEY_SIZE; i++)
            reversed_signature[i] = sigstruct->signature[OE_KEY_SIZE - 1 - i];

        oe_rsa_public_key_t pk;
#ifdef OE_BUILD_ENCLAVE
        mbedtls_pk_context pkctx;
        mbedtls_pk_init(&pkctx);
        const mbedtls_pk_info_t* info =
            mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);
        mbedtls_pk_setup(&pkctx, info);

        mbedtls_rsa_context* rsa_ctx = mbedtls_pk_rsa(pkctx);
        mbedtls_rsa_init(rsa_ctx, 0, 0);
        mbedtls_rsa_import_raw(
            rsa_ctx,
            reversed_modulus,
            OE_KEY_SIZE, // N
            NULL,
            0,
            NULL,
            0,
            NULL,
            0, // P Q D
            reversed_exponent,
            OE_EXPONENT_SIZE);
        if (mbedtls_rsa_check_pubkey(rsa_ctx) != 0)
            OE_RAISE(OE_INVALID_PARAMETER);
        mbedtls_pk_context* ikey = &pkctx;
#else
        BIGNUM* rm = BN_bin2bn(reversed_modulus, OE_KEY_SIZE, 0);
        BIGNUM* re = BN_bin2bn(reversed_exponent, OE_EXPONENT_SIZE, 0);
        RSA* rsa = RSA_new();
        RSA_set0_key(rsa, rm, re, NULL);
        EVP_PKEY* ikey = EVP_PKEY_new();
        EVP_PKEY_assign_RSA(ikey, rsa);
#endif
        oe_rsa_public_key_init(&pk, ikey);

        OE_CHECK(oe_rsa_public_key_verify(
            &pk,
            OE_HASH_TYPE_SHA256,
            msg_hsh.buf,
            sizeof(msg_hsh.buf),
            reversed_signature,
            OE_KEY_SIZE));

        oe_rsa_public_key_free(&pk);

#ifdef OE_BUILD_ENCLAVE
        mbedtls_pk_free(ikey);
#else
        EVP_PKEY_free(ikey);
#endif
    }

done:

    return result;
}