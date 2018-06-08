// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "ec.h"
#include <mbedtls/asn1.h>
#include <mbedtls/asn1write.h>
#include <mbedtls/ecp.h>
#include <openenclave/bits/enclavelibc.h>
#include <openenclave/bits/raise.h>
#include <openenclave/enclave.h>
#include "key.h"
#include "pem.h"
#include "random.h"

static uint64_t _PRIVATE_KEY_MAGIC = 0xf12c37bb02814eeb;
static uint64_t _PUBLIC_KEY_MAGIC = 0xd7490a56f6504ee6;

OE_STATIC_ASSERT(sizeof(OE_PrivateKey) <= sizeof(OE_ECPrivateKey));
OE_STATIC_ASSERT(sizeof(OE_PublicKey) <= sizeof(OE_ECPublicKey));

static mbedtls_ecp_group_id _GetGroupID(OE_ECType ecType)
{
    switch (ecType)
    {
        case OE_EC_TYPE_SECP256R1:
            return MBEDTLS_ECP_DP_SECP256R1;
        default:
            return MBEDTLS_ECP_DP_NONE;
    }
}

static OE_Result _CopyKey(
    mbedtls_pk_context* dest,
    const mbedtls_pk_context* src,
    bool copyPrivateFields)
{
    OE_Result result = OE_UNEXPECTED;
    const mbedtls_pk_info_t* info;

    if (dest)
        mbedtls_pk_init(dest);

    /* Check parameters */
    if (!dest || !src)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Lookup the info for this key type */
    if (!(info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)))
        OE_RAISE(OE_WRONG_TYPE);

    /* Setup the context for this key type */
    if (mbedtls_pk_setup(dest, info) != 0)
        OE_RAISE(OE_FAILURE);

    /* Copy all fields of the key structure */
    {
        mbedtls_ecp_keypair* ecDest = mbedtls_pk_ec(*dest);
        const mbedtls_ecp_keypair* ecSrc = mbedtls_pk_ec(*src);

        if (!ecDest || !ecSrc)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_ecp_group_copy(&ecDest->grp, &ecSrc->grp) != 0)
            OE_RAISE(OE_FAILURE);

        if (copyPrivateFields)
        {
            if (mbedtls_mpi_copy(&ecDest->d, &ecSrc->d) != 0)
                OE_RAISE(OE_FAILURE);
        }

        if (mbedtls_ecp_copy(&ecDest->Q, &ecSrc->Q) != 0)
            OE_RAISE(OE_FAILURE);
    }

    result = OE_OK;

done:

    if (result != OE_OK)
        mbedtls_pk_free(dest);

    return result;
}

static OE_Result _GenerateKeyPair(
    OE_ECType ecType,
    OE_PrivateKey* privateKey,
    OE_PublicKey* publicKey)
{
    OE_Result result = OE_UNEXPECTED;
    mbedtls_ctr_drbg_context* drbg;
    mbedtls_pk_context pk;
    int curve;

    /* Initialize structures */
    mbedtls_pk_init(&pk);

    if (privateKey)
        OE_Memset(privateKey, 0, sizeof(*privateKey));

    if (publicKey)
        OE_Memset(publicKey, 0, sizeof(*publicKey));

    /* Check for invalid parameters */
    if (!privateKey || !publicKey)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Get the group id and curve info structure for this EC type */
    {
        const mbedtls_ecp_curve_info* info;
        mbedtls_ecp_group_id groupID;

        if ((groupID = _GetGroupID(ecType)) == MBEDTLS_ECP_DP_NONE)
            OE_RAISE(OE_FAILURE);

        if (!(info = mbedtls_ecp_curve_info_from_grp_id(groupID)))
            OE_RAISE(OE_INVALID_PARAMETER);

        curve = info->grp_id;
    }

    /* Get the drbg object */
    if (!(drbg = OE_MBEDTLS_GetDrbg()))
        OE_RAISE(OE_FAILURE);

    /* Create key struct */
    if (mbedtls_pk_setup(&pk, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)) != 0)
        OE_RAISE(OE_FAILURE);

    /* Generate the EC key */
    if (mbedtls_ecp_gen_key(
            curve, mbedtls_pk_ec(pk), mbedtls_ctr_drbg_random, drbg) != 0)
    {
        OE_RAISE(OE_FAILURE);
    }

    /* Initialize the private key parameter */
    OE_CHECK(OE_PrivateKeyInit(privateKey, &pk, _CopyKey, _PRIVATE_KEY_MAGIC));

    /* Initialize the public key parameter */
    OE_CHECK(OE_PublicKeyInit(publicKey, &pk, _CopyKey, _PUBLIC_KEY_MAGIC));

    result = OE_OK;

