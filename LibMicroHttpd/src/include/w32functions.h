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
 * @file include/w32functions.h
 * @brief  internal functions for W32 systems
 * @author Karlson2k (Evgeny Grin)
 */

#ifndef MHD_W32FUNCTIONS_H
#define MHD_W32FUNCTIONS_H
#ifndef _WIN32
#error w32functions.h is designed only for W32 systems
#endif

#include "platform.h"
#include <errno.h>
#include <winsock2.h>
#include "platform_interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MHDW32ERRBASE 3300

#ifndef EWOULDBLOCK
#define EWOULDBLOCK (MHDW32ERRBASE+1)
#endif
#ifndef EINPROGRESS
#define EINPROGRESS (MHDW32ERRBASE+2)
#endif
#ifndef EALREADY
#define EALREADY (MHDW32ERRBASE+3)
#endif
#ifndef ENOTSOCK
#define ENOTSOCK (MHDW32ERRBASE+4)
#endif
#ifndef EDESTADDRREQ
#define EDESTADDRREQ (MHDW32ERRBASE+5)
#endif
#ifndef EMSGSIZE
#define EMSGSIZE (MHDW32ERRBASE+6)
#endif
#ifndef EPROTOTYPE
#define EPROTOTYPE (MHDW32ERRBASE+7)
#endif
#ifndef ENOPROTOOPT
#define ENOPROTOOPT (MHDW32ERRBASE+8)
#endif
#ifndef EPROTONOSUPPORT
#define EPROTONOSUPPORT (MHDW32ERRBASE+9)
#endif
#ifndef EOPNOTSUPP
#define EOPNOTSUPP (MHDW32ERRBASE+10)
#endif
#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT (MHDW32ERRBASE+11)
#endif
#ifndef EADDRINUSE
#define EADDRINUSE (MHDW32ERRBASE+12)
#endif
#ifndef EADDRNOTAVAIL
#define EADDRNOTAVAIL (MHDW32ERRBASE+13)
#endif
#ifndef ENETDOWN
#define ENETDOWN (MHDW32ERRBASE+14)
#endif
#ifndef ENETUNREACH
#define ENETUNREACH (MHDW32ERRBASE+15)
#endif
#ifndef ENETRESET
#define ENETRESET (MHDW32ERRBASE+16)
#endif
#ifndef ECONNABORTED
#define ECONNABORTED (MHDW32ERRBASE+17)
#endif
#ifndef ECONNRESET
#define ECONNRESET (MHDW32ERRBASE+18)
#endif
#ifndef ENOBUFS
#define ENOBUFS (MHDW32ERRBASE+19)
#endif
#ifndef EISCONN
#define EISCONN (MHDW32ERRBASE+20)
#endif
#ifndef ENOTCONN
#define ENOTCONN (MHDW32ERRBASE+21)
#endif
#ifndef ETOOMANYREFS
#define ETOOMANYREFS (MHDW32ERRBASE+22)
#endif
#ifndef ECONNREFUSED
#define ECONNREFUSED (MHDW32ERRBASE+23)
#endif
#ifndef ELOOP
#define ELOOP (MHDW32ERRBASE+24)
#endif
#ifndef EHOSTDOWN
#define EHOSTDOWN (MHDW32ERRBASE+25)
#endif
#ifndef EHOSTUNREACH
#define EHOSTUNREACH (MHDW32ERRBASE+26)
#endif
#ifndef EPROCLIM
#define EPROCLIM (MHDW32ERRBASE+27)
#endif
#ifndef EUSERS
#define EUSERS (MHDW32ERRBASE+28)
#endif
#ifndef EDQUOT
#define EDQUOT (MHDW32ERRBASE+29)
#endif
#ifndef ESTALE
#define ESTALE (MHDW32ERRBASE+30)
#endif
#ifndef EREMOTE
#define EREMOTE (MHDW32ERRBASE+31)
#endif
#ifndef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT (MHDW32ERRBASE+32)
#endif
#ifndef EPFNOSUPPORT
#define EPFNOSUPPORT (MHDW32ERRBASE+33)
#endif
#ifndef ESHUTDOWN
#define ESHUTDOWN (MHDW32ERRBASE+34)
#endif
#ifndef ENODATA
#define ENODATA (MHDW32ERRBASE+35)
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT (MHDW32ERRBASE+36)
#endif

/**
 * Return errno equivalent of last winsock error
 * @return errno equivalent of last winsock error
 */
int MHD_W32_errno_from_winsock_(void);

/**
 * Return pointer to string description of errnum error
 * Works fine with both standard errno errnums
 * and errnums from MHD_W32_errno_from_winsock_
 * @param errnum the errno or value from MHD_W32_errno_from_winsock_()
 * @return pointer to string description of error
 */
const char* MHD_W32_strerror_(int errnum);

/**
 * Return pointer to string description of last winsock error
 * @return pointer to string description of last winsock error
 */
const char* MHD_W32_strerror_last_winsock_(void);

/**
 * Set last winsock error to equivalent of given errno value
 * @param errnum the errno value to set
 */
void MHD_W32_set_last_winsock_error_(int errnum);

/**
 * Create pair of mutually connected TCP/IP sockets on loopback address
 * @param sockets_pair array to receive resulted sockets
 * @return zero on success, -1 otherwise
 */
int MHD_W32_pair_of_sockets_(SOCKET sockets_pair[2]);

/**
 * Generate 31-bit pseudo random number.
 * Function initialize itself at first call to current time.
 * @return 31-bit pseudo random number.
 */
int MHD_W32_random_(void);

/* Emulate snprintf function on W32 */
int W32_snprintf(char *__restrict s, size_t n, const char *__restrict format, ...);

#ifndef _MSC_FULL_VER
/* Thread name available only for VC-compiler */
static void W32_SetThreadName(const DWORD thread_id, const char *thread_name)
{ }
#else  /* _MSC_FULL_VER */
/**
 * Set thread name
 * @param thread_id ID of thread, -1 for current thread
 * @param thread_name name to set
 */
void W32_SetThreadName(const DWORD thread_id, const char *thread_name);
#endif

#ifdef __cplusplus
}
#endif
#endif /* MHD_W32FUNCTIONS_H */
