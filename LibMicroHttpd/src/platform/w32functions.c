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
 * @file platform/w32functions.h
 * @brief  internal functions for W32 systems
 * @author Karlson2k (Evgeny Grin)
 */

#include "w32functions.h"
#include <errno.h>
#include <winsock2.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>


/**
 * Return errno equivalent of last winsock error
 * @return errno equivalent of last winsock error
 */
int MHD_W32_errno_from_winsock_(void)
{
  switch(WSAGetLastError())
  {
  case 0:                      return 0;
  case WSA_INVALID_HANDLE:     return EBADF;
  case WSA_NOT_ENOUGH_MEMORY:  return ENOMEM;
  case WSA_INVALID_PARAMETER:  return EINVAL;
  case WSAEINTR:               return EINTR;
  case WSAEWOULDBLOCK:         return EWOULDBLOCK;
  case WSAEINPROGRESS:         return EINPROGRESS;
  case WSAEALREADY:            return EALREADY;
  case WSAENOTSOCK:            return ENOTSOCK;
  case WSAEDESTADDRREQ:        return EDESTADDRREQ;
  case WSAEMSGSIZE:            return EMSGSIZE;
  case WSAEPROTOTYPE:          return EPROTOTYPE;
  case WSAENOPROTOOPT:         return ENOPROTOOPT;
  case WSAEPROTONOSUPPORT:     return EPROTONOSUPPORT;
  case WSAESOCKTNOSUPPORT:     return ESOCKTNOSUPPORT;
  case WSAEOPNOTSUPP:          return EOPNOTSUPP;
  case WSAEPFNOSUPPORT:        return EPFNOSUPPORT;
  case WSAEAFNOSUPPORT:        return EAFNOSUPPORT;
  case WSAEADDRINUSE:          return EADDRINUSE;
  case WSAEADDRNOTAVAIL:       return EADDRNOTAVAIL;
  case WSAENETDOWN:            return ENETDOWN;
  case WSAENETUNREACH:         return ENETUNREACH;
  case WSAENETRESET:           return ENETRESET;
  case WSAECONNABORTED:        return ECONNABORTED;
  case WSAECONNRESET:          return ECONNRESET;
  case WSAENOBUFS:             return ENOBUFS;
  case WSAEISCONN:             return EISCONN;
  case WSAENOTCONN:            return ENOTCONN;
  case WSAESHUTDOWN:           return ESHUTDOWN;
  case WSAETOOMANYREFS:        return ETOOMANYREFS;
  case WSAETIMEDOUT:           return ETIMEDOUT;
  case WSAECONNREFUSED:        return ECONNREFUSED;
  case WSAELOOP:               return ELOOP;
  case WSAENAMETOOLONG:        return ENAMETOOLONG;
  case WSAEHOSTDOWN:           return EHOSTDOWN;
  case WSAEHOSTUNREACH:        return EHOSTUNREACH;
  case WSAENOTEMPTY:           return ENOTEMPTY;
  case WSAEPROCLIM:            return EPROCLIM;
  case WSAEUSERS:              return EUSERS;
  case WSAEDQUOT:              return EDQUOT;
  case WSAESTALE:              return ESTALE;
  case WSAEREMOTE:             return EREMOTE;
  case WSAEINVAL:              return EINVAL;
  case WSAEFAULT:              return EFAULT;
  case WSANO_DATA:             return ENODATA;
  /* Rough equivalents */
  case WSAEDISCON:             return ECONNRESET;
  case WSAEINVALIDPROCTABLE:   return EFAULT;
  case WSASYSNOTREADY:
  case WSANOTINITIALISED:
  case WSASYSCALLFAILURE:      return ENOBUFS;
  case WSAVERNOTSUPPORTED:     return EOPNOTSUPP;
  case WSAEREFUSED:            return EIO;
  }
  return EINVAL;
}

/**
 * Return pointer to string description of errnum error
 * Works fine with both standard errno errnums
 * and errnums from MHD_W32_errno_from_winsock_
 * @param errnum the errno or value from MHD_W32_errno_from_winsock_()
 * @return pointer to string description of error
 */
