/*
  This file is part of libmicrohttpd
  Copyright (C) 2007-2014 Daniel Pittman and Christian Grothoff

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

/**
 * @file microhttpd/daemon.c
 * @brief  A minimal-HTTP server library
 * @author Daniel Pittman
 * @author Christian Grothoff
 */
#include "platform.h"
#include "internal.h"
#include "response.h"
#include "connection.h"
#include "memorypool.h"
#include "mhd_limits.h"
#include "autoinit_funcs.h"
#include "mhd_mono_clock.h"

#if HAVE_SEARCH_H
#include <search.h>
#else
#include "tsearch.h"
#endif

#if HTTPS_SUPPORT
#include "connection_https.h"
#include <gcrypt.h>
#endif

#if defined(HAVE_POLL_H) && defined(HAVE_POLL)
#include <poll.h>
#endif

#ifdef LINUX
#include <sys/sendfile.h>
#endif

#ifndef _MHD_FD_SETSIZE_IS_DEFAULT
#include "sysfdsetsize.h"
#endif /* !_MHD_FD_SETSIZE_IS_DEFAULT */

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>
#include <process.h>
#endif

#ifndef HAVE_ACCEPT4
#define HAVE_ACCEPT4 0
#endif

/**
 * Default connection limit.
 */
#ifdef MHD_POSIX_SOCKETS
#define MHD_MAX_CONNECTIONS_DEFAULT (FD_SETSIZE - 4)
#else
#define MHD_MAX_CONNECTIONS_DEFAULT (FD_SETSIZE - 2)
#endif

/**
 * Default memory allowed per connection.
 */
#define MHD_POOL_SIZE_DEFAULT (32 * 1024)

#ifdef TCP_FASTOPEN
/**
 * Default TCP fastopen queue size.
 */
#define MHD_TCP_FASTOPEN_QUEUE_SIZE_DEFAULT 10
#endif

/**
 * Print extra messages with reasons for closing
 * sockets? (only adds non-error messages).
 */
#define DEBUG_CLOSE MHD_NO

/**
 * Print extra messages when establishing
 * connections? (only adds non-error messages).
 */
#define DEBUG_CONNECT MHD_NO

#ifndef LINUX
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
#endif

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif

#ifndef EPOLL_CLOEXEC
#define EPOLL_CLOEXEC 0
#endif


/**
 * Default implementation of the panic function,
 * prints an error message and aborts.
 *
 * @param cls unused
 * @param file name of the file with the problem
 * @param line line number with the problem
 * @param reason error message with details
 */
static void
mhd_panic_std (void *cls,
	       const char *file,
	       unsigned int line,
	       const char *reason)
{
#if HAVE_MESSAGES
  fprintf (stderr,
           "Fatal error in GNU libmicrohttpd %s:%u: %s\n",
	   file, line, reason);
#endif
  abort ();
}


/**
 * Handler for fatal errors.
 */
MHD_PanicCallback mhd_panic;

/**
 * Closure argument for "mhd_panic".
 */
void *mhd_panic_cls;

#ifdef _WIN32
/**
 * Track initialization of winsock
 */
static int mhd_winsock_inited_ = 0;
#endif

/**
 * Trace up to and return master daemon. If the supplied daemon
 * is a master, then return the daemon itself.
 *
 * @param daemon handle to a daemon
 * @return master daemon handle
 */
static struct MHD_Daemon*
MHD_get_master (struct MHD_Daemon *daemon)
{
  while (NULL != daemon->master)
    daemon = daemon->master;
  return daemon;
}


/**
 * Maintain connection count for single address.
 */
struct MHD_IPCount
{
  /**
   * Address family. AF_INET or AF_INET6 for now.
   */
  int family;

  /**
   * Actual address.
   */
  union
  {
    /**
     * IPv4 address.
     */
    struct in_addr ipv4;
#if HAVE_INET6
    /**
     * IPv6 address.
     */
    struct in6_addr ipv6;
#endif
  } addr;

  /**
   * Counter.
   */
  unsigned int count;
};


/**
 * Lock shared structure for IP connection counts and connection DLLs.
 *
 * @param daemon handle to daemon where lock is
 */
static void
MHD_ip_count_lock (struct MHD_Daemon *daemon)
{
  if (MHD_YES != MHD_mutex_lock_(&daemon->per_ip_connection_mutex))
    {
      MHD_PANIC ("Failed to acquire IP connection limit mutex\n");
    }
}


/**
 * Unlock shared structure for IP connection counts and connection DLLs.
 *
 * @param daemon handle to daemon where lock is
 */
static void
MHD_ip_count_unlock (struct MHD_Daemon *daemon)
{
  if (MHD_YES != MHD_mutex_unlock_(&daemon->per_ip_connection_mutex))
    {
      MHD_PANIC ("Failed to release IP connection limit mutex\n");
    }
}


/**
 * Tree comparison function for IP addresses (supplied to tsearch() family).
 * We compare everything in the struct up through the beginning of the
 * 'count' field.
 *
 * @param a1 first address to compare
 * @param a2 second address to compare
 * @return -1, 0 or 1 depending on result of compare
 */
static int
MHD_ip_addr_compare (const void *a1, const void *a2)
{
  return memcmp (a1, a2, offsetof (struct MHD_IPCount, count));
}


/**
 * Parse address and initialize 'key' using the address.
 *
 * @param addr address to parse
 * @param addrlen number of bytes in addr
 * @param key where to store the parsed address
 * @return #MHD_YES on success and #MHD_NO otherwise (e.g., invalid address type)
 */
static int
MHD_ip_addr_to_key (const struct sockaddr *addr,
		    socklen_t addrlen,
		    struct MHD_IPCount *key)
{
  memset(key, 0, sizeof(*key));

  /* IPv4 addresses */
  if (sizeof (struct sockaddr_in) == addrlen)
    {
      const struct sockaddr_in *addr4 = (const struct sockaddr_in*) addr;
      key->family = AF_INET;
      memcpy (&key->addr.ipv4, &addr4->sin_addr, sizeof(addr4->sin_addr));
      return MHD_YES;
    }

#if HAVE_INET6
  /* IPv6 addresses */
  if (sizeof (struct sockaddr_in6) == addrlen)
    {
      const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6*) addr;
      key->family = AF_INET6;
      memcpy (&key->addr.ipv6, &addr6->sin6_addr, sizeof(addr6->sin6_addr));
      return MHD_YES;
    }
#endif

  /* Some other address */
  return MHD_NO;
}


/**
 * Check if IP address is over its limit.
 *
 * @param daemon handle to daemon where connection counts are tracked
 * @param addr address to add (or increment counter)
 * @param addrlen number of bytes in addr
 * @return Return #MHD_YES if IP below limit, #MHD_NO if IP has surpassed limit.
 *   Also returns #MHD_NO if fails to allocate memory.
 */
static int
MHD_ip_limit_add (struct MHD_Daemon *daemon,
		  const struct sockaddr *addr,
		  socklen_t addrlen)
{
  struct MHD_IPCount *key;
  void **nodep;
  void *node;
  int result;

  daemon = MHD_get_master (daemon);
  /* Ignore if no connection limit assigned */
  if (0 == daemon->per_ip_connection_limit)
    return MHD_YES;

  if (NULL == (key = malloc (sizeof(*key))))
    return MHD_NO;

  /* Initialize key */
  if (MHD_NO == MHD_ip_addr_to_key (addr, addrlen, key))
    {
      /* Allow unhandled address types through */
      free (key);
      return MHD_YES;
    }
  MHD_ip_count_lock (daemon);

  /* Search for the IP address */
  if (NULL == (nodep = tsearch (key,
				&daemon->per_ip_connection_count,
				&MHD_ip_addr_compare)))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Failed to add IP connection count node\n");
#endif
      MHD_ip_count_unlock (daemon);
      free (key);
      return MHD_NO;
    }
  node = *nodep;
  /* If we got an existing node back, free the one we created */
  if (node != key)
    free(key);
  key = (struct MHD_IPCount *) node;
  /* Test if there is room for another connection; if so,
   * increment count */
  result = (key->count < daemon->per_ip_connection_limit) ? MHD_YES : MHD_NO;
  if (MHD_YES == result)
    ++key->count;

  MHD_ip_count_unlock (daemon);
  return result;
}


/**
 * Decrement connection count for IP address, removing from table
 * count reaches 0.
 *
 * @param daemon handle to daemon where connection counts are tracked
 * @param addr address to remove (or decrement counter)
 * @param addrlen number of bytes in @a addr
 */
static void
MHD_ip_limit_del (struct MHD_Daemon *daemon,
		  const struct sockaddr *addr,
		  socklen_t addrlen)
{
  struct MHD_IPCount search_key;
  struct MHD_IPCount *found_key;
  void **nodep;

  daemon = MHD_get_master (daemon);
  /* Ignore if no connection limit assigned */
  if (0 == daemon->per_ip_connection_limit)
    return;
  /* Initialize search key */
  if (MHD_NO == MHD_ip_addr_to_key (addr, addrlen, &search_key))
    return;

  MHD_ip_count_lock (daemon);

  /* Search for the IP address */
  if (NULL == (nodep = tfind (&search_key,
			      &daemon->per_ip_connection_count,
			      &MHD_ip_addr_compare)))
    {
      /* Something's wrong if we couldn't find an IP address
       * that was previously added */
      MHD_PANIC ("Failed to find previously-added IP address\n");
    }
  found_key = (struct MHD_IPCount *) *nodep;
  /* Validate existing count for IP address */
  if (0 == found_key->count)
    {
      MHD_PANIC ("Previously-added IP address had 0 count\n");
    }
  /* Remove the node entirely if count reduces to 0 */
  if (0 == --found_key->count)
    {
      tdelete (found_key,
	       &daemon->per_ip_connection_count,
	       &MHD_ip_addr_compare);
      free (found_key);
    }

  MHD_ip_count_unlock (daemon);
}


#if HTTPS_SUPPORT
/**
 * Callback for receiving data from the socket.
 *
 * @param connection the MHD_Connection structure
 * @param other where to write received data to
 * @param i maximum size of other (in bytes)
 * @return number of bytes actually received
 */
static ssize_t
recv_tls_adapter (struct MHD_Connection *connection, void *other, size_t i)
{
  ssize_t res;

  if (MHD_YES == connection->tls_read_ready)
    {
      connection->daemon->num_tls_read_ready--;
      connection->tls_read_ready = MHD_NO;
    }
  res = gnutls_record_recv (connection->tls_session, other, i);
  if ( (GNUTLS_E_AGAIN == res) ||
       (GNUTLS_E_INTERRUPTED == res) )
    {
      MHD_set_socket_errno_ (EINTR);
#if EPOLL_SUPPORT
      connection->epoll_state &= ~MHD_EPOLL_STATE_READ_READY;
#endif
      return -1;
    }
  if (res < 0)
    {
      /* Likely 'GNUTLS_E_INVALID_SESSION' (client communication
	 disrupted); set errno to something caller will interpret
	 correctly as a hard error */
      MHD_set_socket_errno_ (ECONNRESET);
      return res;
    }
  if ((size_t)res == i)
    {
      connection->tls_read_ready = MHD_YES;
      connection->daemon->num_tls_read_ready++;
    }
  return res;
}


/**
 * Callback for writing data to the socket.
 *
 * @param connection the MHD connection structure
 * @param other data to write
 * @param i number of bytes to write
 * @return actual number of bytes written
 */
static ssize_t
send_tls_adapter (struct MHD_Connection *connection,
                  const void *other, size_t i)
{
  int res;

  res = gnutls_record_send (connection->tls_session, other, i);
  if ( (GNUTLS_E_AGAIN == res) ||
       (GNUTLS_E_INTERRUPTED == res) )
    {
      MHD_set_socket_errno_ (EINTR);
#if EPOLL_SUPPORT
      connection->epoll_state &= ~MHD_EPOLL_STATE_WRITE_READY;
#endif
      return -1;
    }
  if (res < 0)
    {
      /* some other GNUTLS error, should set 'errno'; as we do not
         really understand the error (not listed in GnuTLS
         documentation explicitly), we set 'errno' to something that
         will cause the connection to fail. */
      MHD_set_socket_errno_ (ECONNRESET);
      return -1;
    }
  return res;
}


/**
 * Read and setup our certificate and key.
 *
 * @param daemon handle to daemon to initialize
 * @return 0 on success
 */
static int
MHD_init_daemon_certificate (struct MHD_Daemon *daemon)
{
  gnutls_datum_t key;
  gnutls_datum_t cert;
  int ret;

#if GNUTLS_VERSION_MAJOR >= 3
  if (NULL != daemon->cert_callback)
    {
      gnutls_certificate_set_retrieve_function2 (daemon->x509_cred,
                                                 daemon->cert_callback);
    }
#endif
  if (NULL != daemon->https_mem_trust)
    {
      cert.data = (unsigned char *) daemon->https_mem_trust;
      cert.size = strlen (daemon->https_mem_trust);
      if (gnutls_certificate_set_x509_trust_mem (daemon->x509_cred, &cert,
						 GNUTLS_X509_FMT_PEM) < 0)
	{
#if HAVE_MESSAGES
	  MHD_DLOG(daemon,
		   "Bad trust certificate format\n");
#endif
	  return -1;
	}
    }

  if (MHD_YES == daemon->have_dhparams)
    {
      gnutls_certificate_set_dh_params (daemon->x509_cred,
                                        daemon->https_mem_dhparams);
    }
  /* certificate & key loaded from memory */
  if ( (NULL != daemon->https_mem_cert) &&
       (NULL != daemon->https_mem_key) )
    {
      key.data = (unsigned char *) daemon->https_mem_key;
      key.size = strlen (daemon->https_mem_key);
      cert.data = (unsigned char *) daemon->https_mem_cert;
      cert.size = strlen (daemon->https_mem_cert);

      if (NULL != daemon->https_key_password) {
#if GNUTLS_VERSION_NUMBER >= 0x030111
        ret = gnutls_certificate_set_x509_key_mem2 (daemon->x509_cred,
                                                    &cert, &key,
                                                    GNUTLS_X509_FMT_PEM,
                                                    daemon->https_key_password,
                                                    0);
#else
#if HAVE_MESSAGES
	MHD_DLOG (daemon,
                  "Failed to setup x509 certificate/key: pre 3.X.X version " \
		  "of GnuTLS does not support setting key password");
#endif
	return -1;
#endif
      }
      else
        ret = gnutls_certificate_set_x509_key_mem (daemon->x509_cred,
                                                   &cert, &key,
                                                   GNUTLS_X509_FMT_PEM);
#if HAVE_MESSAGES
      if (0 != ret)
        MHD_DLOG (daemon,
                  "GnuTLS failed to setup x509 certificate/key: %s\n",
                  gnutls_strerror (ret));
#endif
      return ret;
    }
#if GNUTLS_VERSION_MAJOR >= 3
  if (NULL != daemon->cert_callback)
    return 0;
#endif
#if HAVE_MESSAGES
  MHD_DLOG (daemon,
            "You need to specify a certificate and key location\n");
#endif
  return -1;
}


/**
 * Initialize security aspects of the HTTPS daemon
 *
 * @param daemon handle to daemon to initialize
 * @return 0 on success
 */
static int
MHD_TLS_init (struct MHD_Daemon *daemon)
{
  switch (daemon->cred_type)
    {
    case GNUTLS_CRD_CERTIFICATE:
      if (0 !=
          gnutls_certificate_allocate_credentials (&daemon->x509_cred))
        return GNUTLS_E_MEMORY_ERROR;
      return MHD_init_daemon_certificate (daemon);
    default:
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Error: invalid credentials type %d specified.\n",
                daemon->cred_type);
#endif
      return -1;
    }
}
#endif


/**
 * Add @a fd to the @a set.  If @a fd is
 * greater than @a max_fd, set @a max_fd to @a fd.
 *
 * @param fd file descriptor to add to the @a set
 * @param set set to modify
 * @param max_fd maximum value to potentially update
 * @param fd_setsize value of FD_SETSIZE
 * @return #MHD_YES on success, #MHD_NO otherwise
 */
static int
add_to_fd_set (MHD_socket fd,
	       fd_set *set,
	       MHD_socket *max_fd,
	       unsigned int fd_setsize)
{
  if (NULL == set)
    return MHD_NO;
#ifdef MHD_WINSOCK_SOCKETS
  if (set->fd_count >= fd_setsize)
    {
      if (FD_ISSET(fd, set))
        return MHD_YES;
      else
        return MHD_NO;
    }
#else  /* ! MHD_WINSOCK_SOCKETS */
  if (fd >= (MHD_socket)fd_setsize)
    return MHD_NO;
#endif /* ! MHD_WINSOCK_SOCKETS */
  FD_SET (fd, set);
  if ( (NULL != max_fd) && (MHD_INVALID_SOCKET != fd) &&
       ((fd > *max_fd) || (MHD_INVALID_SOCKET == *max_fd)) )
    *max_fd = fd;

  return MHD_YES;
}

#undef MHD_get_fdset

/**
 * Obtain the `select()` sets for this daemon.
 * Daemon's FDs will be added to fd_sets. To get only
 * daemon FDs in fd_sets, call FD_ZERO for each fd_set
 * before calling this function. FD_SETSIZE is assumed
 * to be platform's default.
 *
 * @param daemon daemon to get sets from
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set
 * @param max_fd increased to largest FD added (if larger
 *               than existing value); can be NULL
 * @return #MHD_YES on success, #MHD_NO if this
 *         daemon was not started with the right
 *         options for this call or any FD didn't
 *         fit fd_set.
 * @ingroup event
 */
int
MHD_get_fdset (struct MHD_Daemon *daemon,
               fd_set *read_fd_set,
               fd_set *write_fd_set,
	       fd_set *except_fd_set,
	       MHD_socket *max_fd)
{
  return MHD_get_fdset2(daemon, read_fd_set,
      write_fd_set, except_fd_set,
      max_fd, _MHD_SYS_DEFAULT_FD_SETSIZE);
}

/**
 * Obtain the `select()` sets for this daemon.
 * Daemon's FDs will be added to fd_sets. To get only
 * daemon FDs in fd_sets, call FD_ZERO for each fd_set
 * before calling this function. Passing custom FD_SETSIZE
 * as @a fd_setsize allow usage of larger/smaller than
 * platform's default fd_sets.
 *
 * @param daemon daemon to get sets from
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set
 * @param max_fd increased to largest FD added (if larger
 *               than existing value); can be NULL
 * @param fd_setsize value of FD_SETSIZE
 * @return #MHD_YES on success, #MHD_NO if this
 *         daemon was not started with the right
 *         options for this call or any FD didn't
 *         fit fd_set.
 * @ingroup event
 */
int
MHD_get_fdset2 (struct MHD_Daemon *daemon,
               fd_set *read_fd_set,
               fd_set *write_fd_set,
               fd_set *except_fd_set,
               MHD_socket *max_fd,
               unsigned int fd_setsize)
{
  struct MHD_Connection *pos;
  int result = MHD_YES;

  if ( (NULL == daemon)
       || (NULL == read_fd_set)
       || (NULL == write_fd_set)
       || (MHD_YES == daemon->shutdown)
       || (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
       || (0 != (daemon->options & MHD_USE_POLL)))
    return MHD_NO;
#if EPOLL_SUPPORT
  if (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY))
    {
      /* we're in epoll mode, use the epoll FD as a stand-in for
	 the entire event set */

      return add_to_fd_set (daemon->epoll_fd, read_fd_set, max_fd, fd_setsize);
    }
#endif
  if (MHD_INVALID_SOCKET != daemon->socket_fd &&
      MHD_YES != add_to_fd_set (daemon->socket_fd, read_fd_set, max_fd, fd_setsize))
    result = MHD_NO;

  for (pos = daemon->connections_head; NULL != pos; pos = pos->next)
    {
      switch (pos->event_loop_info)
	{
	case MHD_EVENT_LOOP_INFO_READ:
	  if (MHD_YES != add_to_fd_set (pos->socket_fd, read_fd_set, max_fd, fd_setsize))
	    result = MHD_NO;
	  break;
	case MHD_EVENT_LOOP_INFO_WRITE:
	  if (MHD_YES != add_to_fd_set (pos->socket_fd, write_fd_set, max_fd, fd_setsize))
	    result = MHD_NO;
	  if (pos->read_buffer_size > pos->read_buffer_offset &&
	      MHD_YES != add_to_fd_set (pos->socket_fd, read_fd_set, max_fd, fd_setsize))
            result = MHD_NO;
	  break;
	case MHD_EVENT_LOOP_INFO_BLOCK:
	  if (pos->read_buffer_size > pos->read_buffer_offset &&
	      MHD_YES != add_to_fd_set (pos->socket_fd, read_fd_set, max_fd, fd_setsize))
            result = MHD_NO;
	  break;
	case MHD_EVENT_LOOP_INFO_CLEANUP:
	  /* this should never happen */
	  break;
	}
    }
#if DEBUG_CONNECT
#if HAVE_MESSAGES
  if (NULL != max_fd)
    MHD_DLOG (daemon,
              "Maximum socket in select set: %d\n",
              *max_fd);
#endif
#endif
  return result;
}


