// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_RSA_H
#define _OE_RSA_H

#include <openenclave/bits/result.h>
#include <openenclave/bits/types.h>
#include "hash.h"
#include "sha.h"

OE_EXTERNC_BEGIN

/* Opaque representation of a private RSA key */
typedef struct _oe_rsa_private_key
{
    /* Internal implementation */
    uint64_t impl[4];
} oe_rsa_private_key_t;

/* Opaque representation of a public RSA key */
typedef struct _oe_rsa_public_key
{
    /* Internal implementation */
    uint64_t impl[4];
} oe_rsa_public_key_t;

/**
 * Reads a private RSA key from PEM data
 *
 * This function reads a private RSA key from PEM data with the following PEM
 * headers.
 *
 *     -----BEGIN RSA PRIVATE KEY-----
 *     ...
 *     -----END RSA PRIVATE KEY-----
 *
 * The caller is responsible for releasing the key by passing it to
 * oe_rsa_private_key_free().
 *
 * @param pemData zero-terminated PEM data
 * @param pemSize size of the PEM data (including the zero-terminator)
 * @param privateKey initialized key handle upon return
 *
 * @return OE_OK upon success
 */
oe_result_t oe_rsa_private_key_read_pem(
    const uint8_t* pemData,
    size_t pemSize,
    oe_rsa_private_key_t* privateKey);

/**
 * Reads a public RSA key from PEM data
 *
 * This function reads a public RSA key from PEM data with the following PEM
 * headers.
 *
 *     -----BEGIN PUBLIC KEY-----
 *     ...
 *     -----END PUBLIC KEY-----
 *
 * The caller is responsible for releasing the key by passing it to
 * oe_rsa_public_key_free().
 *
 * @param pemData zero-terminated PEM data
 * @param pemSize size of the PEM data (including the zero-terminator)
 * @param publicKey initialized key handle upon return
 *
 * @return OE_OK upon success
 */
oe_result_t oe_rsa_public_key_read_pem(
    const uint8_t* pemData,
    size_t pemSize,
    oe_rsa_public_key_t* publicKey);

/**
 * Writes a private RSA key to PEM format
 *
 * This function writes a private RSA key to PEM data with the following PEM
 * headers.
 *
 *     -----BEGIN RSA PRIVATE KEY-----
 *     ...
 *     -----END RSA PRIVATE KEY-----
 *
 * @param privateKey key to be written
 * @param pemData buffer where PEM data will be written
 * @param[in,out] pemSize buffer size (in); PEM data size (out)
 *
 * @return OE_OK upon success
 * @return OE_BUFFER_TOO_SMALL PEM buffer is too small
 */
oe_result_t oe_rsa_private_key_write_pem(
    const oe_rsa_private_key_t* privateKey,
    uint8_t* pemData,
    size_t* pemSize);

/**
 * Writes a public RSA key to PEM format
 *
 * This function writes a public RSA key to PEM data with the following PEM
 * headers.
 *
 *     -----BEGIN PUBLIC KEY-----
 *     ...
 *     -----END PUBLIC KEY-----
 *
 * @param publicKey key to be written
 * @param pemData buffer where PEM data will be written
 * @param[in,out] pemSize buffer size (in); PEM data size (out)
 *
 * @return OE_OK upon success
 * @return OE_BUFFER_TOO_SMALL PEM buffer is too small
 */
oe_result_t oe_rsa_public_key_write_pem(
    const oe_rsa_public_key_t* publicKey,
    uint8_t* pemData,
    size_t* pemSize);

/**
 * Releases an RSA private key
 *
 * This function releases the given RSA private key.
 *
 * @param key handle of key being released
 *
 * @return OE_OK upon success
 */
oe_result_t oe_rsa_private_key_free(oe_rsa_private_key_t* privateKey);

/**
 * Releases an RSA public key
 *
 * This function releases the given RSA public key.
 *
 * @param key handle of key being released
 *
 * @return OE_OK upon success
 */
oe_result_t oe_rsa_public_key_free(oe_rsa_public_key_t* publicKey);