const char* MHD_W32_strerror_(int errnum)
{
  switch(errnum)
  {
  case 0:
    return "No error";
  case EWOULDBLOCK:
    return "Operation would block";
  case EINPROGRESS:
    return "Connection already in progress";
  case EALREADY:
    return "Socket already connected";
  case ENOTSOCK:
    return "Socket operation on non-socket";
  case EDESTADDRREQ:
    return "Destination address required";
  case EMSGSIZE:
    return "Message too long";
  case EPROTOTYPE:
    return "Protocol wrong type for socket";
  case ENOPROTOOPT:
    return "Protocol not available";
  case EPROTONOSUPPORT:
    return "Unknown protocol";
  case ESOCKTNOSUPPORT:
    return "Socket type not supported";
  case EOPNOTSUPP:
    return "Operation not supported on socket";
  case EPFNOSUPPORT:
    return "Protocol family not supported";
  case EAFNOSUPPORT:
    return "Address family not supported by protocol family";
  case EADDRINUSE:
    return "Address already in use";
  case EADDRNOTAVAIL:
    return "Cannot assign requested address";
  case ENETDOWN:
    return "Network is down";
  case ENETUNREACH:
    return "Network is unreachable";
  case ENETRESET:
    return "Network dropped connection on reset";
  case ECONNABORTED:
    return "Software caused connection abort";
  case ECONNRESET:
    return "Connection reset by peer";
  case ENOBUFS:
    return "No system resources available";
  case EISCONN:
    return "Socket is already connected";
  case ENOTCONN:
    return "Socket is not connected";
  case ESHUTDOWN:
    return "Can't send after socket shutdown";
  case ETOOMANYREFS:
    return "Too many references: cannot splice";
  case ETIMEDOUT:
    return "Connection timed out";
  case ECONNREFUSED:
    return "Connection refused";
  case ELOOP:
    return "Cannot translate name";
  case EHOSTDOWN:
    return "Host is down";
  case EHOSTUNREACH:
    return "Host is unreachable";
  case EPROCLIM:
    return "Too many processes";
  case EUSERS:
    return "Too many users";
  case EDQUOT:
    return "Disk quota exceeded";
  case ESTALE:
    return "Stale file handle reference";
  case EREMOTE:
    return "Resource is remote";
  case ENODATA:
    return "No data available";
  }
  return strerror(errnum);
}

/**
 * Return pointer to string description of last winsock error
 * @return pointer to string description of last winsock error
 */
