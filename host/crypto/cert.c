// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <ctype.h>
#include <openenclave/bits/cert.h>
#include <openenclave/bits/enclavelibc.h>
#include <openenclave/bits/pem.h>
#include <openenclave/bits/raise.h>
#include <openenclave/result.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "ec.h"
#include "init.h"
#include "rsa.h"

/*
**==============================================================================
**
** Local definitions:
**
**==============================================================================
*/

/* Randomly generated magic number */
#define OE_CERT_MAGIC 0xbc8e184285de4d2a

static void _SetErr(OE_VerifyCertError* error, const char* str)
{
    if (error)
        OE_Strlcpy(error->buf, str, sizeof(error->buf));
}

typedef struct _Cert
{
    uint64_t magic;
    X509* x509;
} Cert;

static void _CertInit(Cert* impl, X509* x509)
{
    if (impl)
    {
        impl->magic = OE_CERT_MAGIC;
        impl->x509 = x509;
    }
}

static bool _CertIsValid(const Cert* impl)
{
    return impl && (impl->magic == OE_CERT_MAGIC) && impl->x509;
}

static void _CertClear(Cert* impl)
{
    if (impl)
    {
        impl->magic = 0;
        impl->x509 = NULL;
    }
}

/* Randomly generated magic number */
#define OE_CERT_CHAIN_MAGIC 0xa5ddf70fb28f4480

typedef struct _CertChain
{
    uint64_t magic;
    STACK_OF(X509) * sk;
} CertChain;

static void _CertChainInit(CertChain* impl, STACK_OF(X509) * sk)
{
    if (impl)
    {
        impl->magic = OE_CERT_CHAIN_MAGIC;
        impl->sk = sk;
    }
}

static bool _CertChainIsValid(const CertChain* impl)
{
    return impl && (impl->magic == OE_CERT_CHAIN_MAGIC) && impl->sk;
}

static void _CertChainClear(CertChain* impl)
{
    if (impl)
    {
        impl->magic = 0;
        impl->sk = NULL;
    }
}

static STACK_OF(X509) * _ReadCertChain(const char* pem)
{
    STACK_OF(X509)* result = NULL;
    STACK_OF(X509)* sk = NULL;
    BIO* bio = NULL;
    X509* x509 = NULL;

    // Check parameters:
    if (!pem)
        goto done;

    // Create empty X509 stack:
    if (!(sk = sk_X509_new_null()))
        goto done;

    while (*pem)
    {
        const char* end;

        /* The PEM certificate must start with this */
        if (strncmp(
                pem, OE_PEM_BEGIN_CERTIFICATE, OE_PEM_BEGIN_CERTIFICATE_LEN) !=
            0)
            goto done;

        /* Find the end of this PEM certificate */
        {
            if (!(end = strstr(pem, OE_PEM_END_CERTIFICATE)))
                goto done;

            end += OE_PEM_END_CERTIFICATE_LEN;
        }

        /* Skip trailing spaces */
        while (isspace(*end))
            end++;

        /* Create a BIO for this certificate */
        if (!(bio = BIO_new_mem_buf(pem, end - pem)))
            goto done;

        /* Read BIO into X509 object */
        if (!(x509 = PEM_read_bio_X509(bio, NULL, 0, NULL)))
            goto done;

        // Push certificate onto stack:
        {
            if (!sk_X509_push(sk, x509))
                goto done;

            x509 = NULL;
        }

        // Release the bio:
        BIO_free(bio);
        bio = NULL;

        pem = end;
    }

    result = sk;
    sk = NULL;

done:

    if (bio)
        BIO_free(bio);

    if (sk)
        sk_X509_pop_free(sk, X509_free);

    return result;
}

