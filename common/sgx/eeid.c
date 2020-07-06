// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <stdio.h>
#include <string.h>

#include <openenclave/bits/attestation.h>
#include <openenclave/bits/eeid.h>
#include <openenclave/bits/sgx/sgxtypes.h>
#include <openenclave/internal/crypto/sha.h>
#include <openenclave/internal/eeid.h>
#include <openenclave/internal/hexdump.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/safecrt.h>
#include <openenclave/internal/types.h>

#include "sgxmeasure.h"

#ifdef OE_BUILD_ENCLAVE
#include <openenclave/enclave.h>
#include "../../enclave/crypto/key.h"
#include "../../enclave/crypto/rsa.h"
#else
#include <openenclave/host.h>
#include <openssl/opensslv.h>
#include <openssl/rsa.h>
#include "../../host/crypto/openssl/key.h"
#include "../../host/crypto/openssl/rsa.h"
#endif

int is_eeid_base_image(const oe_enclave_size_settings_t* sizes)
{
    return sizes->num_heap_pages == 0 && sizes->num_stack_pages == 0 &&
           sizes->num_tcs == 1;
}

oe_result_t oe_create_eeid_sgx(oe_eeid_t** eeid)
{
    oe_result_t result = OE_UNEXPECTED;
    size_t signature_size = sizeof(sgx_sigstruct_t);

    if (!eeid)
        OE_RAISE(OE_INVALID_PARAMETER);

    *eeid = calloc(sizeof(oe_eeid_t) + signature_size, 1);
    if (!*eeid)
        OE_RAISE(OE_OUT_OF_MEMORY);
    (*eeid)->signature_size = signature_size;

    result = OE_OK;
done:
    return result;
}

static oe_result_t serialize_element(
    const char* name,
    char** position,
    size_t* remaining,
    const uint8_t* element,
    size_t element_size)
{
    oe_result_t result = OE_UNEXPECTED;

    size_t name_sz = strlen(name);

    if (*remaining < 2 * element_size + 1 + name_sz + 1)
        OE_RAISE(OE_BUFFER_TOO_SMALL);

    snprintf(*position, *remaining, "%s=", name);
    *position += name_sz + 1;
    *remaining -= name_sz + 1;

    oe_hex_string(*position, *remaining, element, element_size);
    *position += 2 * element_size;
    **position = '\n';
    *position += 1;
    *remaining -= 2 * element_size + 1;

    result = OE_OK;

done:
    return result;
}

static oe_result_t deserialize_element(
    const char** position,
    size_t* remaining,
    uint8_t* element,
    size_t element_size)
{
    oe_result_t result = OE_UNEXPECTED;

    // Skip name
    while (**position != ' ')
    {
        *position += 1;
        *remaining -= 1;
    }
    *position += 1;
    *remaining -= 1;

    if (*remaining < 2 * element_size + 2)
        return OE_OUT_OF_BOUNDS;

    for (size_t i = 0; i < element_size; i++)
    {
        unsigned digit;
        if (sscanf(*position, "%02x", &digit) != 1)
            OE_RAISE(OE_INVALID_PARAMETER);
        element[i] = (uint8_t)digit;
        *position += 2;
        *remaining -= 2;
    }

    if (**position != '\n')
        OE_RAISE(OE_INVALID_PARAMETER);

    *position += 1;
    *remaining -= 1;

    result = OE_OK;

done:
    return result;
}

