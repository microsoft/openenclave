// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <openenclave/internal/cert.h>
#include <openenclave/internal/ec.h>
#include <openenclave/internal/hexdump.h>
#include <openenclave/internal/raise.h>
#include <openenclave/internal/random.h>
#include <openenclave/internal/rsa.h>
#include <openenclave/internal/sha.h>
#include <openenclave/internal/tests.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ec_tests.h"
#include "../hash.h"
#include "../random_tests.h"
#include "../rsa_tests.h"
#include "../sha_tests.h"

const char* arg0;

int main(int argc, const char* argv[])
{
    arg0 = argv[0];

    /* Run the tests */
    TestEC();
    TestRandom();
    TestRSA();
    TestSHA();

    printf("=== passed all tests (%s)\n", arg0);

    return 0;
}