/**
 * Main function of the thread that handles an individual
 * connection when #MHD_USE_THREAD_PER_CONNECTION is set.
 *
 * @param data the `struct MHD_Connection` this thread will handle
 * @return always 0
 */
static MHD_THRD_RTRN_TYPE_ MHD_THRD_CALL_SPEC_
MHD_handle_connection (void *data)
{
  struct MHD_Connection *con = data;
  int num_ready;
  fd_set rs;
  fd_set ws;
  MHD_socket maxsock;
  struct timeval tv;
  struct timeval *tvp;
  unsigned int timeout;
  time_t now;
#if WINDOWS
  MHD_pipe spipe = con->daemon->wpipe[0];
  char tmp;
#ifdef HAVE_POLL
  int extra_slot;
#endif /* HAVE_POLL */
#define EXTRA_SLOTS 1
#else  /* !WINDOWS */
#define EXTRA_SLOTS 0
#endif /* !WINDOWS */
#ifdef HAVE_POLL
  struct pollfd p[1 + EXTRA_SLOTS];
#endif

  timeout = con->daemon->connection_timeout;
  while ( (MHD_YES != con->daemon->shutdown) &&
	  (MHD_CONNECTION_CLOSED != con->state) )
    {
      tvp = NULL;
#if HTTPS_SUPPORT
      if (MHD_YES == con->tls_read_ready)
	{
	  /* do not block (more data may be inside of TLS buffers waiting for us) */
	  tv.tv_sec = 0;
	  tv.tv_usec = 0;
	  tvp = &tv;
	}
#endif
      if (NULL == tvp && timeout > 0)
	{
	  now = MHD_monotonic_sec_counter();
	  if (now - con->last_activity > timeout)
	    tv.tv_sec = 0;
          else
            {
              const time_t seconds_left = timeout - (now - con->last_activity);
#ifndef _WIN32
              tv.tv_sec = seconds_left;
#else  /* _WIN32 */
              if (seconds_left > TIMEVAL_TV_SEC_MAX)
                tv.tv_sec = TIMEVAL_TV_SEC_MAX;
              else
                tv.tv_sec = (_MHD_TIMEVAL_TV_SEC_TYPE)seconds_left;
#endif /* _WIN32 */
            }
	  tv.tv_usec = 0;
	  tvp = &tv;
	}
      if (0 == (con->daemon->options & MHD_USE_POLL))
	{
	  /* use select */
	  int err_state = 0;
	  FD_ZERO (&rs);
	  FD_ZERO (&ws);
	  maxsock = MHD_INVALID_SOCKET;
	  switch (con->event_loop_info)
	    {
	    case MHD_EVENT_LOOP_INFO_READ:
	      if (MHD_YES !=
                  add_to_fd_set (con->socket_fd, &rs, &maxsock, FD_SETSIZE))
	        err_state = 1;
	      break;
	    case MHD_EVENT_LOOP_INFO_WRITE:
	      if (MHD_YES !=
                  add_to_fd_set (con->socket_fd, &ws, &maxsock, FD_SETSIZE))
                err_state = 1;
	      if ( (con->read_buffer_size > con->read_buffer_offset) &&
                   (MHD_YES !=
                    add_to_fd_set (con->socket_fd, &rs, &maxsock, FD_SETSIZE)) )
	        err_state = 1;
	      break;
	    case MHD_EVENT_LOOP_INFO_BLOCK:
	      if ( (con->read_buffer_size > con->read_buffer_offset) &&
                   (MHD_YES !=
                    add_to_fd_set (con->socket_fd, &rs, &maxsock, FD_SETSIZE)) )
	        err_state = 1;
	      tv.tv_sec = 0;
	      tv.tv_usec = 0;
	      tvp = &tv;
	      break;
	    case MHD_EVENT_LOOP_INFO_CLEANUP:
	      /* how did we get here!? */
	      goto exit;
	    }
#if WINDOWS
          if (MHD_INVALID_PIPE_ != spipe)
            {
              if (MHD_YES !=
                  add_to_fd_set (spipe, &rs, &maxsock, FD_SETSIZE))
                err_state = 1;
            }
#endif
            if (0 != err_state)
              {
#if HAVE_MESSAGES
                MHD_DLOG (con->daemon,
                          "Can't add FD to fd_set\n");
#endif
                goto exit;
              }

	  num_ready = MHD_SYS_select_ (maxsock + 1, &rs, &ws, NULL, tvp);
	  if (num_ready < 0)
	    {
	      if (EINTR == MHD_socket_errno_)
		continue;
#if HAVE_MESSAGES
	      MHD_DLOG (con->daemon,
			"Error during select (%d): `%s'\n",
			MHD_socket_errno_,
			MHD_socket_last_strerr_ ());
#endif
	      break;
	    }
#if WINDOWS
          /* drain signaling pipe */
          if ( (MHD_INVALID_PIPE_ != spipe) &&
               (FD_ISSET (spipe, &rs)) )
            (void) MHD_pipe_read_ (spipe, &tmp, sizeof (tmp));
#endif
	  /* call appropriate connection handler if necessary */
	  if ( (FD_ISSET (con->socket_fd, &rs))
#if HTTPS_SUPPORT
	       || (MHD_YES == con->tls_read_ready)
#endif
	       )
	    con->read_handler (con);
	  if (FD_ISSET (con->socket_fd, &ws))
	    con->write_handler (con);
	  if (MHD_NO == con->idle_handler (con))
	    goto exit;
	}
#ifdef HAVE_POLL
      else
	{
	  /* use poll */
	  memset (&p, 0, sizeof (p));
	  p[0].fd = con->socket_fd;
	  switch (con->event_loop_info)
	    {
	    case MHD_EVENT_LOOP_INFO_READ:
	      p[0].events |= POLLIN;
	      break;
	    case MHD_EVENT_LOOP_INFO_WRITE:
	      p[0].events |= POLLOUT;
	      if (con->read_buffer_size > con->read_buffer_offset)
		p[0].events |= POLLIN;
	      break;
	    case MHD_EVENT_LOOP_INFO_BLOCK:
	      if (con->read_buffer_size > con->read_buffer_offset)
		p[0].events |= POLLIN;
	      tv.tv_sec = 0;
	      tv.tv_usec = 0;
	      tvp = &tv;
	      break;
	    case MHD_EVENT_LOOP_INFO_CLEANUP:
	      /* how did we get here!? */
	      goto exit;
	    }
#if WINDOWS
          extra_slot = 0;
          if (MHD_INVALID_PIPE_ != spipe)
            {
              p[1].events |= POLLIN;
              p[1].fd = spipe;
              p[1].revents = 0;
              extra_slot = 1;
            }
#endif
	  if (MHD_sys_poll_ (p,
#if WINDOWS
                    1 + extra_slot,
#else
                    1,
#endif
		    (NULL == tvp) ? -1 : tv.tv_sec * 1000) < 0)
	    {
	      if (EINTR == MHD_socket_errno_)
		continue;
#if HAVE_MESSAGES
	      MHD_DLOG (con->daemon,
                        "Error during poll: `%s'\n",
			MHD_socket_last_strerr_ ());
#endif
	      break;
	    }
#if WINDOWS
          /* drain signaling pipe */
          if ( (MHD_INVALID_PIPE_ != spipe) &&
               (0 != (p[1].revents & (POLLERR | POLLHUP))) )
            (void) MHD_pipe_read_ (spipe, &tmp, sizeof (tmp));
#endif
	  if ( (0 != (p[0].revents & POLLIN))
#if HTTPS_SUPPORT
	       || (MHD_YES == con->tls_read_ready)
#endif
	       )
	    con->read_handler (con);
	  if (0 != (p[0].revents & POLLOUT))
	    con->write_handler (con);
	  if (0 != (p[0].revents & (POLLERR | POLLHUP)))
	    MHD_connection_close_ (con,
                                   MHD_REQUEST_TERMINATED_WITH_ERROR);
	  if (MHD_NO == con->idle_handler (con))
	    goto exit;
	}
#endif
    }
  if (MHD_CONNECTION_IN_CLEANUP != con->state)
    {
#if DEBUG_CLOSE
#if HAVE_MESSAGES
      MHD_DLOG (con->daemon,
                "Processing thread terminating, closing connection\n");
#endif
#endif
      if (MHD_CONNECTION_CLOSED != con->state)
	MHD_connection_close_ (con,
                               MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN);
      con->idle_handler (con);
    }
exit:
  if (NULL != con->response)
    {
      MHD_destroy_response (con->response);
      con->response = NULL;
    }

  if (NULL != con->daemon->notify_connection)
    con->daemon->notify_connection (con->daemon->notify_connection_cls,
                                    con,
                                    &con->socket_context,
                                    MHD_CONNECTION_NOTIFY_CLOSED);
  if (MHD_INVALID_SOCKET != con->socket_fd)
    {
#ifdef WINDOWS
      shutdown (con->socket_fd, SHUT_WR);
#endif
      if (0 != MHD_socket_close_ (con->socket_fd))
        MHD_PANIC ("close failed\n");
      con->socket_fd = MHD_INVALID_SOCKET;
    }
  return (MHD_THRD_RTRN_TYPE_) 0;
}


/**
 * Callback for receiving data from the socket.
 *
 * @param connection the MHD connection structure
 * @param other where to write received data to
 * @param i maximum size of other (in bytes)
 * @return number of bytes actually received
 */
static ssize_t
recv_param_adapter (struct MHD_Connection *connection,
		    void *other,
		    size_t i)
{
  ssize_t ret;
#if EPOLL_SUPPORT
  const size_t requested_size = i;
#endif

  if ( (MHD_INVALID_SOCKET == connection->socket_fd) ||
       (MHD_CONNECTION_CLOSED == connection->state) )
    {
      MHD_set_socket_errno_ (ENOTCONN);
      return -1;
    }
#ifdef MHD_POSIX_SOCKETS
  if (i > SSIZE_MAX)
    i = SSIZE_MAX; /* return value limit */
#else  /* MHD_WINSOCK_SOCKETS */
  if (i > INT_MAX)
    i = INT_MAX; /* return value limit */
#endif /* MHD_WINSOCK_SOCKETS */

  ret = (ssize_t)recv (connection->socket_fd, other, (_MHD_socket_funcs_size)i, MSG_NOSIGNAL);
#if EPOLL_SUPPORT
  if ( (0 > ret) || (requested_size > (size_t) ret))
    {
      /* partial read --- no longer read-ready */
      connection->epoll_state &= ~MHD_EPOLL_STATE_READ_READY;
    }
#endif
  return ret;
}


/**
 * Callback for writing data to the socket.
 *
 * @param connection the MHD connection structure
 * @param other data to write
 * @param i number of bytes to write
 * @return actual number of bytes written
 */
static ssize_t
send_param_adapter (struct MHD_Connection *connection,
                    const void *other,
		    size_t i)
{
  ssize_t ret;
#if EPOLL_SUPPORT
  const size_t requested_size = i;
#endif
#if LINUX
  MHD_socket fd;
#endif

  if ( (MHD_INVALID_SOCKET == connection->socket_fd) ||
       (MHD_CONNECTION_CLOSED == connection->state) )
    {
      MHD_set_socket_errno_ (ENOTCONN);
      return -1;
    }
#ifdef MHD_POSIX_SOCKETS
  if (i > SSIZE_MAX)
    i = SSIZE_MAX; /* return value limit */
#else  /* MHD_WINSOCK_SOCKETS */
  if (i > INT_MAX)
    i = INT_MAX; /* return value limit */
#endif /* MHD_WINSOCK_SOCKETS */

  if (0 != (connection->daemon->options & MHD_USE_SSL))
    return (ssize_t)send (connection->socket_fd, other, (_MHD_socket_funcs_size)i, MSG_NOSIGNAL);
#if LINUX
  if ( (connection->write_buffer_append_offset ==
	connection->write_buffer_send_offset) &&
       (NULL != connection->response) &&
       (-1 != (fd = connection->response->fd)) )
    {
      /* can use sendfile */
      uint64_t left;
      uint64_t offsetu64;
      int err;
#ifndef HAVE_SENDFILE64
      off_t offset;
#else  /* HAVE_SENDFILE64 */
      off64_t offset;
#endif /* HAVE_SENDFILE64 */
      offsetu64 = connection->response_write_position + connection->response->fd_off;
      left = connection->response->total_size - connection->response_write_position;
#ifndef HAVE_SENDFILE64
      offset = (off_t) offsetu64;
      if ( (offsetu64 <= (uint64_t) OFF_T_MAX) &&
           (0 < (ret = sendfile (connection->socket_fd, fd, &offset, left))) )
#else  /* HAVE_SENDFILE64 */
      offset = (off64_t) offsetu64;
      if ( (offsetu64 <= (uint64_t) OFF64_T_MAX) &&
	   (0 < (ret = sendfile64 (connection->socket_fd, fd, &offset, left))) )
#endif /* HAVE_SENDFILE64 */
	{
#if EPOLL_SUPPORT
          if (requested_size > (size_t) ret)
	    {
	      /* partial write --- no longer write-ready */
	      connection->epoll_state &= ~MHD_EPOLL_STATE_WRITE_READY;
	    }
#endif
	  return ret;
	}
      err = MHD_socket_errno_;
      if ( (EINTR == err) || (EAGAIN == err) || (EWOULDBLOCK == err) )
	return 0;
      if ( (EINVAL == err) || (EBADF == err) )
	return -1;
      /* None of the 'usual' sendfile errors occurred, so we should try
	 to fall back to 'SEND'; see also this thread for info on
	 odd libc/Linux behavior with sendfile:
	 http://lists.gnu.org/archive/html/libmicrohttpd/2011-02/msg00015.html */
    }
#endif
  ret = (ssize_t)send (connection->socket_fd, other, (_MHD_socket_funcs_size)i, MSG_NOSIGNAL);
#if EPOLL_SUPPORT
  if ( (0 > ret) || (requested_size > (size_t) ret) )
    {
      /* partial write --- no longer write-ready */
      connection->epoll_state &= ~MHD_EPOLL_STATE_WRITE_READY;
    }
#endif
  /* Handle broken kernel / libc, returning -1 but not setting errno;
     kill connection as that should be safe; reported on mailinglist here:
     http://lists.gnu.org/archive/html/libmicrohttpd/2014-10/msg00023.html */
  if ( (0 > ret) && (0 == MHD_socket_errno_) )
    MHD_set_socket_errno_(ECONNRESET);
  return ret;
}


/**
 * Signature of main function for a thread.
 *
 * @param cls closure argument for the function
 * @return termination code from the thread
 */
typedef MHD_THRD_RTRN_TYPE_ (MHD_THRD_CALL_SPEC_ *ThreadStartRoutine)(void *cls);


/**
 * Create a thread and set the attributes according to our options.
 *
 * @param thread handle to initialize
 * @param daemon daemon with options
 * @param start_routine main function of thread
 * @param arg argument for start_routine
 * @return 0 on success
 */
static int
create_thread (MHD_thread_handle_ *thread,
	       const struct MHD_Daemon *daemon,
	       ThreadStartRoutine start_routine,
	       void *arg)
{
#if defined(MHD_USE_POSIX_THREADS)
  pthread_attr_t attr;
  pthread_attr_t *pattr;
  int ret;

  if (0 != daemon->thread_stack_size)
    {
      if (0 != (ret = pthread_attr_init (&attr)))
	goto ERR;
      if (0 != (ret = pthread_attr_setstacksize (&attr, daemon->thread_stack_size)))
	{
	  pthread_attr_destroy (&attr);
	  goto ERR;
	}
      pattr = &attr;
    }
  else
    {
      pattr = NULL;
    }
  ret = pthread_create (thread, pattr,
			start_routine, arg);
#ifdef HAVE_PTHREAD_SETNAME_NP
  (void) pthread_setname_np (*thread, "libmicrohttpd");
#endif /* HAVE_PTHREAD_SETNAME_NP */
  if (0 != daemon->thread_stack_size)
    pthread_attr_destroy (&attr);
  return ret;
 ERR:
#if HAVE_MESSAGES
  MHD_DLOG (daemon,
	    "Failed to set thread stack size\n");
#endif
  errno = EINVAL;
  return ret;
#elif defined(MHD_USE_W32_THREADS)
  unsigned threadID;
  *thread = (HANDLE)_beginthreadex(NULL, (unsigned)daemon->thread_stack_size, start_routine,
                          arg, 0, &threadID);
  if (NULL == (*thread))
    return errno;

  W32_SetThreadName(threadID, "libmicrohttpd");

  return 0;
#endif
}


/**
 * Add another client connection to the set of connections
 * managed by MHD.  This API is usually not needed (since
 * MHD will accept inbound connections on the server socket).
 * Use this API in special cases, for example if your HTTP
 * server is behind NAT and needs to connect out to the
 * HTTP client.
 *
 * The given client socket will be managed (and closed!) by MHD after
 * this call and must no longer be used directly by the application
 * afterwards.
 *
 * Per-IP connection limits are ignored when using this API.
 *
 * @param daemon daemon that manages the connection
 * @param client_socket socket to manage (MHD will expect
 *        to receive an HTTP request from this socket next).
 * @param addr IP address of the client
 * @param addrlen number of bytes in @a addr
 * @param external_add perform additional operations needed due
 *        to the application calling us directly
 * @return #MHD_YES on success, #MHD_NO if this daemon could
 *        not handle the connection (i.e. malloc failed, etc).
 *        The socket will be closed in any case; 'errno' is
 *        set to indicate further details about the error.
 */
static int
internal_add_connection (struct MHD_Daemon *daemon,
			 MHD_socket client_socket,
			 const struct sockaddr *addr,
			 socklen_t addrlen,
			 int external_add)
{
  struct MHD_Connection *connection;
  int res_thread_create;
  unsigned int i;
  int eno;
  struct MHD_Daemon *worker;
#if OSX
  static int on = 1;
#endif

  if (NULL != daemon->worker_pool)
    {
      /* have a pool, try to find a pool with capacity; we use the
	 socket as the initial offset into the pool for load
	 balancing */
      for (i=0;i<daemon->worker_pool_size;i++)
        {
          worker = &daemon->worker_pool[(i + client_socket) % daemon->worker_pool_size];
          if (worker->connections < worker->connection_limit)
            return internal_add_connection (worker,
                                            client_socket,
                                            addr, addrlen,
                                            external_add);
        }
      /* all pools are at their connection limit, must refuse */
      if (0 != MHD_socket_close_ (client_socket))
	MHD_PANIC ("close failed\n");
#if ENFILE
      errno = ENFILE;
#endif
      return MHD_NO;
    }

#ifndef MHD_WINSOCK_SOCKETS
  if ( (client_socket >= FD_SETSIZE) &&
       (0 == (daemon->options & (MHD_USE_POLL | MHD_USE_EPOLL_LINUX_ONLY))) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Socket descriptor larger than FD_SETSIZE: %d > %d\n",
		client_socket,
		FD_SETSIZE);
#endif
      if (0 != MHD_socket_close_ (client_socket))
	MHD_PANIC ("close failed\n");
#if EINVAL
      errno = EINVAL;
#endif
      return MHD_NO;
    }
#endif


