// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/tests.h>

static int done = 0;
static unsigned int c = 0;
static int hits1 = 0;
static int hits2 = 0;

#define AESNI_INSTRUCTIONS 0x02000000u

int test_cpuid_instruction(unsigned int what)
{
    if (!done)
    {
        asm("movl  $1, %%eax   \n\t"
            "cpuid             \n\t"
            : "=c"(c)
            :
            : "eax", "ebx", "edx");
        // This should be executed only once.
        ++hits1;
        done = 1;
    }

    // This should be executed 3 times.
    ++hits2;
    return ((c & what) != 0) ? 1 : 0;
}

static int init =
    (test_cpuid_instruction(500),
     test_cpuid_instruction(600),
     test_cpuid_instruction(AESNI_INSTRUCTIONS));

OE_ECALL void TestCpuidInGlobalConstructors(void*)
{
    OE_TEST(init == 1);
    OE_TEST(done == 1);
    OE_TEST(c != 0);
    OE_TEST(hits1 == 1);
    OE_TEST(hits2 == 3);
    OE_HostPrintf("TestCpuidInGlobalConstructors: completed successfully.\n");
}
