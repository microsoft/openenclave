// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/bits/error.h>
#include <openenclave/bits/tests.h>
#include <openenclave/host.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../args.h"

static OE_Enclave* enclave;

int main(int argc, const char* argv[])
{
    OE_Result result;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE\n", argv[0]);
        exit(1);
    }

    const uint32_t flags = OE_GetCreateFlags();

    if ((result = OE_CreateEnclave(argv[1], flags, &enclave)) != OE_OK)
    {
        OE_PutErr("OE_CreateEnclave(): result=%u", result);
        return 1;
    }

    /* Invoke tests */
    {
        OE_Result res = OE_FAILURE;

        result = OE_CallEnclave(enclave, "TestAllocaDealloc", &res);
        OE_TEST(result == OE_OK);
        OE_TEST(res == OE_OK);
    }

    OE_TerminateEnclave(enclave);

    printf("=== passed all tests (%s)\n", argv[0]);

    return 0;
}
