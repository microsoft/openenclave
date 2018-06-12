// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "attestation.h"
#include <string.h>
#include "crypto.h"
#include "log.h"

/**
 * Generate a quote for the given data. The SHA256 digest of the data is stored
 * in the reportData field of the generated quote.
 */
bool GenerateQuote(
    const uint8_t* data,
    const uint32_t dataSize,
    uint8_t* quoteBuffer,
    uint32_t* quoteBufferSize)
{
    uint8_t sha256[32];
    Sha256(data, dataSize, sha256);

    // To generate a quote that can be attested remotely by an enclave running
    // on a different platform, pass the OE_REPORT_OPTIONS_REMOTE_ATTESTATION
    // option. This uses the trusted quoting enclave to generate the report
    // based on this enclave's local report.
    // To generate a quote that just needs to be attested by another enclave
    // running on the same platform, pass 0 instead. This uses the EREPORT
    // instruction to generate this enclave's local report.
    // Both kinds of reports can be verified using the OE_VerifyReport function.
    OE_Result result = OE_GetReport(
        OE_REPORT_OPTIONS_REMOTE_ATTESTATION,
        sha256, // Store sha256 in reportData field
        sizeof(sha256),
        NULL, // optParams must be null
        0,
        quoteBuffer,
        quoteBufferSize);

    if (result != OE_OK)
    {
        ENC_DEBUG_PRINTF("OE_GetReport failed.");
        return false;
    }

    ENC_DEBUG_PRINTF("GenerateQuote succeeded.");
    return result == OE_OK;
}

// The SHA-256 hash of the public key in the private.pem file used to sign the
// enclave. This value is populated in the authorID sub-field of a parsed
// OE_Report's identity field.
const uint8_t g_MRSigner[] = {0xCA, 0x9A, 0xD7, 0x33, 0x14, 0x48, 0x98, 0x0A,
                              0xA2, 0x88, 0x90, 0xCE, 0x73, 0xE4, 0x33, 0x63,
                              0x83, 0x77, 0xF1, 0x79, 0xAB, 0x44, 0x56, 0xB2,
                              0xFE, 0x23, 0x71, 0x93, 0x19, 0x3A, 0x8D, 0x0A};
/**
 * Attest the given quote and accompanying data. The quote is first attested
 * using the OE_VerifyReport API. This ensures the authenticity of the enclave
 * that generated the quote. Next, to establish trust of the enclave that
 * generated the quote, the mrsigner, productID, isvsvn values are checked to
 * see if they are predefined trusted values. Once the enclave's trust has been
 * established, the validity of accompanying data is ensured by comparing its
 * SHA256 digest against the reportData field.
 */
bool AttestQuote(
    const uint8_t* quote,
    uint32_t quoteSize,
    const uint8_t* data,
    uint32_t dataSize)
{
    // While attesting, the quote being attested must not be tampered with.
    // Ensure that it has been copied over to the enclave.
    if (!OE_IsWithinEnclave(quote, quoteSize))
    {
        ENC_DEBUG_PRINTF("Cannot attest quote in host memory. Unsafe.");
        return false;
    }

    // Verify the quote to ensure its authenticity.
    OE_Report parsedReport = {0};
    OE_Result result = OE_VerifyReport(quote, quoteSize, &parsedReport);
    if (result != OE_OK)
    {
        ENC_DEBUG_PRINTF("OE_VerifyReport failed.");
        return false;
    }

    // AuthorID is the hash of the public signing key that was used to sign an
    // enclave.
    // Check that the enclave was signed by an trusted entity.
    if (memcmp(
            parsedReport.identity.authorID, g_MRSigner, sizeof(g_MRSigner)) !=
        0)
        return false;

    // Check the enclave's product id and security version
    // See enc.conf for values specified when signing the enclave.
    if (parsedReport.identity.productID[0] != 1)
        return false;

    if (parsedReport.identity.securityVersion < 1)
        return false;

    uint8_t sha256[32];
    Sha256(data, dataSize, sha256);

    if (memcmp(parsedReport.reportData, sha256, sizeof(sha256)) != 0)
    {
        ENC_DEBUG_PRINTF("SHA256 mismatch.");
        return false;
    }

    ENC_DEBUG_PRINTF("Quote attestation succeeded.");
    return true;
}