done:

    mbedtls_pk_free(&pk);

    if (result != OE_OK)
    {
        OE_PrivateKeyFree(privateKey, _PRIVATE_KEY_MAGIC);
        OE_PublicKeyFree(publicKey, _PUBLIC_KEY_MAGIC);
    }

    return result;
}

static OE_Result OE_PublicKeyEqual(
    const OE_PublicKey* publicKey1,
    const OE_PublicKey* publicKey2,
    bool* equal)
{
    OE_Result result = OE_UNEXPECTED;

    if (equal)
        *equal = false;

    /* Reject bad parameters */
    if (!OE_PublicKeyIsValid(publicKey1, _PUBLIC_KEY_MAGIC) ||
        !OE_PublicKeyIsValid(publicKey2, _PUBLIC_KEY_MAGIC) || !equal)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Compare the groups and EC points */
    {
        const mbedtls_ecp_keypair* ec1 = mbedtls_pk_ec(publicKey1->pk);
        const mbedtls_ecp_keypair* ec2 = mbedtls_pk_ec(publicKey2->pk);

        if (!ec1 || !ec2)
            OE_RAISE(OE_INVALID_PARAMETER);

        if (ec1->grp.id == ec2->grp.id &&
            mbedtls_ecp_point_cmp(&ec1->Q, &ec2->Q) == 0)
        {
            *equal = true;
        }
    }

    result = OE_OK;

done:
    return result;
}

OE_Result OE_ECPublicKeyInit(
    OE_ECPublicKey* publicKey,
    const mbedtls_pk_context* pk)
{
    return OE_PublicKeyInit(
        (OE_PublicKey*)publicKey, pk, _CopyKey, _PUBLIC_KEY_MAGIC);
}

OE_Result OE_ECPrivateKeyReadPEM(
    const uint8_t* pemData,
    size_t pemSize,
    OE_ECPrivateKey* privateKey)
{
    return OE_PrivateKeyReadPEM(
        pemData,
        pemSize,
        (OE_PrivateKey*)privateKey,
        MBEDTLS_PK_ECKEY,
        _PRIVATE_KEY_MAGIC);
}

OE_Result OE_ECPrivateKeyWritePEM(
    const OE_ECPrivateKey* privateKey,
    uint8_t* pemData,
    size_t* pemSize)
{
    return OE_PrivateKeyWritePEM(
        (const OE_PrivateKey*)privateKey, pemData, pemSize, _PRIVATE_KEY_MAGIC);
}

OE_Result OE_ECPublicKeyReadPEM(
    const uint8_t* pemData,
    size_t pemSize,
    OE_ECPublicKey* privateKey)
{
    return OE_PublicKeyReadPEM(
        pemData,
        pemSize,
        (OE_PublicKey*)privateKey,
        MBEDTLS_PK_ECKEY,
        _PUBLIC_KEY_MAGIC);
}

OE_Result OE_ECPublicKeyWritePEM(
    const OE_ECPublicKey* privateKey,
    uint8_t* pemData,
    size_t* pemSize)
{
    return OE_PublicKeyWritePEM(
        (const OE_PublicKey*)privateKey, pemData, pemSize, _PUBLIC_KEY_MAGIC);
}

OE_Result OE_ECPrivateKeyFree(OE_ECPrivateKey* privateKey)
{
    return OE_PrivateKeyFree((OE_PrivateKey*)privateKey, _PRIVATE_KEY_MAGIC);
}

OE_Result OE_ECPublicKeyFree(OE_ECPublicKey* publicKey)
{
    return OE_PublicKeyFree((OE_PublicKey*)publicKey, _PUBLIC_KEY_MAGIC);
}

OE_Result OE_ECPrivateKeySign(
    const OE_ECPrivateKey* privateKey,
    OE_HashType hashType,
    const void* hashData,
    size_t hashSize,
    uint8_t* signature,
    size_t* signatureSize)
{
    return OE_PrivateKeySign(
        (OE_PrivateKey*)privateKey,
        hashType,
        hashData,
        hashSize,
        signature,
        signatureSize,
        _PRIVATE_KEY_MAGIC);
}

OE_Result OE_ECPublicKeyVerify(
    const OE_ECPublicKey* publicKey,
    OE_HashType hashType,
    const void* hashData,
    size_t hashSize,
    const uint8_t* signature,
    size_t signatureSize)
{
    return OE_PublicKeyVerify(
        (OE_PublicKey*)publicKey,
        hashType,
        hashData,
        hashSize,
        signature,
        signatureSize,
        _PUBLIC_KEY_MAGIC);
}