// Note: this needs to be updated when oe_eeid_t changes.
oe_result_t oe_serialize_eeid(
    const oe_eeid_t* eeid,
    const uint8_t* data,
    size_t data_size,
    char* buf,
    size_t buf_size)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!eeid || !buf || !buf_size)
        OE_RAISE(OE_INVALID_PARAMETER);

    char** position = &buf;
    size_t remaining = buf_size;

    OE_CHECK(serialize_element(
        "version",
        position,
        &remaining,
        (uint8_t*)&eeid->version,
        sizeof(eeid->version)));
    OE_CHECK(serialize_element(
        "H",
        position,
        &remaining,
        (uint8_t*)eeid->hash_state.H,
        sizeof(eeid->hash_state.H)));
    OE_CHECK(serialize_element(
        "N",
        position,
        &remaining,
        (uint8_t*)eeid->hash_state.N,
        sizeof(eeid->hash_state.N)));
    OE_CHECK(serialize_element(
        "size_settings",
        position,
        &remaining,
        (uint8_t*)&eeid->size_settings,
        sizeof(eeid->size_settings)));
    OE_CHECK(serialize_element(
        "vaddr",
        position,
        &remaining,
        (uint8_t*)&eeid->vaddr,
        sizeof(eeid->vaddr)));
    OE_CHECK(serialize_element(
        "entry_point",
        position,
        &remaining,
        (uint8_t*)&eeid->entry_point,
        sizeof(eeid->entry_point)));
    OE_CHECK(serialize_element(
        "signature_size",
        position,
        &remaining,
        (uint8_t*)&eeid->signature_size,
        sizeof(eeid->signature_size)));
    OE_CHECK(serialize_element(
        "data_size",
        position,
        &remaining,
        (uint8_t*)&data_size,
        sizeof(data_size)));
    OE_CHECK(serialize_element(
        "data", position, &remaining, (uint8_t*)data, data_size));

    **position = '\0';

    result = OE_OK;

done:
    return result;
}

// Note: this needs to be updated when oe_eeid_t changes.
oe_result_t oe_deserialize_eeid(
    const char* buf,
    size_t buf_size,
    oe_eeid_t* eeid,
    uint8_t** data,
    size_t* data_size)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!buf || !buf_size || !eeid)
        OE_RAISE(OE_INVALID_PARAMETER);

    const char** position = &buf;
    size_t remaining = buf_size;

    memset(eeid, 0, sizeof(oe_eeid_t));

    OE_CHECK(deserialize_element(
        position, &remaining, (uint8_t*)&eeid->version, sizeof(eeid->version)));
    if (eeid->version != OE_EEID_VERSION)
        OE_RAISE(OE_INVALID_PARAMETER);
    OE_CHECK(deserialize_element(
        position,
        &remaining,
        (uint8_t*)eeid->hash_state.H,
        sizeof(eeid->hash_state.H)));
    OE_CHECK(deserialize_element(
        position,
        &remaining,
        (uint8_t*)eeid->hash_state.N,
        sizeof(eeid->hash_state.N)));
    OE_CHECK(deserialize_element(
        position,
        &remaining,
        (uint8_t*)&eeid->size_settings,
        sizeof(eeid->size_settings)));
    OE_CHECK(deserialize_element(
        position, &remaining, (uint8_t*)&eeid->vaddr, sizeof(eeid->vaddr)));
    OE_CHECK(deserialize_element(
        position,
        &remaining,
        (uint8_t*)&eeid->entry_point,
        sizeof(eeid->entry_point)));
    OE_CHECK(deserialize_element(
        position,
        &remaining,
        (uint8_t*)&eeid->signature_size,
        sizeof(eeid->signature_size)));
    eeid = realloc(eeid, sizeof(oe_eeid_t) + eeid->signature_size);
    if (!eeid)
        OE_RAISE(OE_OUT_OF_MEMORY);
    OE_CHECK(deserialize_element(
        position, &remaining, (uint8_t*)eeid->signature, eeid->signature_size));
    OE_CHECK(deserialize_element(
        position, &remaining, (uint8_t*)data_size, sizeof(*data_size)));
    if (data)
    {
        *data = malloc(*data_size);
        if (!*data)
            OE_RAISE(OE_OUT_OF_MEMORY);
        OE_CHECK(deserialize_element(
            position, &remaining, (uint8_t*)data, *data_size));
    }

    result = OE_OK;

done:
    return result;
}

static oe_result_t _measure_page(
    oe_sha256_context_t* hctx,
    uint64_t base,
    void* page,
    uint64_t* vaddr,
    bool extend,
    bool readonly)
{
    oe_result_t result = OE_UNEXPECTED;

    uint64_t flags = SGX_SECINFO_REG | SGX_SECINFO_R;

    if (!readonly)
        flags |= SGX_SECINFO_W;

    OE_CHECK(oe_sgx_measure_load_enclave_data(
        hctx, base, base + *vaddr, (uint64_t)page, flags, extend));
    *vaddr += OE_PAGE_SIZE;
    result = OE_OK;

done:
    return result;
}