const char* MHD_W32_strerror_last_winsock_(void)
{
  switch (WSAGetLastError())
    {
  case 0:
    return "No error";
  case WSA_INVALID_HANDLE:
    return "Specified event object handle is invalid";
  case WSA_NOT_ENOUGH_MEMORY:
    return "Insufficient memory available";
  case WSA_INVALID_PARAMETER:
    return "One or more parameters are invalid";
  case WSA_OPERATION_ABORTED:
    return "Overlapped operation aborted";
  case WSA_IO_INCOMPLETE:
    return "Overlapped I/O event object not in signaled state";
  case WSA_IO_PENDING:
    return "Overlapped operations will complete later";
  case WSAEINTR:
    return "Interrupted function call";
  case WSAEBADF:
    return "File handle is not valid";
  case WSAEACCES:
    return "Permission denied";
  case WSAEFAULT:
    return "Bad address";
  case WSAEINVAL:
    return "Invalid argument";
  case WSAEMFILE:
    return "Too many open files";
  case WSAEWOULDBLOCK:
    return "Resource temporarily unavailable";
  case WSAEINPROGRESS:
    return "Operation now in progress";
  case WSAEALREADY:
    return "Operation already in progress";
  case WSAENOTSOCK:
    return "Socket operation on nonsocket";
  case WSAEDESTADDRREQ:
    return "Destination address required";
  case WSAEMSGSIZE:
    return "Message too long";
  case WSAEPROTOTYPE:
    return "Protocol wrong type for socket";
  case WSAENOPROTOOPT:
    return "Bad protocol option";
  case WSAEPROTONOSUPPORT:
    return "Protocol not supported";
  case WSAESOCKTNOSUPPORT:
    return "Socket type not supported";
  case WSAEOPNOTSUPP:
    return "Operation not supported";
  case WSAEPFNOSUPPORT:
    return "Protocol family not supported";
  case WSAEAFNOSUPPORT:
    return "Address family not supported by protocol family";
  case WSAEADDRINUSE:
    return "Address already in use";
  case WSAEADDRNOTAVAIL:
    return "Cannot assign requested address";
  case WSAENETDOWN:
    return "Network is down";
  case WSAENETUNREACH:
    return "Network is unreachable";
  case WSAENETRESET:
    return "Network dropped connection on reset";
  case WSAECONNABORTED:
    return "Software caused connection abort";
  case WSAECONNRESET:
    return "Connection reset by peer";
  case WSAENOBUFS:
    return "No buffer space available";
  case WSAEISCONN:
    return "Socket is already connected";
  case WSAENOTCONN:
    return "Socket is not connected";
  case WSAESHUTDOWN:
    return "Cannot send after socket shutdown";
  case WSAETOOMANYREFS:
    return "Too many references";
  case WSAETIMEDOUT:
    return "Connection timed out";
  case WSAECONNREFUSED:
    return "Connection refused";
  case WSAELOOP:
    return "Cannot translate name";
  case WSAENAMETOOLONG:
    return "Name too long";
  case WSAEHOSTDOWN:
    return "Host is down";
  case WSAEHOSTUNREACH:
    return "No route to host";
  case WSAENOTEMPTY:
    return "Directory not empty";
  case WSAEPROCLIM:
    return "Too many processes";
  case WSAEUSERS:
    return "User quota exceeded";
  case WSAEDQUOT:
    return "Disk quota exceeded";
  case WSAESTALE:
    return "Stale file handle reference";
  case WSAEREMOTE:
    return "Item is remote";
  case WSASYSNOTREADY:
    return "Network subsystem is unavailable";
  case WSAVERNOTSUPPORTED:
    return "Winsock.dll version out of range";
  case WSANOTINITIALISED:
    return "Successful WSAStartup not yet performed";
  case WSAEDISCON:
    return "Graceful shutdown in progress";
  case WSAENOMORE:
    return "No more results";
  case WSAECANCELLED:
    return "Call has been canceled";
  case WSAEINVALIDPROCTABLE:
    return "Procedure call table is invalid";
  case WSAEINVALIDPROVIDER:
    return "Service provider is invalid";
  case WSAEPROVIDERFAILEDINIT:
    return "Service provider failed to initialize";
  case WSASYSCALLFAILURE:
    return "System call failure";
  case WSASERVICE_NOT_FOUND:
    return "Service not found";
  case WSATYPE_NOT_FOUND:
    return "Class type not found";
  case WSA_E_NO_MORE:
    return "No more results";
  case WSA_E_CANCELLED:
    return "Call was canceled";
  case WSAEREFUSED:
    return "Database query was refused";
  case WSAHOST_NOT_FOUND:
    return "Host not found";
  case WSATRY_AGAIN:
    return "Nonauthoritative host not found";
  case WSANO_RECOVERY:
    return "This is a nonrecoverable error";
  case WSANO_DATA:
    return "Valid name, no data record of requested type";
  case WSA_QOS_RECEIVERS:
    return "QoS receivers";
  case WSA_QOS_SENDERS:
    return "QoS senders";
  case WSA_QOS_NO_SENDERS:
    return "No QoS senders";
  case WSA_QOS_NO_RECEIVERS:
    return "QoS no receivers";
  case WSA_QOS_REQUEST_CONFIRMED:
    return "QoS request confirmed";
  case WSA_QOS_ADMISSION_FAILURE:
    return "QoS admission error";
  case WSA_QOS_POLICY_FAILURE:
    return "QoS policy failure";
  case WSA_QOS_BAD_STYLE:
    return "QoS bad style";
  case WSA_QOS_BAD_OBJECT:
    return "QoS bad object";
  case WSA_QOS_TRAFFIC_CTRL_ERROR:
    return "QoS traffic control error";
  case WSA_QOS_GENERIC_ERROR:
    return "QoS generic error";
  case WSA_QOS_ESERVICETYPE:
    return "QoS service type error";
  case WSA_QOS_EFLOWSPEC:
    return "QoS flowspec error";
  case WSA_QOS_EPROVSPECBUF:
    return "Invalid QoS provider buffer";
  case WSA_QOS_EFILTERSTYLE:
    return "Invalid QoS filter style";
  case WSA_QOS_EFILTERTYPE:
    return "Invalid QoS filter type";
  case WSA_QOS_EFILTERCOUNT:
    return "Incorrect QoS filter count";
  case WSA_QOS_EOBJLENGTH:
    return "Invalid QoS object length";
  case WSA_QOS_EFLOWCOUNT:
    return "Incorrect QoS flow count";
  case WSA_QOS_EUNKOWNPSOBJ:
    return "Unrecognized QoS object";
  case WSA_QOS_EPOLICYOBJ:
    return "Invalid QoS policy object";
  case WSA_QOS_EFLOWDESC:
    return "Invalid QoS flow descriptor";
  case WSA_QOS_EPSFLOWSPEC:
    return "Invalid QoS provider-specific flowspec";
  case WSA_QOS_EPSFILTERSPEC:
    return "Invalid QoS provider-specific filterspec";
  case WSA_QOS_ESDMODEOBJ:
    return "Invalid QoS shape discard mode object";
  case WSA_QOS_ESHAPERATEOBJ:
    return "Invalid QoS shaping rate object";
  case WSA_QOS_RESERVED_PETYPE:
    return "Reserved policy QoS element type";
    }
  return "Unknown winsock error";
}

