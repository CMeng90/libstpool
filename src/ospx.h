/*
 *  COPYRIGHT (C) 2014 - 2020, piggy_xrh
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *	  Stpool is portable and efficient tasks pool library, it can works on diferent 
 * platforms such as Windows, linux, unix and ARM.  
 *
 *    If you have any troubles or questions on using the library, contact me.
 *
 * 	  (Email: piggy_xrh@163.com  QQ: 1169732280)
 * 	  blog: http://www.oschina.net/code/snippet_1987090_44422
 */

#ifndef __OSPX_H__
#define __OSPX_H__

/* <piggy_xrh@163.com>
 */
#ifdef _WIN32
#ifdef _USRDLL
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif
#else
#define EXPORT
#endif
#include "ospx_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* If you want to operate the error APIs which is defined
 * in the ospx_error.h in the thread,  you should call
 * OSPX_library_init with LB_F_ERRLIB flag before your calling
 * any error APIs. 
 */
#define LB_F_ERRLIB       0x1

EXPORT int  OSPX_library_init(long lflags);
EXPORT void OSPX_library_end();

/* i <1~x> */
#define OSPX_bitset(address, i) (((uint8_t *)address)[(i + 7)/8 -1] |= ((uint8_t)1 << (i-1)%8))
#define OSPX_bitget(address, i) (((uint8_t *)address)[(i + 7)/8 -1] & ((uint8_t)1 << (i-1)%8))
#define OSPX_bitclr(address, i) (((uint8_t *)address)[(i + 7)/8 -1] &= ~((uint8_t)1 << (i-1)%8))

/* Thread */
EXPORT int OSPX_pthread_create(OSPX_pthread_t *handle, int joinable, int (*routine)(void *arglst), void *arglst);
EXPORT int OSPX_pthread_join(OSPX_pthread_t handle, int *ret);
EXPORT int OSPX_pthread_detach(OSPX_pthread_t handle);
EXPORT OSPX_pthread_t OSPX_pthread_self();

/* Thread ID */
#ifndef _WIN32
#define OSPX_pthread_id() (OSPX_pthread_t)pthread_self()
#else
#define OSPX_pthread_id() (OSPX_pthread_t)GetCurrentThreadId()
#endif

#ifndef _WIN32
/************************************************LINUX****************************************/
#define OSPX_srandom srandom
#define OSPX_random  random
#define OSPX_msleep(rest) usleep(rest * 1000)
#define OSPX_sleep(rest)  sleep(rest)
#define OSPX_gettimeofday gettimeofday
EXPORT  long OSPX_interlocked_add(long volatile *target, long inc);

/* NOTE: The return value of the interfaces below is 
 * the same as the POSIX interfaces.
 */
#define OSPX_pthread_key_create(key)        pthread_key_create(key, NULL)
#define OSPX_pthread_key_delete(key)        pthread_key_delete(key)
#define OSPX_pthread_getspecific(key)       pthread_getspecific(key)
#define OSPX_pthread_setspecific(key, val)  pthread_setspecific(key, val);

/* Mutex */
EXPORT  int OSPX_pthread_mutex_init(OSPX_pthread_mutex_t *, int recursive);    
#define OSPX_pthread_mutex_lock(mut)    pthread_mutex_lock(mut)
#define OSPX_pthread_mutex_trylock(mut) pthread_mutex_trylock(mut)
#define OSPX_pthread_mutex_unlock(mut)  pthread_mutex_unlock(mut)
#define OSPX_pthread_mutex_destroy(mut) pthread_mutex_destroy(mut)

/* Condition */
#define OSPX_pthread_cond_init(cond)      pthread_cond_init(cond, NULL)
#define OSPX_pthread_cond_wait(cond, mut) OSPX_pthread_cond_timedwait(cond, mut, NULL)
#define OSPX_pthread_cond_signal(cond)    pthread_cond_signal(cond)
#define OSPX_pthread_cond_broadcast(cond) pthread_cond_broadcast(cond)
#define OSPX_pthread_cond_destroy(cond)   pthread_cond_destroy(cond)
EXPORT int OSPX_pthread_cond_timedwait(OSPX_pthread_cond_t *, OSPX_pthread_mutex_t *, long *timeout/*ms*/);

/* RWlock */
#define OSPX_pthread_rwlock_init(rwlock)      pthread_rwlock_init(rwlock, NULL)
#define OSPX_pthread_rwlock_rdlock(rwlock)    pthread_rwlock_rdlock(rwlock)
#define OSPX_pthread_rwlock_tryrdlock(rwlock) pthread_rwlock_tryrdlock(rwlock)
#define OSPX_pthread_rwlock_wrlock(rwlock)    pthread_rwlock_wrlock(rwlock)
#define OSPX_pthread_rwlock_trywrlock(rwlock) pthread_rwlock_trywrlock(rwlock)
#define OSPX_pthread_rwlock_unlock(rwlock)    pthread_rwlock_unlock(rwlock)
#define OSPX_pthread_rwlock_destroy(rwlock)   pthread_rwlock_destroy(rwlock)

