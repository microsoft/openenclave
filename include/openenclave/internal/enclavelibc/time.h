// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _ENCLAVELIBC_TIME_H
#define _ENCLAVELIBC_TIME_H

#include "bits/common.h"
#include "sys/time.h"

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

struct timespec 
{
    time_t tv_sec;
    long tv_nsec;
};

OE_INLINE
time_t time(time_t* tloc)
{
    return __enclavelibc.time(tloc);
}

OE_INLINE
struct tm* gmtime(const time_t* timep)
{
    return __enclavelibc.gmtime(timep);
}

OE_INLINE
struct tm* gmtime_r(const time_t* timep, struct tm* result)
{
    return __enclavelibc.gmtime_r(timep, result);
}

#endif /* _ENCLAVELIBC_TIME_H */