/* Clone the certificate to clear any verification state */
static X509* _CloneX509(X509* x509)
{
    X509* ret = NULL;
    BIO* out = NULL;
    BIO* in = NULL;
    BUF_MEM* mem;

    if (!x509)
        goto done;

    if (!(out = BIO_new(BIO_s_mem())))
        goto done;

    if (!PEM_write_bio_X509(out, x509))
        goto done;

    if (!BIO_get_mem_ptr(out, &mem))
        goto done;

    if (!(in = BIO_new_mem_buf(mem->data, mem->length)))
        goto done;

    ret = PEM_read_bio_X509(in, NULL, 0, NULL);

done:

    if (out)
        BIO_free(out);

    if (in)
        BIO_free(in);

    return ret;
}

/* Needed because some versions of OpenSSL do not support X509_up_ref() */
static int _X509_up_ref(X509* x509)
{
    if (!x509)
        return 0;

    CRYPTO_add(&x509->references, 1, CRYPTO_LOCK_X509);
    return 1;
}

static OE_Result _CertChainGetLength(const CertChain* impl, int* length)
{
    OE_Result result = OE_UNEXPECTED;
    int num;

    *length = 0;

    if ((num = sk_X509_num(impl->sk)) <= 0)
        OE_RAISE(OE_FAILURE);

    *length = (size_t)num;

    result = OE_OK;

done:
    return result;
}

/* Verify a certificate against a chain of length 1 */
static OE_Result _VerifyCert(X509* cert_, X509* chain_)
{
    OE_Result result = OE_UNEXPECTED;
    X509_STORE_CTX* ctx = NULL;
    X509* cert = NULL;
    X509* chain = NULL;
    STACK_OF(X509)* sk = NULL;

    /* Clone the certificate to clear any cached verification state */
    if (!(cert = _CloneX509(cert_)))
        OE_RAISE(OE_FAILURE);

    /* Clone the chain to clear any cached verification state */
    if (!(chain = _CloneX509(chain_)))
        OE_RAISE(OE_FAILURE);

    /* Create a context for verification */
    if (!(ctx = X509_STORE_CTX_new()))
        OE_RAISE(OE_FAILURE);

    /* Initialize the context that will be used to verify the certificate */
    if (!X509_STORE_CTX_init(ctx, NULL, NULL, NULL))
        OE_RAISE(OE_FAILURE);

    /* Inject the certificate into the verification context */
    X509_STORE_CTX_set_cert(ctx, cert);

    /* Build a certificate chain that contains a single certificate */
    {
        if (!(sk = sk_X509_new(NULL)))
            OE_RAISE(OE_FAILURE);

        if (!sk_X509_push(sk, chain))
            OE_RAISE(OE_FAILURE);

        if (!_X509_up_ref(chain))
            OE_RAISE(OE_FAILURE);
    }

    /* Set the CA chain into the verification context */
    X509_STORE_CTX_trusted_stack(ctx, sk);

    /* Finally verify the certificate */
    if (!X509_verify_cert(ctx))
        OE_RAISE(OE_FAILURE);

    result = OE_OK;

done:

    if (cert)
        X509_free(cert);

    if (chain)
        X509_free(chain);

    if (ctx)
        X509_STORE_CTX_free(ctx);

    if (sk)
        sk_X509_pop_free(sk, X509_free);

    return result;
}

/* Verify each certificate in the chain against its predecessor. */
static OE_Result _VerifyWholeChain(STACK_OF(X509) * chain)
{
    OE_Result result = OE_UNEXPECTED;

    if (!chain)
        OE_RAISE(OE_INVALID_PARAMETER);

    for (int i = 0, n = sk_X509_num(chain); i < n; i++)
    {
        if (i + 1 != n)
        {
            X509* cert = sk_X509_value(chain, i);
            X509* certNext = sk_X509_value(chain, i + 1);

            if (!cert || !certNext)
                OE_RAISE(OE_FAILURE);

            OE_CHECK(_VerifyCert(certNext, cert));
        }
    }

    result = OE_OK;

done:
    return result;
}

/*
**==============================================================================
**
** Public functions
**
**==============================================================================
*/

