#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

extern "C" int __cxa_atexit(void (*func)(void *), void *arg, void *d)
{
    // The standard requires calling the function in proper order upon
    // module unload.
    //
    // FIXME: Implement
    //
    //assert("__cxa_atexit()" == NULL);
    //abort();
}
