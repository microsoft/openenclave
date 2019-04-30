// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/bits/stdio.h>

// Include the trusted helloworld header that is generated
// during the build. This file is generated by calling the
// sdk tool sgx_edger8r against the helloworld.edl file.
#include "[[project-name]]_t.h"

// This is the function that the host calls. It prints
// a message in the enclave before calling back out to
// the host to print a message from there too.
int enclave_test(void)
{
    // Print a message from the enclave. Note that this
    // does not directly call printf, but calls into the
    // host and calls printf from there. This is because
    // the printf function is not part of the enclave
    // as it requires support from the kernel.
    printf("Hello world from the enclave\n");

    // Call back into the host
    int retval;
    oe_result_t result = host_test(&retval, "The other message!");
    if (result != OE_OK)
    {
        fprintf(
            stderr,
            "Call to host_helloworld failed: result=%u (%s)\n",
            result,
            oe_result_str(result));
    }
    return result;
}