OE_Result OE_CertReadPEM(const void* pemData, size_t pemSize, OE_Cert* cert)
{
    OE_Result result = OE_UNEXPECTED;
    Cert* impl = (Cert*)cert;
    BIO* bio = NULL;
    X509* x509 = NULL;

    /* Zero-initialize the implementation */
    if (impl)
        impl->magic = 0;

    /* Check parameters */
    if (!pemData || !pemSize || !cert)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Must have pemSize-1 non-zero characters followed by zero-terminator */
    if (strnlen((const char*)pemData, pemSize) != pemSize - 1)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Initialize OpenSSL (if not already initialized) */
    OE_InitializeOpenSSL();

    /* Create a BIO object for reading the PEM data */
    if (!(bio = BIO_new_mem_buf(pemData, pemSize)))
        OE_RAISE(OE_FAILURE);

    /* Convert the PEM BIO into a certificate object */
    if (!(x509 = PEM_read_bio_X509(bio, NULL, 0, NULL)))
        OE_RAISE(OE_FAILURE);

    _CertInit(impl, x509);
    x509 = NULL;

    result = OE_OK;

done:

    if (bio)
        BIO_free(bio);

    if (x509)
        X509_free(x509);

    return result;
}

OE_Result OE_CertFree(OE_Cert* cert)
{
    OE_Result result = OE_UNEXPECTED;
    Cert* impl = (Cert*)cert;

    /* Check parameters */
    if (!_CertIsValid(impl))
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Free the certificate */
    X509_free(impl->x509);
    _CertClear(impl);

    result = OE_OK;

done:
    return result;
}

OE_Result OE_CertChainReadPEM(
    const void* pemData,
    size_t pemSize,
    OE_CertChain* chain)
{
    OE_Result result = OE_UNEXPECTED;
    CertChain* impl = (CertChain*)chain;
    STACK_OF(X509)* sk = NULL;

    /* Zero-initialize the implementation */
    if (impl)
        impl->magic = 0;

    /* Check parameters */
    if (!pemData || !pemSize || !chain)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Must have pemSize-1 non-zero characters followed by zero-terminator */
    if (strnlen((const char*)pemData, pemSize) != pemSize - 1)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Initialize OpenSSL (if not already initialized) */
    OE_InitializeOpenSSL();

    /* Read the certificate chain into memory */
    if (!(sk = _ReadCertChain((const char*)pemData)))
        OE_RAISE(OE_FAILURE);

    /* Verify the whole certificate chain */
    OE_CHECK(_VerifyWholeChain(sk));

    _CertChainInit(impl, sk);

    result = OE_OK;

done:

    return result;
}

OE_Result OE_CertChainFree(OE_CertChain* chain)
{
    OE_Result result = OE_UNEXPECTED;
    CertChain* impl = (CertChain*)chain;

    /* Check the parameter */
    if (_CertChainIsValid(impl))
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Release the stack of certificates */
    sk_X509_pop_free(impl->sk, X509_free);

    /* Clear the implementation */
    _CertChainClear(impl);

    result = OE_OK;

done:
    return result;
}

