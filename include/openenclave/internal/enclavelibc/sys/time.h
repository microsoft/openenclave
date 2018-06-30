// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _OE_ENCLAVELIBC_SYS_TIME_H
#define _OE_ENCLAVELIBC_SYS_TIME_H

#include "../bits/common.h"

OE_ENCLAVELIBC_EXTERNC_BEGIN

struct oe_timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
};

struct oe_timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

#if defined(OE_ENCLAVELIBC_NEED_STDC_NAMES)

struct timeval
{
    time_t tv_sec;
    suseconds_t tv_usec;
};

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

#endif /* defined(OE_ENCLAVELIBC_NEED_STDC_NAMES) */

OE_ENCLAVELIBC_EXTERNC_END

#endif /* _OE_ENCLAVELIBC_SYS_TIME_H */