#if HAVE_MESSAGES
#if DEBUG_CONNECT
  MHD_DLOG (daemon,
            "Accepted connection on socket %d\n",
            client_socket);
#endif
#endif
  if ( (daemon->connections == daemon->connection_limit) ||
       (MHD_NO == MHD_ip_limit_add (daemon, addr, addrlen)) )
    {
      /* above connection limit - reject */
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Server reached connection limit (closing inbound connection)\n");
#endif
      if (0 != MHD_socket_close_ (client_socket))
	MHD_PANIC ("close failed\n");
#if ENFILE
      errno = ENFILE;
#endif
      return MHD_NO;
    }

  /* apply connection acceptance policy if present */
  if ( (NULL != daemon->apc) &&
       (MHD_NO == daemon->apc (daemon->apc_cls,
			       addr, addrlen)) )
    {
#if DEBUG_CLOSE
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Connection rejected, closing connection\n");
#endif
#endif
      if (0 != MHD_socket_close_ (client_socket))
	MHD_PANIC ("close failed\n");
      MHD_ip_limit_del (daemon, addr, addrlen);
#if EACCESS
      errno = EACCESS;
#endif
      return MHD_NO;
    }

#if OSX
#ifdef SOL_SOCKET
#ifdef SO_NOSIGPIPE
  setsockopt (client_socket,
	      SOL_SOCKET, SO_NOSIGPIPE,
	      &on, sizeof (on));
#endif
#endif
#endif

  if (NULL == (connection = malloc (sizeof (struct MHD_Connection))))
    {
      eno = errno;
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Error allocating memory: %s\n",
		MHD_strerror_ (errno));
#endif
      if (0 != MHD_socket_close_ (client_socket))
	MHD_PANIC ("close failed\n");
      MHD_ip_limit_del (daemon, addr, addrlen);
      errno = eno;
      return MHD_NO;
    }
  memset (connection,
          0,
          sizeof (struct MHD_Connection));
  connection->pool = MHD_pool_create (daemon->pool_size);
  if (NULL == connection->pool)
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Error allocating memory: %s\n",
		MHD_strerror_ (errno));
#endif
      if (0 != MHD_socket_close_ (client_socket))
	MHD_PANIC ("close failed\n");
      MHD_ip_limit_del (daemon, addr, addrlen);
      free (connection);
#if ENOMEM
      errno = ENOMEM;
#endif
      return MHD_NO;
    }

  connection->connection_timeout = daemon->connection_timeout;
  if (NULL == (connection->addr = malloc (addrlen)))
    {
      eno = errno;
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Error allocating memory: %s\n",
		MHD_strerror_ (errno));
#endif
      if (0 != MHD_socket_close_ (client_socket))
	MHD_PANIC ("close failed\n");
      MHD_ip_limit_del (daemon, addr, addrlen);
      MHD_pool_destroy (connection->pool);
      free (connection);
      errno = eno;
      return MHD_NO;
    }
  memcpy (connection->addr, addr, addrlen);
  connection->addr_len = addrlen;
  connection->socket_fd = client_socket;
  connection->daemon = daemon;
  connection->last_activity = MHD_monotonic_sec_counter();

  /* set default connection handlers  */
  MHD_set_http_callbacks_ (connection);
  connection->recv_cls = &recv_param_adapter;
  connection->send_cls = &send_param_adapter;

  if (0 == (connection->daemon->options & MHD_USE_EPOLL_TURBO))
    {
      /* non-blocking sockets are required on most systems and for GNUtls;
	 however, they somehow cause serious problems on CYGWIN (#1824);
	 in turbo mode, we assume that non-blocking was already set
	 by 'accept4' or whoever calls 'MHD_add_connection' */
#ifdef CYGWIN
      if (0 != (daemon->options & MHD_USE_SSL))
#endif
	{
	  /* make socket non-blocking */
#if !defined(MHD_WINSOCK_SOCKETS)
	  int flags = fcntl (connection->socket_fd, F_GETFL);
	  if ( (-1 == flags) ||
	       (0 != fcntl (connection->socket_fd, F_SETFL, flags | O_NONBLOCK)) )
	    {
#if HAVE_MESSAGES
	      MHD_DLOG (daemon,
			"Failed to make socket non-blocking: %s\n",
			MHD_socket_last_strerr_ ());
#endif
	    }
#else
	  unsigned long flags = 1;
	  if (0 != ioctlsocket (connection->socket_fd, FIONBIO, &flags))
	    {
#if HAVE_MESSAGES
	      MHD_DLOG (daemon,
			"Failed to make socket non-blocking: %s\n",
			MHD_socket_last_strerr_ ());
#endif
	    }
#endif
	}
    }

#if HTTPS_SUPPORT
  if (0 != (daemon->options & MHD_USE_SSL))
    {
      connection->recv_cls = &recv_tls_adapter;
      connection->send_cls = &send_tls_adapter;
      connection->state = MHD_TLS_CONNECTION_INIT;
      MHD_set_https_callbacks (connection);
      gnutls_init (&connection->tls_session, GNUTLS_SERVER);
      gnutls_priority_set (connection->tls_session,
			   daemon->priority_cache);
      switch (daemon->cred_type)
        {
          /* set needed credentials for certificate authentication. */
        case GNUTLS_CRD_CERTIFICATE:
          gnutls_credentials_set (connection->tls_session,
				  GNUTLS_CRD_CERTIFICATE,
				  daemon->x509_cred);
          break;
        default:
#if HAVE_MESSAGES
          MHD_DLOG (connection->daemon,
                    "Failed to setup TLS credentials: unknown credential type %d\n",
                    daemon->cred_type);
#endif
          if (0 != MHD_socket_close_ (client_socket))
	    MHD_PANIC ("close failed\n");
          MHD_ip_limit_del (daemon, addr, addrlen);
          free (connection->addr);
          free (connection);
          MHD_PANIC ("Unknown credential type");
#if EINVAL
	  errno = EINVAL;
#endif
 	  return MHD_NO;
        }
      gnutls_transport_set_ptr (connection->tls_session,
				(gnutls_transport_ptr_t) connection);
      gnutls_transport_set_pull_function (connection->tls_session,
					  (gnutls_pull_func) &recv_param_adapter);
      gnutls_transport_set_push_function (connection->tls_session,
					  (gnutls_push_func) &send_param_adapter);

      if (daemon->https_mem_trust)
	  gnutls_certificate_server_set_request (connection->tls_session,
						 GNUTLS_CERT_REQUEST);
    }
#endif

  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_lock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to acquire cleanup mutex\n");
  XDLL_insert (daemon->normal_timeout_head,
	       daemon->normal_timeout_tail,
	       connection);
  DLL_insert (daemon->connections_head,
	      daemon->connections_tail,
	      connection);
  if  ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
	(MHD_YES != MHD_mutex_unlock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to release cleanup mutex\n");

  if (NULL != daemon->notify_connection)
    daemon->notify_connection (daemon->notify_connection_cls,
                               connection,
                               &connection->socket_context,
                               MHD_CONNECTION_NOTIFY_STARTED);

  /* attempt to create handler thread */
  if (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    {
      res_thread_create = create_thread (&connection->pid,
                                         daemon,
					 &MHD_handle_connection,
                                         connection);
      if (0 != res_thread_create)
        {
	  eno = errno;
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Failed to create a thread: %s\n",
                    MHD_strerror_ (res_thread_create));
#endif
	  goto cleanup;
        }
    }
  else
    if ( (MHD_YES == external_add) &&
	 (MHD_INVALID_PIPE_ != daemon->wpipe[1]) &&
	 (1 != MHD_pipe_write_ (daemon->wpipe[1], "n", 1)) )
      {
#if HAVE_MESSAGES
	MHD_DLOG (daemon,
		  "failed to signal new connection via pipe");
#endif
      }
#if EPOLL_SUPPORT
  if (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY))
    {
      if (0 == (daemon->options & MHD_USE_EPOLL_TURBO))
	{
	  struct epoll_event event;

	  event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	  event.data.ptr = connection;
	  if (0 != epoll_ctl (daemon->epoll_fd,
			      EPOLL_CTL_ADD,
			      client_socket,
			      &event))
	    {
	      eno = errno;
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "Call to epoll_ctl failed: %s\n",
                        MHD_socket_last_strerr_ ());
#endif
	      goto cleanup;
	    }
	  connection->epoll_state |= MHD_EPOLL_STATE_IN_EPOLL_SET;
	}
      else
	{
	  connection->epoll_state |= MHD_EPOLL_STATE_READ_READY | MHD_EPOLL_STATE_WRITE_READY
	    | MHD_EPOLL_STATE_IN_EREADY_EDLL;
	  EDLL_insert (daemon->eready_head,
		       daemon->eready_tail,
		       connection);
	}
    }
#endif
  daemon->connections++;
  return MHD_YES;
 cleanup:
  if (NULL != daemon->notify_connection)
    daemon->notify_connection (daemon->notify_connection_cls,
                               connection,
                               &connection->socket_context,
                               MHD_CONNECTION_NOTIFY_CLOSED);
  if (0 != MHD_socket_close_ (client_socket))
    MHD_PANIC ("close failed\n");
  MHD_ip_limit_del (daemon, addr, addrlen);
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_lock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to acquire cleanup mutex\n");
  DLL_remove (daemon->connections_head,
	      daemon->connections_tail,
	      connection);
  XDLL_remove (daemon->normal_timeout_head,
	       daemon->normal_timeout_tail,
	       connection);
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_unlock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to release cleanup mutex\n");
  MHD_pool_destroy (connection->pool);
  free (connection->addr);
  free (connection);
#if EINVAL
  errno = eno;
#endif
  return MHD_NO;
}


/**
 * Suspend handling of network data for a given connection.  This can
 * be used to dequeue a connection from MHD's event loop (external
 * select, internal select or thread pool; not applicable to
 * thread-per-connection!) for a while.
 *
 * If you use this API in conjunction with a internal select or a
 * thread pool, you must set the option #MHD_USE_PIPE_FOR_SHUTDOWN to
 * ensure that a resumed connection is immediately processed by MHD.
 *
 * Suspended connections continue to count against the total number of
 * connections allowed (per daemon, as well as per IP, if such limits
 * are set).  Suspended connections will NOT time out; timeouts will
 * restart when the connection handling is resumed.  While a
 * connection is suspended, MHD will not detect disconnects by the
 * client.
 *
 * The only safe time to suspend a connection is from the
 * #MHD_AccessHandlerCallback.
 *
 * Finally, it is an API violation to call #MHD_stop_daemon while
 * having suspended connections (this will at least create memory and
 * socket leaks or lead to undefined behavior).  You must explicitly
 * resume all connections before stopping the daemon.
 *
 * @param connection the connection to suspend
 */
void
MHD_suspend_connection (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon;

  daemon = connection->daemon;
  if (MHD_USE_SUSPEND_RESUME != (daemon->options & MHD_USE_SUSPEND_RESUME))
    MHD_PANIC ("Cannot suspend connections without enabling MHD_USE_SUSPEND_RESUME!\n");
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_lock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to acquire cleanup mutex\n");
  DLL_remove (daemon->connections_head,
              daemon->connections_tail,
              connection);
  DLL_insert (daemon->suspended_connections_head,
              daemon->suspended_connections_tail,
              connection);
  if (connection->connection_timeout == daemon->connection_timeout)
    XDLL_remove (daemon->normal_timeout_head,
                 daemon->normal_timeout_tail,
                 connection);
  else
    XDLL_remove (daemon->manual_timeout_head,
                 daemon->manual_timeout_tail,
                 connection);
#if EPOLL_SUPPORT
  if (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY))
    {
      if (0 != (connection->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL))
        {
          EDLL_remove (daemon->eready_head,
                       daemon->eready_tail,
                       connection);
          connection->epoll_state &= ~MHD_EPOLL_STATE_IN_EREADY_EDLL;
        }
      if (0 != (connection->epoll_state & MHD_EPOLL_STATE_IN_EPOLL_SET))
        {
          if (0 != epoll_ctl (daemon->epoll_fd,
                              EPOLL_CTL_DEL,
                              connection->socket_fd,
                              NULL))
            MHD_PANIC ("Failed to remove FD from epoll set\n");
          connection->epoll_state &= ~MHD_EPOLL_STATE_IN_EPOLL_SET;
        }
      connection->epoll_state |= MHD_EPOLL_STATE_SUSPENDED;
    }
#endif
  connection->suspended = MHD_YES;
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_unlock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to release cleanup mutex\n");
}


/**
 * Resume handling of network data for suspended connection.  It is
 * safe to resume a suspended connection at any time.  Calling this function
 * on a connection that was not previously suspended will result
 * in undefined behavior.
 *
 * @param connection the connection to resume
 */
void
MHD_resume_connection (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon;

  daemon = connection->daemon;
  if (MHD_USE_SUSPEND_RESUME != (daemon->options & MHD_USE_SUSPEND_RESUME))
    MHD_PANIC ("Cannot resume connections without enabling MHD_USE_SUSPEND_RESUME!\n");
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_lock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to acquire cleanup mutex\n");
  connection->resuming = MHD_YES;
  daemon->resuming = MHD_YES;
  if ( (MHD_INVALID_PIPE_ != daemon->wpipe[1]) &&
       (1 != MHD_pipe_write_ (daemon->wpipe[1], "r", 1)) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "failed to signal resume via pipe");
#endif
    }
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_unlock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to release cleanup mutex\n");
}


/**
 * Run through the suspended connections and move any that are no
 * longer suspended back to the active state.
 *
 * @param daemon daemon context
 * @return #MHD_YES if a connection was actually resumed
 */
static int
resume_suspended_connections (struct MHD_Daemon *daemon)
{
  struct MHD_Connection *pos;
  struct MHD_Connection *next = NULL;
  int ret;

  ret = MHD_NO;
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_lock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to acquire cleanup mutex\n");
  if (MHD_YES == daemon->resuming)
    next = daemon->suspended_connections_head;

  while (NULL != (pos = next))
    {
      next = pos->next;
      if (MHD_NO == pos->resuming)
        continue;
      ret = MHD_YES;
      DLL_remove (daemon->suspended_connections_head,
                  daemon->suspended_connections_tail,
                  pos);
      DLL_insert (daemon->connections_head,
                  daemon->connections_tail,
                  pos);
      if (pos->connection_timeout == daemon->connection_timeout)
        XDLL_insert (daemon->normal_timeout_head,
                     daemon->normal_timeout_tail,
                     pos);
      else
        XDLL_insert (daemon->manual_timeout_head,
                     daemon->manual_timeout_tail,
                     pos);
#if EPOLL_SUPPORT
      if (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY))
        {
          if (0 != (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL))
            MHD_PANIC ("Resumed connection was already in EREADY set\n");
          /* we always mark resumed connections as ready, as we
             might have missed the edge poll event during suspension */
          EDLL_insert (daemon->eready_head,
                       daemon->eready_tail,
                       pos);
          pos->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL;
          pos->epoll_state &= ~MHD_EPOLL_STATE_SUSPENDED;
        }
#endif
      pos->suspended = MHD_NO;
      pos->resuming = MHD_NO;
    }
  daemon->resuming = MHD_NO;
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_unlock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to release cleanup mutex\n");
  return ret;
}


/**
 * Change socket options to be non-blocking, non-inheritable.
 *
 * @param daemon daemon context
 * @param sock socket to manipulate
 */
static void
make_nonblocking_noninheritable (struct MHD_Daemon *daemon,
				 MHD_socket sock)
{
#ifdef MHD_WINSOCK_SOCKETS
  DWORD dwFlags;
  unsigned long flags = 1;

  if (0 != ioctlsocket (sock, FIONBIO, &flags))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Failed to make socket non-blocking: %s\n",
		MHD_socket_last_strerr_ ());
#endif
    }
  if (!GetHandleInformation ((HANDLE) sock, &dwFlags) ||
      ((dwFlags != (dwFlags & ~HANDLE_FLAG_INHERIT)) &&
       !SetHandleInformation ((HANDLE) sock, HANDLE_FLAG_INHERIT, 0)))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Failed to make socket non-inheritable: %u\n",
		(unsigned int) GetLastError ());
#endif
    }
#else
  int flags;
  int nonblock;

  nonblock = O_NONBLOCK;
#ifdef CYGWIN
  if (0 == (daemon->options & MHD_USE_SSL))
    nonblock = 0;
#endif
  flags = fcntl (sock, F_GETFD);
  if ( ( (-1 == flags) ||
	 ( (flags != (flags | FD_CLOEXEC)) &&
	   (0 != fcntl (sock, F_SETFD, flags | nonblock | FD_CLOEXEC)) ) ) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Failed to make socket non-inheritable: %s\n",
		MHD_socket_last_strerr_ ());
#endif
    }
#endif
}


/**
 * Add another client connection to the set of connections managed by
 * MHD.  This API is usually not needed (since MHD will accept inbound
 * connections on the server socket).  Use this API in special cases,
 * for example if your HTTP server is behind NAT and needs to connect
 * out to the HTTP client, or if you are building a proxy.
 *
 * If you use this API in conjunction with a internal select or a
 * thread pool, you must set the option
 * #MHD_USE_PIPE_FOR_SHUTDOWN to ensure that the freshly added
 * connection is immediately processed by MHD.
 *
 * The given client socket will be managed (and closed!) by MHD after
 * this call and must no longer be used directly by the application
 * afterwards.
 *
 * Per-IP connection limits are ignored when using this API.
 *
 * @param daemon daemon that manages the connection
 * @param client_socket socket to manage (MHD will expect
 *        to receive an HTTP request from this socket next).
 * @param addr IP address of the client
 * @param addrlen number of bytes in @a addr
 * @return #MHD_YES on success, #MHD_NO if this daemon could
 *        not handle the connection (i.e. malloc() failed, etc).
 *        The socket will be closed in any case; `errno` is
 *        set to indicate further details about the error.
 * @ingroup specialized
 */
int
MHD_add_connection (struct MHD_Daemon *daemon,
		    MHD_socket client_socket,
		    const struct sockaddr *addr,
		    socklen_t addrlen)
{
  make_nonblocking_noninheritable (daemon,
				   client_socket);
  return internal_add_connection (daemon,
				  client_socket,
				  addr, addrlen,
				  MHD_YES);
}


/**
 * Accept an incoming connection and create the MHD_Connection object for
 * it.  This function also enforces policy by way of checking with the
 * accept policy callback.
 *
 * @param daemon handle with the listen socket
 * @return #MHD_YES on success (connections denied by policy or due
 *         to 'out of memory' and similar errors) are still considered
 *         successful as far as #MHD_accept_connection() is concerned);
 *         a return code of #MHD_NO only refers to the actual
 *         accept() system call.
 */
static int
MHD_accept_connection (struct MHD_Daemon *daemon)
{
#if HAVE_INET6
  struct sockaddr_in6 addrstorage;
#else
  struct sockaddr_in addrstorage;
#endif
  struct sockaddr *addr = (struct sockaddr *) &addrstorage;
  socklen_t addrlen;
  MHD_socket s;
  MHD_socket fd;
  int nonblock;

  addrlen = sizeof (addrstorage);
  memset (addr, 0, sizeof (addrstorage));
  if (MHD_INVALID_SOCKET == (fd = daemon->socket_fd))
    return MHD_NO;
#ifdef HAVE_SOCK_NONBLOCK
  nonblock = SOCK_NONBLOCK;
#else
  nonblock = 0;
#endif
#ifdef CYGWIN
  if (0 == (daemon->options & MHD_USE_SSL))
    nonblock = 0;
#endif
#if HAVE_ACCEPT4
  s = accept4 (fd, addr, &addrlen, SOCK_CLOEXEC | nonblock);
#else
  s = accept (fd, addr, &addrlen);
#endif
  if ((MHD_INVALID_SOCKET == s) || (addrlen <= 0))
    {
#if HAVE_MESSAGES
      const int err = MHD_socket_errno_;
      /* This could be a common occurance with multiple worker threads */
      if ( (EINVAL == err) &&
           (MHD_INVALID_SOCKET == daemon->socket_fd) )
        return MHD_NO; /* can happen during shutdown */
      if ((EAGAIN != err) && (EWOULDBLOCK != err))
        MHD_DLOG (daemon,
		  "Error accepting connection: %s\n",
		  MHD_socket_last_strerr_ ());
#endif
      if (MHD_INVALID_SOCKET != s)
        {
          if (0 != MHD_socket_close_ (s))
	    MHD_PANIC ("close failed\n");
          /* just in case */
        }
      return MHD_NO;
    }
#if !defined(HAVE_ACCEPT4) || HAVE_ACCEPT4+0 == 0 || !defined(HAVE_SOCK_NONBLOCK) || SOCK_CLOEXEC+0 == 0
  make_nonblocking_noninheritable (daemon, s);
#endif
#if HAVE_MESSAGES
#if DEBUG_CONNECT
  MHD_DLOG (daemon,
            "Accepted connection on socket %d\n",
            s);
#endif
#endif
  (void) internal_add_connection (daemon, s,
				  addr, addrlen,
				  MHD_NO);
  return MHD_YES;
}


