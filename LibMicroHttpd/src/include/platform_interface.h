/*
  This file is part of libmicrohttpd
  Copyright (C) 2014 Karlson2k (Evgeny Grin)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.
  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file include/platform_interface.h
 * @brief  internal platform abstraction functions
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_PLATFORM_INTERFACE_H
#define MHD_PLATFORM_INTERFACE_H

#include "platform.h"
#if defined(_WIN32) && !defined(__CYGWIN__)
#include "w32functions.h"
#endif

/* *****************************
     General function mapping
   *****************************/
#if !defined(_WIN32) || defined(__CYGWIN__)
/**
 * Check two strings case-insensitive equality
 * @param a first string to check
 * @param b second string to check
 * @return boolean true if strings are equal, boolean false if strings are unequal
 */
#define MHD_str_equal_caseless_(a,b) (0==strcasecmp((a),(b)))
#else
/**
 * Check two strings case-insensitive equality
 * @param a first string to check
 * @param b second string to check
 * @return boolean true if strings are equal, boolean false if strings are unequal
 */
#define MHD_str_equal_caseless_(a,b) (0==_stricmp((a),(b)))
#endif

#if !defined(_WIN32) || defined(__CYGWIN__)
/**
 * Check not more than n chars in two strings case-insensitive equality
 * @param a first string to check
 * @param b second string to check
 * @param n maximum number of chars to check
 * @return boolean true if strings are equal, boolean false if strings are unequal
 */
#define MHD_str_equal_caseless_n_(a,b,n) (0==strncasecmp((a),(b),(n)))
#else
/**
 * Check not more than n chars in two strings case-insensitive equality
 * @param a first string to check
 * @param b second string to check
 * @param n maximum number of chars to check
 * @return boolean true if strings are equal, boolean false if strings are unequal
 */
#define MHD_str_equal_caseless_n_(a,b,n) (0==_strnicmp((a),(b),(n)))
#endif

/* Platform-independent snprintf name */
#if defined(HAVE_SNPRINTF)
#define MHD_snprintf_ snprintf
#else  /* ! HAVE_SNPRINTF */
#if defined(_WIN32)
#define MHD_snprintf_ W32_snprintf
#else  /* ! _WIN32*/
#error Your platform does not support snprintf() and MHD does not know how to emulate it on your platform.
#endif /* ! _WIN32*/
#endif /* ! HAVE_SNPRINTF */


/**
 * _MHD_socket_funcs_size is type used to specify size for send and recv
 * functions
 */
#if !defined(MHD_WINSOCK_SOCKETS)
typedef size_t _MHD_socket_funcs_size;
#else
typedef int _MHD_socket_funcs_size;
#endif

/**
 * MHD_socket_close_(fd) close any FDs (non-W32) / close only socket
 * FDs (W32).  Note that on HP-UNIX, this function may leak the FD if
 * errno is set to EINTR.  Do not use HP-UNIX.
 *
 * @param fd descriptor to close
 * @return 0 on success (error codes like EINTR and EIO are counted as success,
 *           only EBADF counts as an error!)
 */
#if !defined(MHD_WINSOCK_SOCKETS)
#define MHD_socket_close_(fd) (((0 != close(fd)) && (EBADF == errno)) ? -1 : 0)
#else
#define MHD_socket_close_(fd) closesocket((fd))
#endif

/**
 * MHD_socket_errno_ is errno of last function (non-W32) / errno of
 * last socket function (W32)
 */
#if !defined(MHD_WINSOCK_SOCKETS)
#define MHD_socket_errno_ errno
#else
#define MHD_socket_errno_ MHD_W32_errno_from_winsock_()
#endif

/* MHD_socket_last_strerr_ is description string of last errno (non-W32) /
 *                            description string of last socket error (W32) */
#if !defined(MHD_WINSOCK_SOCKETS)
#define MHD_socket_last_strerr_() strerror(errno)
#else
#define MHD_socket_last_strerr_() MHD_W32_strerror_last_winsock_()
#endif

/* MHD_strerror_ is strerror (both non-W32/W32) */
#if !defined(MHD_WINSOCK_SOCKETS)
#define MHD_strerror_(errnum) strerror((errnum))
#else
#define MHD_strerror_(errnum) MHD_W32_strerror_((errnum))
#endif

/* MHD_set_socket_errno_ set errno to errnum (non-W32) / set socket last error to errnum (W32) */
#if !defined(MHD_WINSOCK_SOCKETS)
#define MHD_set_socket_errno_(errnum) errno=(errnum)
#else
#define MHD_set_socket_errno_(errnum) MHD_W32_set_last_winsock_error_((errnum))
#endif

