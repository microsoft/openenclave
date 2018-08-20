// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/host.h>
#include <openenclave/internal/error.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/sgxcreate.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include "../../../host/enclave.h"
#include "../args.h"

static void _CheckProperties(
    oe_sgx_enclave_properties_t* props,
    bool isSigned,
    uint16_t product_id,
    uint16_t security_version,
    uint64_t attributes,
    uint64_t num_heap_pages,
    uint64_t num_stack_pages,
    uint64_t num_tcs)
{
    const oe_enclave_properties_header_t* header = &props->header;
    const oe_sgx_enclave_config_t* config = &props->config;

    /* Check the header */
    OE_TEST(header->size == sizeof(oe_sgx_enclave_properties_t));
    OE_TEST(header->enclave_type == OE_ENCLAVE_TYPE_SGX);
    OE_TEST(header->size_settings.num_heap_pages == num_heap_pages);
    OE_TEST(header->size_settings.num_stack_pages == num_stack_pages);
    OE_TEST(header->size_settings.num_tcs == num_tcs);

    /* Check the SGX config */
    OE_TEST(config->product_id == product_id);
    OE_TEST(config->security_version == security_version);
    OE_TEST(config->padding == 0);
    OE_TEST(config->attributes == attributes);

    /* Initialize a zero-filled sigstruct */
    const uint8_t sigstruct[OE_SGX_SIGSTRUCT_SIZE] = {0};

    /* Check for presence or absence of the signature */
    if (isSigned)
        OE_TEST(memcmp(props->sigstruct, sigstruct, sizeof(sigstruct)) != 0);
    else
        OE_TEST(memcmp(props->sigstruct, sigstruct, sizeof(sigstruct)) == 0);
}

static oe_result_t _SGXLoadEnclaveProperties(
    const char* path,
    oe_sgx_enclave_properties_t* properties)
{
    oe_result_t result = OE_UNEXPECTED;
    Elf64 elf = ELF64_INIT;

    if (properties)
        memset(properties, 0, sizeof(oe_sgx_enclave_properties_t));

    /* Check parameters */
    if (!path || !properties)
        OE_RAISE(OE_INVALID_PARAMETER);

    /* Load the ELF image */
    if (Elf64_Load(path, &elf) != 0)
        OE_RAISE(OE_FAILURE);

    /* Load the SGX enclave properties */
    if (oe_sgx_load_properties(&elf, OE_INFO_SECTION_NAME, properties) != OE_OK)
    {
        OE_RAISE(OE_NOT_FOUND);
    }

    result = OE_OK;

done:

    if (elf.magic == ELF_MAGIC)
        Elf64_Unload(&elf);

    return result;
}

int main(int argc, const char* argv[])
{
    oe_result_t result;
    oe_enclave_t* enclave = NULL;
    bool isSigned = false;
    oe_sgx_enclave_properties_t properties;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    /* Extract "signed" or "unsigned" command-line argument */
    if (strcmp(argv[2], "signed") == 0)
    {
        isSigned = true;
    }
    else if (strcmp(argv[2], "unsigned") == 0)
    {
        isSigned = false;
    }
    else
    {
        fprintf(stderr, "%s: invalid argument: %s\n", argv[0], argv[2]);
        exit(1);
    }

    /* Load the enclave properties */
    if ((result = _SGXLoadEnclaveProperties(argv[1], &properties)) != OE_OK)
    {
        oe_put_err("oe_sgx_load_properties(): result=%u", result);
    }

    const uint32_t flags = oe_get_create_flags();

    if ((result = oe_create_enclave(
             argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave)) != OE_OK)
        oe_put_err("oe_create_enclave(): result=%u", result);

    /* Check expected enclave property values */
    if (isSigned)
    {
        _CheckProperties(
            &properties,
            isSigned,
            1111,                                        /* product_id */
            2222,                                        /* security_version */
            OE_SGX_FLAGS_DEBUG | OE_SGX_FLAGS_MODE64BIT, /* attributes */
            2048,                                        /* num_heap_pages  */
            1024,                                        /* num_stack_pages */
            8);                                          /* num_tcs */
    }
    else
    {
        _CheckProperties(
            &properties,
            isSigned,
            1234,                                        /* product_id */
            5678,                                        /* security_version */
            OE_SGX_FLAGS_DEBUG | OE_SGX_FLAGS_MODE64BIT, /* attributes */
            1024,                                        /* num_heap_pages  */
            512,                                         /* num_stack_pages */
            4);                                          /* num_tcs */
    }

    Args args;
    memset(&args, 0, sizeof(args));
    args.ret = -1;

    if ((result = oe_call_enclave(enclave, "Test", &args)) != OE_OK)
        oe_put_err("oe_call_enclave() failed: result=%u", result);

    if (args.ret != 0)
        oe_put_err("ECALL failed args.result=%d", args.ret);

    oe_terminate_enclave(enclave);

    printf("=== passed all tests (echo)\n");

    return 0;
}