/**
 * Free resources associated with all closed connections.
 * (destroy responses, free buffers, etc.).  All closed
 * connections are kept in the "cleanup" doubly-linked list.
 *
 * @param daemon daemon to clean up
 */
static void
MHD_cleanup_connections (struct MHD_Daemon *daemon)
{
  struct MHD_Connection *pos;

  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_lock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to acquire cleanup mutex\n");
  while (NULL != (pos = daemon->cleanup_head))
    {
      DLL_remove (daemon->cleanup_head,
		  daemon->cleanup_tail,
		  pos);
      if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
	   (MHD_NO == pos->thread_joined) )
	{
	  if (0 != MHD_join_thread_ (pos->pid))
	    {
	      MHD_PANIC ("Failed to join a thread\n");
	    }
	}
      MHD_pool_destroy (pos->pool);
#if HTTPS_SUPPORT
      if (NULL != pos->tls_session)
	gnutls_deinit (pos->tls_session);
#endif
      daemon->connections--;
      if (NULL != daemon->notify_connection)
        daemon->notify_connection (daemon->notify_connection_cls,
                                   pos,
                                   &pos->socket_context,
                                   MHD_CONNECTION_NOTIFY_CLOSED);
      MHD_ip_limit_del (daemon, pos->addr, pos->addr_len);
#if EPOLL_SUPPORT
      if (0 != (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL))
	{
	  EDLL_remove (daemon->eready_head,
		       daemon->eready_tail,
		       pos);
	  pos->epoll_state &= ~MHD_EPOLL_STATE_IN_EREADY_EDLL;
	}
      if ( (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY)) &&
	   (MHD_INVALID_SOCKET != daemon->epoll_fd) &&
	   (0 != (pos->epoll_state & MHD_EPOLL_STATE_IN_EPOLL_SET)) )
	{
	  /* epoll documentation suggests that closing a FD
	     automatically removes it from the epoll set; however,
	     this is not true as if we fail to do manually remove it,
	     we are still seeing an event for this fd in epoll,
	     causing grief (use-after-free...) --- at least on my
	     system. */
	  if (0 != epoll_ctl (daemon->epoll_fd,
			      EPOLL_CTL_DEL,
			      pos->socket_fd,
			      NULL))
	    MHD_PANIC ("Failed to remove FD from epoll set\n");
	  pos->epoll_state &= ~MHD_EPOLL_STATE_IN_EPOLL_SET;
	}
#endif
      if (NULL != pos->response)
	{
	  MHD_destroy_response (pos->response);
	  pos->response = NULL;
	}
      if (MHD_INVALID_SOCKET != pos->socket_fd)
	{
#ifdef WINDOWS
	  shutdown (pos->socket_fd, SHUT_WR);
#endif
	  if (0 != MHD_socket_close_ (pos->socket_fd))
	    MHD_PANIC ("close failed\n");
	}
      if (NULL != pos->addr)
	free (pos->addr);
      free (pos);
    }
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_unlock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to release cleanup mutex\n");
}


/**
 * Obtain timeout value for `select()` for this daemon (only needed if
 * connection timeout is used).  The returned value is how long
 * `select()` or `poll()` should at most block, not the timeout value set
 * for connections.  This function MUST NOT be called if MHD is
 * running with #MHD_USE_THREAD_PER_CONNECTION.
 *
 * @param daemon daemon to query for timeout
 * @param timeout set to the timeout (in milliseconds)
 * @return #MHD_YES on success, #MHD_NO if timeouts are
 *        not used (or no connections exist that would
 *        necessiate the use of a timeout right now).
 * @ingroup event
 */
int
MHD_get_timeout (struct MHD_Daemon *daemon,
		 MHD_UNSIGNED_LONG_LONG *timeout)
{
  time_t earliest_deadline;
  time_t now;
  struct MHD_Connection *pos;
  int have_timeout;

  if (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Illegal call to MHD_get_timeout\n");
#endif
      return MHD_NO;
    }

#if HTTPS_SUPPORT
  if (0 != daemon->num_tls_read_ready)
    {
      /* if there is any TLS connection with data ready for
	 reading, we must not block in the event loop */
      *timeout = 0;
      return MHD_YES;
    }
#endif

  have_timeout = MHD_NO;
  earliest_deadline = 0; /* avoid compiler warnings */
  for (pos = daemon->manual_timeout_head; NULL != pos; pos = pos->nextX)
    {
      if (0 != pos->connection_timeout)
	{
	  if ( (! have_timeout) ||
	       (earliest_deadline > pos->last_activity + pos->connection_timeout) )
	    earliest_deadline = pos->last_activity + pos->connection_timeout;
#if HTTPS_SUPPORT
	  if (  (0 != (daemon->options & MHD_USE_SSL)) &&
		(0 != gnutls_record_check_pending (pos->tls_session)) )
	    earliest_deadline = 0;
#endif
	  have_timeout = MHD_YES;
	}
    }
  /* normal timeouts are sorted, so we only need to look at the 'head' */
  pos = daemon->normal_timeout_head;
  if ( (NULL != pos) &&
       (0 != pos->connection_timeout) )
    {
      if ( (! have_timeout) ||
	   (earliest_deadline > pos->last_activity + pos->connection_timeout) )
	earliest_deadline = pos->last_activity + pos->connection_timeout;
#if HTTPS_SUPPORT
      if (  (0 != (daemon->options & MHD_USE_SSL)) &&
	    (0 != gnutls_record_check_pending (pos->tls_session)) )
	earliest_deadline = 0;
#endif
      have_timeout = MHD_YES;
    }

  if (MHD_NO == have_timeout)
    return MHD_NO;
  now = MHD_monotonic_sec_counter();
  if (earliest_deadline < now)
    *timeout = 0;
  else
    {
      const time_t second_left = earliest_deadline - now;
      if (second_left > ULLONG_MAX / 1000)
        *timeout = ULLONG_MAX;
      else
        *timeout = 1000 * second_left;
  }
  return MHD_YES;
}


/**
 * Run webserver operations. This method should be called by clients
 * in combination with #MHD_get_fdset if the client-controlled select
 * method is used.
 *
 * You can use this function instead of #MHD_run if you called
 * `select()` on the result from #MHD_get_fdset.  File descriptors in
 * the sets that are not controlled by MHD will be ignored.  Calling
 * this function instead of #MHD_run is more efficient as MHD will
 * not have to call `select()` again to determine which operations are
 * ready.
 *
 * @param daemon daemon to run select loop for
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set (not used, can be NULL)
 * @return #MHD_NO on serious errors, #MHD_YES on success
 * @ingroup event
 */
int
MHD_run_from_select (struct MHD_Daemon *daemon,
		     const fd_set *read_fd_set,
		     const fd_set *write_fd_set,
		     const fd_set *except_fd_set)
{
  MHD_socket ds;
  char tmp;
  struct MHD_Connection *pos;
  struct MHD_Connection *next;
  unsigned int mask = MHD_USE_SUSPEND_RESUME | MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY |
    MHD_USE_SELECT_INTERNALLY | MHD_USE_POLL_INTERNALLY | MHD_USE_THREAD_PER_CONNECTION;

  /* Resuming external connections when using an extern mainloop  */
  if (MHD_USE_SUSPEND_RESUME == (daemon->options & mask))
    resume_suspended_connections (daemon);

#if EPOLL_SUPPORT
  if (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY))
    {
      /* we're in epoll mode, the epoll FD stands for
	 the entire event set! */
      if (daemon->epoll_fd >= FD_SETSIZE)
	return MHD_NO; /* poll fd too big, fail hard */
      if (FD_ISSET (daemon->epoll_fd, read_fd_set))
	return MHD_run (daemon);
      return MHD_YES;
    }
#endif

  /* select connection thread handling type */
  if ( (MHD_INVALID_SOCKET != (ds = daemon->socket_fd)) &&
       (FD_ISSET (ds, read_fd_set)) )
    (void) MHD_accept_connection (daemon);
  /* drain signaling pipe to avoid spinning select */
  if ( (MHD_INVALID_PIPE_ != daemon->wpipe[0]) &&
       (FD_ISSET (daemon->wpipe[0], read_fd_set)) )
    (void) MHD_pipe_read_ (daemon->wpipe[0], &tmp, sizeof (tmp));

  if (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    {
      /* do not have a thread per connection, process all connections now */
      next = daemon->connections_head;
      while (NULL != (pos = next))
        {
	  next = pos->next;
          ds = pos->socket_fd;
          if (MHD_INVALID_SOCKET == ds)
	    continue;
	  switch (pos->event_loop_info)
	    {
	    case MHD_EVENT_LOOP_INFO_READ:
	      if ( (FD_ISSET (ds, read_fd_set))
#if HTTPS_SUPPORT
		   || (MHD_YES == pos->tls_read_ready)
#endif
		   )
		pos->read_handler (pos);
	      break;
	    case MHD_EVENT_LOOP_INFO_WRITE:
	      if ( (FD_ISSET (ds, read_fd_set)) &&
		   (pos->read_buffer_size > pos->read_buffer_offset) )
		pos->read_handler (pos);
	      if (FD_ISSET (ds, write_fd_set))
		pos->write_handler (pos);
	      break;
	    case MHD_EVENT_LOOP_INFO_BLOCK:
	      if ( (FD_ISSET (ds, read_fd_set)) &&
		   (pos->read_buffer_size > pos->read_buffer_offset) )
		pos->read_handler (pos);
	      break;
	    case MHD_EVENT_LOOP_INFO_CLEANUP:
	      /* should never happen */
	      break;
	    }
	  pos->idle_handler (pos);
        }
    }
  MHD_cleanup_connections (daemon);
  return MHD_YES;
}


/**
 * Main internal select() call.  Will compute select sets, call select()
 * and then #MHD_run_from_select with the result.
 *
 * @param daemon daemon to run select() loop for
 * @param may_block #MHD_YES if blocking, #MHD_NO if non-blocking
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static int
MHD_select (struct MHD_Daemon *daemon,
	    int may_block)
{
  int num_ready;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket maxsock;
  struct timeval timeout;
  struct timeval *tv;
  MHD_UNSIGNED_LONG_LONG ltimeout;
  int err_state;

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (MHD_YES == daemon->shutdown)
    return MHD_NO;
  FD_ZERO (&rs);
  FD_ZERO (&ws);
  FD_ZERO (&es);
  maxsock = MHD_INVALID_SOCKET;
  err_state = MHD_NO;
  if (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    {
      if ( (MHD_USE_SUSPEND_RESUME == (daemon->options & MHD_USE_SUSPEND_RESUME)) &&
           (MHD_YES == resume_suspended_connections (daemon)) )
        may_block = MHD_NO;

      /* single-threaded, go over everything */
      if (MHD_NO == MHD_get_fdset2 (daemon, &rs, &ws, &es, &maxsock, FD_SETSIZE))
        {
#if HAVE_MESSAGES
        MHD_DLOG (daemon, "Could not obtain daemon fdsets");
#endif
          err_state = MHD_YES;
        }

      /* If we're at the connection limit, no need to
         accept new connections; however, make sure
         we do not miss the shutdown, so only do this
         optimization if we have a shutdown signaling
         pipe. */
      if ( (MHD_INVALID_SOCKET != daemon->socket_fd) &&
           (daemon->connections == daemon->connection_limit) &&
           (0 != (daemon->options & MHD_USE_PIPE_FOR_SHUTDOWN)) )
        FD_CLR (daemon->socket_fd, &rs);
    }
  else
    {
      /* accept only, have one thread per connection */
      if ( (MHD_INVALID_SOCKET != daemon->socket_fd) &&
           (MHD_YES != add_to_fd_set (daemon->socket_fd,
                                      &rs,
                                      &maxsock,
                                      FD_SETSIZE)) )
        {
#if HAVE_MESSAGES
          MHD_DLOG (daemon, "Could not add listen socket to fdset");
#endif
          return MHD_NO;
        }
    }
  if ( (MHD_INVALID_PIPE_ != daemon->wpipe[0]) &&
       (MHD_YES != add_to_fd_set (daemon->wpipe[0],
                                  &rs,
                                  &maxsock,
                                  FD_SETSIZE)) )
    {
#if defined(MHD_WINSOCK_SOCKETS)
      /* fdset limit reached, new connections
         cannot be handled. Remove listen socket FD
         from fdset and retry to add pipe FD. */
      if (MHD_INVALID_SOCKET != daemon->socket_fd)
        {
          FD_CLR (daemon->socket_fd, &rs);
          if (MHD_YES != add_to_fd_set (daemon->wpipe[0],
                                        &rs,
                                        &maxsock,
                                        FD_SETSIZE))
            {
#endif /* MHD_WINSOCK_SOCKETS */
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "Could not add control pipe FD to fdset");
#endif
              err_state = MHD_YES;
#if defined(MHD_WINSOCK_SOCKETS)
            }
        }
#endif /* MHD_WINSOCK_SOCKETS */
    }

  tv = NULL;
  if (MHD_YES == err_state)
    may_block = MHD_NO;
  if (MHD_NO == may_block)
    {
      timeout.tv_usec = 0;
      timeout.tv_sec = 0;
      tv = &timeout;
    }
  else if ( (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
	    (MHD_YES == MHD_get_timeout (daemon, &ltimeout)) )
    {
      /* ltimeout is in ms */
      timeout.tv_usec = (ltimeout % 1000) * 1000;
      if (ltimeout / 1000 > TIMEVAL_TV_SEC_MAX)
        timeout.tv_sec = TIMEVAL_TV_SEC_MAX;
      else
        timeout.tv_sec = (_MHD_TIMEVAL_TV_SEC_TYPE)(ltimeout / 1000);
      tv = &timeout;
    }
  num_ready = MHD_SYS_select_ (maxsock + 1, &rs, &ws, &es, tv);
  if (MHD_YES == daemon->shutdown)
    return MHD_NO;
  if (num_ready < 0)
    {
      if (EINTR == MHD_socket_errno_)
        return (MHD_NO == err_state) ? MHD_YES : MHD_NO;
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "select failed: %s\n",
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
  if (MHD_YES == MHD_run_from_select (daemon, &rs, &ws, &es))
    return (MHD_NO == err_state) ? MHD_YES : MHD_NO;
  return MHD_NO;
}


#ifdef HAVE_POLL
/**
 * Process all of our connections and possibly the server
 * socket using poll().
 *
 * @param daemon daemon to run poll loop for
 * @param may_block #MHD_YES if blocking, #MHD_NO if non-blocking
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static int
MHD_poll_all (struct MHD_Daemon *daemon,
	      int may_block)
{
  unsigned int num_connections;
  struct MHD_Connection *pos;
  struct MHD_Connection *next;

  if ( (MHD_USE_SUSPEND_RESUME == (daemon->options & MHD_USE_SUSPEND_RESUME)) &&
       (MHD_YES == resume_suspended_connections (daemon)) )
    may_block = MHD_NO;

  /* count number of connections and thus determine poll set size */
  num_connections = 0;
  for (pos = daemon->connections_head; NULL != pos; pos = pos->next)
    num_connections++;
  {
    MHD_UNSIGNED_LONG_LONG ltimeout;
    unsigned int i;
    int timeout;
    unsigned int poll_server;
    int poll_listen;
    int poll_pipe;
    char tmp;
    struct pollfd *p;

    p = malloc(sizeof (struct pollfd) * (2 + num_connections));
    if (NULL == p)
      {
#if HAVE_MESSAGES
        MHD_DLOG(daemon,
                 "Error allocating memory: %s\n",
                 MHD_strerror_(errno));
#endif
        return MHD_NO;
      }
    memset (p, 0, sizeof (struct pollfd) * (2 + num_connections));
    poll_server = 0;
    poll_listen = -1;
    if ( (MHD_INVALID_SOCKET != daemon->socket_fd) &&
	 (daemon->connections < daemon->connection_limit) )
      {
	/* only listen if we are not at the connection limit */
	p[poll_server].fd = daemon->socket_fd;
	p[poll_server].events = POLLIN;
	p[poll_server].revents = 0;
	poll_listen = (int) poll_server;
	poll_server++;
      }
    poll_pipe = -1;
    if (MHD_INVALID_PIPE_ != daemon->wpipe[0])
      {
	p[poll_server].fd = daemon->wpipe[0];
	p[poll_server].events = POLLIN;
	p[poll_server].revents = 0;
        poll_pipe = (int) poll_server;
	poll_server++;
      }
    if (may_block == MHD_NO)
      timeout = 0;
    else if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) ||
	      (MHD_YES != MHD_get_timeout (daemon, &ltimeout)) )
      timeout = -1;
    else
      timeout = (ltimeout > INT_MAX) ? INT_MAX : (int) ltimeout;

    i = 0;
    for (pos = daemon->connections_head; NULL != pos; pos = pos->next)
      {
	p[poll_server+i].fd = pos->socket_fd;
	switch (pos->event_loop_info)
	  {
	  case MHD_EVENT_LOOP_INFO_READ:
	    p[poll_server+i].events |= POLLIN;
	    break;
	  case MHD_EVENT_LOOP_INFO_WRITE:
	    p[poll_server+i].events |= POLLOUT;
	    if (pos->read_buffer_size > pos->read_buffer_offset)
	      p[poll_server+i].events |= POLLIN;
	    break;
	  case MHD_EVENT_LOOP_INFO_BLOCK:
	    if (pos->read_buffer_size > pos->read_buffer_offset)
	      p[poll_server+i].events |= POLLIN;
	    break;
	  case MHD_EVENT_LOOP_INFO_CLEANUP:
	    timeout = 0; /* clean up "pos" immediately */
	    break;
	  }
	i++;
      }
    if (0 == poll_server + num_connections)
      {
        free(p);
        return MHD_YES;
      }
    if (MHD_sys_poll_(p, poll_server + num_connections, timeout) < 0)
      {
	if (EINTR == MHD_socket_errno_)
      {
        free(p);
        return MHD_YES;
      }
#if HAVE_MESSAGES
	MHD_DLOG (daemon,
		  "poll failed: %s\n",
		  MHD_socket_last_strerr_ ());
#endif
        free(p);
	return MHD_NO;
      }
    /* handle shutdown */
    if (MHD_YES == daemon->shutdown)
      {
        free(p);
        return MHD_NO;
      }
    i = 0;
    next = daemon->connections_head;
    while (NULL != (pos = next))
      {
	next = pos->next;
	switch (pos->event_loop_info)
	  {
	  case MHD_EVENT_LOOP_INFO_READ:
	    /* first, sanity checks */
	    if (i >= num_connections)
	      break; /* connection list changed somehow, retry later ... */
	    if (p[poll_server+i].fd != pos->socket_fd)
	      break; /* fd mismatch, something else happened, retry later ... */
	    /* normal handling */
	    if (0 != (p[poll_server+i].revents & POLLIN))
	      pos->read_handler (pos);
	    pos->idle_handler (pos);
	    i++;
	    break;
	  case MHD_EVENT_LOOP_INFO_WRITE:
	    /* first, sanity checks */
	    if (i >= num_connections)
	      break; /* connection list changed somehow, retry later ... */
	    if (p[poll_server+i].fd != pos->socket_fd)
	      break; /* fd mismatch, something else happened, retry later ... */
	    /* normal handling */
	    if (0 != (p[poll_server+i].revents & POLLIN))
	      pos->read_handler (pos);
	    if (0 != (p[poll_server+i].revents & POLLOUT))
	      pos->write_handler (pos);
	    pos->idle_handler (pos);
	    i++;
	    break;
	  case MHD_EVENT_LOOP_INFO_BLOCK:
	    if (0 != (p[poll_server+i].revents & POLLIN))
	      pos->read_handler (pos);
	    pos->idle_handler (pos);
	    break;
	  case MHD_EVENT_LOOP_INFO_CLEANUP:
	    pos->idle_handler (pos);
	    break;
	  }
      }
    /* handle 'listen' FD */
    if ( (-1 != poll_listen) &&
	 (0 != (p[poll_listen].revents & POLLIN)) )
      (void) MHD_accept_connection (daemon);

    /* handle pipe FD */
    if ( (-1 != poll_pipe) &&
         (0 != (p[poll_pipe].revents & POLLIN)) )
      (void) MHD_pipe_read_ (daemon->wpipe[0], &tmp, sizeof (tmp));

    free(p);
  }
  return MHD_YES;
}