/**
 * Set last winsock error to equivalent of given errno value
 * @param errnum the errno value to set
 */
void MHD_W32_set_last_winsock_error_(int errnum)
{
  switch (errnum)
    {
  case 0:
    WSASetLastError(0);
    break;
  case EBADF:
    WSASetLastError(WSA_INVALID_HANDLE);
    break;
  case ENOMEM:
    WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
    break;
  case EINVAL:
    WSASetLastError(WSA_INVALID_PARAMETER);
    break;
  case EINTR:
    WSASetLastError(WSAEINTR);
    break;
  case EWOULDBLOCK:
    WSASetLastError(WSAEWOULDBLOCK);
    break;
  case EINPROGRESS:
    WSASetLastError(WSAEINPROGRESS);
    break;
  case EALREADY:
    WSASetLastError(WSAEALREADY);
    break;
  case ENOTSOCK:
    WSASetLastError(WSAENOTSOCK);
    break;
  case EDESTADDRREQ:
    WSASetLastError(WSAEDESTADDRREQ);
    break;
  case EMSGSIZE:
    WSASetLastError(WSAEMSGSIZE);
    break;
  case EPROTOTYPE:
    WSASetLastError(WSAEPROTOTYPE);
    break;
  case ENOPROTOOPT:
    WSASetLastError(WSAENOPROTOOPT);
    break;
  case EPROTONOSUPPORT:
    WSASetLastError(WSAEPROTONOSUPPORT);
    break;
  case ESOCKTNOSUPPORT:
    WSASetLastError(WSAESOCKTNOSUPPORT);
    break;
  case EOPNOTSUPP:
    WSASetLastError(WSAEOPNOTSUPP);
    break;
  case EPFNOSUPPORT:
    WSASetLastError(WSAEPFNOSUPPORT);
    break;
  case EAFNOSUPPORT:
    WSASetLastError(WSAEAFNOSUPPORT);
    break;
  case EADDRINUSE:
    WSASetLastError(WSAEADDRINUSE);
    break;
  case EADDRNOTAVAIL:
    WSASetLastError(WSAEADDRNOTAVAIL);
    break;
  case ENETDOWN:
    WSASetLastError(WSAENETDOWN);
    break;
  case ENETUNREACH:
    WSASetLastError(WSAENETUNREACH);
    break;
  case ENETRESET:
    WSASetLastError(WSAENETRESET);
    break;
  case ECONNABORTED:
    WSASetLastError(WSAECONNABORTED);
    break;
  case ECONNRESET:
    WSASetLastError(WSAECONNRESET);
    break;
  case ENOBUFS:
    WSASetLastError(WSAENOBUFS);
    break;
  case EISCONN:
    WSASetLastError(WSAEISCONN);
    break;
  case ENOTCONN:
    WSASetLastError(WSAENOTCONN);
    break;
  case ESHUTDOWN:
    WSASetLastError(WSAESHUTDOWN);
    break;
  case ETOOMANYREFS:
    WSASetLastError(WSAETOOMANYREFS);
    break;
  case ETIMEDOUT:
    WSASetLastError(WSAETIMEDOUT);
    break;
  case ECONNREFUSED:
    WSASetLastError(WSAECONNREFUSED);
    break;
  case ELOOP:
    WSASetLastError(WSAELOOP);
    break;
  case ENAMETOOLONG:
    WSASetLastError(WSAENAMETOOLONG);
    break;
  case EHOSTDOWN:
    WSASetLastError(WSAEHOSTDOWN);
    break;
  case EHOSTUNREACH:
    WSASetLastError(WSAEHOSTUNREACH);
    break;
  case ENOTEMPTY:
    WSASetLastError(WSAENOTEMPTY);
    break;
  case EPROCLIM:
    WSASetLastError(WSAEPROCLIM);
    break;
  case EUSERS:
    WSASetLastError(WSAEUSERS);
    break;
  case EDQUOT:
    WSASetLastError(WSAEDQUOT);
    break;
  case ESTALE:
    WSASetLastError(WSAESTALE);
    break;
  case EREMOTE:
    WSASetLastError(WSAEREMOTE);
    break;
  case EFAULT:
    WSASetLastError(WSAEFAULT);
    break;
  case ENODATA:
    WSASetLastError(WSANO_DATA);
    break;
#if EAGAIN != EWOULDBLOCK
  case EAGAIN:
    WSASetLastError(WSAEWOULDBLOCK);
    break;
#endif
  /* Rough equivalent */
  case EIO:
    WSASetLastError(WSAEREFUSED);
    break;

  default: /* Unmapped errors */
    WSASetLastError(WSAENOBUFS);
    break;
    }
}

