// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/**
 * @file exception.h
 *
 * This file defines data structures to setup vectored exception handlers in the
 * enclave.
 *
 */
#ifndef _OE_BITS_EXCEPTION_H
#define _OE_BITS_EXCEPTION_H

#include <openenclave/bits/defs.h>
#include <openenclave/bits/types.h>

OE_EXTERNC_BEGIN

/**
 * Exception codes used by the vectored exception handlers
 */

#define OE_EXCEPTION_DIVIDE_BY_ZERO 0x0
#define OE_EXCEPTION_BREAKPOINT 0x1
#define OE_EXCEPTION_BOUND_OUT_OF_RANGE 0x2
#define OE_EXCEPTION_ILLEGAL_INSTRUCTION 0x3
#define OE_EXCEPTION_ACCESS_VIOLATION 0x4
#define OE_EXCEPTION_PAGE_FAULT 0x5
#define OE_EXCEPTION_X87_FLOAT_POINT 0x6
#define OE_EXCEPTION_MISALIGNMENT 0x7
#define OE_EXCEPTION_SIMD_FLOAT_POINT 0x8
#define OE_EXCEPTION_UNKNOWN 0xFFFFFFFF

/**
 * Exception flags used by the vectored exception handlers
 */

#define OE_EXCEPTION_HARDWARE 0x1
#define OE_EXCEPTION_SOFTWARE 0x2

/**
 * @typedef oe_basic_xstate_t: typedef to structure _oe_basic_xstate
 * @struct _oe_basic_xstate: Blob that contains X87 and SSE data
 */
typedef struct _oe_basic_xstate
{
    uint8_t blob[512]; /**< Holds XState i.e. X87 and SSE data */
} OE_ALIGNED(16) oe_basic_xstate_t;

/**
 * @typedef oe_context_t: typedef to structure _oe_context
 * @struct _oe_context: Register state to  be saved before an exception and
 * restored after the exception has been handled in the enclave.
 */
typedef struct _oe_context
{
    uint64_t flags; /**< Flags */

    /**< Integer registers */
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;

    uint64_t rbp;
    uint64_t rsp;

    uint64_t rdi;
    uint64_t rsi;

    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;

    uint64_t rip;

    // Don't need to manipulate the segment registers directly.
    // Ignore them: CS, DS, ES, SS, GS, and FS.

    uint32_t mxcsr; /**< SSE control flags */

    oe_basic_xstate_t basic_xstate; /**< Basic XState */

    // Don't need to manipulate other XSTATE (AVX etc.).
} oe_context_t;

/**
 * Exception context structure with the exception code, flags, address and
 * calling context of the exception.
 */
typedef struct _oe_exception_record
{
    // Exception code.
    uint32_t code;

    // Exception flags.
    uint32_t flags;

    // Exception address.
    uint64_t address;

    // Context.
    oe_context_t* context;
} oe_exception_record_t;

/**
 * oe_vectored_exception_handler_t: Pointer to Vectored exception handler
 * registered in the enclave.
 * @param exceptionContext - Holds the exception code, flags, address and
 * calling context.
 */
typedef uint64_t (*oe_vectored_exception_handler_t)(
    oe_exception_record_t* exceptionContext);

OE_EXTERNC_END

#endif /* _OE_BITS_EXCEPTION_H */
