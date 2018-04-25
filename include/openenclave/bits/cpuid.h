// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_CPUID_H
#define _OE_CPUID_H

#include <openenclave/types.h>

#define OE_CPUID_OPCODE 0xA20F
#define OE_CPUID_LEAF_COUNT 8

#define OE_CPUID_RAX 0
#define OE_CPUID_RBX 1
#define OE_CPUID_RCX 2
#define OE_CPUID_RDX 3
#define OE_CPUID_REG_COUNT 4

int __get_cpuid_count(
    unsigned int __leaf,
    unsigned int __subleaf,
    unsigned int* __eax,
    unsigned int* __ebx,
    unsigned int* __ecx,
    unsigned int* __edx);

#endif /* _OE_CPUID_H */
