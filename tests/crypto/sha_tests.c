// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#if defined(OE_BUILD_ENCLAVE)
#include <openenclave/enclave.h>
#endif

#include "sha_tests.h"
#include "hash.h"
#include <openenclave/bits/sha.h>
#include <openenclave/bits/tests.h>
#include <stdio.h>
#include <string.h>

// Test computation of SHA-256 hash over an ASCII alphabet string.
void TestSHA()
{
    printf("=== begin TestSHA25\n");

    OE_SHA256 hash;
    OE_SHA256Context ctx;
    OE_SHA256Init(&ctx);
    OE_SHA256Update(&ctx, ALPHABET, strlen(ALPHABET));
    OE_SHA256Final(&ctx, &hash);
    OE_TEST(memcmp(&hash, &ALPHABET_HASH, sizeof(OE_SHA256)) == 0);

    printf("=== passed TestSHA25\n");
}
