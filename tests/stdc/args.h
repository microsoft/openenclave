#ifndef _stdc_args_h
#define _stdc_args_h

#include <stddef.h>

typedef struct _TestArgs
{
    char buf1[1024];
    char buf2[1024];
    int strdupOk;
} TestArgs;

#endif /* _stdc_args_h */