/* Semaphore */
#define OSPX_sem_init(sem, value) sem_init(sem, 0, value)
#define OSPX_sem_post(sem)        sem_post(sem)
#define OSPX_sem_wait(sem)        OSPX_sem_timedwait(sem, NULL)
#define OSPX_sem_destroy(sem)     sem_destroy(sem)
#define OSPX_sem_getvalue(sem, val) sem_getvalue(sem, val)
EXPORT int OSPX_sem_timedwait(OSPX_sem_t *sem, long *timeout /*ms*/);

/* Share memory */
#define OSPX_shm_open(name, oflag, mode)                 shm_open(name, oflag, mode)
#define OSPX_shm_unlink(name)                            shm_unlink(name)
#define OSPX_mmap(addr, length, prot, flags, fd, offset) mmap(addr, length, prot, flags, fd, offset)
#define OSPX_munmap(addr, length)                        munmap(addr, length)
/*********************************************************************************************/
#else
/************************************************WIN32****************************************/
#define OSPX_srandom srand
#define OSPX_random  rand
#define OSPX_msleep(rest) Sleep(rest)
#define OSPX_sleep(rest)  Sleep(rest * 1000)
EXPORT int OSPX_gettimeofday(struct timeval *, struct timezone *);
#define OSPX_interlocked_add(target, inc) InterlockedAdd(target, inc)
#define OSPX_interlocked_exchange(targe, val) InterlockedExchange(target, val)

#define OSPX_pthread_key_create(key)       (*(key) = TlsAlloc(), -1 == *(key) ? ENOMEM : 0)
#define OSPX_pthread_key_delete(key)       (TlsFree(key) ? 0 : EINVAL)
#define OSPX_pthread_getspecific(key)       TlsGetValue(key)
#define OSPX_pthread_setspecific(key, val)  (TlsSetValue(key, val) ? 0 : EINVAL)

#define OSPX_pthread_mutex_init(mut, recursive)  (InitializeCriticalSection(mut), 0)
#define OSPX_pthread_mutex_lock(mut)    (EnterCriticalSection(mut), 0)
#define OSPX_pthread_mutex_trylock(mut) (TryEnterCriticalSection(mut) ? 0 : EBUSY)
#define OSPX_pthread_mutex_unlock(mut)  (LeaveCriticalSection(mut), 0)
#define OSPX_pthread_mutex_destroy(mut) (DeleteCriticalSection(mut), 0)

EXPORT int OSPX_pthread_cond_init(OSPX_pthread_cond_t *);
#define OSPX_pthread_cond_wait(cond, mut) OSPX_pthread_cond_timedwait(cond, mut, NULL)
EXPORT int OSPX_pthread_cond_timedwait(OSPX_pthread_cond_t *, OSPX_pthread_mutex_t *, long *);
EXPORT int OSPX_pthread_cond_signal(OSPX_pthread_cond_t *);
EXPORT int OSPX_pthread_cond_broadcast(OSPX_pthread_cond_t *);
EXPORT int OSPX_pthread_cond_destroy(OSPX_pthread_cond_t *);

EXPORT int OSPX_pthread_rwlock_init(OSPX_pthread_rwlock_t *);
EXPORT int OSPX_pthread_rwlock_rdlock(OSPX_pthread_rwlock_t *);
EXPORT int OSPX_pthread_rwlock_tryrdlock(OSPX_pthread_rwlock_t *);
EXPORT int OSPX_pthread_rwlock_wrlock(OSPX_pthread_rwlock_t *);
EXPORT int OSPX_pthread_rwlock_trywrlock(OSPX_pthread_rwlock_t *);
EXPORT int OSPX_pthread_rwlock_unlock(OSPX_pthread_rwlock_t *);
EXPORT int OSPX_pthread_rwlock_destroy(OSPX_pthread_rwlock_t *);

EXPORT int OSPX_sem_init(OSPX_sem_t *, unsigned int);
EXPORT int OSPX_sem_timedwait(OSPX_sem_t *, long *);
EXPORT int OSPX_sem_post(OSPX_sem_t *);
EXPORT int OSPX_sem_wait(OSPX_sem_t *);   
EXPORT int OSPX_sem_getvalue(OSPX_sem_t *, int *);
EXPORT int OSPX_sem_destroy(OSPX_sem_t *);

/* We do not support the posix share memory since it
 * is not neccessary for us at present. We can use APIs
 * (CreateFileMapping/OpenFileMapping/...) to reach
 * our goal.
 */
/*********************************************************************************************/
#endif
#ifdef __cplusplus
}
#endif


#endif
