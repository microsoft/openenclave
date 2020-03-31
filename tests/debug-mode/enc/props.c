// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>

OE_SET_ENCLAVE_SGX(
    1234,  /* ProductID */
    5678,  /* SecurityVersion */
    false, /* AllowDebug */
    4096,  /* HeapPageCount */
    512,   /* StackPageCount */
    4);    /* TCSCount */