/**
 * Create pair of mutually connected TCP/IP sockets on loopback address
 * @param sockets_pair array to receive resulted sockets
 * @return zero on success, -1 otherwise
 */
int MHD_W32_pair_of_sockets_(SOCKET sockets_pair[2])
{
  int i;
  if (!sockets_pair)
    {
      errno = EINVAL;
      return -1;
    }

#define PAIRMAXTRYIES 800
  for (i = 0; i < PAIRMAXTRYIES; i++)
    {
      struct sockaddr_in listen_addr;
      SOCKET listen_s;
      static const int c_addinlen = sizeof(struct sockaddr_in); /* help compiler to optimize */
      int addr_len = c_addinlen;
      int opt = 1;

      listen_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (INVALID_SOCKET == listen_s)
        break; /* can't create even single socket */

      listen_addr.sin_family = AF_INET;
      listen_addr.sin_port = htons(0);
      listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      if (0 == bind(listen_s, (struct sockaddr*) &listen_addr, c_addinlen)
          && 0 == listen(listen_s, 1)
          && 0 == getsockname(listen_s, (struct sockaddr*) &listen_addr,
                  &addr_len))
        {
          SOCKET client_s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
          if (INVALID_SOCKET != client_s)
            {
              if (0 == ioctlsocket(client_s, FIONBIO, (u_long*) &opt)
                  && (0 == connect(client_s, (struct sockaddr*) &listen_addr, c_addinlen)
                      || WSAGetLastError() == WSAEWOULDBLOCK))
                {
                  struct sockaddr_in accepted_from_addr;
                  SOCKET server_s;
                  addr_len = c_addinlen;
                  server_s = accept(listen_s,
                      (struct sockaddr*) &accepted_from_addr, &addr_len);
                  if (INVALID_SOCKET != server_s)
                    {
                      struct sockaddr_in client_addr;
                      addr_len = c_addinlen;
                      opt = 0;
                      if (0 == getsockname(client_s, (struct sockaddr*) &client_addr, &addr_len)
                          && accepted_from_addr.sin_family == client_addr.sin_family
                          && accepted_from_addr.sin_port == client_addr.sin_port
                          && accepted_from_addr.sin_addr.s_addr == client_addr.sin_addr.s_addr
                          && 0 == ioctlsocket(client_s, FIONBIO, (u_long*) &opt)
                          && 0 == ioctlsocket(server_s, FIONBIO, (u_long*) &opt))
                        {
                          closesocket(listen_s);
                          sockets_pair[0] = client_s;
                          sockets_pair[1] = server_s;
                          return 0;
                        }
                      closesocket(server_s);
                    }
                }
              closesocket(client_s);
            }
        }
      closesocket(listen_s);
    }

  sockets_pair[0] = INVALID_SOCKET;
  sockets_pair[1] = INVALID_SOCKET;
  return -1;
}