/* MHD_SYS_select_ is wrapper macro for system select() function */
#if !defined(MHD_WINSOCK_SOCKETS)
#define MHD_SYS_select_(n,r,w,e,t) select((n),(r),(w),(e),(t))
#else
#define MHD_SYS_select_(n,r,w,e,t) select((int)0,(r),(w),(e),(t))
#endif

#if defined(HAVE_POLL)
/* MHD_sys_poll_ is wrapper macro for system poll() function */
#if !defined(MHD_WINSOCK_SOCKETS)
#define MHD_sys_poll_ poll
#else  /* MHD_WINSOCK_SOCKETS */
#define MHD_sys_poll_ WSAPoll
#endif /* MHD_WINSOCK_SOCKETS */
#endif /* HAVE_POLL */

/* MHD_pipe_ create pipe (!MHD_DONT_USE_PIPES) /
 *           create two connected sockets (MHD_DONT_USE_PIPES) */
#ifndef MHD_DONT_USE_PIPES
#define MHD_pipe_(fdarr) pipe((fdarr))
#else /* MHD_DONT_USE_PIPES */
#if !defined(_WIN32) || defined(__CYGWIN__)
#define MHD_pipe_(fdarr) socketpair(AF_LOCAL, SOCK_STREAM, 0, (fdarr))
#else /* !defined(_WIN32) || defined(__CYGWIN__) */
#define MHD_pipe_(fdarr) MHD_W32_pair_of_sockets_((fdarr))
#endif /* !defined(_WIN32) || defined(__CYGWIN__) */
#endif /* MHD_DONT_USE_PIPES */

/* MHD_pipe_errno_ is errno of last function (!MHD_DONT_USE_PIPES) /
 *                    errno of last emulated pipe function (MHD_DONT_USE_PIPES) */
#ifndef MHD_DONT_USE_PIPES
#define MHD_pipe_errno_ errno
#else
#define MHD_pipe_errno_ MHD_socket_errno_
#endif

/* MHD_pipe_last_strerror_ is description string of last errno (!MHD_DONT_USE_PIPES) /
 *                            description string of last pipe error (MHD_DONT_USE_PIPES) */
#ifndef MHD_DONT_USE_PIPES
#define MHD_pipe_last_strerror_() strerror(errno)
#else
#define MHD_pipe_last_strerror_() MHD_socket_last_strerr_()
#endif

/* MHD_pipe_write_ write data to real pipe (!MHD_DONT_USE_PIPES) /
 *                 write data to emulated pipe (MHD_DONT_USE_PIPES) */
#ifndef MHD_DONT_USE_PIPES
#define MHD_pipe_write_(fd, ptr, sz) write((fd), (const void*)(ptr), (sz))
#else
#define MHD_pipe_write_(fd, ptr, sz) send((fd), (const char*)(ptr), (sz), 0)
#endif

/* MHD_pipe_read_ read data from real pipe (!MHD_DONT_USE_PIPES) /
 *                read data from emulated pipe (MHD_DONT_USE_PIPES) */
#ifndef MHD_DONT_USE_PIPES
#define MHD_pipe_read_(fd, ptr, sz) read((fd), (void*)(ptr), (sz))
#else
#define MHD_pipe_read_(fd, ptr, sz) recv((fd), (char*)(ptr), (sz), 0)
#endif

/* MHD_pipe_close_(fd) close any FDs (non-W32) /
 *                     close emulated pipe FDs (W32) */
#ifndef MHD_DONT_USE_PIPES
#define MHD_pipe_close_(fd) close((fd))
#else
#define MHD_pipe_close_(fd) MHD_socket_close_((fd))
#endif

/* MHD_INVALID_PIPE_ is a value of bad pipe FD */
#ifndef MHD_DONT_USE_PIPES
#define MHD_INVALID_PIPE_ (-1)
#else
#define MHD_INVALID_PIPE_ MHD_INVALID_SOCKET
#endif

#if !defined(_WIN32) || defined(__CYGWIN__)
#define MHD_random_() random()
#else
#define MHD_random_() MHD_W32_random_()
#endif

#if defined(MHD_USE_POSIX_THREADS)
typedef pthread_t MHD_thread_handle_;
#elif defined(MHD_USE_W32_THREADS)
#include <windows.h>
typedef HANDLE MHD_thread_handle_;
#else
#error "No threading API is available."
#endif

