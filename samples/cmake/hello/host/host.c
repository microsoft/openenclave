// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <limits.h>
#include <openenclave/bits/tests.h>
#include <openenclave/host.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../args.h"

OE_OCALL void Hello(void* args_)
{
    Args* args = (Args*)args_;

    if (!(args->out = strdup(args->in)))
    {
        args->ret = -1;
        return;
    }

    printf("host: hello!\n");

    args->ret = 0;
}

int main(int argc, const char* argv[])
{
    OE_Result result;
    OE_Enclave* enclave = NULL;

    printf("==== %s\n", argv[0]);

    /* Check argument count */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    /* Create an enclave from the file given by argv[1] */
    {
        const uint64_t flags = OE_GetCreateFlags();

        if ((result = OE_CreateEnclave(
                 argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave)) !=
            OE_OK)
        {
            fprintf(stderr, "OE_CreateEnclave(): result=%u", result);
            exit(1);
        }
    }

    /* Call into Hello() function in the enclave */
    {
        Args args;
        memset(&args, 0, sizeof(args));
        args.ret = -1;

        if (!(args.in = strdup("Hello World")))
        {
            fprintf(stderr, "strdup() failed");
            exit(1);
        }

        if ((result = OE_CallEnclave(enclave, "Hello", &args)) != OE_OK)
        {
            fprintf(stderr, "OE_CallEnclave() failed: result=%u", result);
            exit(1);
        }

        if (args.ret != 0)
        {
            fprintf(stderr, "ECALL failed args.result=%d", args.ret);
            exit(1);
        }

        if (strcmp(args.in, args.out) != 0)
        {
            fprintf(stderr, "ecall failed: %s != %s\n", args.in, args.out);
            exit(1);
        }

        free((char*)args.in);
        free((char*)args.out);
    }

    /* Terminate the enclave */
    OE_TerminateEnclave(enclave);

    return 0;
}
