#include <openenclave/enclave.h>
#include <stdio.h>
#include <string.h>
#include "../args.h"

OE_ECALL void Hello(void* args_)
{
    Args* args = (Args*)args_;

    if (!OE_IsOutsideEnclave(args, sizeof(Args)))
    {
        args->ret = -1;
        return;
    }

    if (strcmp(args->in, "Hello World") != 0)
    {
        args->ret = -1;
        return;
    }

    if (OE_CallHost("Hello", args) != OE_OK)
    {
        args->ret = -1;
        return;
    }

    OE_HostPrintf("enclave: hello!\n");

    args->ret = 0;
}
