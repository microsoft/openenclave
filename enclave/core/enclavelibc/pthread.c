// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/enclavelibc.h>

OE_STATIC_ASSERT(sizeof(oe_pthread_t) == sizeof(oe_thread_t));
OE_STATIC_ASSERT(sizeof(oe_pthread_attr_t) == sizeof(oe_pthread_attr_t));
OE_STATIC_ASSERT(sizeof(oe_pthread_once_t) == sizeof(oe_once_t));
OE_STATIC_ASSERT(sizeof(oe_pthread_spinlock_t) == sizeof(oe_spinlock_t));
OE_STATIC_ASSERT(sizeof(oe_pthread_key_t) == sizeof(oe_thread_key_t));
OE_STATIC_ASSERT(sizeof(oe_pthread_mutex_t) == sizeof(oe_mutex_t));
OE_STATIC_ASSERT(sizeof(oe_pthread_rwlock_t) == sizeof(oe_rwlock_t));
OE_STATIC_ASSERT(sizeof(oe_pthread_cond_t) == sizeof(oe_cond_t));

/* Map an oe_result_t to a POSIX error number */
static int _to_errno(oe_result_t result)
{
    switch (result)
    {
        case OE_OK:
            return 0;
        case OE_INVALID_PARAMETER:
            return OE_EINVAL;
        case OE_BUSY:
            return OE_EBUSY;
        case OE_NOT_OWNER:
            return OE_EPERM;
        case OE_OUT_OF_MEMORY:
            return OE_ENOMEM;
        default:
            return OE_EINVAL; /* unreachable */
    }
}

/*
**==============================================================================
**
** oe_pthread_t
**
**==============================================================================
*/

oe_pthread_t oe_pthread_self()
{
    return (oe_pthread_t)oe_thread_self();
}

int oe_pthread_equal(oe_pthread_t thread1, oe_pthread_t thread2)
{
    return (int)oe_thread_equal((oe_thread_t)thread1, (oe_thread_t)thread2);
}

int oe_pthread_create(
    oe_pthread_t* thread,
    const oe_pthread_attr_t* attr,
    void* (*start_routine)(void*),
    void* arg)
{
    oe_assert("oe_pthread_create(): panic" == NULL);
    return -1;
}

int oe_pthread_join(oe_pthread_t thread, void** retval)
{
    oe_assert("oe_pthread_join(): panic" == NULL);
    return -1;
}

int oe_pthread_detach(oe_pthread_t thread)
{
    oe_assert("oe_pthread_detach(): panic" == NULL);
    return -1;
}

/*
**==============================================================================
**
** oe_pthread_once_t
**
**==============================================================================
*/

int oe_pthread_once(oe_pthread_once_t* once, void (*func)(void))
{
    return _to_errno(oe_once((oe_once_t*)once, func));
}

/*
**==============================================================================
**
** oe_pthread_spinlock_t
**
**==============================================================================
*/

int oe_pthread_spin_init(oe_pthread_spinlock_t* spinlock, int pshared)
{
    return _to_errno(oe_spin_init((oe_spinlock_t*)spinlock));
}

int oe_pthread_spin_lock(oe_pthread_spinlock_t* spinlock)
{
    return _to_errno(oe_spin_lock((oe_spinlock_t*)spinlock));
}

int oe_pthread_spin_unlock(oe_pthread_spinlock_t* spinlock)
{
    return _to_errno(oe_spin_unlock((oe_spinlock_t*)spinlock));
}

int oe_pthread_spin_destroy(oe_pthread_spinlock_t* spinlock)
{
    return _to_errno(oe_spin_destroy((oe_spinlock_t*)spinlock));
}

/*
**==============================================================================
**
** oe_pthread_mutex_t
**
**==============================================================================
*/

int oe_pthread_mutexattr_init(oe_pthread_mutexattr_t* attr)
{
    return 0;
}

int oe_pthread_mutexattr_settype(oe_pthread_mutexattr_t* attr, int type)
{
    return 0;
}

int oe_pthread_mutexattr_destroy(oe_pthread_mutexattr_t* attr)
{
    return 0;
}