/**
 * Process only the listen socket using poll().
 *
 * @param daemon daemon to run poll loop for
 * @param may_block #MHD_YES if blocking, #MHD_NO if non-blocking
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static int
MHD_poll_listen_socket (struct MHD_Daemon *daemon,
			int may_block)
{
  struct pollfd p[2];
  int timeout;
  unsigned int poll_count;
  int poll_listen;

  memset (&p, 0, sizeof (p));
  poll_count = 0;
  poll_listen = -1;
  if (MHD_INVALID_SOCKET != daemon->socket_fd)
    {
      p[poll_count].fd = daemon->socket_fd;
      p[poll_count].events = POLLIN;
      p[poll_count].revents = 0;
      poll_listen = poll_count;
      poll_count++;
    }
  if (MHD_INVALID_PIPE_ != daemon->wpipe[0])
    {
      p[poll_count].fd = daemon->wpipe[0];
      p[poll_count].events = POLLIN;
      p[poll_count].revents = 0;
      poll_count++;
    }
  if (MHD_NO == may_block)
    timeout = 0;
  else
    timeout = -1;
  if (0 == poll_count)
    return MHD_YES;
  if (MHD_sys_poll_(p, poll_count, timeout) < 0)
    {
      if (EINTR == MHD_socket_errno_)
	return MHD_YES;
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "poll failed: %s\n",
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
  /* handle shutdown */
  if (MHD_YES == daemon->shutdown)
    return MHD_NO;
  if ( (-1 != poll_listen) &&
       (0 != (p[poll_listen].revents & POLLIN)) )
    (void) MHD_accept_connection (daemon);
  return MHD_YES;
}
#endif


/**
 * Do poll()-based processing.
 *
 * @param daemon daemon to run poll()-loop for
 * @param may_block #MHD_YES if blocking, #MHD_NO if non-blocking
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static int
MHD_poll (struct MHD_Daemon *daemon,
	  int may_block)
{
#ifdef HAVE_POLL
  if (MHD_YES == daemon->shutdown)
    return MHD_NO;
  if (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    return MHD_poll_all (daemon, may_block);
  else
    return MHD_poll_listen_socket (daemon, may_block);
#else
  return MHD_NO;
#endif
}


#if EPOLL_SUPPORT

/**
 * How many events to we process at most per epoll() call?  Trade-off
 * between required stack-size and number of system calls we have to
 * make; 128 should be way enough to avoid more than one system call
 * for most scenarios, and still be moderate in stack size
 * consumption.  Embedded systems might want to choose a smaller value
 * --- but why use epoll() on such a system in the first place?
 */
#define MAX_EVENTS 128


/**
 * Do epoll()-based processing (this function is allowed to
 * block if @a may_block is set to #MHD_YES).
 *
 * @param daemon daemon to run poll loop for
 * @param may_block #MHD_YES if blocking, #MHD_NO if non-blocking
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static int
MHD_epoll (struct MHD_Daemon *daemon,
	   int may_block)
{
  struct MHD_Connection *pos;
  struct MHD_Connection *next;
  struct epoll_event events[MAX_EVENTS];
  struct epoll_event event;
  int timeout_ms;
  MHD_UNSIGNED_LONG_LONG timeout_ll;
  int num_events;
  unsigned int i;
  unsigned int series_length;
  char tmp;

  if (-1 == daemon->epoll_fd)
    return MHD_NO; /* we're down! */
  if (MHD_YES == daemon->shutdown)
    return MHD_NO;
  if ( (MHD_INVALID_SOCKET != daemon->socket_fd) &&
       (daemon->connections < daemon->connection_limit) &&
       (MHD_NO == daemon->listen_socket_in_epoll) )
    {
      event.events = EPOLLIN;
      event.data.ptr = daemon;
      if (0 != epoll_ctl (daemon->epoll_fd,
			  EPOLL_CTL_ADD,
			  daemon->socket_fd,
			  &event))
	{
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Call to epoll_ctl failed: %s\n",
                    MHD_socket_last_strerr_ ());
#endif
	  return MHD_NO;
	}
      daemon->listen_socket_in_epoll = MHD_YES;
    }
  if ( (MHD_YES == daemon->listen_socket_in_epoll) &&
       (daemon->connections == daemon->connection_limit) )
    {
      /* we're at the connection limit, disable listen socket
	 for event loop for now */
      if (0 != epoll_ctl (daemon->epoll_fd,
			  EPOLL_CTL_DEL,
			  daemon->socket_fd,
			  NULL))
	MHD_PANIC ("Failed to remove listen FD from epoll set\n");
      daemon->listen_socket_in_epoll = MHD_NO;
    }
  if (MHD_YES == may_block)
    {
      if (MHD_YES == MHD_get_timeout (daemon,
				      &timeout_ll))
	{
	  if (timeout_ll >= (MHD_UNSIGNED_LONG_LONG) INT_MAX)
	    timeout_ms = INT_MAX;
	  else
	    timeout_ms = (int) timeout_ll;
	}
      else
	timeout_ms = -1;
    }
  else
    timeout_ms = 0;

  /* drain 'epoll' event queue; need to iterate as we get at most
     MAX_EVENTS in one system call here; in practice this should
     pretty much mean only one round, but better an extra loop here
     than unfair behavior... */
  num_events = MAX_EVENTS;
  while (MAX_EVENTS == num_events)
    {
      /* update event masks */
      num_events = epoll_wait (daemon->epoll_fd,
			       events, MAX_EVENTS, timeout_ms);
      if (-1 == num_events)
	{
	  if (EINTR == MHD_socket_errno_)
	    return MHD_YES;
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Call to epoll_wait failed: %s\n",
                    MHD_socket_last_strerr_ ());
#endif
	  return MHD_NO;
	}
      for (i=0;i<(unsigned int) num_events;i++)
	{
	  if (NULL == events[i].data.ptr)
	    continue; /* shutdown signal! */
          if ( (MHD_INVALID_PIPE_ != daemon->wpipe[0]) &&
               (daemon->wpipe[0] == events[i].data.fd) )
            {
              (void) MHD_pipe_read_ (daemon->wpipe[0], &tmp, sizeof (tmp));
              continue;
            }
	  if (daemon != events[i].data.ptr)
	    {
	      /* this is an event relating to a 'normal' connection,
		 remember the event and if appropriate mark the
		 connection as 'eready'. */
	      pos = events[i].data.ptr;
	      if (0 != (events[i].events & EPOLLIN))
		{
		  pos->epoll_state |= MHD_EPOLL_STATE_READ_READY;
		  if ( ( (MHD_EVENT_LOOP_INFO_READ == pos->event_loop_info) ||
			 (pos->read_buffer_size > pos->read_buffer_offset) ) &&
		       (0 == (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL) ) )
		    {
		      EDLL_insert (daemon->eready_head,
				   daemon->eready_tail,
				   pos);
		      pos->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL;
		    }
		}
	      if (0 != (events[i].events & EPOLLOUT))
		{
		  pos->epoll_state |= MHD_EPOLL_STATE_WRITE_READY;
		  if ( (MHD_EVENT_LOOP_INFO_WRITE == pos->event_loop_info) &&
		       (0 == (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL) ) )
		    {
		      EDLL_insert (daemon->eready_head,
				   daemon->eready_tail,
				   pos);
		      pos->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL;
		    }
		}
	    }
	  else /* must be listen socket */
	    {
	      /* run 'accept' until it fails or we are not allowed to take
		 on more connections */
	      series_length = 0;
	      while ( (MHD_YES == MHD_accept_connection (daemon)) &&
		      (daemon->connections < daemon->connection_limit) &&
		      (series_length < 128) )
                series_length++;
	    }
	}
    }

  /* we handle resumes here because we may have ready connections
     that will not be placed into the epoll list immediately. */
  if ( (MHD_USE_SUSPEND_RESUME == (daemon->options & MHD_USE_SUSPEND_RESUME)) &&
       (MHD_YES == resume_suspended_connections (daemon)) )
    may_block = MHD_NO;

  /* process events for connections */
  while (NULL != (pos = daemon->eready_tail))
    {
      EDLL_remove (daemon->eready_head,
		   daemon->eready_tail,
		   pos);
      pos->epoll_state &= ~MHD_EPOLL_STATE_IN_EREADY_EDLL;
      if (MHD_EVENT_LOOP_INFO_READ == pos->event_loop_info)
	pos->read_handler (pos);
      if (MHD_EVENT_LOOP_INFO_WRITE == pos->event_loop_info)
	pos->write_handler (pos);
      pos->idle_handler (pos);
    }

  /* Finally, handle timed-out connections; we need to do this here
     as the epoll mechanism won't call the 'idle_handler' on everything,
     as the other event loops do.  As timeouts do not get an explicit
     event, we need to find those connections that might have timed out
     here.

     Connections with custom timeouts must all be looked at, as we
     do not bother to sort that (presumably very short) list. */
  next = daemon->manual_timeout_head;
  while (NULL != (pos = next))
    {
      next = pos->nextX;
      pos->idle_handler (pos);
    }
  /* Connections with the default timeout are sorted by prepending
     them to the head of the list whenever we touch the connection;
     thus it sufficies to iterate from the tail until the first
     connection is NOT timed out */
  next = daemon->normal_timeout_tail;
  while (NULL != (pos = next))
    {
      next = pos->prevX;
      pos->idle_handler (pos);
      if (MHD_CONNECTION_CLOSED != pos->state)
	break; /* sorted by timeout, no need to visit the rest! */
    }
  return MHD_YES;
}
#endif


/**
 * Run webserver operations (without blocking unless in client
 * callbacks).  This method should be called by clients in combination
 * with #MHD_get_fdset if the client-controlled select method is used.
 *
 * This function is a convenience method, which is useful if the
 * fd_sets from #MHD_get_fdset were not directly passed to `select()`;
 * with this function, MHD will internally do the appropriate `select()`
 * call itself again.  While it is always safe to call #MHD_run (in
 * external select mode), you should call #MHD_run_from_select if
 * performance is important (as it saves an expensive call to
 * `select()`).
 *
 * @param daemon daemon to run
 * @return #MHD_YES on success, #MHD_NO if this
 *         daemon was not started with the right
 *         options for this call.
 * @ingroup event
 */
int
MHD_run (struct MHD_Daemon *daemon)
{
  if ( (MHD_YES == daemon->shutdown) ||
       (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) ||
       (0 != (daemon->options & MHD_USE_SELECT_INTERNALLY)) )
    return MHD_NO;
  if (0 != (daemon->options & MHD_USE_POLL))
  {
    MHD_poll (daemon, MHD_NO);
    MHD_cleanup_connections (daemon);
  }
#if EPOLL_SUPPORT
  else if (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY))
  {
    MHD_epoll (daemon, MHD_NO);
    MHD_cleanup_connections (daemon);
  }
#endif
  else
  {
    MHD_select (daemon, MHD_NO);
    /* MHD_select does MHD_cleanup_connections already */
  }
  return MHD_YES;
}


/**
 * Thread that runs the select loop until the daemon
 * is explicitly shut down.
 *
 * @param cls 'struct MHD_Deamon' to run select loop in a thread for
 * @return always 0 (on shutdown)
 */
static MHD_THRD_RTRN_TYPE_ MHD_THRD_CALL_SPEC_
MHD_select_thread (void *cls)
{
  struct MHD_Daemon *daemon = cls;

  while (MHD_YES != daemon->shutdown)
    {
      if (0 != (daemon->options & MHD_USE_POLL))
	MHD_poll (daemon, MHD_YES);
#if EPOLL_SUPPORT
      else if (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY))
	MHD_epoll (daemon, MHD_YES);
#endif
      else
	MHD_select (daemon, MHD_YES);
      MHD_cleanup_connections (daemon);
    }
  return (MHD_THRD_RTRN_TYPE_)0;
}


/**
 * Process escape sequences ('%HH') Updates val in place; the
 * result should be UTF-8 encoded and cannot be larger than the input.
 * The result must also still be 0-terminated.
 *
 * @param cls closure (use NULL)
 * @param connection handle to connection, not used
 * @param val value to unescape (modified in the process)
 * @return length of the resulting val (strlen(val) maybe
 *  shorter afterwards due to elimination of escape sequences)
 */
static size_t
unescape_wrapper (void *cls,
                  struct MHD_Connection *connection,
                  char *val)
{
  return MHD_http_unescape (val);
}


/**
 * Start a webserver on the given port.  Variadic version of
 * #MHD_start_daemon_va.
 *
 * @param flags combination of `enum MHD_FLAG` values
 * @param port port to bind to
 * @param apc callback to call to check which clients
 *        will be allowed to connect; you can pass NULL
 *        in which case connections from any IP will be
 *        accepted
 * @param apc_cls extra argument to @a apc
 * @param dh handler called for all requests (repeatedly)
 * @param dh_cls extra argument to @a dh
 * @return NULL on error, handle to daemon on success
 * @ingroup event
 */
struct MHD_Daemon *
MHD_start_daemon (unsigned int flags,
                  uint16_t port,
                  MHD_AcceptPolicyCallback apc,
                  void *apc_cls,
                  MHD_AccessHandlerCallback dh, void *dh_cls, ...)
{
  struct MHD_Daemon *daemon;
  va_list ap;

  va_start (ap, dh_cls);
  daemon = MHD_start_daemon_va (flags, port, apc, apc_cls, dh, dh_cls, ap);
  va_end (ap);
  return daemon;
}


/**
 * Stop accepting connections from the listening socket.  Allows
 * clients to continue processing, but stops accepting new
 * connections.  Note that the caller is responsible for closing the
 * returned socket; however, if MHD is run using threads (anything but
 * external select mode), it must not be closed until AFTER
 * #MHD_stop_daemon has been called (as it is theoretically possible
 * that an existing thread is still using it).
 *
 * Note that some thread modes require the caller to have passed
 * #MHD_USE_PIPE_FOR_SHUTDOWN when using this API.  If this daemon is
 * in one of those modes and this option was not given to
 * #MHD_start_daemon, this function will return #MHD_INVALID_SOCKET.
 *
 * @param daemon daemon to stop accepting new connections for
 * @return old listen socket on success, #MHD_INVALID_SOCKET if
 *         the daemon was already not listening anymore
 * @ingroup specialized
 */
MHD_socket
MHD_quiesce_daemon (struct MHD_Daemon *daemon)
{
  unsigned int i;
  MHD_socket ret;

  ret = daemon->socket_fd;
  if (MHD_INVALID_SOCKET == ret)
    return MHD_INVALID_SOCKET;
  if ( (MHD_INVALID_PIPE_ == daemon->wpipe[1]) &&
       (0 != (daemon->options & (MHD_USE_SELECT_INTERNALLY | MHD_USE_THREAD_PER_CONNECTION))) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Using MHD_quiesce_daemon in this mode requires MHD_USE_PIPE_FOR_SHUTDOWN\n");
#endif
      return MHD_INVALID_SOCKET;
    }

  if (NULL != daemon->worker_pool)
    for (i = 0; i < daemon->worker_pool_size; i++)
      {
	daemon->worker_pool[i].socket_fd = MHD_INVALID_SOCKET;
#if EPOLL_SUPPORT
	if ( (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY)) &&
	     (-1 != daemon->worker_pool[i].epoll_fd) &&
	     (MHD_YES == daemon->worker_pool[i].listen_socket_in_epoll) )
	  {
	    if (0 != epoll_ctl (daemon->worker_pool[i].epoll_fd,
				EPOLL_CTL_DEL,
				ret,
				NULL))
	      MHD_PANIC ("Failed to remove listen FD from epoll set\n");
	    daemon->worker_pool[i].listen_socket_in_epoll = MHD_NO;
	  }
#endif
      }
  daemon->socket_fd = MHD_INVALID_SOCKET;
#if EPOLL_SUPPORT
  if ( (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY)) &&
       (-1 != daemon->epoll_fd) &&
       (MHD_YES == daemon->listen_socket_in_epoll) )
    {
      if (0 != epoll_ctl (daemon->epoll_fd,
			  EPOLL_CTL_DEL,
			  ret,
			  NULL))
	MHD_PANIC ("Failed to remove listen FD from epoll set\n");
      daemon->listen_socket_in_epoll = MHD_NO;
    }
#endif
  return ret;
}


/**
 * Signature of the MHD custom logger function.
 *
 * @param cls closure
 * @param format format string
 * @param va arguments to the format string (fprintf-style)
 */
typedef void (*VfprintfFunctionPointerType)(void *cls,
					    const char *format,
					    va_list va);


/**
 * Parse a list of options given as varargs.
 *
 * @param daemon the daemon to initialize
 * @param servaddr where to store the server's listen address
 * @param ap the options
 * @return #MHD_YES on success, #MHD_NO on error
 */
static int
parse_options_va (struct MHD_Daemon *daemon,
		  const struct sockaddr **servaddr,
		  va_list ap);


/**
 * Parse a list of options given as varargs.
 *
 * @param daemon the daemon to initialize
 * @param servaddr where to store the server's listen address
 * @param ... the options
 * @return #MHD_YES on success, #MHD_NO on error
 */
static int
parse_options (struct MHD_Daemon *daemon,
	       const struct sockaddr **servaddr,
	       ...)
{
  va_list ap;
  int ret;

  va_start (ap, servaddr);
  ret = parse_options_va (daemon, servaddr, ap);
  va_end (ap);
  return ret;
}


/**
 * Parse a list of options given as varargs.
 *
 * @param daemon the daemon to initialize
 * @param servaddr where to store the server's listen address
 * @param ap the options
 * @return #MHD_YES on success, #MHD_NO on error
 */
