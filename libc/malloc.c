#include <openenclave/enclave.h>
#include <openenclave/bits/enclavelibc.h>
#include <openenclave/bits/globals.h>
#include <openenclave/bits/fault.h>
#include <openenclave/bits/malloc.h>
#include <stdio.h>
#include <assert.h>

static void __wait(
    volatile int *addr, 
    volatile int *waiters, 
    int val, 
    int priv);

static void __wake(
    volatile void *addr, 
    int cnt, 
    int priv);

#define _PTHREAD_IMPL_H
#define malloc musl_malloc
#define calloc musl_calloc
#define free musl_free
#define realloc musl_realloc
#define memalign musl_memalign
#define posix_memalign musl_posix_memalign
#include "../3rdparty/musl/musl/src/malloc/malloc.c"
#include "../3rdparty/musl/musl/src/malloc/calloc.c"
#include "../3rdparty/musl/musl/src/malloc/memalign.c"
#include "../3rdparty/musl/musl/src/malloc/posix_memalign.c"
#undef malloc
#undef calloc
#undef free
#undef realloc
#undef memalign
#undef posix_memalign

static OE_Mutex _mutex = OE_MUTEX_INITIALIZER;

static void __wait(
    volatile int *addr, 
    volatile int *waiters, 
    int val, /* will be 1 */
    int priv)
{
    int spins = 100;

    while (spins-- && (!waiters || !*waiters)) 
    {
        if (*addr == val) 
            a_spin();
        else 
            return;
    }

    if (waiters) 
        a_inc(waiters);

    while (*addr == val) 
    {
        OE_MutexLock(&_mutex);

        if (*addr != val)
        {
            OE_MutexUnlock(&_mutex);
            break;
        }

        OE_MutexUnlock(&_mutex);
    }

    if (waiters) 
        a_dec(waiters);
}

static void __wake(
    volatile void *addr, 
    int cnt, /* will be 1 */
    int priv) /* ignored */
{
    if (addr)
    {
        OE_MutexLock(&_mutex);
        *((volatile int*)addr) = 0;
        OE_MutexUnlock(&_mutex);
    }
}

/*
**==============================================================================
**
** malloc wrappers around MUSL calls (with failure callbacks)
**
**==============================================================================
*/

/* Enable MUSL locking on first entry */
OE_INLINE void _EnableMUSLLocking()
{
    if (libc.threads_minus_1 == 0)
    {
        static OE_Spinlock _lock = OE_SPINLOCK_INITIALIZER;
        OE_SpinLock(&_lock);
        libc.threads_minus_1 = 1;
        OE_SpinUnlock(&_lock);
    }
}

static OE_AllocationFailureCallback _failureCallback;

void OE_SetAllocationFailureCallback(OE_AllocationFailureCallback function)
{
    _failureCallback = function;
}

void *malloc(size_t size)
{
    _EnableMUSLLocking();
    void* p = musl_malloc(size);

    if (!p && size)
    {
        errno = ENOMEM;

        if (_failureCallback)
            _failureCallback(__FILE__, __LINE__, __FUNCTION__, size);
    }

    return p;
}

void free(void *ptr)
{
    _EnableMUSLLocking();
    musl_free(ptr);
}

void *calloc(size_t nmemb, size_t size)
{
    void *musl_calloc(size_t nmemb, size_t size);

    _EnableMUSLLocking();
    void* p = musl_calloc(nmemb, size);

    if (!p && nmemb && size)
    {
        errno = ENOMEM;

        if (_failureCallback)
            _failureCallback(__FILE__, __LINE__, __FUNCTION__, nmemb * size);
    }

    return p;
}

void *realloc(void *ptr, size_t size)
{
    _EnableMUSLLocking();
    void* p = musl_realloc(ptr, size);

    if (!p && size)
    {
        errno = ENOMEM;

        if (_failureCallback)
            _failureCallback(__FILE__, __LINE__, __FUNCTION__, size);
    }

    return p;
}

void *memalign(size_t alignment, size_t size)
{
    _EnableMUSLLocking();
    void* p = musl_memalign(alignment, size);

    if (!p && size)
    {
        errno = ENOMEM;

        if (_failureCallback)
            _failureCallback(__FILE__, __LINE__, __FUNCTION__, size);
    }

    return p;
}

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    _EnableMUSLLocking();
    int rc = musl_posix_memalign(memptr, alignment, size);

    if (rc != 0 && size)
    {
        errno = ENOMEM;

        if (_failureCallback)
            _failureCallback(__FILE__, __LINE__, __FUNCTION__, size);
    }

    return rc;
}
