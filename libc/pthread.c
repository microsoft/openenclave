// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <assert.h>
#include <openenclave/enclave.h>
#include <pthread.h>

#ifdef pthread_equal
#undef pthread_equal
#endif

OE_STATIC_ASSERT(sizeof(pthread_once_t) == sizeof(OE_OnceType));
OE_STATIC_ASSERT(sizeof(pthread_spinlock_t) == sizeof(OE_Spinlock));

OE_STATIC_ASSERT(sizeof(pthread_mutex_t) >= sizeof(OE_Mutex));
OE_STATIC_ASSERT(sizeof(pthread_cond_t) >= sizeof(OE_Cond));
OE_STATIC_ASSERT(sizeof(pthread_rwlock_t) >= sizeof(OE_RWLock));

int OE_RWLockUnLock(OE_RWLock* readWriteLock);

/*
**==============================================================================
**
** pthread_t
**
**==============================================================================
*/

pthread_t pthread_self()
{
    return (pthread_t)OE_ThreadSelf();
}

int pthread_equal(pthread_t thread1, pthread_t thread2)
{
    return OE_ThreadEqual((OE_Thread)thread1, (OE_Thread)thread2);
}

int pthread_create(
    pthread_t* thread,
    const pthread_attr_t* attr,
    void* (*start_routine)(void*),
    void* arg)
{
    OE_Assert("pthread_create(): panic" == NULL);
    return -1;
}

int pthread_join(pthread_t thread, void** retval)
{
    assert("pthread_join(): panic" == NULL);
    return -1;
}

int pthread_detach(pthread_t thread)
{
    assert("pthread_detach(): panic" == NULL);
    return -1;
}

/*
**==============================================================================
**
** pthread_once_t
**
**==============================================================================
*/

int pthread_once(pthread_once_t* once, void (*func)(void))
{
    return OE_Once((OE_OnceType*)once, func);
}

/*
**==============================================================================
**
** pthread_spinlock_t
**
**==============================================================================
*/

int pthread_spin_init(pthread_spinlock_t* spinlock, int pshared)
{
    return OE_SpinInit((OE_Spinlock*)spinlock);
}

int pthread_spin_lock(pthread_spinlock_t* spinlock)
{
    return OE_SpinLock((OE_Spinlock*)spinlock);
}

int pthread_spin_unlock(pthread_spinlock_t* spinlock)
{
    return OE_SpinUnlock((OE_Spinlock*)spinlock);
}

int pthread_spin_destroy(pthread_spinlock_t* spinlock)
{
    return OE_SpinDestroy((OE_Spinlock*)spinlock);
}

/*
**==============================================================================
**
** pthread_mutex_t
**
**==============================================================================
*/

int pthread_mutexattr_init(pthread_mutexattr_t* attr)
{
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type)
{
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t* attr)
{
    return 0;
}

int pthread_mutex_init(pthread_mutex_t* m, const pthread_mutexattr_t* attr)
{
    return OE_MutexInit((OE_Mutex*)m);
}

int pthread_mutex_lock(pthread_mutex_t* m)
{
    return OE_MutexLock((OE_Mutex*)m);
}

int pthread_mutex_trylock(pthread_mutex_t* m)
{
    return OE_MutexTryLock((OE_Mutex*)m);
}

int pthread_mutex_unlock(pthread_mutex_t* m)
{
    return OE_MutexUnlock((OE_Mutex*)m);
}

int pthread_mutex_destroy(pthread_mutex_t* m)
{
    return OE_MutexDestroy((OE_Mutex*)m);
}

/*
**==============================================================================
**
** pthread_rwlock_t
**
**==============================================================================
*/

int pthread_rwlock_init(
    pthread_rwlock_t* rwlock,
    const pthread_rwlockattr_t* attr)
{
    return OE_RWLockInit((OE_RWLock*)rwlock);
}

int pthread_rwlock_rdlock(pthread_rwlock_t* rwlock)
{
    return OE_RWLockReadLock((OE_RWLock*)rwlock);
}

int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock)
{
    return OE_RWLockWriteLock((OE_RWLock*)rwlock);
}

int pthread_rwlock_unlock(pthread_rwlock_t* rwlock)
{
    return OE_RWLockUnLock((OE_RWLock*)rwlock);
}

int pthread_rwlock_destroy(pthread_rwlock_t* rwlock)
{
    return OE_RWLockDestroy((OE_RWLock*)rwlock);
}

/*
**==============================================================================
**
** pthread_cond_t
**
**==============================================================================
*/

int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr)
{
    return OE_CondInit((OE_Cond*)cond);
}

int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
    return OE_CondWait((OE_Cond*)cond, (OE_Mutex*)mutex);
}

int pthread_cond_timedwait(
    pthread_cond_t* cond,
    pthread_mutex_t* mutex,
    const struct timespec* ts)
{
    assert("pthread_cond_timedwait(): panic" == NULL);
    return -1;
}

int pthread_cond_signal(pthread_cond_t* cond)
{
    return OE_CondSignal((OE_Cond*)cond);
}

int pthread_cond_broadcast(pthread_cond_t* cond)
{
    return OE_CondBroadcast((OE_Cond*)cond);
}

int pthread_cond_destroy(pthread_cond_t* cond)
{
    return OE_CondDestroy((OE_Cond*)cond);
}

/*
**==============================================================================
**
** pthread_key_t (thread specific data)
**
**==============================================================================
*/

int pthread_key_create(pthread_key_t* key, void (*destructor)(void* value))
{
    return OE_ThreadKeyCreate((OE_ThreadKey*)key, destructor);
}

int pthread_key_delete(pthread_key_t key)
{
    return OE_ThreadKeyDelete(key);
}

int pthread_setspecific(pthread_key_t key, const void* value)
{
    return OE_ThreadSetSpecific(key, value);
}

void* pthread_getspecific(pthread_key_t key)
{
    return OE_ThreadGetSpecific(key);
}