static int
parse_options_va (struct MHD_Daemon *daemon,
		  const struct sockaddr **servaddr,
		  va_list ap)
{
  enum MHD_OPTION opt;
  struct MHD_OptionItem *oa;
  unsigned int i;
#if HTTPS_SUPPORT
  int ret;
  const char *pstr;
#endif

  while (MHD_OPTION_END != (opt = (enum MHD_OPTION) va_arg (ap, int)))
    {
      switch (opt)
        {
        case MHD_OPTION_CONNECTION_MEMORY_LIMIT:
          daemon->pool_size = va_arg (ap, size_t);
          break;
        case MHD_OPTION_CONNECTION_MEMORY_INCREMENT:
          daemon->pool_increment= va_arg (ap, size_t);
          break;
        case MHD_OPTION_CONNECTION_LIMIT:
          daemon->connection_limit = va_arg (ap, unsigned int);
          break;
        case MHD_OPTION_CONNECTION_TIMEOUT:
          daemon->connection_timeout = va_arg (ap, unsigned int);
          break;
        case MHD_OPTION_NOTIFY_COMPLETED:
          daemon->notify_completed =
            va_arg (ap, MHD_RequestCompletedCallback);
          daemon->notify_completed_cls = va_arg (ap, void *);
          break;
        case MHD_OPTION_NOTIFY_CONNECTION:
          daemon->notify_connection =
            va_arg (ap, MHD_NotifyConnectionCallback);
          daemon->notify_connection_cls = va_arg (ap, void *);
          break;
        case MHD_OPTION_PER_IP_CONNECTION_LIMIT:
          daemon->per_ip_connection_limit = va_arg (ap, unsigned int);
          break;
        case MHD_OPTION_SOCK_ADDR:
          *servaddr = va_arg (ap, const struct sockaddr *);
          break;
        case MHD_OPTION_URI_LOG_CALLBACK:
          daemon->uri_log_callback =
            va_arg (ap, LogCallback);
          daemon->uri_log_callback_cls = va_arg (ap, void *);
          break;
        case MHD_OPTION_THREAD_POOL_SIZE:
          daemon->worker_pool_size = va_arg (ap, unsigned int);
	  if (daemon->worker_pool_size >= (SIZE_MAX / sizeof (struct MHD_Daemon)))
	    {
#if HAVE_MESSAGES
	      MHD_DLOG (daemon,
			"Specified thread pool size (%u) too big\n",
			daemon->worker_pool_size);
#endif
	      return MHD_NO;
	    }
          break;
#if HTTPS_SUPPORT
        case MHD_OPTION_HTTPS_MEM_KEY:
	  if (0 != (daemon->options & MHD_USE_SSL))
	    daemon->https_mem_key = va_arg (ap, const char *);
#if HAVE_MESSAGES
	  else
	    MHD_DLOG (daemon,
		      "MHD HTTPS option %d passed to MHD but MHD_USE_SSL not set\n",
		      opt);
#endif
          break;
        case MHD_OPTION_HTTPS_KEY_PASSWORD:
	  if (0 != (daemon->options & MHD_USE_SSL))
	    daemon->https_key_password = va_arg (ap, const char *);
#if HAVE_MESSAGES
	  else
	    MHD_DLOG (daemon,
		      "MHD HTTPS option %d passed to MHD but MHD_USE_SSL not set\n",
		      opt);
#endif
          break;
        case MHD_OPTION_HTTPS_MEM_CERT:
	  if (0 != (daemon->options & MHD_USE_SSL))
	    daemon->https_mem_cert = va_arg (ap, const char *);
#if HAVE_MESSAGES
	  else
	    MHD_DLOG (daemon,
		      "MHD HTTPS option %d passed to MHD but MHD_USE_SSL not set\n",
		      opt);
#endif
          break;
        case MHD_OPTION_HTTPS_MEM_TRUST:
	  if (0 != (daemon->options & MHD_USE_SSL))
	    daemon->https_mem_trust = va_arg (ap, const char *);
#if HAVE_MESSAGES
	  else
	    MHD_DLOG (daemon,
		      "MHD HTTPS option %d passed to MHD but MHD_USE_SSL not set\n",
		      opt);
#endif
          break;
	case MHD_OPTION_HTTPS_CRED_TYPE:
	  daemon->cred_type = (gnutls_credentials_type_t) va_arg (ap, int);
	  break;
        case MHD_OPTION_HTTPS_MEM_DHPARAMS:
          if (0 != (daemon->options & MHD_USE_SSL))
            {
              const char *arg = va_arg (ap, const char *);
              gnutls_datum_t dhpar;

              if (gnutls_dh_params_init (&daemon->https_mem_dhparams) < 0)
                {
#if HAVE_MESSAGES
                  MHD_DLOG(daemon,
                           "Error initializing DH parameters\n");
#endif
                  return MHD_NO;
                }
              dhpar.data = (unsigned char *) arg;
              dhpar.size = strlen (arg);
              if (gnutls_dh_params_import_pkcs3 (daemon->https_mem_dhparams, &dhpar,
                                                 GNUTLS_X509_FMT_PEM) < 0)
                {
#if HAVE_MESSAGES
                  MHD_DLOG(daemon,
                           "Bad Diffie-Hellman parameters format\n");
#endif
                  gnutls_dh_params_deinit (daemon->https_mem_dhparams);
                  return MHD_NO;
                }
              daemon->have_dhparams = MHD_YES;
            }
          else
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "MHD HTTPS option %d passed to MHD but MHD_USE_SSL not set\n",
                        opt);
#endif
              return MHD_NO;
            }
          break;
        case MHD_OPTION_HTTPS_PRIORITIES:
	  if (0 != (daemon->options & MHD_USE_SSL))
	    {
	      gnutls_priority_deinit (daemon->priority_cache);
	      ret = gnutls_priority_init (&daemon->priority_cache,
					  pstr = va_arg (ap, const char*),
					  NULL);
	      if (GNUTLS_E_SUCCESS != ret)
	      {
#if HAVE_MESSAGES
		MHD_DLOG (daemon,
			  "Setting priorities to `%s' failed: %s\n",
			  pstr,
			  gnutls_strerror (ret));
#endif
		daemon->priority_cache = NULL;
		return MHD_NO;
	      }
	    }
          break;
        case MHD_OPTION_HTTPS_CERT_CALLBACK:
#if GNUTLS_VERSION_MAJOR < 3
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "MHD_OPTION_HTTPS_CERT_CALLBACK requires building MHD with GnuTLS >= 3.0\n");
#endif
          return MHD_NO;
#else
          if (0 != (daemon->options & MHD_USE_SSL))
            daemon->cert_callback = va_arg (ap, gnutls_certificate_retrieve_function2 *);
          break;
#endif
#endif
#ifdef DAUTH_SUPPORT
	case MHD_OPTION_DIGEST_AUTH_RANDOM:
	  daemon->digest_auth_rand_size = va_arg (ap, size_t);
	  daemon->digest_auth_random = va_arg (ap, const char *);
	  break;
	case MHD_OPTION_NONCE_NC_SIZE:
	  daemon->nonce_nc_size = va_arg (ap, unsigned int);
	  break;
#endif
	case MHD_OPTION_LISTEN_SOCKET:
	  daemon->socket_fd = va_arg (ap, MHD_socket);
	  break;
        case MHD_OPTION_EXTERNAL_LOGGER:
#if HAVE_MESSAGES
          daemon->custom_error_log =
            va_arg (ap, VfprintfFunctionPointerType);
          daemon->custom_error_log_cls = va_arg (ap, void *);
#else
          va_arg (ap, VfprintfFunctionPointerType);
          va_arg (ap, void *);
#endif
          break;
        case MHD_OPTION_THREAD_STACK_SIZE:
          daemon->thread_stack_size = va_arg (ap, size_t);
          break;
#ifdef TCP_FASTOPEN
        case MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE:
          daemon->fastopen_queue_size = va_arg (ap, unsigned int);
          break;
#endif
	case MHD_OPTION_LISTENING_ADDRESS_REUSE:
	  daemon->listening_address_reuse = va_arg (ap, unsigned int) ? 1 : -1;
	  break;
	case MHD_OPTION_LISTEN_BACKLOG_SIZE:
	  daemon->listen_backlog_size = va_arg (ap, unsigned int);
	  break;
	case MHD_OPTION_ARRAY:
	  oa = va_arg (ap, struct MHD_OptionItem*);
	  i = 0;
	  while (MHD_OPTION_END != (opt = oa[i].option))
	    {
	      switch (opt)
		{
		  /* all options taking 'size_t' */
		case MHD_OPTION_CONNECTION_MEMORY_LIMIT:
		case MHD_OPTION_CONNECTION_MEMORY_INCREMENT:
		case MHD_OPTION_THREAD_STACK_SIZE:
		  if (MHD_YES != parse_options (daemon,
						servaddr,
						opt,
						(size_t) oa[i].value,
						MHD_OPTION_END))
		    return MHD_NO;
		  break;
		  /* all options taking 'unsigned int' */
		case MHD_OPTION_NONCE_NC_SIZE:
		case MHD_OPTION_CONNECTION_LIMIT:
		case MHD_OPTION_CONNECTION_TIMEOUT:
		case MHD_OPTION_PER_IP_CONNECTION_LIMIT:
		case MHD_OPTION_THREAD_POOL_SIZE:
                case MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE:
		case MHD_OPTION_LISTENING_ADDRESS_REUSE:
		case MHD_OPTION_LISTEN_BACKLOG_SIZE:
		  if (MHD_YES != parse_options (daemon,
						servaddr,
						opt,
						(unsigned int) oa[i].value,
						MHD_OPTION_END))
		    return MHD_NO;
		  break;
		  /* all options taking 'enum' */
		case MHD_OPTION_HTTPS_CRED_TYPE:
		  if (MHD_YES != parse_options (daemon,
						servaddr,
						opt,
						(int) oa[i].value,
						MHD_OPTION_END))
		    return MHD_NO;
		  break;
                  /* all options taking 'MHD_socket' */
                case MHD_OPTION_LISTEN_SOCKET:
                  if (MHD_YES != parse_options (daemon,
                                                servaddr,
                                                opt,
                                                (MHD_socket) oa[i].value,
                                                MHD_OPTION_END))
                    return MHD_NO;
                  break;
		  /* all options taking one pointer */
		case MHD_OPTION_SOCK_ADDR:
		case MHD_OPTION_HTTPS_MEM_KEY:
		case MHD_OPTION_HTTPS_KEY_PASSWORD:
		case MHD_OPTION_HTTPS_MEM_CERT:
		case MHD_OPTION_HTTPS_MEM_TRUST:
	        case MHD_OPTION_HTTPS_MEM_DHPARAMS:
		case MHD_OPTION_HTTPS_PRIORITIES:
		case MHD_OPTION_ARRAY:
                case MHD_OPTION_HTTPS_CERT_CALLBACK:
		  if (MHD_YES != parse_options (daemon,
						servaddr,
						opt,
						oa[i].ptr_value,
						MHD_OPTION_END))
		    return MHD_NO;
		  break;
		  /* all options taking two pointers */
		case MHD_OPTION_NOTIFY_COMPLETED:
		case MHD_OPTION_NOTIFY_CONNECTION:
		case MHD_OPTION_URI_LOG_CALLBACK:
		case MHD_OPTION_EXTERNAL_LOGGER:
		case MHD_OPTION_UNESCAPE_CALLBACK:
		  if (MHD_YES != parse_options (daemon,
						servaddr,
						opt,
						(void *) oa[i].value,
						oa[i].ptr_value,
						MHD_OPTION_END))
		    return MHD_NO;
		  break;
		  /* options taking size_t-number followed by pointer */
		case MHD_OPTION_DIGEST_AUTH_RANDOM:
		  if (MHD_YES != parse_options (daemon,
						servaddr,
						opt,
						(size_t) oa[i].value,
						oa[i].ptr_value,
						MHD_OPTION_END))
		    return MHD_NO;
		  break;
		default:
		  return MHD_NO;
		}
	      i++;
	    }
	  break;
        case MHD_OPTION_UNESCAPE_CALLBACK:
          daemon->unescape_callback =
            va_arg (ap, UnescapeCallback);
          daemon->unescape_callback_cls = va_arg (ap, void *);
          break;
        default:
#if HAVE_MESSAGES
          if (((opt >= MHD_OPTION_HTTPS_MEM_KEY) &&
              (opt <= MHD_OPTION_HTTPS_PRIORITIES)) || (opt == MHD_OPTION_HTTPS_MEM_TRUST))
            {
              MHD_DLOG (daemon,
			"MHD HTTPS option %d passed to MHD compiled without HTTPS support\n",
			opt);
            }
          else
            {
              MHD_DLOG (daemon,
			"Invalid option %d! (Did you terminate the list with MHD_OPTION_END?)\n",
			opt);
            }
#endif
	  return MHD_NO;
        }
    }
  return MHD_YES;
}


/**
 * Create a listen socket, if possible with SOCK_CLOEXEC flag set.
 *
 * @param daemon daemon for which we create the socket
 * @param domain socket domain (i.e. PF_INET)
 * @param type socket type (usually SOCK_STREAM)
 * @param protocol desired protocol, 0 for default
 */
static MHD_socket
create_socket (struct MHD_Daemon *daemon,
	       int domain, int type, int protocol)
{
  int ctype = type | SOCK_CLOEXEC;
  MHD_socket fd;

  /* use SOCK_STREAM rather than ai_socktype: some getaddrinfo
   * implementations do not set ai_socktype, e.g. RHL6.2. */
  fd = socket (domain, ctype, protocol);
  if ( (MHD_INVALID_SOCKET == fd) && (EINVAL == MHD_socket_errno_) && (0 != SOCK_CLOEXEC) )
  {
    ctype = type;
    fd = socket(domain, type, protocol);
  }
  if (MHD_INVALID_SOCKET == fd)
    return MHD_INVALID_SOCKET;
  if (type == ctype)
    make_nonblocking_noninheritable (daemon, fd);
  return fd;
}


#if EPOLL_SUPPORT
/**
 * Setup epoll() FD for the daemon and initialize it to listen
 * on the listen FD.
 *
 * @param daemon daemon to initialize for epoll()
 * @return #MHD_YES on success, #MHD_NO on failure
 */
static int
setup_epoll_to_listen (struct MHD_Daemon *daemon)
{
  struct epoll_event event;

#ifdef HAVE_EPOLL_CREATE1
  daemon->epoll_fd = epoll_create1 (EPOLL_CLOEXEC);
#else  /* !HAVE_EPOLL_CREATE1 */
  daemon->epoll_fd = epoll_create (MAX_EVENTS);
#endif /* !HAVE_EPOLL_CREATE1 */
  if (-1 == daemon->epoll_fd)
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Call to epoll_create1 failed: %s\n",
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
#ifndef HAVE_EPOLL_CREATE1
  else
    {
      int fdflags = fcntl (daemon->epoll_fd, F_GETFD);
      if (0 > fdflags || 0 > fcntl (daemon->epoll_fd, F_SETFD, fdflags | FD_CLOEXEC))
        {
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Failed to change flags on epoll fd: %s\n",
                    MHD_socket_last_strerr_ ());
#endif /* HAVE_MESSAGES */
        }
    }
#endif /* !HAVE_EPOLL_CREATE1 */
  if (0 == EPOLL_CLOEXEC)
    make_nonblocking_noninheritable (daemon,
				     daemon->epoll_fd);
  if (MHD_INVALID_SOCKET == daemon->socket_fd)
    return MHD_YES; /* non-listening daemon */
  event.events = EPOLLIN;
  event.data.ptr = daemon;
  if (0 != epoll_ctl (daemon->epoll_fd,
		      EPOLL_CTL_ADD,
		      daemon->socket_fd,
		      &event))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Call to epoll_ctl failed: %s\n",
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
  if ( (MHD_INVALID_PIPE_ != daemon->wpipe[0]) &&
       (MHD_USE_SUSPEND_RESUME == (daemon->options & MHD_USE_SUSPEND_RESUME)) )
    {
      event.events = EPOLLIN | EPOLLET;
      event.data.ptr = NULL;
      event.data.fd = daemon->wpipe[0];
      if (0 != epoll_ctl (daemon->epoll_fd,
                          EPOLL_CTL_ADD,
                          daemon->wpipe[0],
                          &event))
        {
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Call to epoll_ctl failed: %s\n",
                    MHD_socket_last_strerr_ ());
#endif
          return MHD_NO;
        }
    }
  daemon->listen_socket_in_epoll = MHD_YES;
  return MHD_YES;
}
#endif


/**
 * Start a webserver on the given port.
 *
 * @param flags combination of `enum MHD_FLAG` values
 * @param port port to bind to (in host byte order)
 * @param apc callback to call to check which clients
 *        will be allowed to connect; you can pass NULL
 *        in which case connections from any IP will be
 *        accepted
 * @param apc_cls extra argument to @a apc
 * @param dh handler called for all requests (repeatedly)
 * @param dh_cls extra argument to @a dh
 * @param ap list of options (type-value pairs,
 *        terminated with #MHD_OPTION_END).
 * @return NULL on error, handle to daemon on success
 * @ingroup event
 */
struct MHD_Daemon *
MHD_start_daemon_va (unsigned int flags,
                     uint16_t port,
                     MHD_AcceptPolicyCallback apc,
                     void *apc_cls,
                     MHD_AccessHandlerCallback dh, void *dh_cls,
		     va_list ap)
{
  const _MHD_SOCKOPT_BOOL_TYPE on = 1;
  struct MHD_Daemon *daemon;
  MHD_socket socket_fd;
  struct sockaddr_in servaddr4;
#if HAVE_INET6
  struct sockaddr_in6 servaddr6;
#endif
  const struct sockaddr *servaddr = NULL;
  socklen_t addrlen;
  unsigned int i;
  int res_thread_create;
  int use_pipe;

#ifndef HAVE_INET6
  if (0 != (flags & MHD_USE_IPv6))
    return NULL;
#endif
#ifndef HAVE_POLL
  if (0 != (flags & MHD_USE_POLL))
    return NULL;
#endif
#if ! HTTPS_SUPPORT
  if (0 != (flags & MHD_USE_SSL))
    return NULL;
#endif
#ifndef TCP_FASTOPEN
  if (0 != (flags & MHD_USE_TCP_FASTOPEN))
    return NULL;
#endif
  if (NULL == dh)
    return NULL;
  if (NULL == (daemon = malloc (sizeof (struct MHD_Daemon))))
    return NULL;
  memset (daemon, 0, sizeof (struct MHD_Daemon));
#if EPOLL_SUPPORT
  daemon->epoll_fd = -1;
#endif
  /* try to open listen socket */
#if HTTPS_SUPPORT
  if (0 != (flags & MHD_USE_SSL))
    {
      gnutls_priority_init (&daemon->priority_cache,
			    "NORMAL",
			    NULL);
    }
#endif
  daemon->socket_fd = MHD_INVALID_SOCKET;
  daemon->listening_address_reuse = 0;
  daemon->options = flags;
#if defined(MHD_WINSOCK_SOCKETS) || defined(CYGWIN)
  /* Winsock is broken with respect to 'shutdown';
     this disables us calling 'shutdown' on W32. */
  daemon->options |= MHD_USE_EPOLL_TURBO;
#endif
  daemon->port = port;
  daemon->apc = apc;
  daemon->apc_cls = apc_cls;
  daemon->default_handler = dh;
  daemon->default_handler_cls = dh_cls;
  daemon->connections = 0;
  daemon->connection_limit = MHD_MAX_CONNECTIONS_DEFAULT;
  daemon->pool_size = MHD_POOL_SIZE_DEFAULT;
  daemon->pool_increment = MHD_BUF_INC_SIZE;
  daemon->unescape_callback = &unescape_wrapper;
  daemon->connection_timeout = 0;       /* no timeout */
  daemon->wpipe[0] = MHD_INVALID_PIPE_;
  daemon->wpipe[1] = MHD_INVALID_PIPE_;
#ifdef SOMAXCONN
  daemon->listen_backlog_size = SOMAXCONN;
#else  /* !SOMAXCONN */
  daemon->listen_backlog_size = 511; /* should be safe value */
#endif /* !SOMAXCONN */
#if HAVE_MESSAGES
  daemon->custom_error_log = (MHD_LogCallback) &vfprintf;
  daemon->custom_error_log_cls = stderr;
#endif
#ifdef HAVE_LISTEN_SHUTDOWN
  use_pipe = (0 != (daemon->options & (MHD_USE_NO_LISTEN_SOCKET | MHD_USE_PIPE_FOR_SHUTDOWN)));
#else
  use_pipe = 1; /* yes, must use pipe to signal shutdown */
#endif
  if (0 == (flags & (MHD_USE_SELECT_INTERNALLY | MHD_USE_THREAD_PER_CONNECTION)))
    use_pipe = 0; /* useless if we are using 'external' select */
  if ( (use_pipe) && (0 != MHD_pipe_ (daemon->wpipe)) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Failed to create control pipe: %s\n",
		MHD_strerror_ (errno));
#endif
      free (daemon);
      return NULL;
    }
#ifndef MHD_WINSOCK_SOCKETS
  if ( (0 == (flags & (MHD_USE_POLL | MHD_USE_EPOLL_LINUX_ONLY))) &&
       (1 == use_pipe) &&
       (daemon->wpipe[0] >= FD_SETSIZE) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"file descriptor for control pipe exceeds maximum value\n");
#endif
      if (0 != MHD_pipe_close_ (daemon->wpipe[0]))
	MHD_PANIC ("close failed\n");
      if (0 != MHD_pipe_close_ (daemon->wpipe[1]))
	MHD_PANIC ("close failed\n");
      free (daemon);
      return NULL;
    }
