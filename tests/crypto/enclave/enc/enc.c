// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <openenclave/bits/cert.h>
#include <openenclave/bits/ec.h>
#include <openenclave/bits/hexdump.h>
#include <openenclave/bits/random.h>
#include <openenclave/bits/rsa.h>
#include <openenclave/bits/sha.h>
#include <openenclave/bits/tests.h>
#include <openenclave/enclave.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../tests.c"

OE_ECALL void Test(void* args_)
{
    RunAllTests();
}
