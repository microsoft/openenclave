// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_CPUIDCOUNT_H
#define _OE_CPUIDCOUNT_H

int __get_cpuid_count(
    unsigned int __leaf,
    unsigned int __subleaf,
    unsigned int* __eax,
    unsigned int* __ebx,
    unsigned int* __ecx,
    unsigned int* __edx);

#endif /* _OE_CPUIDCOUNT_H */
