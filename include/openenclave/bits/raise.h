// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

/**
 * \file raise.h
 *
 * This file defines macros to simplify functions that return OE_Result.
 * For example, consider the following function definition.
 *
 *     OE_Result Func1(const char* param)
 *     {
 *         OE_Result result = OE_UNEXPECTED;
 *         OE_Result tmpResult;
 *
 *         if (!param)
 *         {
 *             result = OE_INVALID_PARAMETER;
 *             goto done;
 *         }
 *
 *         tmpResult = Func2(param);
 *         if (tmpResult != OE_OK)
 *         {
 *             result = tmpResult;
 *             goto done;
 *         }
 *
 *         tmpResult = Func3(param);
 *         if (tmpResult != OE_OK)
 *         {
 *             result = tmpResult;
 *             goto done;
 *         }
 *
 *         result = OE_OK;
 *
 *     done:
 *         return result;
 *     }
 *
 * These macros can be used to simplify the function as follows.
 *
 *     OE_Result Func1(const char* param)
 *     {
 *         OE_Result result = OE_UNEXPECTED;
 *         OE_Result tmpResult;
 *
 *         if (!param)
 *             OE_RAISE(OE_INVALID_PARAMETER);
 *
 *         OE_CHECK(Func2(param));
 *         OE_CHECK(Func3(param));
 *
 *         result = OE_OK;
 *
 *     done:
 *         return result;
 *     }
 *
 */

#ifndef _OE_RAISE_H
#define _OE_RAISE_H

#include "../defs.h"
#include "../result.h"

OE_EXTERNC_BEGIN

// This macro sets the 'result' and jumps to the 'done' label.
#define OE_RAISE(RESULT)        \
    do                          \
    {                           \
        result = (RESULT);      \
        OE_RAISE_TRACE(result); \
        goto done;              \
    } while (0)

// This macro checks whether the expression argument evaluates to OE_OK. If not
// it sets the 'result' to the evaluation of the expression and jumps to the
// 'done' label.
#define OE_CHECK(EXPRESSION)               \
    do                                     \
    {                                      \
        OE_Result _result_ = (EXPRESSION); \
                                           \
        if (_result_ != OE_OK)             \
            OE_RAISE(result);              \
    } while (0)

// This macro is used to trace the OE_RAISE macro. It is empty by default but
// may be defined prior to including this header file.
#if !defined(OE_RAISE_TRACE)
#define OE_RAISE_TRACE(RESULT) \
    do                         \
    {                          \
    } while (0)
#endif

OE_EXTERNC_END

#endif /* _OE_RAISE_H */