static oe_result_t _remeasure_eeid_page(
    oe_sha256_context_t* hctx,
    uint64_t base,
    uint64_t* vaddr,
    const oe_eeid_t* eeid,
    const OE_SHA256* config_id)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!eeid || !config_id)
        OE_RAISE(OE_VERIFY_FAILED);

    size_t eeid_page_size = sizeof(oe_eeid_page_t) + eeid->signature_size;
    if (eeid_page_size > OE_PAGE_SIZE)
        return OE_VERIFY_FAILED;

    oe_eeid_page_t* eeid_page = calloc(1, OE_PAGE_SIZE);
    memcpy(&eeid_page->eeid, eeid, sizeof(oe_eeid_t) + eeid->signature_size);
    memcpy(eeid_page->config_id.buf, config_id->buf, sizeof(config_id->buf));

    OE_CHECK(_add_page(hctx, base, eeid_page, vaddr, true, true));
    free(eeid_page);

    result = OE_OK;

done:
    return result;
}

oe_result_t oe_remeasure_memory_pages(
    const oe_eeid_t* eeid,
    const OE_SHA256* config_id,
    OE_SHA256* computed_enclave_hash,
    bool with_eeid_page)
{
    oe_result_t result;
    oe_sha256_context_t hctx;
    oe_sha256_restore(&hctx, eeid->hash_state.H, eeid->hash_state.N);

    uint64_t base = 0x0ab0c0d0e0f;
    oe_page_t blank_pg, stack_pg, tcs_pg;
    memset(blank_pg.data, 0, sizeof(blank_pg));
    memset(stack_pg.data, 0xcc, sizeof(stack_pg));

    uint64_t vaddr = eeid->vaddr;

    if (with_eeid_page)
        OE_CHECK(_remeasure_eeid_page(&hctx, base, &vaddr, eeid, config_id));
    else
        OE_CHECK(_add_page(&hctx, base, blank_pg.data, &vaddr, true, true));

    // This is where we replay the addition of memory pages, both, for
    // verification of the extended image hash (with_eeid_pages=true) and
    // the base image hash, for which there are no EEID pages, but one TCS
    // page.

    for (size_t i = 0; i < eeid->size_settings.num_heap_pages; i++)
        OE_CHECK(_measure_page(&hctx, base, &blank_pg, &vaddr, false, false));

    for (size_t i = 0; i < eeid->size_settings.num_tcs; i++)
    {
        vaddr += OE_PAGE_SIZE; /* guard page */

        for (size_t i = 0; i < eeid->size_settings.num_stack_pages; i++)
            OE_CHECK(
                _measure_page(&hctx, base, &stack_pg, &vaddr, true, false));

        vaddr += OE_PAGE_SIZE; /* guard page */

        sgx_tcs_t* tcs;
        memset(&tcs_pg, 0, sizeof(tcs_pg));
        tcs = (sgx_tcs_t*)&tcs_pg;
        tcs->flags = 0;
        tcs->ossa = vaddr + OE_PAGE_SIZE;
        tcs->cssa = 0;
        tcs->nssa = 2;
        tcs->oentry = eeid->entry_point;
        tcs->fsbase = vaddr + (5 * OE_PAGE_SIZE);
        tcs->gsbase = tcs->fsbase;
        tcs->fslimit = 0xFFFFFFFF;
        tcs->gslimit = 0xFFFFFFFF;

        OE_CHECK(oe_sgx_measure_load_enclave_data(
            &hctx,
            base,
            base + vaddr,
            (uint64_t)&tcs_pg,
            SGX_SECINFO_TCS,
            true));

        vaddr += OE_PAGE_SIZE;

        for (size_t i = 0; i < 2; i++)
            _measure_page(&hctx, base, &blank_pg, &vaddr, true, false);

        vaddr += OE_PAGE_SIZE; /* guard page */

        for (size_t i = 0; i < 2; i++)
            _measure_page(&hctx, base, &blank_pg, &vaddr, true, false);
    }

    oe_sha256_final(&hctx, computed_enclave_hash);

done:
    return OE_OK;
}

