// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/bits/tests.h>
#include <openenclave/host.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(OE_USE_LIBSGX)
#include <libsgx/sgx_ql_oe_wrapper.h>
#else
#include <openenclave/bits/aesm.h>
#endif

#define SKIP_RETURN_CODE 2

int main(int argc, const char* argv[])
{
    const uint32_t flags = OE_GetCreateFlags();
    if ((flags & OE_ENCLAVE_FLAG_SIMULATE) != 0)
    {
        printf(
            "=== Skipped unsupported test in simulation mode "
            "(aesm)\n");
        return SKIP_RETURN_CODE;
    }

#if defined(OE_USE_LIBSGX)
    quote3_error_t err;
    sgx_target_info_t targetInfo = {};
    if (SGX_QL_SUCCESS != (err = sgx_qe_get_target_info(&targetInfo)))
    {
        printf("FAILED: Call returned %x\n", err);
        return -1;
    }
#else
    AESM* aesm;
    if (!(aesm = AESMConnect()))
    {
        fprintf(stderr, "%s: failed to connect\n", argv[0]);
        exit(1);
    }
#endif

    printf("=== passed all tests (%s)\n", argv[0]);

    return 0;
}