OE_Result OE_CertVerify(
    OE_Cert* cert,
    OE_CertChain* chain,
    OE_CRL* crl, /* ATTN: placeholder for future feature work */
    OE_VerifyCertError* error)
{
    OE_Result result = OE_UNEXPECTED;
    Cert* certImpl = (Cert*)cert;
    CertChain* chainImpl = (CertChain*)chain;
    X509_STORE_CTX* ctx = NULL;
    X509* x509 = NULL;

    /* Initialize error to NULL for now */
    if (error)
        *error->buf = '\0';

    /* Check for invalid cert parameter */
    if (!_CertIsValid(certImpl))
    {
        _SetErr(error, "invalid cert parameter");
        OE_RAISE(OE_INVALID_PARAMETER);
    }

    /* Check for invalid chain parameter */
    if (!_CertChainIsValid(chainImpl))
    {
        _SetErr(error, "invalid chain parameter");
        OE_RAISE(OE_INVALID_PARAMETER);
    }

    /* We must make a copy of the certificate, else previous successful
     * verifications cause subsequent bad verifications to succeed. It is
     * likely that some state is stored in the certificate upon successful
     * verification. We can clear this by making a copy.
     */
    if (!(x509 = _CloneX509(certImpl->x509)))
    {
        _SetErr(error, "invalid X509 certificate");
        OE_RAISE(OE_FAILURE);
    }

    /* Initialize OpenSSL (if not already initialized) */
    OE_InitializeOpenSSL();

    /* Create a context for verification */
    if (!(ctx = X509_STORE_CTX_new()))
    {
        _SetErr(error, "failed to allocate X509 context");
        OE_RAISE(OE_FAILURE);
    }

    /* Initialize the context that will be used to verify the certificate */
    if (!X509_STORE_CTX_init(ctx, NULL, NULL, NULL))
    {
        _SetErr(error, "failed to initialize X509 context");
        OE_RAISE(OE_FAILURE);
    }

    /* Set the certificate into the verification context */
    X509_STORE_CTX_set_cert(ctx, x509);

    /* Set the CA chain into the verification context */
    X509_STORE_CTX_trusted_stack(ctx, chainImpl->sk);

    /* Finally verify the certificate */
    if (!X509_verify_cert(ctx))
    {
        if (error)
            _SetErr(error, X509_verify_cert_error_string(ctx->error));

        OE_RAISE(OE_VERIFY_FAILED);
    }

    result = OE_OK;

done:

    if (ctx)
        X509_STORE_CTX_free(ctx);

    if (x509)
        X509_free(x509);

    return result;
}

OE_Result OE_CertGetRSAPublicKey(
    const OE_Cert* cert,
    OE_RSAPublicKey* publicKey)
{
    OE_Result result = OE_UNEXPECTED;
    const Cert* impl = (const Cert*)cert;
    EVP_PKEY* pkey = NULL;
    RSA* rsa = NULL;

    /* Clear public key for all error pathways */
    if (publicKey)
        memset(publicKey, 0, sizeof(OE_RSAPublicKey));

    /* Reject invalid parameters */
    if (!_CertIsValid(impl) || !publicKey)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Get public key (increments reference count) */
    if (!(pkey = X509_get_pubkey(impl->x509)))
        OE_RAISE(OE_FAILURE);

    /* Get RSA public key (increments reference count) */
    if (!(rsa = EVP_PKEY_get1_RSA(pkey)))
        OE_RAISE(OE_WRONG_TYPE);

    /* Initialize the RSA public key */
    OE_RSAPublicKeyInit(publicKey, pkey);
    pkey = NULL;

    result = OE_OK;

done:

    if (pkey)
    {
        /* Decrement reference count (incremented above) */
        EVP_PKEY_free(pkey);
    }

    return result;
}

OE_Result OE_CertGetECPublicKey(const OE_Cert* cert, OE_ECPublicKey* publicKey)
{
    OE_Result result = OE_UNEXPECTED;
    const Cert* impl = (const Cert*)cert;
    EVP_PKEY* pkey = NULL;

    /* Clear public key for all error pathways */
    if (publicKey)
        memset(publicKey, 0, sizeof(OE_ECPublicKey));

    /* Reject invalid parameters */
    if (!_CertIsValid(impl) || !publicKey)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Get public key (increments reference count) */
    if (!(pkey = X509_get_pubkey(impl->x509)))
        OE_RAISE(OE_FAILURE);

    /* If this is not an EC key */
    {
        EC_KEY* ec;

        if (!(ec = EVP_PKEY_get1_EC_KEY(pkey)))
            OE_RAISE(OE_FAILURE);

        EC_KEY_free(ec);
    }

    /* Initialize the EC public key */
    OE_ECPublicKeyInit(publicKey, pkey);
    pkey = NULL;

    result = OE_OK;

done:

    if (pkey)
    {
        /* Decrement reference count (incremented above) */
        EVP_PKEY_free(pkey);
    }

    return result;
}