static bool is_zero(const uint8_t* buf, size_t sz)
{
    while (sz != 0)
        if (buf[--sz] != 0)
            return false;
    return true;
}

static oe_result_t verify_base_image_signature(const sgx_sigstruct_t* sigstruct)
{
    oe_result_t result = OE_UNEXPECTED;
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
        reversed_exponent[i] = sigstruct->exponent[OE_EXPONENT_SIZE - 1 - i];

    uint8_t reversed_signature[OE_KEY_SIZE];
    for (size_t i = 0; i < OE_KEY_SIZE; i++)
        reversed_signature[i] = sigstruct->signature[OE_KEY_SIZE - 1 - i];

    oe_rsa_public_key_t pk;

#ifdef WINDOWS_BUILD
#error EEID is currently no supported on Windows
#endif

#ifdef OE_BUILD_ENCLAVE
    mbedtls_pk_context pkctx;
    mbedtls_pk_init(&pkctx);
    const mbedtls_pk_info_t* info = mbedtls_pk_info_from_type(MBEDTLS_PK_RSA);
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
#if OPENSSL_VERSION_NUMBER < 0x1010100fL
#error OpenSSL 1.0.2 not supported
#endif
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
    // The OpenSSL flavour of oe_rsa_public_key_init does not copy the key,
    // so oe_rsa_public_key_free already freed it.
#endif

    result = OE_OK;

done:
    return result;
}

oe_result_t verify_eeid(
    const uint8_t* reported_enclave_hash,
    const uint8_t* reported_enclave_signer,
    uint16_t reported_product_id,
    uint32_t reported_security_version,
    uint64_t reported_attributes,
    const uint8_t** base_enclave_hash,
    const oe_eeid_t* eeid,
    const OE_SHA256* config_id,
    const oe_enclave_size_settings_t* base_image_sizes)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!eeid)
        OE_RAISE(OE_INVALID_PARAMETER);

    if (eeid->signature_size != 1808) // We only support SGX sigstructs for now.
        OE_RAISE(OE_UNSUPPORTED);

    // Compute expected enclave hash
    OE_SHA256 computed_enclave_hash;
    oe_remeasure_memory_pages(eeid, config_id, &computed_enclave_hash, true);

    // Check recomputed enclave hash against reported enclave hash
    if (memcmp(
            computed_enclave_hash.buf, reported_enclave_hash, OE_SHA256_SIZE) !=
        0)
        OE_RAISE(OE_VERIFY_FAILED);

    if (memcmp(
            OE_DEBUG_PUBLIC_KEY, reported_enclave_signer, OE_SIGNER_ID_SIZE) !=
        0)
        OE_RAISE(OE_VERIFY_FAILED);

    const sgx_sigstruct_t* sigstruct = (sgx_sigstruct_t*)eeid->signature;

    // Compute and check base image hash
    *base_enclave_hash = sigstruct->enclavehash;
    OE_SHA256 computed_base_enclave_hash;
    oe_eeid_t tmp_eeid = *eeid;
    tmp_eeid.size_settings = *base_image_sizes;

    oe_remeasure_memory_pages(
        &tmp_eeid, NULL, &computed_base_enclave_hash, false);

    if (memcmp(
            computed_base_enclave_hash.buf,
            *base_enclave_hash,
            OE_SHA256_SIZE) != 0)
        OE_RAISE(OE_VERIFY_FAILED);

    // Check other image properties have not changed
    bool base_debug = sigstruct->attributes.flags & SGX_FLAGS_DEBUG;
    bool extended_debug = reported_attributes & OE_REPORT_ATTRIBUTES_DEBUG;

    if (base_debug != extended_debug ||
        sigstruct->isvprodid != reported_product_id ||
        sigstruct->isvsvn != reported_security_version)
        OE_RAISE(OE_VERIFY_FAILED);

    // Check old signature (new signature has been checked above)
    if (base_debug && is_zero(sigstruct->signature, OE_KEY_SIZE))
        return OE_OK; // Unsigned debug image is ok.
    else
        OE_CHECK(verify_base_image_signature(sigstruct));

    result = OE_OK;