#if defined(MHD_USE_POSIX_THREADS)
#define MHD_THRD_RTRN_TYPE_ void*
#define MHD_THRD_CALL_SPEC_
#elif defined(MHD_USE_W32_THREADS)
#define MHD_THRD_RTRN_TYPE_ unsigned
#define MHD_THRD_CALL_SPEC_ __stdcall
#endif

#if defined(MHD_USE_POSIX_THREADS)
/**
 * Wait until specified thread is ended
 * @param thread ID to watch
 * @return zero on success, nonzero on failure
 */
#define MHD_join_thread_(thread) pthread_join((thread), NULL)
#elif defined(MHD_USE_W32_THREADS)
/**
 * Wait until specified thread is ended
 * Close thread handle on success
 * @param thread handle to watch
 * @return zero on success, nonzero on failure
 */
#define MHD_join_thread_(thread) (WAIT_OBJECT_0 == WaitForSingleObject((thread), INFINITE) ? (CloseHandle((thread)), 0) : 1 )
#endif

#if defined(MHD_USE_W32_THREADS)
#define MHD_W32_MUTEX_ 1
#include <windows.h>
typedef CRITICAL_SECTION MHD_mutex_;
#elif defined(HAVE_PTHREAD_H) && defined(MHD_USE_POSIX_THREADS)
#define MHD_PTHREAD_MUTEX_ 1
typedef pthread_mutex_t MHD_mutex_;
#else
#error "No base mutex API is available."
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Create new mutex.
 * @param mutex pointer to the mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_create_(mutex) \
  ((0 == pthread_mutex_init ((mutex), NULL)) ? MHD_YES : MHD_NO)
#elif defined(MHD_W32_MUTEX_)
/**
 * Create new mutex.
 * @param mutex pointer to mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_create_(mutex) \
  ((NULL != (mutex) && 0 != InitializeCriticalSectionAndSpinCount((mutex),2000)) ? MHD_YES : MHD_NO)
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Destroy previously created mutex.
 * @param mutex pointer to mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_destroy_(mutex) \
  ((0 == pthread_mutex_destroy ((mutex))) ? MHD_YES : MHD_NO)
#elif defined(MHD_W32_MUTEX_)
/**
 * Destroy previously created mutex.
 * @param mutex pointer to mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_destroy_(mutex) \
  ((NULL != (mutex)) ? (DeleteCriticalSection(mutex), MHD_YES) : MHD_NO)
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Acquire lock on previously created mutex.
 * If mutex was already locked by other thread, function
 * blocks until mutex becomes available.
 * @param mutex pointer to mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_lock_(mutex) \
  ((0 == pthread_mutex_lock((mutex))) ? MHD_YES : MHD_NO)
#elif defined(MHD_W32_MUTEX_)
/**
 * Acquire lock on previously created mutex.
 * If mutex was already locked by other thread, function
 * blocks until mutex becomes available.
 * @param mutex pointer to mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_lock_(mutex) \
  ((NULL != (mutex)) ? (EnterCriticalSection((mutex)), MHD_YES) : MHD_NO)
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Try to acquire lock on previously created mutex.
 * Function returns immediately.
 * @param mutex pointer to mutex
 * @return #MHD_YES if mutex is locked, #MHD_NO if
 * mutex was not locked.
 */
#define MHD_mutex_trylock_(mutex) \
  ((0 == pthread_mutex_trylock((mutex))) ? MHD_YES : MHD_NO)
#elif defined(MHD_W32_MUTEX_)
/**
 * Try to acquire lock on previously created mutex.
 * Function returns immediately.
 * @param mutex pointer to mutex
 * @return #MHD_YES if mutex is locked, #MHD_NO if
 * mutex was not locked.
 */
#define MHD_mutex_trylock_(mutex) \
  ((NULL != (mutex) && 0 != TryEnterCriticalSection ((mutex))) ? MHD_YES : MHD_NO)
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Unlock previously created and locked mutex.
 * @param mutex pointer to mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_unlock_(mutex) \
  ((0 == pthread_mutex_unlock((mutex))) ? MHD_YES : MHD_NO)
#elif defined(MHD_W32_MUTEX_)
/**
 * Unlock previously created and locked mutex.
 * @param mutex pointer to mutex
 * @return #MHD_YES on success, #MHD_NO on failure
 */
#define MHD_mutex_unlock_(mutex) \
  ((NULL != (mutex)) ? (LeaveCriticalSection((mutex)), MHD_YES) : MHD_NO)
#endif

#endif /* MHD_PLATFORM_INTERFACE_H */