#endif
#ifdef DAUTH_SUPPORT
  daemon->digest_auth_rand_size = 0;
  daemon->digest_auth_random = NULL;
  daemon->nonce_nc_size = 4; /* tiny */
#endif
#if HTTPS_SUPPORT
  if (0 != (flags & MHD_USE_SSL))
    {
      daemon->cred_type = GNUTLS_CRD_CERTIFICATE;
    }
#endif


  if (MHD_YES != parse_options_va (daemon, &servaddr, ap))
    {
#if HTTPS_SUPPORT
      if ( (0 != (flags & MHD_USE_SSL)) &&
	   (NULL != daemon->priority_cache) )
	gnutls_priority_deinit (daemon->priority_cache);
#endif
      free (daemon);
      return NULL;
    }
#ifdef DAUTH_SUPPORT
  if (daemon->nonce_nc_size > 0)
    {
      if ( ( (size_t) (daemon->nonce_nc_size * sizeof (struct MHD_NonceNc))) /
	   sizeof(struct MHD_NonceNc) != daemon->nonce_nc_size)
	{
#if HAVE_MESSAGES
	  MHD_DLOG (daemon,
		    "Specified value for NC_SIZE too large\n");
#endif
#if HTTPS_SUPPORT
	  if (0 != (flags & MHD_USE_SSL))
	    gnutls_priority_deinit (daemon->priority_cache);
#endif
	  free (daemon);
	  return NULL;
	}
      daemon->nnc = malloc (daemon->nonce_nc_size * sizeof (struct MHD_NonceNc));
      if (NULL == daemon->nnc)
	{
#if HAVE_MESSAGES
	  MHD_DLOG (daemon,
		    "Failed to allocate memory for nonce-nc map: %s\n",
		    MHD_strerror_ (errno));
#endif
#if HTTPS_SUPPORT
	  if (0 != (flags & MHD_USE_SSL))
	    gnutls_priority_deinit (daemon->priority_cache);
#endif
	  free (daemon);
	  return NULL;
	}
    }

  if (MHD_YES != MHD_mutex_create_ (&daemon->nnc_lock))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"MHD failed to initialize nonce-nc mutex\n");
#endif
#if HTTPS_SUPPORT
      if (0 != (flags & MHD_USE_SSL))
	gnutls_priority_deinit (daemon->priority_cache);
#endif
      free (daemon->nnc);
      free (daemon);
      return NULL;
    }
#endif

  /* Thread pooling currently works only with internal select thread model */
  if ( (0 == (flags & MHD_USE_SELECT_INTERNALLY)) &&
       (daemon->worker_pool_size > 0) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"MHD thread pooling only works with MHD_USE_SELECT_INTERNALLY\n");
#endif
      goto free_and_fail;
    }

  if ( (MHD_USE_SUSPEND_RESUME == (flags & MHD_USE_SUSPEND_RESUME)) &&
       (0 != (flags & MHD_USE_THREAD_PER_CONNECTION)) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Combining MHD_USE_THREAD_PER_CONNECTION and MHD_USE_SUSPEND_RESUME is not supported.\n");
#endif
      goto free_and_fail;
    }

#ifdef __SYMBIAN32__
  if (0 != (flags & (MHD_USE_SELECT_INTERNALLY | MHD_USE_THREAD_PER_CONNECTION)))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Threaded operations are not supported on Symbian.\n");
#endif
      goto free_and_fail;
    }
#endif
  if ( (MHD_INVALID_SOCKET == daemon->socket_fd) &&
       (0 == (daemon->options & MHD_USE_NO_LISTEN_SOCKET)) )
    {
      /* try to open listen socket */
      if (0 != (flags & MHD_USE_IPv6))
	socket_fd = create_socket (daemon,
				   PF_INET6, SOCK_STREAM, 0);
      else
	socket_fd = create_socket (daemon,
				   PF_INET, SOCK_STREAM, 0);
      if (MHD_INVALID_SOCKET == socket_fd)
	{
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Call to socket failed: %s\n",
                    MHD_socket_last_strerr_ ());
#endif
	  goto free_and_fail;
	}

      /* Apply the socket options according to listening_address_reuse. */
      if (0 == daemon->listening_address_reuse)
        {
          /* No user requirement, use "traditional" default SO_REUSEADDR,
           and do not fail if it doesn't work */
          if (0 > setsockopt (socket_fd,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              (void*)&on, sizeof (on)))
          {
#if HAVE_MESSAGES
            MHD_DLOG (daemon,
                      "setsockopt failed: %s\n",
                      MHD_socket_last_strerr_ ());
#endif
          }
        }
      else if (daemon->listening_address_reuse > 0)
        {
          /* User requested to allow reusing listening address:port.
           * Use SO_REUSEADDR on Windows and SO_REUSEPORT on most platforms.
           * Fail if SO_REUSEPORT does not exist or setsockopt fails.
           */
#ifdef _WIN32
          /* SO_REUSEADDR on W32 has the same semantics
             as SO_REUSEPORT on BSD/Linux */
          if (0 > setsockopt (socket_fd,
                              SOL_SOCKET,
                              SO_REUSEADDR,
                              (void*)&on, sizeof (on)))
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "setsockopt failed: %s\n",
                        MHD_socket_last_strerr_ ());
#endif
              goto free_and_fail;
            }
#else
#ifndef SO_REUSEPORT
#ifdef LINUX
/* Supported since Linux 3.9, but often not present (or commented out)
   in the headers at this time; but 15 is reserved for this and
   thus should be safe to use. */
#define SO_REUSEPORT 15
#endif
#endif
#ifdef SO_REUSEPORT
          if (0 > setsockopt (socket_fd,
                              SOL_SOCKET,
                              SO_REUSEPORT,
                              (void*)&on, sizeof (on)))
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "setsockopt failed: %s\n",
                        MHD_socket_last_strerr_ ());
#endif
              goto free_and_fail;
            }
#else
          /* we're supposed to allow address:port re-use, but
             on this platform we cannot; fail hard */
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Cannot allow listening address reuse: SO_REUSEPORT not defined\n");
#endif
          goto free_and_fail;
#endif
#endif
        }
      else /* if (daemon->listening_address_reuse < 0) */
        {
          /* User requested to disallow reusing listening address:port.
           * Do nothing except for Windows where SO_EXCLUSIVEADDRUSE
           * is used. Fail if it does not exist or setsockopt fails.
           */
#ifdef _WIN32
#ifdef SO_EXCLUSIVEADDRUSE
          if (0 > setsockopt (socket_fd,
                              SOL_SOCKET,
                              SO_EXCLUSIVEADDRUSE,
                              (void*)&on, sizeof (on)))
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "setsockopt failed: %s\n",
                        MHD_socket_last_strerr_ ());
#endif
              goto free_and_fail;
            }
#else /* SO_EXCLUSIVEADDRUSE not defined on W32? */
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Cannot disallow listening address reuse: SO_EXCLUSIVEADDRUSE not defined\n");
#endif
          goto free_and_fail;
#endif
#endif /* _WIN32 */
        }

      /* check for user supplied sockaddr */
#if HAVE_INET6
      if (0 != (flags & MHD_USE_IPv6))
	addrlen = sizeof (struct sockaddr_in6);
      else
#endif
	addrlen = sizeof (struct sockaddr_in);
      if (NULL == servaddr)
	{
#if HAVE_INET6
	  if (0 != (flags & MHD_USE_IPv6))
	    {
	      memset (&servaddr6, 0, sizeof (struct sockaddr_in6));
	      servaddr6.sin6_family = AF_INET6;
	      servaddr6.sin6_port = htons (port);
#if HAVE_SOCKADDR_IN_SIN_LEN
	      servaddr6.sin6_len = sizeof (struct sockaddr_in6);
#endif
	      servaddr = (struct sockaddr *) &servaddr6;
	    }
	  else
#endif
	    {
	      memset (&servaddr4, 0, sizeof (struct sockaddr_in));
	      servaddr4.sin_family = AF_INET;
	      servaddr4.sin_port = htons (port);
#if HAVE_SOCKADDR_IN_SIN_LEN
	      servaddr4.sin_len = sizeof (struct sockaddr_in);
#endif
	      servaddr = (struct sockaddr *) &servaddr4;
	    }
	}
      daemon->socket_fd = socket_fd;

      if (0 != (flags & MHD_USE_IPv6))
	{
#ifdef IPPROTO_IPV6
#ifdef IPV6_V6ONLY
	  /* Note: "IPV6_V6ONLY" is declared by Windows Vista ff., see "IPPROTO_IPV6 Socket Options"
	     (http://msdn.microsoft.com/en-us/library/ms738574%28v=VS.85%29.aspx);
	     and may also be missing on older POSIX systems; good luck if you have any of those,
	     your IPv6 socket may then also bind against IPv4 anyway... */
	  const _MHD_SOCKOPT_BOOL_TYPE v6_only =
            (MHD_USE_DUAL_STACK != (flags & MHD_USE_DUAL_STACK));
	  if (0 > setsockopt (socket_fd,
                              IPPROTO_IPV6, IPV6_V6ONLY,
                              (const void*)&v6_only, sizeof (v6_only)))
      {
#if HAVE_MESSAGES
            MHD_DLOG (daemon,
                      "setsockopt failed: %s\n",
                      MHD_socket_last_strerr_ ());
#endif
      }
#endif
#endif
	}
      if (-1 == bind (socket_fd, servaddr, addrlen))
	{
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Failed to bind to port %u: %s\n",
                    (unsigned int) port,
                    MHD_socket_last_strerr_ ());
#endif
	  if (0 != MHD_socket_close_ (socket_fd))
	    MHD_PANIC ("close failed\n");
	  goto free_and_fail;
	}
#ifdef TCP_FASTOPEN
      if (0 != (flags & MHD_USE_TCP_FASTOPEN))
      {
        if (0 == daemon->fastopen_queue_size)
          daemon->fastopen_queue_size = MHD_TCP_FASTOPEN_QUEUE_SIZE_DEFAULT;
        if (0 != setsockopt (socket_fd,
                             IPPROTO_TCP, TCP_FASTOPEN,
                             &daemon->fastopen_queue_size,
                             sizeof (daemon->fastopen_queue_size)))
        {
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "setsockopt failed: %s\n",
                    MHD_socket_last_strerr_ ());
#endif
        }
      }
#endif
#if EPOLL_SUPPORT
      if (0 != (flags & MHD_USE_EPOLL_LINUX_ONLY))
	{
	  int sk_flags = fcntl (socket_fd, F_GETFL);
	  if (0 != fcntl (socket_fd, F_SETFL, sk_flags | O_NONBLOCK))
	    {
#if HAVE_MESSAGES
	      MHD_DLOG (daemon,
			"Failed to make listen socket non-blocking: %s\n",
			MHD_socket_last_strerr_ ());
#endif
	      if (0 != MHD_socket_close_ (socket_fd))
		MHD_PANIC ("close failed\n");
	      goto free_and_fail;
	    }
	}
#endif
      if (listen (socket_fd, daemon->listen_backlog_size) < 0)
	{
#if HAVE_MESSAGES
          MHD_DLOG (daemon,
                    "Failed to listen for connections: %s\n",
                    MHD_socket_last_strerr_ ());
#endif
	  if (0 != MHD_socket_close_ (socket_fd))
	    MHD_PANIC ("close failed\n");
	  goto free_and_fail;
	}
    }
  else
    {
      socket_fd = daemon->socket_fd;
    }
#ifndef MHD_WINSOCK_SOCKETS
  if ( (socket_fd >= FD_SETSIZE) &&
       (0 == (flags & (MHD_USE_POLL | MHD_USE_EPOLL_LINUX_ONLY)) ) )
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Socket descriptor larger than FD_SETSIZE: %d > %d\n",
                socket_fd,
                FD_SETSIZE);
#endif
      if (0 != MHD_socket_close_ (socket_fd))
	MHD_PANIC ("close failed\n");
      goto free_and_fail;
    }
#endif

#if EPOLL_SUPPORT
  if ( (0 != (flags & MHD_USE_EPOLL_LINUX_ONLY)) &&
       (0 == daemon->worker_pool_size) &&
       (0 == (daemon->options & MHD_USE_NO_LISTEN_SOCKET)) )
    {
      if (0 != (flags & MHD_USE_THREAD_PER_CONNECTION))
	{
#if HAVE_MESSAGES
	  MHD_DLOG (daemon,
		    "Combining MHD_USE_THREAD_PER_CONNECTION and MHD_USE_EPOLL_LINUX_ONLY is not supported.\n");
#endif
	  goto free_and_fail;
	}
      if (MHD_YES != setup_epoll_to_listen (daemon))
	goto free_and_fail;
    }
#else
  if (0 != (flags & MHD_USE_EPOLL_LINUX_ONLY))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"epoll is not supported on this platform by this build.\n");
#endif
      goto free_and_fail;
    }
#endif

  if (MHD_YES != MHD_mutex_create_ (&daemon->per_ip_connection_mutex))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
               "MHD failed to initialize IP connection limit mutex\n");
#endif
      if ( (MHD_INVALID_SOCKET != socket_fd) &&
	   (0 != MHD_socket_close_ (socket_fd)) )
	MHD_PANIC ("close failed\n");
      goto free_and_fail;
    }
  if (MHD_YES != MHD_mutex_create_ (&daemon->cleanup_connection_mutex))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
               "MHD failed to initialize IP connection limit mutex\n");
#endif
      (void) MHD_mutex_destroy_ (&daemon->cleanup_connection_mutex);
      if ( (MHD_INVALID_SOCKET != socket_fd) &&
	   (0 != MHD_socket_close_ (socket_fd)) )
	MHD_PANIC ("close failed\n");
      goto free_and_fail;
    }

#if HTTPS_SUPPORT
  /* initialize HTTPS daemon certificate aspects & send / recv functions */
  if ((0 != (flags & MHD_USE_SSL)) && (0 != MHD_TLS_init (daemon)))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
		"Failed to initialize TLS support\n");
#endif
      if ( (MHD_INVALID_SOCKET != socket_fd) &&
	   (0 != MHD_socket_close_ (socket_fd)) )
	MHD_PANIC ("close failed\n");
      (void) MHD_mutex_destroy_ (&daemon->cleanup_connection_mutex);
      (void) MHD_mutex_destroy_ (&daemon->per_ip_connection_mutex);
      goto free_and_fail;
    }
#endif
  if ( ( (0 != (flags & MHD_USE_THREAD_PER_CONNECTION)) ||
	 ( (0 != (flags & MHD_USE_SELECT_INTERNALLY)) &&
	   (0 == daemon->worker_pool_size)) ) &&
       (0 == (daemon->options & MHD_USE_NO_LISTEN_SOCKET)) &&
       (0 != (res_thread_create =
	      create_thread (&daemon->pid, daemon, &MHD_select_thread, daemon))))
    {
#if HAVE_MESSAGES
      MHD_DLOG (daemon,
                "Failed to create listen thread: %s\n",
		MHD_strerror_ (res_thread_create));
#endif
      (void) MHD_mutex_destroy_ (&daemon->cleanup_connection_mutex);
      (void) MHD_mutex_destroy_ (&daemon->per_ip_connection_mutex);
      if ( (MHD_INVALID_SOCKET != socket_fd) &&
	   (0 != MHD_socket_close_ (socket_fd)) )
	MHD_PANIC ("close failed\n");
      goto free_and_fail;
    }
  if ( (daemon->worker_pool_size > 0) &&
       (0 == (daemon->options & MHD_USE_NO_LISTEN_SOCKET)) )
    {
#if !defined(MHD_WINSOCK_SOCKETS)
      int sk_flags;
#else
      unsigned long sk_flags;
#endif

      /* Coarse-grained count of connections per thread (note error
       * due to integer division). Also keep track of how many
       * connections are leftover after an equal split. */
      unsigned int conns_per_thread = daemon->connection_limit
                                      / daemon->worker_pool_size;
      unsigned int leftover_conns = daemon->connection_limit
                                    % daemon->worker_pool_size;

      i = 0; /* we need this in case fcntl or malloc fails */

      /* Accept must be non-blocking. Multiple children may wake up
       * to handle a new connection, but only one will win the race.
       * The others must immediately return. */
#if !defined(MHD_WINSOCK_SOCKETS)
      sk_flags = fcntl (socket_fd, F_GETFL);
      if (sk_flags < 0)
        goto thread_failed;
      if (0 != fcntl (socket_fd, F_SETFL, sk_flags | O_NONBLOCK))
        goto thread_failed;
#else
      sk_flags = 1;
      if (SOCKET_ERROR == ioctlsocket (socket_fd, FIONBIO, &sk_flags))
        goto thread_failed;
#endif /* MHD_WINSOCK_SOCKETS */

      /* Allocate memory for pooled objects */
      daemon->worker_pool = malloc (sizeof (struct MHD_Daemon)
                                    * daemon->worker_pool_size);
      if (NULL == daemon->worker_pool)
        goto thread_failed;

      /* Start the workers in the pool */
      for (i = 0; i < daemon->worker_pool_size; ++i)
        {
          /* Create copy of the Daemon object for each worker */
          struct MHD_Daemon *d = &daemon->worker_pool[i];

          memcpy (d, daemon, sizeof (struct MHD_Daemon));
          /* Adjust pooling params for worker daemons; note that memcpy()
             has already copied MHD_USE_SELECT_INTERNALLY thread model into
             the worker threads. */
          d->master = daemon;
          d->worker_pool_size = 0;
          d->worker_pool = NULL;

          if ( (MHD_USE_SUSPEND_RESUME == (flags & MHD_USE_SUSPEND_RESUME)) &&
               (0 != MHD_pipe_ (d->wpipe)) )
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "Failed to create worker control pipe: %s\n",
                        MHD_pipe_last_strerror_() );
#endif
              goto thread_failed;
            }
#ifndef MHD_WINSOCK_SOCKETS
          if ( (0 == (flags & (MHD_USE_POLL | MHD_USE_EPOLL_LINUX_ONLY))) &&
               (MHD_USE_SUSPEND_RESUME == (flags & MHD_USE_SUSPEND_RESUME)) &&
               (d->wpipe[0] >= FD_SETSIZE) )
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "File descriptor for worker control pipe exceeds maximum value\n");
#endif
              if (0 != MHD_pipe_close_ (d->wpipe[0]))
                MHD_PANIC ("close failed\n");
              if (0 != MHD_pipe_close_ (d->wpipe[1]))
                MHD_PANIC ("close failed\n");
              goto thread_failed;
            }
#endif

          /* Divide available connections evenly amongst the threads.
           * Thread indexes in [0, leftover_conns) each get one of the
           * leftover connections. */
          d->connection_limit = conns_per_thread;
          if (i < leftover_conns)
            ++d->connection_limit;
#if EPOLL_SUPPORT
	  if ( (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY)) &&
	       (MHD_YES != setup_epoll_to_listen (d)) )
	    goto thread_failed;
#endif
          /* Must init cleanup connection mutex for each worker */
          if (MHD_YES != MHD_mutex_create_ (&d->cleanup_connection_mutex))
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                       "MHD failed to initialize cleanup connection mutex for thread worker %d\n", i);
#endif
              goto thread_failed;
            }

          /* Spawn the worker thread */
          if (0 != (res_thread_create =
		    create_thread (&d->pid, daemon, &MHD_select_thread, d)))
            {
#if HAVE_MESSAGES
              MHD_DLOG (daemon,
                        "Failed to create pool thread: %s\n",
			MHD_strerror_ (res_thread_create));
#endif
              /* Free memory for this worker; cleanup below handles
               * all previously-created workers. */
              (void) MHD_mutex_destroy_ (&d->cleanup_connection_mutex);
              goto thread_failed;
            }
        }
    }
#if HTTPS_SUPPORT
  /* API promises to never use the password after initialization,
     so we additionally NULL it here to not deref a dangling pointer. */
  daemon->https_key_password = NULL;
#endif /* HTTPS_SUPPORT */

  return daemon;