OE_Result OE_ECGenerateKeyPair(
    OE_ECType type,
    OE_ECPrivateKey* privateKey,
    OE_ECPublicKey* publicKey)
{
    return _GenerateKeyPair(
        type, (OE_PrivateKey*)privateKey, (OE_PublicKey*)publicKey);
}

OE_Result OE_ECPublicKeyEqual(
    const OE_ECPublicKey* publicKey1,
    const OE_ECPublicKey* publicKey2,
    bool* equal)
{
    return OE_PublicKeyEqual(
        (OE_PublicKey*)publicKey1, (OE_PublicKey*)publicKey2, equal);
}

OE_Result OE_ECPublicKeyFromCoordinates(
    OE_ECPublicKey* publicKey,
    OE_ECType ecType,
    const uint8_t* xData,
    size_t xSize,
    const uint8_t* yData,
    size_t ySize)
{
    OE_Result result = OE_UNEXPECTED;
    OE_PublicKey* impl = (OE_PublicKey*)publicKey;
    const mbedtls_pk_info_t* info;

    if (publicKey)
        OE_Memset(publicKey, 0, sizeof(OE_ECPublicKey));

    if (impl)
        mbedtls_pk_init(&impl->pk);

    /* Reject invalid parameters */
    if (!publicKey || !xData || !xSize || !yData || !ySize)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Lookup the info for this key type */
    if (!(info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY)))
        OE_RAISE(OE_WRONG_TYPE);

    /* Setup the context for this key type */
    if (mbedtls_pk_setup(&impl->pk, info) != 0)
        OE_RAISE(OE_FAILURE);

    /* Initialize the key */
    {
        mbedtls_ecp_keypair* ecp = mbedtls_pk_ec(impl->pk);
        mbedtls_ecp_group_id groupID;

        if ((groupID = _GetGroupID(ecType)) == MBEDTLS_ECP_DP_NONE)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_ecp_group_load(&ecp->grp, groupID) != 0)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_mpi_read_binary(&ecp->Q.X, xData, xSize) != 0)
            OE_RAISE(OE_FAILURE);

        if (mbedtls_mpi_read_binary(&ecp->Q.Y, yData, ySize) != 0)
            OE_RAISE(OE_FAILURE);

        // Used internally by MBEDTLS. Set Z to 1 to indicate that X-Y
        // represents a standard coordinate point. Zero indicates that the
        // point is zero or infinite, and values >= 2 have internal meaning
        // only to MBEDTLS.
        if (mbedtls_mpi_lset(&ecp->Q.Z, 1) != 0)
            OE_RAISE(OE_FAILURE);
    }

    /* Set the magic number */
    impl->magic = _PUBLIC_KEY_MAGIC;

    result = OE_OK;

done:

    if (result != OE_OK && impl)
        mbedtls_pk_free(&impl->pk);

    return result;
}

OE_Result OE_ECDSASignatureWriteDER(
    unsigned char* signature,
    size_t* signatureSize,
    const uint8_t* rData,
    size_t rSize,
    const uint8_t* sData,
    size_t sSize)
{
    OE_Result result = OE_UNEXPECTED;
    mbedtls_mpi r;
    mbedtls_mpi s;
    unsigned char buf[MBEDTLS_ECDSA_MAX_LEN];
    unsigned char* p = buf + sizeof(buf);
    int n;
    size_t len = 0;

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    /* Reject invalid parameters */
    if (!signatureSize || !rData || !rSize || !sData || !sSize)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* If signature is null, then signatureSize must be zero */
    if (!signature && *signatureSize != 0)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Convert raw R data to big number */
    if (mbedtls_mpi_read_binary(&r, rData, rSize) != 0)
        OE_RAISE(OE_FAILURE);

    /* Convert raw S data to big number */
    if (mbedtls_mpi_read_binary(&s, sData, sSize) != 0)
        OE_RAISE(OE_FAILURE);

    /* Write S to ASN.1 */
    {
        if ((n = mbedtls_asn1_write_mpi(&p, buf, &s)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Write R to ASN.1 */
    {
        if ((n = mbedtls_asn1_write_mpi(&p, buf, &r)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Write the length to ASN.1 */
    {
        if ((n = mbedtls_asn1_write_len(&p, buf, len)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Write the tag to ASN.1 */
    {
        unsigned char tag = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;

        if ((n = mbedtls_asn1_write_tag(&p, buf, tag)) < 0)
            OE_RAISE(OE_FAILURE);

        len += n;
    }

    /* Check that buffer is big enough */
    if (len > *signatureSize)
    {
        *signatureSize = len;
        OE_RAISE(OE_BUFFER_TOO_SMALL);
    }

    OE_Memcpy(signature, p, len);
    *signatureSize = len;

    result = OE_OK;

done:

    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);

    return result;
}
