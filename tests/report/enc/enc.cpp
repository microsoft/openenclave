// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/bits/enclavelibc.h>
#include <openenclave/bits/tests.h>
#include <openenclave/enclave.h>

uint8_t gUniqueID[32];

uint8_t gAuthorID[32] = {202, 154, 215, 51, 20,  72,  152, 10,  162, 136, 144,
                         206, 115, 228, 51, 99,  131, 119, 241, 121, 171, 68,
                         86,  178, 254, 35, 113, 147, 25,  58,  141, 10};

uint8_t gProductID[16] = {0};

static bool CheckReportData(
    uint8_t* reportBuffer,
    uint32_t reportSize,
    const uint8_t* reportData,
    uint32_t reportDataSize)
{
    OE_Report parsedReport = {0};
    OE_TEST(OE_ParseReport(reportBuffer, reportSize, &parsedReport) == OE_OK);

    return (
        OE_Memcmp(parsedReport.reportData, reportData, reportDataSize) == 0);
}

static bool ValidateReport(
    uint8_t* reportBuffer,
    uint32_t reportSize,
    bool remote,
    const uint8_t* reportData,
    uint32_t reportDataSize)
{
    SGX_Quote* sgxQuote = NULL;
    SGX_Report* sgxReport = NULL;

    OE_Report parsedReport = {0};

    static bool firstTime = true;

    OE_TEST(OE_ParseReport(reportBuffer, reportSize, &parsedReport) == OE_OK);

    /* Validate header. */
    OE_TEST(parsedReport.type == OE_ENCLAVE_TYPE_SGX);
    OE_TEST(
        OE_Memcmp(parsedReport.reportData, reportData, reportDataSize) == 0);

    /* Validate pointer fields. */

    if (remote)
    {
        sgxQuote = (SGX_Quote*)reportBuffer;
        OE_TEST(reportSize > sizeof(SGX_Quote));

        OE_TEST(
            parsedReport.reportData == sgxQuote->report_body.reportData.field);
        OE_TEST(parsedReport.reportDataSize == sizeof(SGX_ReportData));
        OE_TEST(parsedReport.enclaveReport == (uint8_t*)&sgxQuote->report_body);
        OE_TEST(parsedReport.enclaveReportSize == sizeof(SGX_ReportBody));
    }
    else
    {
        OE_TEST(reportSize == sizeof(SGX_Report));
        sgxReport = (SGX_Report*)reportBuffer;

        OE_TEST(parsedReport.reportData == sgxReport->body.reportData.field);
        OE_TEST(parsedReport.reportDataSize == sizeof(SGX_ReportData));
        OE_TEST(parsedReport.enclaveReport == (uint8_t*)&sgxReport->body);
        OE_TEST(parsedReport.enclaveReportSize == sizeof(SGX_ReportBody));
    }

    /* Validate identity. */
    OE_TEST(parsedReport.identity.idVersion == 0x0);
    OE_TEST(parsedReport.identity.securityVersion == 0x0);

    if (remote)
    {
        OE_TEST(parsedReport.identity.attributes & OE_REPORT_ATTRIBUTES_REMOTE);
        OE_TEST(parsedReport.identity.attributes & OE_REPORT_ATTRIBUTES_DEBUG);
        OE_TEST(
            !(parsedReport.identity.attributes &
              OE_REPORT_ATTRIBUTES_RESERVED));
    }
    else
    {
        OE_TEST(
            !(parsedReport.identity.attributes & OE_REPORT_ATTRIBUTES_REMOTE));
        OE_TEST(parsedReport.identity.attributes & OE_REPORT_ATTRIBUTES_DEBUG);
        OE_TEST(
            !(parsedReport.identity.attributes &
              OE_REPORT_ATTRIBUTES_RESERVED));
    }

    if (firstTime)
    {
        OE_Memcpy(
            gUniqueID,
            parsedReport.identity.uniqueID,
            sizeof(parsedReport.identity.uniqueID));

        firstTime = false;
    }

    OE_TEST(
        OE_Memcmp(
            parsedReport.identity.uniqueID,
            gUniqueID,
            sizeof(parsedReport.identity.uniqueID)) == 0);

    OE_TEST(
        OE_Memcmp(
            parsedReport.identity.authorID,
            gAuthorID,
            sizeof(parsedReport.identity.authorID)) == 0);

    OE_TEST(
        OE_Memcmp(
            parsedReport.identity.productID,
            gProductID,
            sizeof(parsedReport.identity.productID)) == 0);

    return true;
}

