// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/host.h>
#include <stdio.h>
#include "ecalls.h"

OE_Enclave* CreateEnclave(const char* enclavePath)
{
    OE_Enclave* enclave = NULL;
    OE_Result result = OE_CreateEnclave(
        enclavePath,
        OE_ENCLAVE_TYPE_SGX,
        OE_ENCLAVE_FLAG_DEBUG,
        NULL,
        0,
        &enclave);

    if (result != OE_OK)
    {
        printf("OE_CreateEnclave failed. %s", OE_ResultStr(result));
        exit(1);
    }

    printf("Enclave created.\n");
    Initialize(enclave);
    return enclave;
}

void TerminateEnclave(OE_Enclave* enclave)
{
    OE_TerminateEnclave(enclave);
    printf("Enclave terminated.\n");
}

int main(int argc, const char* argv[])
{
    /* Check argument count */
    if (argc != 2)
    {
        printf("Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    printf("\n\n=====Creating two enclaves=====\n");

    OE_Enclave* enclave1 = CreateEnclave(argv[1]);
    OE_Enclave* enclave2 = CreateEnclave(argv[1]);

    printf(
        "\n\n=====Requesting quoted encryption key from first enclave=====\n");
    QuotedPublicKey* quotedPublicKey = GetPublicKey(enclave1);
    printf("First enclave's public key: \n%s", quotedPublicKey->pemKey);

    printf(
        "\n\n=====Requesting second enclave to attest first enclave's "
        "quoted public key=====\n");
    StorePublicKey(enclave2, quotedPublicKey);

    printf(
        "\n\n=====Requesting quoted encryption key from second enclave=====\n");
    quotedPublicKey = GetPublicKey(enclave2);
    printf("Second enclave's public key: \n%s", quotedPublicKey->pemKey);

    printf(
        "\n\n=====Requesting first enclave to attest second enclave's "
        "quoted public key=====\n");
    StorePublicKey(enclave1, quotedPublicKey);

    delete quotedPublicKey;

    uint8_t* encryptedData = NULL;
    uint32_t encryptedDataSize = 0;
    printf("\n\n=====Requesting encrypted data from first enclave=====\n");
    GenerateEncryptedData(enclave1, &encryptedData, &encryptedDataSize);

    printf("\n\n=====Sending encrypted data to second enclave=====\n");
    ProcessEncryptedData(enclave2, encryptedData, encryptedDataSize);

    delete encryptedData;
    printf("\n\n=====Requesting encrypted data from second enclave=====\n");
    GenerateEncryptedData(enclave2, &encryptedData, &encryptedDataSize);

    printf("\n\n=====Sending encrypted data to first enclave=====\n");
    ProcessEncryptedData(enclave1, encryptedData, encryptedDataSize);

    printf("\n\n=====Terminating enclaves.=====\n");
    TerminateEnclave(enclave1);
    TerminateEnclave(enclave2);

    printf("\n\n=====Done=====\n");

    return 0;
}