done:
    return result;
}

static oe_result_t _hton_uint32_t(
    uint32_t x,
    uint8_t** position,
    size_t* remaining)
{
    if (*remaining < 4)
        return OE_INVALID_PARAMETER;

    for (size_t i = 0; i < 4; i++)
        (*position)[i] = (x >> (32 - ((i + 1) * 8))) & 0xFF;

    *position += 4;
    *remaining -= 4;

    return OE_OK;
}

static oe_result_t _ntoh_uint32_t(
    const uint8_t** position,
    size_t* remaining,
    uint32_t* x)
{
    if (*remaining < 4)
        return OE_INVALID_PARAMETER;

    *x = 0;
    for (size_t i = 0; i < 4; i++)
        *x = (*x << 8) | (*position)[i];

    *position += 4;
    *remaining -= 4;

    return OE_OK;
}

oe_result_t hton_uint64_t(uint64_t x, uint8_t** position, size_t* remaining)
{
    if (*remaining < 8)
        return OE_INVALID_PARAMETER;

    for (size_t i = 0; i < 8; i++)
        (*position)[i] = (x >> (64 - ((i + 1) * 8))) & 0xFF;

    *position += 8;
    *remaining -= 8;

    return OE_OK;
}

oe_result_t ntoh_uint64_t(
    const uint8_t** position,
    size_t* remaining,
    uint64_t* x)
{
    if (*remaining < 8)
        return OE_INVALID_PARAMETER;

    *x = 0;
    for (size_t i = 0; i < 8; i++)
        *x = (*x << 8) | (*position)[i];

    *position += 8;
    *remaining -= 8;

    return OE_OK;
}

static oe_result_t _hton_buffer(
    const uint8_t* buffer,
    size_t buffer_size,
    uint8_t** position,
    size_t* remaining)
{
    if (*remaining < buffer_size)
        return OE_INVALID_PARAMETER;
    memcpy(*position, buffer, buffer_size);
    *position += buffer_size;
    *remaining -= buffer_size;
    return OE_OK;
}

static oe_result_t _ntoh_buffer(
    const uint8_t** position,
    size_t* remaining,
    uint8_t* buffer,
    size_t buffer_size)
{
    if (*remaining < buffer_size)
        return OE_INVALID_PARAMETER;
    memcpy(buffer, *position, buffer_size);
    *position += buffer_size;
    *remaining -= buffer_size;
    return OE_OK;
}

size_t oe_eeid_byte_size(const oe_eeid_t* eeid)
{
    return sizeof(eeid->version) + sizeof(eeid->hash_state) +
           sizeof(eeid->signature_size) + sizeof(eeid->size_settings) +
           sizeof(eeid->vaddr) + sizeof(eeid->entry_point) +
           eeid->signature_size;
}

oe_result_t oe_eeid_hton(
    const oe_eeid_t* eeid,
    uint8_t* buffer,
    size_t buffer_size)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!eeid || !buffer || buffer_size == 0 ||
        eeid->version != OE_EEID_VERSION)
        OE_RAISE(OE_INVALID_PARAMETER);

    uint8_t* position = buffer;
    size_t remaining = buffer_size;

    OE_CHECK(_hton_uint32_t(eeid->version, &position, &remaining));

    for (size_t i = 0; i < 8; i++)
        OE_CHECK(_hton_uint32_t(eeid->hash_state.H[i], &position, &remaining));
    for (size_t i = 0; i < 2; i++)
        OE_CHECK(_hton_uint32_t(eeid->hash_state.N[i], &position, &remaining));

    OE_CHECK(hton_uint64_t(
        eeid->size_settings.num_heap_pages, &position, &remaining));
    OE_CHECK(hton_uint64_t(
        eeid->size_settings.num_stack_pages, &position, &remaining));
    OE_CHECK(hton_uint64_t(eeid->size_settings.num_tcs, &position, &remaining));

    OE_CHECK(hton_uint64_t(eeid->vaddr, &position, &remaining));
    OE_CHECK(hton_uint64_t(eeid->entry_point, &position, &remaining));

    OE_CHECK(hton_uint64_t(eeid->signature_size, &position, &remaining));
    OE_CHECK(_hton_buffer(
        eeid->signature, eeid->signature_size, &position, &remaining));

    result = OE_OK;