int oe_pthread_mutex_init(
    oe_pthread_mutex_t* m,
    const oe_pthread_mutexattr_t* attr)
{
    return _to_errno(oe_mutex_init((oe_mutex_t*)m));
}

int oe_pthread_mutex_lock(oe_pthread_mutex_t* m)
{
    return _to_errno(oe_mutex_lock((oe_mutex_t*)m));
}

int oe_pthread_mutex_trylock(oe_pthread_mutex_t* m)
{
    return _to_errno(oe_mutex_try_lock((oe_mutex_t*)m));
}

int oe_pthread_mutex_unlock(oe_pthread_mutex_t* m)
{
    return _to_errno(oe_mutex_unlock((oe_mutex_t*)m));
}

int oe_pthread_mutex_destroy(oe_pthread_mutex_t* m)
{
    return _to_errno(oe_mutex_destroy((oe_mutex_t*)m));
}

/*
**==============================================================================
**
** oe_pthread_rwlock_t
**
**==============================================================================
*/

int oe_pthread_rwlock_init(
    oe_pthread_rwlock_t* rwlock,
    const oe_pthread_rwlockattr_t* attr)
{
    return _to_errno(oe_rwlock_init((oe_rwlock_t*)rwlock));
}

int oe_pthread_rwlock_rdlock(oe_pthread_rwlock_t* rwlock)
{
    return _to_errno(oe_rwlock_rdlock((oe_rwlock_t*)rwlock));
}

int oe_pthread_rwlock_wrlock(oe_pthread_rwlock_t* rwlock)
{
    return _to_errno(oe_rwlock_wrlock((oe_rwlock_t*)rwlock));
}

int oe_pthread_rwlock_unlock(oe_pthread_rwlock_t* rwlock)
{
    extern oe_result_t oe_rwlock_unlock(oe_rwlock_t * readWriteLock);
    return _to_errno(oe_rwlock_unlock((oe_rwlock_t*)rwlock));
}

int oe_pthread_rwlock_destroy(oe_pthread_rwlock_t* rwlock)
{
    return _to_errno(oe_rwlock_destroy((oe_rwlock_t*)rwlock));
}

/*
**==============================================================================
**
** oe_pthread_cond_t
**
**==============================================================================
*/

int oe_pthread_cond_init(
    oe_pthread_cond_t* cond,
    const oe_pthread_condattr_t* attr)
{
    return _to_errno(oe_cond_init((oe_cond_t*)cond));
}

int oe_pthread_cond_wait(oe_pthread_cond_t* cond, oe_pthread_mutex_t* mutex)
{
    return _to_errno(oe_cond_wait((oe_cond_t*)cond, (oe_mutex_t*)mutex));
}

int oe_pthread_cond_timedwait(
    oe_pthread_cond_t* cond,
    oe_pthread_mutex_t* mutex,
    const struct oe_timespec* ts)
{
    oe_assert("oe_pthread_cond_timedwait(): panic" == NULL);
    return -1;
}

int oe_pthread_cond_signal(oe_pthread_cond_t* cond)
{
    return _to_errno(oe_cond_signal((oe_cond_t*)cond));
}

int oe_pthread_cond_broadcast(oe_pthread_cond_t* cond)
{
    return _to_errno(oe_cond_broadcast((oe_cond_t*)cond));
}

int oe_pthread_cond_destroy(oe_pthread_cond_t* cond)
{
    return _to_errno(oe_cond_destroy((oe_cond_t*)cond));
}

/*
**==============================================================================
**
** oe_pthread_key_t (thread specific data)
**
**==============================================================================
*/

int oe_pthread_key_create(
    oe_pthread_key_t* key,
    void (*destructor)(void* value))
{
    return _to_errno(oe_thread_key_create((oe_thread_key_t*)key, destructor));
}

int oe_pthread_key_delete(oe_pthread_key_t key)
{
    return _to_errno(oe_thread_key_delete((oe_thread_key_t)key));
}

int oe_pthread_setspecific(oe_pthread_key_t key, const void* value)
{
    return _to_errno(oe_thread_set_specific((oe_thread_key_t)key, value));
}

void* oe_pthread_getspecific(oe_pthread_key_t key)
{
    return oe_thread_get_specific((oe_thread_key_t)key);
}