/**
 * Static variable used by pseudo random number generator
 */
static int32_t rnd_val = 0;
/**
 * Generate 31-bit pseudo random number.
 * Function initialize itself at first call to current time.
 * @return 31-bit pseudo random number.
 */
int MHD_W32_random_(void)
{
  if (0 == rnd_val)
    rnd_val = (int32_t)time(NULL);
  /* stolen from winsup\cygwin\random.cc */
  rnd_val = (16807 * (rnd_val % 127773) - 2836 * (rnd_val / 127773))
               & 0x7fffffff;
  return (int)rnd_val;
}

/* Emulate snprintf function on W32 */
int W32_snprintf(char *__restrict s, size_t n, const char *__restrict format, ...)
{
  int ret;
  va_list args;
  if (0 != n && NULL != s )
  {
    va_start(args, format);
    ret = _vsnprintf(s, n, format, args);
    va_end(args);
    if ((int)n == ret)
      s[n - 1] = 0;
    if (ret >= 0)
      return ret;
  }
  va_start(args, format);
  ret = _vscprintf(format, args);
  va_end(args);
  if (0 <= ret && 0 != n && NULL == s)
    return -1;

  return ret;
}

#ifdef _MSC_FULL_VER
/**
 * Set thread name
 * @param thread_id ID of thread, -1 for current thread
 * @param thread_name name to set
 */
void W32_SetThreadName(const DWORD thread_id, const char *thread_name)
{
  static const DWORD VC_SETNAME_EXC = 0x406D1388;
#pragma pack(push,8)
  struct thread_info_struct
  {
    DWORD type;   /* Must be 0x1000. */
    LPCSTR name;  /* Pointer to name (in user address space). */
    DWORD ID;     /* Thread ID (-1=caller thread). */
    DWORD flags;  /* Reserved for future use, must be zero. */
  } thread_info;
#pragma pack(pop)

  if (NULL == thread_name)
    return;

  thread_info.type  = 0x1000;
  thread_info.name  = thread_name;
  thread_info.ID    = thread_id;
  thread_info.flags = 0;

  __try
  { /* This exception is intercepted by debugger */
    RaiseException(VC_SETNAME_EXC, 0, sizeof(thread_info) / sizeof(ULONG_PTR), (ULONG_PTR*)&thread_info);
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {}
}
#endif /* _MSC_FULL_VER */