thread_failed:
  /* If no worker threads created, then shut down normally. Calling
     MHD_stop_daemon (as we do below) doesn't work here since it
     assumes a 0-sized thread pool means we had been in the default
     MHD_USE_SELECT_INTERNALLY mode. */
  if (0 == i)
    {
      if ( (MHD_INVALID_SOCKET != socket_fd) &&
	   (0 != MHD_socket_close_ (socket_fd)) )
	MHD_PANIC ("close failed\n");
      (void) MHD_mutex_destroy_ (&daemon->cleanup_connection_mutex);
      (void) MHD_mutex_destroy_ (&daemon->per_ip_connection_mutex);
      if (NULL != daemon->worker_pool)
        free (daemon->worker_pool);
      goto free_and_fail;
    }

  /* Shutdown worker threads we've already created. Pretend
     as though we had fully initialized our daemon, but
     with a smaller number of threads than had been
     requested. */
  daemon->worker_pool_size = i;
  MHD_stop_daemon (daemon);
  return NULL;

 free_and_fail:
  /* clean up basic memory state in 'daemon' and return NULL to
     indicate failure */
#if EPOLL_SUPPORT
  if (-1 != daemon->epoll_fd)
    close (daemon->epoll_fd);
#endif
#ifdef DAUTH_SUPPORT
  free (daemon->nnc);
  (void) MHD_mutex_destroy_ (&daemon->nnc_lock);
#endif
#if HTTPS_SUPPORT
  if (0 != (flags & MHD_USE_SSL))
    gnutls_priority_deinit (daemon->priority_cache);
#endif
  free (daemon);
  return NULL;
}


/**
 * Close the given connection, remove it from all of its
 * DLLs and move it into the cleanup queue.
 *
 * @param pos connection to move to cleanup
 */
static void
close_connection (struct MHD_Connection *pos)
{
  struct MHD_Daemon *daemon = pos->daemon;

  MHD_connection_close_ (pos,
                         MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN);
  if (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    return; /* must let thread to the rest */
  if (pos->connection_timeout == pos->daemon->connection_timeout)
    XDLL_remove (daemon->normal_timeout_head,
		 daemon->normal_timeout_tail,
		 pos);
  else
    XDLL_remove (daemon->manual_timeout_head,
		 daemon->manual_timeout_tail,
		 pos);
  DLL_remove (daemon->connections_head,
	      daemon->connections_tail,
	      pos);
  pos->event_loop_info = MHD_EVENT_LOOP_INFO_CLEANUP;
  DLL_insert (daemon->cleanup_head,
	      daemon->cleanup_tail,
	      pos);
}


/**
 * Close all connections for the daemon; must only be called after
 * all of the threads have been joined and there is no more concurrent
 * activity on the connection lists.
 *
 * @param daemon daemon to close down
 */
static void
close_all_connections (struct MHD_Daemon *daemon)
{
  struct MHD_Connection *pos;

  /* first, make sure all threads are aware of shutdown; need to
     traverse DLLs in peace... */
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_lock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to acquire cleanup mutex\n");
  if (NULL != daemon->suspended_connections_head)
    MHD_PANIC ("MHD_stop_daemon() called while we have suspended connections.\n");
  for (pos = daemon->connections_head; NULL != pos; pos = pos->next)
    {
      shutdown (pos->socket_fd,
                (MHD_YES == pos->read_closed) ? SHUT_WR : SHUT_RDWR);
#if MHD_WINSOCK_SOCKETS
      if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
           (MHD_INVALID_PIPE_ != daemon->wpipe[1]) &&
           (1 != MHD_pipe_write_ (daemon->wpipe[1], "e", 1)) )
        MHD_PANIC ("Failed to signal shutdown via pipe");
#endif
    }
  if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
       (MHD_YES != MHD_mutex_unlock_ (&daemon->cleanup_connection_mutex)) )
    MHD_PANIC ("Failed to release cleanup mutex\n");

  /* now, collect per-connection threads */
  if (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    {
      pos = daemon->connections_head;
      while (NULL != pos)
      {
        if (MHD_YES != pos->thread_joined)
          {
            if (0 != MHD_join_thread_ (pos->pid))
              MHD_PANIC ("Failed to join a thread\n");
            pos->thread_joined = MHD_YES;
            /* The thread may have concurrently modified the DLL,
               need to restart from the beginning */
            pos = daemon->connections_head;
            continue;
          }
        pos = pos->next;
      }
    }
  /* now that we're alone, move everyone to cleanup */
  while (NULL != (pos = daemon->connections_head))
  {
    if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
         (MHD_YES != pos->thread_joined) )
      MHD_PANIC ("Failed to join a thread\n");
    close_connection (pos);
  }
  MHD_cleanup_connections (daemon);
}


#if EPOLL_SUPPORT
/**
 * Shutdown epoll()-event loop by adding 'wpipe' to its event set.
 *
 * @param daemon daemon of which the epoll() instance must be signalled
 */
static void
epoll_shutdown (struct MHD_Daemon *daemon)
{
  struct epoll_event event;

  if (MHD_INVALID_PIPE_ == daemon->wpipe[1])
    {
      /* wpipe was required in this mode, how could this happen? */
      MHD_PANIC ("Internal error\n");
    }
  event.events = EPOLLOUT;
  event.data.ptr = NULL;
  if (0 != epoll_ctl (daemon->epoll_fd,
		      EPOLL_CTL_ADD,
		      daemon->wpipe[1],
		      &event))
    MHD_PANIC ("Failed to add wpipe to epoll set to signal termination\n");
}
#endif


/**
 * Shutdown an HTTP daemon.
 *
 * @param daemon daemon to stop
 * @ingroup event
 */
void
MHD_stop_daemon (struct MHD_Daemon *daemon)
{
  MHD_socket fd;
  unsigned int i;

  if (NULL == daemon)
    return;

  if (0 != (MHD_USE_SUSPEND_RESUME & daemon->options))
    resume_suspended_connections (daemon);
  daemon->shutdown = MHD_YES;
  fd = daemon->socket_fd;
  daemon->socket_fd = MHD_INVALID_SOCKET;
  /* Prepare workers for shutdown */
  if (NULL != daemon->worker_pool)
    {
      /* #MHD_USE_NO_LISTEN_SOCKET disables thread pools, hence we need to check */
      for (i = 0; i < daemon->worker_pool_size; ++i)
	{
	  daemon->worker_pool[i].shutdown = MHD_YES;
	  daemon->worker_pool[i].socket_fd = MHD_INVALID_SOCKET;
#if EPOLL_SUPPORT
	  if ( (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY)) &&
	       (-1 != daemon->worker_pool[i].epoll_fd) &&
	       (MHD_INVALID_SOCKET == fd) )
	    epoll_shutdown (&daemon->worker_pool[i]);
#endif
	}
    }
  if (MHD_INVALID_PIPE_ != daemon->wpipe[1])
    {
      if (1 != MHD_pipe_write_ (daemon->wpipe[1], "e", 1))
	MHD_PANIC ("failed to signal shutdown via pipe");
    }
#ifdef HAVE_LISTEN_SHUTDOWN
  else
    {
      /* fd might be MHD_INVALID_SOCKET here due to 'MHD_quiesce_daemon' */
      if ( (MHD_INVALID_SOCKET != fd) &&
           (0 == (daemon->options & MHD_USE_PIPE_FOR_SHUTDOWN)) )
	(void) shutdown (fd, SHUT_RDWR);
    }
#endif
#if EPOLL_SUPPORT
  if ( (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY)) &&
       (-1 != daemon->epoll_fd) &&
       (MHD_INVALID_SOCKET == fd) )
    epoll_shutdown (daemon);
#endif

#if DEBUG_CLOSE
#if HAVE_MESSAGES
  MHD_DLOG (daemon,
            "MHD listen socket shutdown\n");
#endif
#endif


  /* Signal workers to stop and clean them up */
  if (NULL != daemon->worker_pool)
    {
      /* MHD_USE_NO_LISTEN_SOCKET disables thread pools, hence we need to check */
      for (i = 0; i < daemon->worker_pool_size; ++i)
	{
	  if (MHD_INVALID_PIPE_ != daemon->worker_pool[i].wpipe[1])
	    {
	      if (1 != MHD_pipe_write_ (daemon->worker_pool[i].wpipe[1], "e", 1))
		MHD_PANIC ("failed to signal shutdown via pipe");
	    }
	  if (0 != MHD_join_thread_ (daemon->worker_pool[i].pid))
	      MHD_PANIC ("Failed to join a thread\n");
	  close_all_connections (&daemon->worker_pool[i]);
	  (void) MHD_mutex_destroy_ (&daemon->worker_pool[i].cleanup_connection_mutex);
#if EPOLL_SUPPORT
	  if ( (-1 != daemon->worker_pool[i].epoll_fd) &&
	       (0 != MHD_socket_close_ (daemon->worker_pool[i].epoll_fd)) )
	    MHD_PANIC ("close failed\n");
#endif
          if ( (MHD_USE_SUSPEND_RESUME == (daemon->options & MHD_USE_SUSPEND_RESUME)) )
            {
              if (MHD_INVALID_PIPE_ != daemon->worker_pool[i].wpipe[1])
                {
	           if (0 != MHD_pipe_close_ (daemon->worker_pool[i].wpipe[0]))
	             MHD_PANIC ("close failed\n");
	           if (0 != MHD_pipe_close_ (daemon->worker_pool[i].wpipe[1]))
	             MHD_PANIC ("close failed\n");
                }
	    }
	}
      free (daemon->worker_pool);
    }
  else
    {
      /* clean up master threads */
      if ( (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) ||
           ( (0 != (daemon->options & MHD_USE_SELECT_INTERNALLY)) &&
             (0 == daemon->worker_pool_size) ) )
	{
	  if (0 != MHD_join_thread_ (daemon->pid))
	    {
	      MHD_PANIC ("Failed to join a thread\n");
	    }
	}
    }
  close_all_connections (daemon);
  if ( (MHD_INVALID_SOCKET != fd) &&
       (0 != MHD_socket_close_ (fd)) )
    MHD_PANIC ("close failed\n");

  /* TLS clean up */
#if HTTPS_SUPPORT
  if (MHD_YES == daemon->have_dhparams)
    {
      gnutls_dh_params_deinit (daemon->https_mem_dhparams);
      daemon->have_dhparams = MHD_NO;
    }
  if (0 != (daemon->options & MHD_USE_SSL))
    {
      gnutls_priority_deinit (daemon->priority_cache);
      if (daemon->x509_cred)
        gnutls_certificate_free_credentials (daemon->x509_cred);
    }
#endif
#if EPOLL_SUPPORT
  if ( (0 != (daemon->options & MHD_USE_EPOLL_LINUX_ONLY)) &&
       (-1 != daemon->epoll_fd) &&
       (0 != MHD_socket_close_ (daemon->epoll_fd)) )
    MHD_PANIC ("close failed\n");
#endif

#ifdef DAUTH_SUPPORT
  free (daemon->nnc);
  (void) MHD_mutex_destroy_ (&daemon->nnc_lock);
#endif
  (void) MHD_mutex_destroy_ (&daemon->per_ip_connection_mutex);
  (void) MHD_mutex_destroy_ (&daemon->cleanup_connection_mutex);

  if (MHD_INVALID_PIPE_ != daemon->wpipe[1])
    {
      if (0 != MHD_pipe_close_ (daemon->wpipe[0]))
	MHD_PANIC ("close failed\n");
      if (0 != MHD_pipe_close_ (daemon->wpipe[1]))
	MHD_PANIC ("close failed\n");
    }
  free (daemon);
}


/**
 * Obtain information about the given daemon
 * (not fully implemented!).
 *
 * @param daemon what daemon to get information about
 * @param info_type what information is desired?
 * @param ... depends on @a info_type
 * @return NULL if this information is not available
 *         (or if the @a info_type is unknown)
 * @ingroup specialized
 */
const union MHD_DaemonInfo *
MHD_get_daemon_info (struct MHD_Daemon *daemon,
		     enum MHD_DaemonInfoType info_type,
		     ...)
{
  switch (info_type)
    {
    case MHD_DAEMON_INFO_KEY_SIZE:
      return NULL; /* no longer supported */
    case MHD_DAEMON_INFO_MAC_KEY_SIZE:
      return NULL; /* no longer supported */
    case MHD_DAEMON_INFO_LISTEN_FD:
      return (const union MHD_DaemonInfo *) &daemon->socket_fd;
#if EPOLL_SUPPORT
    case MHD_DAEMON_INFO_EPOLL_FD_LINUX_ONLY:
      return (const union MHD_DaemonInfo *) &daemon->epoll_fd;
#endif
    case MHD_DAEMON_INFO_CURRENT_CONNECTIONS:
      MHD_cleanup_connections (daemon);
      if (daemon->worker_pool)
        {
          /* Collect the connection information stored in the workers. */
          unsigned int i;

          daemon->connections = 0;
          for (i=0;i<daemon->worker_pool_size;i++)
            {
              MHD_cleanup_connections (&daemon->worker_pool[i]);
              daemon->connections += daemon->worker_pool[i].connections;
            }
        }
      return (const union MHD_DaemonInfo *) &daemon->connections;
    default:
      return NULL;
    };
}


/**
 * Sets the global error handler to a different implementation.  @a cb
 * will only be called in the case of typically fatal, serious
 * internal consistency issues.  These issues should only arise in the
 * case of serious memory corruption or similar problems with the
 * architecture.  While @a cb is allowed to return and MHD will then
 * try to continue, this is never safe.
 *
 * The default implementation that is used if no panic function is set
 * simply prints an error message and calls `abort()`.  Alternative
 * implementations might call `exit()` or other similar functions.
 *
 * @param cb new error handler
 * @param cls passed to @a cb
 * @ingroup logging
 */
void
MHD_set_panic_func (MHD_PanicCallback cb,
                    void *cls)
{
  mhd_panic = cb;
  mhd_panic_cls = cls;
}


/**
 * Obtain the version of this library
 *
 * @return static version string, e.g. "0.9.9"
 * @ingroup specialized
 */
const char *
MHD_get_version (void)
{
#ifdef PACKAGE_VERSION
  return PACKAGE_VERSION;
#else  /* !PACKAGE_VERSION */
  static char ver[12] = "\0\0\0\0\0\0\0\0\0\0\0";
  if (0 == ver[0])
  {
    int res = MHD_snprintf_(ver, sizeof(ver), "%x.%x.%x",
                            (((int)MHD_VERSION >> 24) & 0xFF),
                            (((int)MHD_VERSION >> 16) & 0xFF),
                            (((int)MHD_VERSION >> 8) & 0xFF));
    if (0 >= res || sizeof(ver) <= res)
      return "0.0.0"; /* Can't return real version*/
  }
  return ver;
#endif /* !PACKAGE_VERSION */
}


/**
 * Get information about supported MHD features.
 * Indicate that MHD was compiled with or without support for
 * particular feature. Some features require additional support
 * by kernel. Kernel support is not checked by this function.
 *
 * @param feature type of requested information
 * @return #MHD_YES if feature is supported by MHD, #MHD_NO if
 * feature is not supported or feature is unknown.
 * @ingroup specialized
 */
_MHD_EXTERN int
MHD_is_feature_supported(enum MHD_FEATURE feature)
{
  switch(feature)
    {
    case MHD_FEATURE_MESSGES:
#if HAVE_MESSAGES
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_SSL:
#if HTTPS_SUPPORT
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_HTTPS_CERT_CALLBACK:
#if HTTPS_SUPPORT && GNUTLS_VERSION_MAJOR >= 3
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_IPv6:
#ifdef HAVE_INET6
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_IPv6_ONLY:
#if defined(IPPROTO_IPV6) && defined(IPV6_V6ONLY)
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_POLL:
#ifdef HAVE_POLL
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_EPOLL:
#if EPOLL_SUPPORT
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_SHUTDOWN_LISTEN_SOCKET:
#ifdef HAVE_LISTEN_SHUTDOWN
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_SOCKETPAIR:
#ifdef MHD_DONT_USE_PIPES
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_TCP_FASTOPEN:
#ifdef TCP_FASTOPEN
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_BASIC_AUTH:
#if BAUTH_SUPPORT
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_DIGEST_AUTH:
#if DAUTH_SUPPORT
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_POSTPROCESSOR:
#if HAVE_POSTPROCESSOR
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_HTTPS_KEY_PASSWORD:
#if HTTPS_SUPPORT && GNUTLS_VERSION_NUMBER >= 0x030111
      return MHD_YES;
#else
      return MHD_NO;
#endif
    case MHD_FEATURE_LARGE_FILE:
#if defined(HAVE___LSEEKI64) || defined(HAVE_LSEEK64)
      return MHD_YES;
#else
      return (sizeof(uint64_t) > sizeof(off_t)) ? MHD_NO : MHD_YES;
#endif
    }
  return MHD_NO;
}


#if HTTPS_SUPPORT && GCRYPT_VERSION_NUMBER < 0x010600
#if defined(MHD_USE_POSIX_THREADS)
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#elif defined(MHD_W32_MUTEX_)

static int
gcry_w32_mutex_init (void **ppmtx)
{
  *ppmtx = malloc (sizeof (MHD_mutex_));

  if (NULL == *ppmtx)
    return ENOMEM;
  if (MHD_YES != MHD_mutex_create_ ((MHD_mutex_*)*ppmtx))
    {
      free (*ppmtx);
      *ppmtx = NULL;
      return EPERM;
    }

  return 0;
}


static int
gcry_w32_mutex_destroy (void **ppmtx)
{
  int res = (MHD_YES == MHD_mutex_destroy_ ((MHD_mutex_*)*ppmtx)) ? 0 : 1;
  free (*ppmtx);
  return res;
}


static int
gcry_w32_mutex_lock (void **ppmtx)
{
  return (MHD_YES == MHD_mutex_lock_ ((MHD_mutex_*)*ppmtx)) ? 0 : 1;
}


static int
gcry_w32_mutex_unlock (void **ppmtx)
{
  return (MHD_YES == MHD_mutex_unlock_ ((MHD_mutex_*)*ppmtx)) ? 0 : 1;
}


static struct gcry_thread_cbs gcry_threads_w32 = {
  (GCRY_THREAD_OPTION_USER | (GCRY_THREAD_OPTION_VERSION << 8)),
  NULL, gcry_w32_mutex_init, gcry_w32_mutex_destroy,
  gcry_w32_mutex_lock, gcry_w32_mutex_unlock,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

#endif /* defined(MHD_W32_MUTEX_) */
#endif /* HTTPS_SUPPORT && GCRYPT_VERSION_NUMBER < 0x010600 */


/**
 * Initialize do setup work.
 */
void
MHD_init(void)
{
#ifdef _WIN32
  WSADATA wsd;
#endif /* _WIN32 */
  mhd_panic = &mhd_panic_std;
  mhd_panic_cls = NULL;

#ifdef _WIN32
  if (0 != WSAStartup(MAKEWORD(2, 2), &wsd))
    MHD_PANIC ("Failed to initialize winsock\n");
  mhd_winsock_inited_ = 1;
  if (2 != LOBYTE(wsd.wVersion) && 2 != HIBYTE(wsd.wVersion))
    MHD_PANIC ("Winsock version 2.2 is not available\n");
#endif
#if HTTPS_SUPPORT
#if GCRYPT_VERSION_NUMBER < 0x010600
#if defined(MHD_USE_POSIX_THREADS)
  if (0 != gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread))
    MHD_PANIC ("Failed to initialise multithreading in libgcrypt\n");
#elif defined(MHD_W32_MUTEX_)
  if (0 != gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_w32))
    MHD_PANIC ("Failed to initialise multithreading in libgcrypt\n");
#endif /* defined(MHD_W32_MUTEX_) */
  gcry_check_version (NULL);
#else
  if (NULL == gcry_check_version ("1.6.0"))
    MHD_PANIC ("libgcrypt is too old. MHD was compiled for libgcrypt 1.6.0 or newer\n");
#endif
  gnutls_global_init ();
#endif
  MHD_monotonic_sec_counter_init();
}


void
MHD_fini(void)
{
#if HTTPS_SUPPORT
  gnutls_global_deinit ();
#endif
#ifdef _WIN32
  if (mhd_winsock_inited_)
    WSACleanup();
#endif
  MHD_monotonic_sec_counter_finish();
}

_SET_INIT_AND_DEINIT_FUNCS(MHD_init, MHD_fini);

/* end of daemon.c */