/**
 * Digitally signs a message with a private RSA key
 *
 * This function uses a private RSA key to sign a message with the given hash.
 *
 * @param privateKey private RSA key of signer
 * @param hashType type of hash parameter
 * @param hashData hash of the message being signed
 * @param hashSize size of the hash data
 * @param signature signature buffer
 * @param[in,out] signatureSize buffer size (in); signature size (out)
 *
 * @return OE_OK on success
 * @return OE_BUFFER_TOO_SMALL signature buffer is too small
 */
oe_result_t oe_rsa_private_key_sign(
    const oe_rsa_private_key_t* privateKey,
    oe_hash_type_t hashType,
    const void* hashData,
    size_t hashSize,
    uint8_t* signature,
    size_t* signatureSize);

/**
 * Verifies that a message was signed by an RSA key
 *
 * This function verifies that the message with the given hash was signed by the
 * given RSA key.
 *
 * @param publicKey public RSA key of signer
 * @param hashType type of hash parameter
 * @param hashData hash of the signed message
 * @param hashSize size of the hash data
 * @param signature expected signature
 * @param signatureSize size of the expected signature
 *
 * @return OE_OK if the message was signed with the given certificate
 */
oe_result_t oe_rsa_public_key_verify(
    const oe_rsa_public_key_t* publicKey,
    oe_hash_type_t hashType,
    const void* hashData,
    size_t hashSize,
    const uint8_t* signature,
    size_t signatureSize);

/**
 * Generates an RSA private-public key pair
 *
 * This function generates an RSA private-public key pair from the given
 * parameters.
 *
 * @param bits the number of bits in the key
 * @param exponent the exponent for this key
 * @param privateKey generated private key
 * @param publicKey generated public key
 *
 * @return OE_OK on success
 */
oe_result_t oe_rsa_generate_key_pair(
    uint64_t bits,
    uint64_t exponent,
    oe_rsa_private_key_t* privateKey,
    oe_rsa_public_key_t* publicKey);

/**
 * Get the modulus from a public RSA key.
 *
 * This function gets the modulus from a public RSA key. The modulus is
 * written to **buffer**.
 *
 * @param publicKey key whose modulus is fetched.
 * @param buffer buffer where modulus is written (may be null).
 * @param bufferSize[in,out] buffer size on input; actual size on output.
 *
 * @return OE_OK upon success
 * @return OE_BUFFER_TOO_SMALL buffer is too small and **bufferSize** contains
 *         the required size.
 */
oe_result_t oe_rsa_public_key_get_modulus(
    const oe_rsa_public_key_t* publicKey,
    uint8_t* buffer,
    size_t* bufferSize);

/**
 * Get the exponent from a public RSA key.
 *
 * This function gets the exponent from a public RSA key. The exponent is
 * written to **buffer**.
 *
 * @param publicKey key whose exponent is fetched.
 * @param buffer buffer where exponent is written (may be null).
 * @param bufferSize[in,out] buffer size on input; actual size on output.
 *
 * @return OE_OK upon success
 * @return OE_BUFFER_TOO_SMALL buffer is too small and **bufferSize** contains
 *         the required size.
 */
oe_result_t oe_rsa_public_key_get_exponent(
    const oe_rsa_public_key_t* publicKey,
    uint8_t* buffer,
    size_t* bufferSize);

/**
 * Determine whether two RSA public keys are identical.
 *
 * This function determines whether two RSA public keys are identical.
 *
 * @param publicKey1 first key.
 * @param publicKey2 second key.
 * @param equal[out] true if the keys are identical.
 *
 * @return OE_OK successful and **equal** is either true or false.
 * @return OE_INVALID_PARAMETER a parameter was invalid.
 *
 */
oe_result_t oe_rsa_public_key_equal(
    const oe_rsa_public_key_t* publicKey1,
    const oe_rsa_public_key_t* publicKey2,
    bool* equal);

OE_EXTERNC_END

#endif /* _OE_RSA_H */