OE_ECALL void TestLocalReport(void* args_)
{
    SGX_TargetInfo* targetInfo = (SGX_TargetInfo*)args_;

    uint32_t reportDataSize = 0;
    uint8_t reportData[OE_REPORT_DATA_SIZE];
    for (uint32_t i = 0; i < OE_REPORT_DATA_SIZE; ++i)
        reportData[i] = i;

    const uint8_t zeros[OE_REPORT_DATA_SIZE] = {0};

    uint32_t reportSize = 1024;
    uint8_t reportBuffer[1024];

    uint8_t optParams[sizeof(SGX_TargetInfo)];
    for (uint32_t i = 0; i < sizeof(optParams); ++i)
        optParams[i] = 0;

    /*
     * Post conditions:
     *     1. On a successfull call, the returned report size must always be
     * sizeof(SGX_Report);
     *     2. Report must contain specified report data or zeros as report data.
     */

    /*
     * Report data parameters scenarios:
     *      1. Report data can be NULL.
     *      2. Report data can be < OE_REPORT_DATA_SIZE
     *      3. Report data can be OE_REPORT_DATA_SIZE
     *      4. Report data cannot exceed OE_REPORT_DATA_SIZE
     */
    {
        reportSize = 1024 * 1024;
        OE_TEST(
            OE_GetReport(0, NULL, 0, NULL, 0, reportBuffer, &reportSize) ==
            OE_OK);
        ValidateReport(
            reportBuffer, reportSize, false, zeros, OE_REPORT_DATA_SIZE);

        reportSize = 1024 * 1024;
        reportDataSize = 16;
        OE_TEST(
            OE_GetReport(
                0,
                reportData,
                reportDataSize,
                NULL,
                0,
                reportBuffer,
                &reportSize) == OE_OK);
        ValidateReport(
            reportBuffer, reportSize, false, reportData, reportDataSize);

        OE_TEST(
            CheckReportData(
                reportBuffer, reportSize, reportData, reportDataSize + 1) ==
            false);

        reportSize = 1024 * 1024;
        reportDataSize = OE_REPORT_DATA_SIZE;
        OE_TEST(
            OE_GetReport(
                0,
                reportData,
                reportDataSize,
                NULL,
                0,
                reportBuffer,
                &reportSize) == OE_OK);
        ValidateReport(
            reportBuffer, reportSize, false, reportData, reportDataSize);

        reportSize = 1024 * 1024;
        reportDataSize = OE_REPORT_DATA_SIZE + 1;
        OE_TEST(
            OE_GetReport(
                0,
                reportData,
                reportDataSize,
                NULL,
                0,
                reportBuffer,
                &reportSize) == OE_INVALID_PARAMETER);
    }

    /*
     * optParams scenarios:
     *     1. If optParams is not null, optParamsSize must be
     * sizeof(SGX_TargetInfo)
     *     2. Otherwise, both must be null/0.
     *     3. optParams can be zeroed out target info.
     *     4. optParams can be a valid target info.
     */
    {
        reportSize = 1024 * 1024;
        OE_TEST(
            OE_GetReport(
                0,
                NULL,
                0,
                NULL,
                sizeof(optParams),
                reportBuffer,
                &reportSize) == OE_INVALID_PARAMETER);
        OE_TEST(
            OE_GetReport(0, NULL, 0, optParams, 5, reportBuffer, &reportSize) ==
            OE_INVALID_PARAMETER);

        reportSize = 1024 * 1024;
        OE_TEST(
            OE_GetReport(0, NULL, 0, NULL, 0, reportBuffer, &reportSize) ==
            OE_OK);
        ValidateReport(
            reportBuffer, reportSize, false, zeros, OE_REPORT_DATA_SIZE);

        reportSize = 1024 * 1024;
        OE_TEST(
            OE_GetReport(
                0,
                NULL,
                0,
                optParams,
                sizeof(SGX_TargetInfo),
                reportBuffer,
                &reportSize) == OE_OK);
        OE_TEST(reportSize == sizeof(SGX_Report));
        ValidateReport(
            reportBuffer, reportSize, false, zeros, OE_REPORT_DATA_SIZE);

        reportSize = 1024 * 1024;
        OE_TEST(
            OE_GetReport(
                0,
                NULL,
                0,
                targetInfo,
                sizeof(optParams),
                reportBuffer,
                &reportSize) == OE_OK);
        OE_TEST(reportSize == sizeof(SGX_Report));
        ValidateReport(
            reportBuffer, reportSize, false, zeros, OE_REPORT_DATA_SIZE);
    }

    /*
     * OE_SMALL_BUFFER scenarios:
     *     a. NULL buffer
     *     b. Size too small.
     */
    {
        reportSize = 1024 * 1204;
        OE_TEST(
            OE_GetReport(0, NULL, 0, NULL, 0, NULL, &reportSize) ==
            OE_BUFFER_TOO_SMALL);

        reportSize = 1;
        OE_TEST(
            OE_GetReport(0, NULL, 0, NULL, 0, reportBuffer, &reportSize) ==
            OE_BUFFER_TOO_SMALL);
    }
}