done:
    return result;
}

oe_result_t oe_eeid_ntoh(
    const uint8_t* buffer,
    size_t buffer_size,
    oe_eeid_t* eeid)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!buffer || buffer_size == 0 || !eeid)
        OE_RAISE(OE_INVALID_PARAMETER);

    const uint8_t* position = buffer;
    size_t remaining = buffer_size;

    OE_CHECK(_ntoh_uint32_t(&position, &remaining, &eeid->version));

    if (eeid->version != OE_EEID_VERSION)
        OE_RAISE(OE_INVALID_PARAMETER);

    for (size_t i = 0; i < 8; i++)
        OE_CHECK(_ntoh_uint32_t(&position, &remaining, &eeid->hash_state.H[i]));
    for (size_t i = 0; i < 2; i++)
        OE_CHECK(_ntoh_uint32_t(&position, &remaining, &eeid->hash_state.N[i]));

    OE_CHECK(ntoh_uint64_t(
        &position, &remaining, &eeid->size_settings.num_heap_pages));
    OE_CHECK(ntoh_uint64_t(
        &position, &remaining, &eeid->size_settings.num_stack_pages));
    OE_CHECK(
        ntoh_uint64_t(&position, &remaining, &eeid->size_settings.num_tcs));

    OE_CHECK(ntoh_uint64_t(&position, &remaining, &eeid->vaddr));
    OE_CHECK(ntoh_uint64_t(&position, &remaining, &eeid->entry_point));

    OE_CHECK(ntoh_uint64_t(&position, &remaining, &eeid->signature_size));
    OE_CHECK(_ntoh_buffer(
        &position, &remaining, eeid->signature, eeid->signature_size));

    result = OE_OK;
done:
    return result;
}

oe_result_t oe_eeid_evidence_hton(
    const oe_eeid_evidence_t* evidence,
    uint8_t* buffer,
    size_t buffer_size)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!buffer || buffer_size == 0 || !evidence)
        OE_RAISE(OE_INVALID_PARAMETER);

    uint8_t* position = buffer;
    size_t remaining = buffer_size;

    OE_CHECK(hton_uint64_t(evidence->sgx_evidence_size, &position, &remaining));
    OE_CHECK(
        hton_uint64_t(evidence->sgx_endorsements_size, &position, &remaining));
    OE_CHECK(hton_uint64_t(evidence->eeid_size, &position, &remaining));
    OE_CHECK(hton_uint64_t(evidence->config_id_size, &position, &remaining));

    size_t data_size = evidence->sgx_evidence_size +
                       evidence->sgx_endorsements_size + evidence->eeid_size +
                       evidence->config_id_size;

    OE_CHECK(_hton_buffer(evidence->data, data_size, &position, &remaining));

    result = OE_OK;
done:
    return result;
}

oe_result_t oe_eeid_evidence_ntoh(
    const uint8_t* buffer,
    size_t buffer_size,
    oe_eeid_evidence_t* evidence)
{
    oe_result_t result = OE_UNEXPECTED;

    if (!buffer || buffer_size == 0 || !evidence)
        OE_RAISE(OE_INVALID_PARAMETER);

    const uint8_t* position = buffer;
    size_t remaining = buffer_size;

    OE_CHECK(
        ntoh_uint64_t(&position, &remaining, &evidence->sgx_evidence_size));
    OE_CHECK(
        ntoh_uint64_t(&position, &remaining, &evidence->sgx_endorsements_size));
    OE_CHECK(ntoh_uint64_t(&position, &remaining, &evidence->eeid_size));
    OE_CHECK(ntoh_uint64_t(&position, &remaining, &evidence->config_id_size));

    size_t data_size = evidence->sgx_evidence_size +
                       evidence->sgx_endorsements_size + evidence->eeid_size +
                       evidence->config_id_size;

    OE_CHECK(_ntoh_buffer(
        &position, &remaining, (uint8_t*)&evidence->data, data_size));

    result = OE_OK;
done:
    return result;
}
