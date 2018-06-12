// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_ENCLAVE_CONTEXT_H
#define _OE_ENCLAVE_CONTEXT_H

#include <openenclave/bits/constants_x64.h>
#include <openenclave/internal/sgxtypes.h>
#include <pthread.h>
#include <sys/user.h>

int OE_ReadProcessMemory(
    pid_t proc,
    void* base_addr,
    void* buffer,
    size_t buffer_size,
    size_t* read_size);

int OE_WriteProcessMemory(
    pid_t proc,
    void* base_addr,
    void* buffer,
    size_t buffer_size,
    size_t* write_size);

bool OE_IsAEP(pid_t pid, struct user_regs_struct* regs);

int OE_GetEnclaveThreadGpr(
    pid_t pid,
    void* tcs_addr,
    struct user_regs_struct* regs);

int OE_SetEnclaveThreadGpr(
    pid_t pid,
    void* tcs_addr,
    struct user_regs_struct* regs);

int OE_GetEnclaveThreadFpr(
    pid_t pid,
    void* tcs_addr,
    struct user_fpregs_struct* regs);

int OE_SetEnclaveThreadFpr(
    pid_t pid,
    void* tcs_addr,
    struct user_fpregs_struct* regs);

int OE_GetEnclaveThreadXState(
    pid_t pid,
    void* tcs_addr,
    void* xstate,
    int64_t xstate_size);

int OE_SetEnclaveThreadXState(
    pid_t pid,
    void* tcs_addr,
    void* xstate,
    int64_t xstate_size);

#endif /* _OE_ENCLAVE_CONTEXT_H */