OE_ECALL void TestRemoteReport(void* args_)
{
    uint32_t reportDataSize = 0;
    uint8_t reportData[OE_REPORT_DATA_SIZE];
    for (uint32_t i = 0; i < OE_REPORT_DATA_SIZE; ++i)
        reportData[i] = i;

    const uint8_t zeros[OE_REPORT_DATA_SIZE] = {0};

    uint32_t reportSize = 2048;
    uint8_t reportBuffer[2048];

    uint8_t optParams[sizeof(SGX_TargetInfo)];
    for (uint32_t i = 0; i < sizeof(optParams); ++i)
        optParams[i] = 0;

    uint32_t options = OE_REPORT_OPTIONS_REMOTE_ATTESTATION;
    /*
     * Post conditions:
     *     1. Report must contain specified report data or zeros as report data.
     */

    /*
     * Report data parameters scenarios:
     *      a. Report data can be NULL.
     *      b. Report data can be < OE_REPORT_DATA_SIZE
     *      c. Report data can be OE_REPORT_DATA_SIZE
     *      d. Report data cannot exceed OE_REPORT_DATA_SIZE
     */
    {
        reportSize = 2048;
        OE_TEST(
            OE_GetReport(
                options, NULL, 0, NULL, 0, reportBuffer, &reportSize) == OE_OK);
        ValidateReport(
            reportBuffer, reportSize, true, zeros, OE_REPORT_DATA_SIZE);

        reportSize = 2048;
        reportDataSize = 16;
        OE_TEST(
            OE_GetReport(
                options,
                reportData,
                reportDataSize,
                NULL,
                0,
                reportBuffer,
                &reportSize) == OE_OK);
        ValidateReport(
            reportBuffer, reportSize, true, reportData, reportDataSize);
        OE_TEST(
            CheckReportData(
                reportBuffer, reportSize, reportData, reportDataSize + 1) ==
            false);

        reportSize = 2048;
        reportDataSize = OE_REPORT_DATA_SIZE;
        OE_TEST(
            OE_GetReport(
                options,
                reportData,
                reportDataSize,
                NULL,
                0,
                reportBuffer,
                &reportSize) == OE_OK);
        ValidateReport(
            reportBuffer, reportSize, true, reportData, reportDataSize);

        reportSize = 2048;
        reportDataSize = OE_REPORT_DATA_SIZE + 1;
        OE_TEST(
            OE_GetReport(
                options,
                reportData,
                reportDataSize,
                NULL,
                0,
                reportBuffer,
                &reportSize) == OE_INVALID_PARAMETER);
    }

    /*
     * optParams scenarios:
     *     1. Both optParams and optParamsSize must be NULL/0.
     */
    {
        reportSize = 2048;
        OE_TEST(
            OE_GetReport(
                options,
                NULL,
                0,
                NULL,
                sizeof(optParams),
                reportBuffer,
                &reportSize) == OE_INVALID_PARAMETER);
        OE_TEST(
            OE_GetReport(
                options, NULL, 0, optParams, 5, reportBuffer, &reportSize) ==
            OE_INVALID_PARAMETER);
    }

    /*
     * OE_SMALL_BUFFER scenarios:
     *     a. NULL buffer
     *     b. Size too small.
     */
    {
        reportSize = 2048;
        OE_TEST(
            OE_GetReport(options, NULL, 0, NULL, 0, NULL, &reportSize) ==
            OE_BUFFER_TOO_SMALL);

        reportSize = 1;
        OE_TEST(
            OE_GetReport(
                options, NULL, 0, NULL, 0, reportBuffer, &reportSize) ==
            OE_BUFFER_TOO_SMALL);
    }
}

OE_ECALL void TestParseReportNegative(void* args_)
{
    uint8_t reportBuffer[2048] = {0};
    OE_Report parsedReport = {0};

    // 1. Null report passed in.
    OE_TEST(OE_ParseReport(NULL, 0, &parsedReport) == OE_INVALID_PARAMETER);

    // 2. Report size less than size of SGX_Report.
    OE_TEST(
        OE_ParseReport(reportBuffer, sizeof(SGX_Report) - 1, &parsedReport) ==
        OE_INVALID_PARAMETER);

    // 3. Report size greater than size of SGX_Report but less than
    // sizeof(SGX_Quote)
    OE_TEST(
        OE_ParseReport(reportBuffer, sizeof(SGX_Quote) - 1, &parsedReport) ==
        OE_INVALID_PARAMETER);

    // 4. NULL parsedReport passed in.
    OE_TEST(
        OE_ParseReport(reportBuffer, sizeof(SGX_Quote), NULL) ==
        OE_INVALID_PARAMETER);
}