OE_Result OE_CertChainGetLength(const OE_CertChain* chain, size_t* length)
{
    OE_Result result = OE_UNEXPECTED;
    const CertChain* impl = (const CertChain*)chain;

    /* Clear the length (for failed return case) */
    if (length)
        *length = 0;

    /* Reject invalid parameters */
    if (!_CertChainIsValid(impl) || !length)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Get the number of certificates in the chain */
    {
        int n;
        OE_CHECK(_CertChainGetLength(impl, &n));
        *length = (size_t)n;
    }

    result = OE_OK;

done:

    return result;
}

OE_Result OE_CertChainGetCert(
    const OE_CertChain* chain,
    size_t index,
    OE_Cert* cert)
{
    OE_Result result = OE_UNEXPECTED;
    const CertChain* impl = (const CertChain*)chain;
    size_t length;
    X509* x509 = NULL;

    /* Clear the output certificate for all error pathways */
    if (cert)
        memset(cert, 0, sizeof(OE_Cert));

    /* Reject invalid parameters */
    if (!_CertChainIsValid(impl) || !cert)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Get the length of the certificate chain */
    {
        int n;
        OE_CHECK(_CertChainGetLength(impl, &n));
        length = (size_t)n;
    }

    /* Check for out of bounds */
    if (index >= length)
        OE_RAISE(OE_OUT_OF_BOUNDS);

    /* Check for overflow when converting to int */
    if (index >= OE_MAX_INT)
        OE_RAISE(OE_INTEGER_OVERFLOW);

    /* Get the certificate with the given index */
    if (!(x509 = sk_X509_value(impl->sk, (int)index)))
        OE_RAISE(OE_FAILURE);

    /* Increment the reference count and initialize the output certificate */
    if (!_X509_up_ref(x509))
        OE_RAISE(OE_FAILURE);
    _CertInit((Cert*)cert, x509);

    result = OE_OK;

done:

    return result;
}

OE_Result OE_CertChainGetRootCert(const OE_CertChain* chain, OE_Cert* cert)
{
    const CertChain* impl = (const CertChain*)chain;
    OE_Result result = OE_UNEXPECTED;
    int n;

    /* Clear the output certificate for all error pathways */
    if (cert)
        memset(cert, 0, sizeof(OE_Cert));

    /* Reject invalid parameters */
    if (!_CertChainIsValid(impl) || !cert)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Get the number of certificates in the chain */
    OE_CHECK(_CertChainGetLength(impl, &n));

    /* Iterate from leaf upwards looking for a self-signed certificate */
    while (n--)
    {
        X509* x509;

        if (!(x509 = sk_X509_value(impl->sk, (int)n)))
            OE_RAISE(OE_FAILURE);

        const X509_NAME* subject = X509_get_subject_name(x509);
        const X509_NAME* issuer = X509_get_issuer_name(x509);

        if (!subject || !issuer)
            OE_RAISE(OE_UNEXPECTED);

        if (X509_NAME_cmp(subject, issuer) == 0)
        {
            _X509_up_ref(x509);
            _CertInit((Cert*)cert, x509);
            OE_RAISE(OE_OK);
        }
    }

    /* No self-signed certificate was found */
    result = OE_NOT_FOUND;

done:
    return result;
}

OE_Result OE_CertChainGetLeafCert(const OE_CertChain* chain, OE_Cert* cert)
{
    OE_Result result = OE_UNEXPECTED;
    size_t length;

    OE_CHECK(OE_CertChainGetLength(chain, &length));
    OE_CHECK(OE_CertChainGetCert(chain, length - 1, cert));
    result = OE_OK;

done:
    return result;
}
