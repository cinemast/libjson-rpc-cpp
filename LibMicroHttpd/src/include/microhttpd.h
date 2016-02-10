/*
     This file is part of libmicrohttpd
     Copyright (C) 2006-2015 Christian Grothoff (and other contributing authors)

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
 * @file microhttpd.h
 * @brief public interface to libmicrohttpd
 * @author Christian Grothoff
 * @author Chris GauthierDickey
 *
 * All symbols defined in this header start with MHD.  MHD is a small
 * HTTP daemon library.  As such, it does not have any API for logging
 * errors (you can only enable or disable logging to stderr).  Also,
 * it may not support all of the HTTP features directly, where
 * applicable, portions of HTTP may have to be handled by clients of
 * the library.
 *
 * The library is supposed to handle everything that it must handle
 * (because the API would not allow clients to do this), such as basic
 * connection management; however, detailed interpretations of headers
 * -- such as range requests -- and HTTP methods are left to clients.
 * The library does understand HEAD and will only send the headers of
 * the response and not the body, even if the client supplied a body.
 * The library also understands headers that control connection
 * management (specifically, "Connection: close" and "Expect: 100
 * continue" are understood and handled automatically).
 *
 * MHD understands POST data and is able to decode certain formats
 * (at the moment only "application/x-www-form-urlencoded" and
 * "mulitpart/formdata"). Unsupported encodings and large POST
 * submissions may require the application to manually process
 * the stream, which is provided to the main application (and thus can be
 * processed, just not conveniently by MHD).
 *
 * The header file defines various constants used by the HTTP protocol.
 * This does not mean that MHD actually interprets all of these
 * values.  The provided constants are exported as a convenience
 * for users of the library.  MHD does not verify that transmitted
 * HTTP headers are part of the standard specification; users of the
 * library are free to define their own extensions of the HTTP
 * standard and use those with MHD.
 *
 * All functions are guaranteed to be completely reentrant and
 * thread-safe (with the exception of #MHD_set_connection_value,
 * which must only be used in a particular context).
 *
 * NEW: Before including "microhttpd.h" you should add the necessary
 * includes to define the `uint64_t`, `size_t`, `fd_set`, `socklen_t`
 * and `struct sockaddr` data types (which headers are needed may
 * depend on your platform; for possible suggestions consult
 * "platform.h" in the MHD distribution).  If you have done so, you
 * should also have a line with "#define MHD_PLATFORM_H" which will
 * prevent this header from trying (and, depending on your platform,
 * failing) to include the right headers.
 *
 * @defgroup event event-loop control
 * MHD API to start and stop the HTTP server and manage the event loop.
 * @defgroup response generation of responses
 * MHD API used to generate responses.
 * @defgroup request handling of requests
 * MHD API used to access information about requests.
 * @defgroup authentication HTTP authentication
 * MHD API related to basic and digest HTTP authentication.
 * @defgroup logging logging
 * MHD API to mange logging and error handling
 * @defgroup specialized misc. specialized functions
 * This group includes functions that do not fit into any particular
 * category and that are rarely used.
 */

#ifndef MHD_MICROHTTPD_H
#define MHD_MICROHTTPD_H

#ifdef __cplusplus
extern "C"
{
#if 0                           /* keep Emacsens' auto-indent happy */
}
#endif
#endif

/* While we generally would like users to use a configure-driven
   build process which detects which headers are present and
   hence works on any platform, we use "standard" includes here
   to build out-of-the-box for beginning users on common systems.

   Once you have a proper build system and go for more exotic
   platforms, you should define MHD_PLATFORM_H in some header that
   you always include *before* "microhttpd.h".  Then the following
   "standard" includes won't be used (which might be a good
   idea, especially on platforms where they do not exist). */
#ifndef MHD_PLATFORM_H
#include <stdarg.h>
#include <stdint.h>
#include <sys/types.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <ws2tcpip.h>
#if defined(_MSC_FULL_VER) && !defined (_SSIZE_T_DEFINED)
#define _SSIZE_T_DEFINED
typedef intptr_t ssize_t;
#endif /* !_SSIZE_T_DEFINED */
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#endif
#endif

#if defined(__CYGWIN__) && !defined(_SYS_TYPES_FD_SET)
/* Do not define __USE_W32_SOCKETS under Cygwin! */
#error Cygwin with winsock fd_set is not supported
#endif

/**
 * Current version of the library.
 * 0x01093001 = 1.9.30-1.
 */
#define MHD_VERSION 0x00094800

/**
 * MHD-internal return code for "YES".
 */
#define MHD_YES 1

/**
 * MHD-internal return code for "NO".
 */
#define MHD_NO 0

/**
 * MHD digest auth internal code for an invalid nonce.
 */
#define MHD_INVALID_NONCE -1

/**
 * Constant used to indicate unknown size (use when
 * creating a response).
 */
#ifdef UINT64_MAX
#define MHD_SIZE_UNKNOWN UINT64_MAX
#else
#define MHD_SIZE_UNKNOWN  ((uint64_t) -1LL)
#endif

#ifdef SIZE_MAX
#define MHD_CONTENT_READER_END_OF_STREAM SIZE_MAX
#define MHD_CONTENT_READER_END_WITH_ERROR (SIZE_MAX - 1)
#else
#define MHD_CONTENT_READER_END_OF_STREAM ((size_t) -1LL)
#define MHD_CONTENT_READER_END_WITH_ERROR (((size_t) -1LL) - 1)
#endif

#ifndef _MHD_EXTERN
#if defined(_WIN32) && defined(MHD_W32LIB)
#define _MHD_EXTERN extern
#elif defined (_WIN32) && defined(MHD_W32DLL)
/* Define MHD_W32DLL when using MHD as W32 .DLL to speed up linker a little */
#define _MHD_EXTERN __declspec(dllimport)
#else
#define _MHD_EXTERN extern
#endif
#endif

#ifndef MHD_SOCKET_DEFINED
/**
 * MHD_socket is type for socket FDs
 */
#if !defined(_WIN32) || defined(_SYS_TYPES_FD_SET)
#define MHD_POSIX_SOCKETS 1
typedef int MHD_socket;
#define MHD_INVALID_SOCKET (-1)
#else /* !defined(_WIN32) || defined(_SYS_TYPES_FD_SET) */
#define MHD_WINSOCK_SOCKETS 1
#include <winsock2.h>
typedef SOCKET MHD_socket;
#define MHD_INVALID_SOCKET (INVALID_SOCKET)
#endif /* !defined(_WIN32) || defined(_SYS_TYPES_FD_SET) */
#define MHD_SOCKET_DEFINED 1
#endif /* MHD_SOCKET_DEFINED */

/**
 * Define MHD_NO_DEPRECATION before including "microhttpd.h" to disable deprecation messages
 */
#ifdef MHD_NO_DEPRECATION
#define _MHD_DEPR_MACRO(msg)
#define _MHD_DEPR_FUNC(msg)
#endif /* MHD_NO_DEPRECATION */

#ifndef _MHD_DEPR_MACRO
#if defined(_MSC_FULL_VER) && _MSC_VER+0 >= 1500
/* VS 2008 or later */
/* Stringify macros */
#define _MHD_INSTRMACRO(a) #a
#define _MHD_STRMACRO(a) _MHD_INSTRMACRO(a)
/* deprecation message */
#define _MHD_DEPR_MACRO(msg) __pragma(message(__FILE__ "(" _MHD_STRMACRO(__LINE__)"): warning: " msg))
#define _MHD_DEPR_IN_MACRO(msg) _MHD_DEPR_MACRO(msg)
#elif defined(__clang__) || defined (__GNUC_PATCHLEVEL__)
/* clang or GCC since 3.0 */
#define _MHD_GCC_PRAG(x) _Pragma (#x)
#if __clang_major__+0 >= 5 || \
  (!defined(__apple_build_version__) && (__clang_major__+0  > 3 || (__clang_major__+0 == 3 && __clang_minor__ >= 3))) || \
  __GNUC__+0 > 4 || (__GNUC__+0 == 4 && __GNUC_MINOR__+0 >= 8)
/* clang >= 3.3 (or XCode's clang >= 5.0) or
   GCC >= 4.8 */
#define _MHD_DEPR_MACRO(msg) _MHD_GCC_PRAG(GCC warning msg)
#define _MHD_DEPR_IN_MACRO(msg) _MHD_DEPR_MACRO(msg)
#else /* older clang or GCC */
/* clang < 3.3, XCode's clang < 5.0, 3.0 <= GCC < 4.8 */
#define _MHD_DEPR_MACRO(msg) _MHD_GCC_PRAG(message msg)
#if (__clang_major__+0  > 2 || (__clang_major__+0 == 2 && __clang_minor__ >= 9)) /* FIXME: clang >= 2.9, earlier versions not tested */
/* clang handles inline pragmas better than GCC */
#define _MHD_DEPR_IN_MACRO(msg) _MHD_DEPR_MACRO(msg)
#endif /* clang >= 2.9 */
#endif
/* #elif defined(SOMEMACRO) */ /* add compiler-specific macros here if required */
#else /* other compilers */
#define _MHD_DEPR_MACRO(msg)
#endif
#endif /* _MHD_DEPR_MACRO */

#ifndef _MHD_DEPR_IN_MACRO
#define _MHD_NO_DEPR_IN_MACRO 1
#define _MHD_DEPR_IN_MACRO(msg)
#endif /* !_MHD_DEPR_IN_MACRO */

#ifndef _MHD_DEPR_FUNC
#if defined(_MSC_FULL_VER) && _MSC_VER+0 >= 1400
/* VS 2005 or later */
#define _MHD_DEPR_FUNC(msg) __declspec(deprecated(msg))
#elif defined(_MSC_FULL_VER) && _MSC_VER+0 >= 1310
/* VS .NET 2003 deprecation do not support custom messages */
#define _MHD_DEPR_FUNC(msg) __declspec(deprecated)
#elif (__GNUC__+0 >= 5) || (defined (__clang__) && \
  (__clang_major__+0 > 2 || (__clang_major__+0 == 2 && __clang_minor__ >= 9)))  /* FIXME: earlier versions not tested */
/* GCC >= 5.0 or clang >= 2.9 */
#define _MHD_DEPR_FUNC(msg) __attribute__((deprecated(msg)))
#elif defined (__clang__) || __GNUC__+0 > 3 || (__GNUC__+0 == 3 && __GNUC_MINOR__+0 >= 1)
/* 3.1 <= GCC < 5.0 or clang < 2.9 */
/* old GCC-style deprecation do not support custom messages */
#define _MHD_DEPR_FUNC(msg) __attribute__((__deprecated__))
/* #elif defined(SOMEMACRO) */ /* add compiler-specific macros here if required */
#else /* other compilers */
#define _MHD_DEPR_FUNC(msg)
#endif
#endif /* _MHD_DEPR_FUNC */

/**
 * Not all architectures and `printf()`'s support the `long long` type.
 * This gives the ability to replace `long long` with just a `long`,
 * standard `int` or a `short`.
 */
#ifndef MHD_LONG_LONG
/**
 * @deprecated use #MHD_UNSIGNED_LONG_LONG instead!
 */
#define MHD_LONG_LONG long long
#define MHD_UNSIGNED_LONG_LONG unsigned long long
#else /* MHD_LONG_LONG */
_MHD_DEPR_MACRO("Macro MHD_LONG_LONG is deprecated, use MHD_UNSIGNED_LONG_LONG")
#endif
/**
 * Format string for printing a variable of type #MHD_LONG_LONG.
 * You should only redefine this if you also define #MHD_LONG_LONG.
 */
#ifndef MHD_LONG_LONG_PRINTF
/**
 * @deprecated use #MHD_UNSIGNED_LONG_LONG_PRINTF instead!
 */
#define MHD_LONG_LONG_PRINTF "ll"
#define MHD_UNSIGNED_LONG_LONG_PRINTF "%llu"
#else /* MHD_LONG_LONG_PRINTF */
_MHD_DEPR_MACRO("Macro MHD_LONG_LONG_PRINTF is deprecated, use MHD_UNSIGNED_LONG_LONG_PRINTF")
#endif


/**
 * @defgroup httpcode HTTP response codes.
 * These are the status codes defined for HTTP responses.
 * @{
 */
#define MHD_HTTP_CONTINUE 100
#define MHD_HTTP_SWITCHING_PROTOCOLS 101
#define MHD_HTTP_PROCESSING 102

#define MHD_HTTP_OK 200
#define MHD_HTTP_CREATED 201
#define MHD_HTTP_ACCEPTED 202
#define MHD_HTTP_NON_AUTHORITATIVE_INFORMATION 203
#define MHD_HTTP_NO_CONTENT 204
#define MHD_HTTP_RESET_CONTENT 205
#define MHD_HTTP_PARTIAL_CONTENT 206
#define MHD_HTTP_MULTI_STATUS 207

#define MHD_HTTP_MULTIPLE_CHOICES 300
#define MHD_HTTP_MOVED_PERMANENTLY 301
#define MHD_HTTP_FOUND 302
#define MHD_HTTP_SEE_OTHER 303
#define MHD_HTTP_NOT_MODIFIED 304
#define MHD_HTTP_USE_PROXY 305
#define MHD_HTTP_SWITCH_PROXY 306
#define MHD_HTTP_TEMPORARY_REDIRECT 307

#define MHD_HTTP_BAD_REQUEST 400
#define MHD_HTTP_UNAUTHORIZED 401
#define MHD_HTTP_PAYMENT_REQUIRED 402
#define MHD_HTTP_FORBIDDEN 403
#define MHD_HTTP_NOT_FOUND 404
#define MHD_HTTP_METHOD_NOT_ALLOWED 405
#define MHD_HTTP_NOT_ACCEPTABLE 406
/** @deprecated */
#define MHD_HTTP_METHOD_NOT_ACCEPTABLE \
  _MHD_DEPR_IN_MACRO("Value MHD_HTTP_METHOD_NOT_ACCEPTABLE is deprecated, use MHD_HTTP_NOT_ACCEPTABLE") 406
#define MHD_HTTP_PROXY_AUTHENTICATION_REQUIRED 407
#define MHD_HTTP_REQUEST_TIMEOUT 408
#define MHD_HTTP_CONFLICT 409
#define MHD_HTTP_GONE 410
#define MHD_HTTP_LENGTH_REQUIRED 411
#define MHD_HTTP_PRECONDITION_FAILED 412
#define MHD_HTTP_REQUEST_ENTITY_TOO_LARGE 413
#define MHD_HTTP_REQUEST_URI_TOO_LONG 414
#define MHD_HTTP_UNSUPPORTED_MEDIA_TYPE 415
#define MHD_HTTP_REQUESTED_RANGE_NOT_SATISFIABLE 416
#define MHD_HTTP_EXPECTATION_FAILED 417
#define MHD_HTTP_UNPROCESSABLE_ENTITY 422
#define MHD_HTTP_LOCKED 423
#define MHD_HTTP_FAILED_DEPENDENCY 424
#define MHD_HTTP_UNORDERED_COLLECTION 425
#define MHD_HTTP_UPGRADE_REQUIRED 426
#define MHD_HTTP_NO_RESPONSE 444
#define MHD_HTTP_RETRY_WITH 449
#define MHD_HTTP_BLOCKED_BY_WINDOWS_PARENTAL_CONTROLS 450
#define MHD_HTTP_UNAVAILABLE_FOR_LEGAL_REASONS 451

#define MHD_HTTP_INTERNAL_SERVER_ERROR 500
#define MHD_HTTP_NOT_IMPLEMENTED 501
#define MHD_HTTP_BAD_GATEWAY 502
#define MHD_HTTP_SERVICE_UNAVAILABLE 503
#define MHD_HTTP_GATEWAY_TIMEOUT 504
#define MHD_HTTP_HTTP_VERSION_NOT_SUPPORTED 505
#define MHD_HTTP_VARIANT_ALSO_NEGOTIATES 506
#define MHD_HTTP_INSUFFICIENT_STORAGE 507
#define MHD_HTTP_BANDWIDTH_LIMIT_EXCEEDED 509
#define MHD_HTTP_NOT_EXTENDED 510

/** @} */ /* end of group httpcode */

/**
 * Returns the string reason phrase for a response code.
 *
 * If we don't have a string for a status code, we give the first
 * message in that status code class.
 */
const char *
MHD_get_reason_phrase_for (unsigned int code);


/**
 * Flag to be or-ed with MHD_HTTP status code for
 * SHOUTcast.  This will cause the response to begin
 * with the SHOUTcast "ICY" line instad of "HTTP".
 * @ingroup specialized
 */
#define MHD_ICY_FLAG ((uint32_t)(((uint32_t)1) << 31))

/**
 * @defgroup headers HTTP headers
 * These are the standard headers found in HTTP requests and responses.
 * @{
 */
/* See also: http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html */
#define MHD_HTTP_HEADER_ACCEPT "Accept"
#define MHD_HTTP_HEADER_ACCEPT_CHARSET "Accept-Charset"
#define MHD_HTTP_HEADER_ACCEPT_ENCODING "Accept-Encoding"
#define MHD_HTTP_HEADER_ACCEPT_LANGUAGE "Accept-Language"
#define MHD_HTTP_HEADER_ACCEPT_RANGES "Accept-Ranges"
#define MHD_HTTP_HEADER_AGE "Age"
#define MHD_HTTP_HEADER_ALLOW "Allow"
#define MHD_HTTP_HEADER_AUTHORIZATION "Authorization"
#define MHD_HTTP_HEADER_CACHE_CONTROL "Cache-Control"
#define MHD_HTTP_HEADER_CONNECTION "Connection"
#define MHD_HTTP_HEADER_CONTENT_ENCODING "Content-Encoding"
#define MHD_HTTP_HEADER_CONTENT_LANGUAGE "Content-Language"
#define MHD_HTTP_HEADER_CONTENT_LENGTH "Content-Length"
#define MHD_HTTP_HEADER_CONTENT_LOCATION "Content-Location"
#define MHD_HTTP_HEADER_CONTENT_MD5 "Content-MD5"
#define MHD_HTTP_HEADER_CONTENT_RANGE "Content-Range"
#define MHD_HTTP_HEADER_CONTENT_TYPE "Content-Type"
#define MHD_HTTP_HEADER_COOKIE "Cookie"
#define MHD_HTTP_HEADER_DATE "Date"
#define MHD_HTTP_HEADER_ETAG "ETag"
#define MHD_HTTP_HEADER_EXPECT "Expect"
#define MHD_HTTP_HEADER_EXPIRES "Expires"
#define MHD_HTTP_HEADER_FROM "From"
#define MHD_HTTP_HEADER_HOST "Host"
#define MHD_HTTP_HEADER_IF_MATCH "If-Match"
#define MHD_HTTP_HEADER_IF_MODIFIED_SINCE "If-Modified-Since"
#define MHD_HTTP_HEADER_IF_NONE_MATCH "If-None-Match"
#define MHD_HTTP_HEADER_IF_RANGE "If-Range"
#define MHD_HTTP_HEADER_IF_UNMODIFIED_SINCE "If-Unmodified-Since"
#define MHD_HTTP_HEADER_LAST_MODIFIED "Last-Modified"
#define MHD_HTTP_HEADER_LOCATION "Location"
#define MHD_HTTP_HEADER_MAX_FORWARDS "Max-Forwards"
#define MHD_HTTP_HEADER_PRAGMA "Pragma"
#define MHD_HTTP_HEADER_PROXY_AUTHENTICATE "Proxy-Authenticate"
#define MHD_HTTP_HEADER_PROXY_AUTHORIZATION "Proxy-Authorization"
#define MHD_HTTP_HEADER_RANGE "Range"
/* This is not a typo, see HTTP spec */
#define MHD_HTTP_HEADER_REFERER "Referer"
#define MHD_HTTP_HEADER_RETRY_AFTER "Retry-After"
#define MHD_HTTP_HEADER_SERVER "Server"
#define MHD_HTTP_HEADER_SET_COOKIE "Set-Cookie"
#define MHD_HTTP_HEADER_SET_COOKIE2 "Set-Cookie2"
#define MHD_HTTP_HEADER_TE "TE"
#define MHD_HTTP_HEADER_TRAILER "Trailer"
#define MHD_HTTP_HEADER_TRANSFER_ENCODING "Transfer-Encoding"
#define MHD_HTTP_HEADER_UPGRADE "Upgrade"
#define MHD_HTTP_HEADER_USER_AGENT "User-Agent"
#define MHD_HTTP_HEADER_VARY "Vary"
#define MHD_HTTP_HEADER_VIA "Via"
#define MHD_HTTP_HEADER_WARNING "Warning"
#define MHD_HTTP_HEADER_WWW_AUTHENTICATE "WWW-Authenticate"
#define MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN "Access-Control-Allow-Origin"
#define MHD_HTTP_HEADER_CONTENT_DISPOSITION "Content-Disposition"

/** @} */ /* end of group headers */

/**
 * @defgroup versions HTTP versions
 * These strings should be used to match against the first line of the
 * HTTP header.
 * @{
 */
#define MHD_HTTP_VERSION_1_0 "HTTP/1.0"
#define MHD_HTTP_VERSION_1_1 "HTTP/1.1"

/** @} */ /* end of group versions */

/**
 * @defgroup methods HTTP methods
 * Standard HTTP methods (as strings).
 * @{
 */
#define MHD_HTTP_METHOD_CONNECT "CONNECT"
#define MHD_HTTP_METHOD_DELETE "DELETE"
#define MHD_HTTP_METHOD_GET "GET"
#define MHD_HTTP_METHOD_HEAD "HEAD"
#define MHD_HTTP_METHOD_OPTIONS "OPTIONS"
#define MHD_HTTP_METHOD_POST "POST"
#define MHD_HTTP_METHOD_PUT "PUT"
#define MHD_HTTP_METHOD_PATCH "PATCH"
#define MHD_HTTP_METHOD_TRACE "TRACE"

/** @} */ /* end of group methods */

/**
 * @defgroup postenc HTTP POST encodings
 * See also: http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4
 * @{
 */
#define MHD_HTTP_POST_ENCODING_FORM_URLENCODED "application/x-www-form-urlencoded"
#define MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA "multipart/form-data"

/** @} */ /* end of group postenc */


/**
 * @brief Handle for the daemon (listening on a socket for HTTP traffic).
 * @ingroup event
 */
struct MHD_Daemon;

/**
 * @brief Handle for a connection / HTTP request.
 *
 * With HTTP/1.1, multiple requests can be run over the same
 * connection.  However, MHD will only show one request per TCP
 * connection to the client at any given time.
 * @ingroup request
 */
struct MHD_Connection;

/**
 * @brief Handle for a response.
 * @ingroup response
 */
struct MHD_Response;

/**
 * @brief Handle for POST processing.
 * @ingroup response
 */
struct MHD_PostProcessor;


/**
 * @brief Flags for the `struct MHD_Daemon`.
 *
 * Note that if neither #MHD_USE_THREAD_PER_CONNECTION nor
 * #MHD_USE_SELECT_INTERNALLY is used, the client wants control over
 * the process and will call the appropriate microhttpd callbacks.
 *
 * Starting the daemon may also fail if a particular option is not
 * implemented or not supported on the target platform (i.e. no
 * support for SSL, threads or IPv6).
 */
enum MHD_FLAG
{
  /**
   * No options selected.
   */
  MHD_NO_FLAG = 0,

  /**
   * Run in debug mode.  If this flag is used, the library should
   * print error messages and warnings to `stderr`.
   */
  MHD_USE_DEBUG = 1,

  /**
   * Run in HTTPS mode.
   */
  MHD_USE_SSL = 2,

  /**
   * Run using one thread per connection.
   */
  MHD_USE_THREAD_PER_CONNECTION = 4,

  /**
   * Run using an internal thread (or thread pool) doing `select()`.
   */
  MHD_USE_SELECT_INTERNALLY = 8,

  /**
   * Run using the IPv6 protocol (otherwise, MHD will just support
   * IPv4).  If you want MHD to support IPv4 and IPv6 using a single
   * socket, pass #MHD_USE_DUAL_STACK, otherwise, if you only pass
   * this option, MHD will try to bind to IPv6-only (resulting in
   * no IPv4 support).
   */
  MHD_USE_IPv6 = 16,

  /**
   * Be pedantic about the protocol (as opposed to as tolerant as
   * possible).  Specifically, at the moment, this flag causes MHD to
   * reject HTTP 1.1 connections without a "Host" header.  This is
   * required by the standard, but of course in violation of the "be
   * as liberal as possible in what you accept" norm.  It is
   * recommended to turn this ON if you are testing clients against
   * MHD, and OFF in production.
   */
  MHD_USE_PEDANTIC_CHECKS = 32,

  /**
   * Use `poll()` instead of `select()`. This allows sockets with `fd >=
   * FD_SETSIZE`.  This option is not compatible with using an
   * 'external' `select()` mode (as there is no API to get the file
   * descriptors for the external select from MHD) and must also not
   * be used in combination with #MHD_USE_EPOLL_LINUX_ONLY.
   */
  MHD_USE_POLL = 64,

  /**
   * Run using an internal thread (or thread pool) doing `poll()`.
   */
  MHD_USE_POLL_INTERNALLY = MHD_USE_SELECT_INTERNALLY | MHD_USE_POLL,

  /**
   * Suppress (automatically) adding the 'Date:' header to HTTP responses.
   * This option should ONLY be used on systems that do not have a clock
   * and that DO provide other mechanisms for cache control.  See also
   * RFC 2616, section 14.18 (exception 3).
   */
  MHD_SUPPRESS_DATE_NO_CLOCK = 128,

  /**
   * Run without a listen socket.  This option only makes sense if
   * #MHD_add_connection is to be used exclusively to connect HTTP
   * clients to the HTTP server.  This option is incompatible with
   * using a thread pool; if it is used, #MHD_OPTION_THREAD_POOL_SIZE
   * is ignored.
   */
  MHD_USE_NO_LISTEN_SOCKET = 256,

  /**
   * Use `epoll()` instead of `select()` or `poll()` for the event loop.
   * This option is only available on Linux; using the option on
   * non-Linux systems will cause #MHD_start_daemon to fail.  Using
   * this option is not supported with MHD_USE_THREAD_PER_CONNECTION.
   */
  MHD_USE_EPOLL_LINUX_ONLY = 512,

  /**
   * Run using an internal thread (or thread pool) doing `epoll()`.
   * This option is only available on Linux; using the option on
   * non-Linux systems will cause #MHD_start_daemon to fail.
   */
  MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY = MHD_USE_SELECT_INTERNALLY | MHD_USE_EPOLL_LINUX_ONLY,

  /**
   * Force MHD to use a signal pipe to notify the event loop (of
   * threads) of our shutdown.  This is required if an appliction uses
   * #MHD_USE_SELECT_INTERNALLY or #MHD_USE_THREAD_PER_CONNECTION and
   * then performs #MHD_quiesce_daemon (which eliminates our ability
   * to signal termination via the listen socket).  In these modes,
   * #MHD_quiesce_daemon will fail if this option was not set.  Also,
   * use of this option is automatic (as in, you do not even have to
   * specify it), if #MHD_USE_NO_LISTEN_SOCKET is specified.  In
   * "external" `select()` mode, this option is always simply ignored.
   * MHD can be build for use a pair of sockets instead of a pipe.
   * Pair of sockets is forced on W32.
   *
   * You must also use this option if you use internal select mode
   * or a thread pool in conjunction with #MHD_add_connection.
   */
  MHD_USE_PIPE_FOR_SHUTDOWN = 1024,

  /**
   * Use a single socket for IPv4 and IPv6.
   */
  MHD_USE_DUAL_STACK = MHD_USE_IPv6 | 2048,

  /**
   * Enable `epoll()` turbo.  Disables certain calls to `shutdown()`
   * and enables aggressive non-blocking optimisitc reads.
   * Most effects only happen with #MHD_USE_EPOLL_LINUX_ONLY.
   * Enalbed always on W32 as winsock does not properly behave
   * with `shutdown()` and this then fixes potential problems.
   */
  MHD_USE_EPOLL_TURBO = 4096,

  /**
   * Enable suspend/resume functions, which also implies setting up
   * pipes to signal resume.
   */
  MHD_USE_SUSPEND_RESUME = 8192 | MHD_USE_PIPE_FOR_SHUTDOWN,

  /**
   * Enable TCP_FASTOPEN option.  This option is only available on Linux with a
   * kernel >= 3.6.  On other systems, using this option cases #MHD_start_daemon
   * to fail.
   */
  MHD_USE_TCP_FASTOPEN = 16384

};


/**
 * Type of a callback function used for logging by MHD.
 *
 * @param cls closure
 * @param fm format string (`printf()`-style)
 * @param ap arguments to @a fm
 * @ingroup logging
 */
typedef void (*MHD_LogCallback)(void *cls, const char *fm, va_list ap);


/**
 * @brief MHD options.
 *
 * Passed in the varargs portion of #MHD_start_daemon.
 */
enum MHD_OPTION
{

  /**
   * No more options / last option.  This is used
   * to terminate the VARARGs list.
   */
  MHD_OPTION_END = 0,

  /**
   * Maximum memory size per connection (followed by a `size_t`).
   * Default is 32 kb (#MHD_POOL_SIZE_DEFAULT).
   * Values above 128k are unlikely to result in much benefit, as half
   * of the memory will be typically used for IO, and TCP buffers are
   * unlikely to support window sizes above 64k on most systems.
   */
  MHD_OPTION_CONNECTION_MEMORY_LIMIT = 1,

  /**
   * Maximum number of concurrent connections to
   * accept (followed by an `unsigned int`).
   */
  MHD_OPTION_CONNECTION_LIMIT = 2,

  /**
   * After how many seconds of inactivity should a
   * connection automatically be timed out? (followed
   * by an `unsigned int`; use zero for no timeout).
   */
  MHD_OPTION_CONNECTION_TIMEOUT = 3,

  /**
   * Register a function that should be called whenever a request has
   * been completed (this can be used for application-specific clean
   * up).  Requests that have never been presented to the application
   * (via #MHD_AccessHandlerCallback) will not result in
   * notifications.
   *
   * This option should be followed by TWO pointers.  First a pointer
   * to a function of type #MHD_RequestCompletedCallback and second a
   * pointer to a closure to pass to the request completed callback.
   * The second pointer maybe NULL.
   */
  MHD_OPTION_NOTIFY_COMPLETED = 4,

  /**
   * Limit on the number of (concurrent) connections made to the
   * server from the same IP address.  Can be used to prevent one
   * IP from taking over all of the allowed connections.  If the
   * same IP tries to establish more than the specified number of
   * connections, they will be immediately rejected.  The option
   * should be followed by an `unsigned int`.  The default is
   * zero, which means no limit on the number of connections
   * from the same IP address.
   */
  MHD_OPTION_PER_IP_CONNECTION_LIMIT = 5,

  /**
   * Bind daemon to the supplied `struct sockaddr`. This option should
   * be followed by a `struct sockaddr *`.  If #MHD_USE_IPv6 is
   * specified, the `struct sockaddr*` should point to a `struct
   * sockaddr_in6`, otherwise to a `struct sockaddr_in`.
   */
  MHD_OPTION_SOCK_ADDR = 6,

  /**
   * Specify a function that should be called before parsing the URI from
   * the client.  The specified callback function can be used for processing
   * the URI (including the options) before it is parsed.  The URI after
   * parsing will no longer contain the options, which maybe inconvenient for
   * logging.  This option should be followed by two arguments, the first
   * one must be of the form
   *
   *     void * my_logger(void *cls, const char *uri, struct MHD_Connection *con)
   *
   * where the return value will be passed as
   * (`* con_cls`) in calls to the #MHD_AccessHandlerCallback
   * when this request is processed later; returning a
   * value of NULL has no special significance (however,
   * note that if you return non-NULL, you can no longer
   * rely on the first call to the access handler having
   * `NULL == *con_cls` on entry;)
   * "cls" will be set to the second argument following
   * #MHD_OPTION_URI_LOG_CALLBACK.  Finally, uri will
   * be the 0-terminated URI of the request.
   *
   * Note that during the time of this call, most of the connection's
   * state is not initialized (as we have not yet parsed he headers).
   * However, information about the connecting client (IP, socket)
   * is available.
   */
  MHD_OPTION_URI_LOG_CALLBACK = 7,

  /**
   * Memory pointer for the private key (key.pem) to be used by the
   * HTTPS daemon.  This option should be followed by a
   * `const char *` argument.
   * This should be used in conjunction with #MHD_OPTION_HTTPS_MEM_CERT.
   */
  MHD_OPTION_HTTPS_MEM_KEY = 8,

  /**
   * Memory pointer for the certificate (cert.pem) to be used by the
   * HTTPS daemon.  This option should be followed by a
   * `const char *` argument.
   * This should be used in conjunction with #MHD_OPTION_HTTPS_MEM_KEY.
   */
  MHD_OPTION_HTTPS_MEM_CERT = 9,

  /**
   * Daemon credentials type.
   * Followed by an argument of type
   * `gnutls_credentials_type_t`.
   */
  MHD_OPTION_HTTPS_CRED_TYPE = 10,

  /**
   * Memory pointer to a `const char *` specifying the
   * cipher algorithm (default: "NORMAL").
   */
  MHD_OPTION_HTTPS_PRIORITIES = 11,

  /**
   * Pass a listen socket for MHD to use (systemd-style).  If this
   * option is used, MHD will not open its own listen socket(s). The
   * argument passed must be of type `MHD_socket` and refer to an
   * existing socket that has been bound to a port and is listening.
   */
  MHD_OPTION_LISTEN_SOCKET = 12,

  /**
   * Use the given function for logging error messages.  This option
   * must be followed by two arguments; the first must be a pointer to
   * a function of type #MHD_LogCallback and the second a pointer
   * `void *` which will be passed as the first argument to the log
   * callback.
   *
   * Note that MHD will not generate any log messages
   * if it was compiled without the "--enable-messages"
   * flag being set.
   */
  MHD_OPTION_EXTERNAL_LOGGER = 13,

  /**
   * Number (`unsigned int`) of threads in thread pool. Enable
   * thread pooling by setting this value to to something
   * greater than 1. Currently, thread model must be
   * #MHD_USE_SELECT_INTERNALLY if thread pooling is enabled
   * (#MHD_start_daemon returns NULL for an unsupported thread
   * model).
   */
  MHD_OPTION_THREAD_POOL_SIZE = 14,

  /**
   * Additional options given in an array of `struct MHD_OptionItem`.
   * The array must be terminated with an entry `{MHD_OPTION_END, 0, NULL}`.
   * An example for code using #MHD_OPTION_ARRAY is:
   *
   *     struct MHD_OptionItem ops[] = {
   *       { MHD_OPTION_CONNECTION_LIMIT, 100, NULL },
   *       { MHD_OPTION_CONNECTION_TIMEOUT, 10, NULL },
   *       { MHD_OPTION_END, 0, NULL }
   *     };
   *     d = MHD_start_daemon (0, 8080, NULL, NULL, dh, NULL,
   *                           MHD_OPTION_ARRAY, ops,
   *                           MHD_OPTION_END);
   *
   * For options that expect a single pointer argument, the
   * second member of the `struct MHD_OptionItem` is ignored.
   * For options that expect two pointer arguments, the first
   * argument must be cast to `intptr_t`.
   */
  MHD_OPTION_ARRAY = 15,

  /**
   * Specify a function that should be called for unescaping escape
   * sequences in URIs and URI arguments.  Note that this function
   * will NOT be used by the `struct MHD_PostProcessor`.  If this
   * option is not specified, the default method will be used which
   * decodes escape sequences of the form "%HH".  This option should
   * be followed by two arguments, the first one must be of the form
   *
   *     size_t my_unescaper(void *cls,
   *                         struct MHD_Connection *c,
   *                         char *s)
   *
   * where the return value must be "strlen(s)" and "s" should be
   * updated.  Note that the unescape function must not lengthen "s"
   * (the result must be shorter than the input and still be
   * 0-terminated).  "cls" will be set to the second argument
   * following #MHD_OPTION_UNESCAPE_CALLBACK.
   */
  MHD_OPTION_UNESCAPE_CALLBACK = 16,

  /**
   * Memory pointer for the random values to be used by the Digest
   * Auth module. This option should be followed by two arguments.
   * First an integer of type  `size_t` which specifies the size
   * of the buffer pointed to by the second argument in bytes.
   * Note that the application must ensure that the buffer of the
   * second argument remains allocated and unmodified while the
   * deamon is running.
   */
  MHD_OPTION_DIGEST_AUTH_RANDOM = 17,

  /**
   * Size of the internal array holding the map of the nonce and
   * the nonce counter. This option should be followed by an `unsigend int`
   * argument.
   */
  MHD_OPTION_NONCE_NC_SIZE = 18,

  /**
   * Desired size of the stack for threads created by MHD. Followed
   * by an argument of type `size_t`.  Use 0 for system default.
   */
  MHD_OPTION_THREAD_STACK_SIZE = 19,

  /**
   * Memory pointer for the certificate (ca.pem) to be used by the
   * HTTPS daemon for client authentification.
   * This option should be followed by a `const char *` argument.
   */
  MHD_OPTION_HTTPS_MEM_TRUST = 20,

  /**
   * Increment to use for growing the read buffer (followed by a
   * `size_t`). Must fit within #MHD_OPTION_CONNECTION_MEMORY_LIMIT.
   */
  MHD_OPTION_CONNECTION_MEMORY_INCREMENT = 21,

  /**
   * Use a callback to determine which X.509 certificate should be
   * used for a given HTTPS connection.  This option should be
   * followed by a argument of type `gnutls_certificate_retrieve_function2 *`.
   * This option provides an
   * alternative to #MHD_OPTION_HTTPS_MEM_KEY,
   * #MHD_OPTION_HTTPS_MEM_CERT.  You must use this version if
   * multiple domains are to be hosted at the same IP address using
   * TLS's Server Name Indication (SNI) extension.  In this case,
   * the callback is expected to select the correct certificate
   * based on the SNI information provided.  The callback is expected
   * to access the SNI data using `gnutls_server_name_get()`.
   * Using this option requires GnuTLS 3.0 or higher.
   */
  MHD_OPTION_HTTPS_CERT_CALLBACK = 22,

  /**
   * When using #MHD_USE_TCP_FASTOPEN, this option changes the default TCP
   * fastopen queue length of 50.  Note that having a larger queue size can
   * cause resource exhaustion attack as the TCP stack has to now allocate
   * resources for the SYN packet along with its DATA.  This option should be
   * followed by an `unsigned int` argument.
   */
  MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE = 23,

  /**
   * Memory pointer for the Diffie-Hellman parameters (dh.pem) to be used by the
   * HTTPS daemon for key exchange.
   * This option must be followed by a `const char *` argument.
   */
  MHD_OPTION_HTTPS_MEM_DHPARAMS = 24,

  /**
   * If present and set to true, allow reusing address:port socket
   * (by using SO_REUSEPORT on most platform, or platform-specific ways).
   * If present and set to false, disallow reusing address:port socket
   * (does nothing on most plaform, but uses SO_EXCLUSIVEADDRUSE on Windows).
   * This option must be followed by a `unsigned int` argument.
   */
  MHD_OPTION_LISTENING_ADDRESS_REUSE = 25,

  /**
   * Memory pointer for a password that decrypts the private key (key.pem)
   * to be used by the HTTPS daemon. This option should be followed by a
   * `const char *` argument.
   * This should be used in conjunction with #MHD_OPTION_HTTPS_MEM_KEY.
   * @sa ::MHD_FEATURE_HTTPS_KEY_PASSWORD
   */
  MHD_OPTION_HTTPS_KEY_PASSWORD = 26,

  /**
   * Register a function that should be called whenever a connection is
   * started or closed.
   *
   * This option should be followed by TWO pointers.  First a pointer
   * to a function of type #MHD_NotifyConnectionCallback and second a
   * pointer to a closure to pass to the request completed callback.
   * The second pointer maybe NULL.
   */
  MHD_OPTION_NOTIFY_CONNECTION = 27,

  /**
   * Allow to change maximum length of the queue of pending connections on
   * listen socket. If not present than default platform-specific SOMAXCONN
   * value is used. This option should be followed by an `unsigned int`
   * argument.
   */
  MHD_OPTION_LISTEN_BACKLOG_SIZE = 28
};


/**
 * Entry in an #MHD_OPTION_ARRAY.
 */
struct MHD_OptionItem
{
  /**
   * Which option is being given.  Use #MHD_OPTION_END
   * to terminate the array.
   */
  enum MHD_OPTION option;

  /**
   * Option value (for integer arguments, and for options requiring
   * two pointer arguments); should be 0 for options that take no
   * arguments or only a single pointer argument.
   */
  intptr_t value;

  /**
   * Pointer option value (use NULL for options taking no arguments
   * or only an integer option).
   */
  void *ptr_value;

};


/**
 * The `enum MHD_ValueKind` specifies the source of
 * the key-value pairs in the HTTP protocol.
 */
enum MHD_ValueKind
{

  /**
   * Response header
   */
  MHD_RESPONSE_HEADER_KIND = 0,

  /**
   * HTTP header.
   */
  MHD_HEADER_KIND = 1,

  /**
   * Cookies.  Note that the original HTTP header containing
   * the cookie(s) will still be available and intact.
   */
  MHD_COOKIE_KIND = 2,

  /**
   * POST data.  This is available only if a content encoding
   * supported by MHD is used (currently only URL encoding),
   * and only if the posted content fits within the available
   * memory pool.  Note that in that case, the upload data
   * given to the #MHD_AccessHandlerCallback will be
   * empty (since it has already been processed).
   */
  MHD_POSTDATA_KIND = 4,

  /**
   * GET (URI) arguments.
   */
  MHD_GET_ARGUMENT_KIND = 8,

  /**
   * HTTP footer (only for HTTP 1.1 chunked encodings).
   */
  MHD_FOOTER_KIND = 16
};


/**
 * The `enum MHD_RequestTerminationCode` specifies reasons
 * why a request has been terminated (or completed).
 * @ingroup request
 */
enum MHD_RequestTerminationCode
{

  /**
   * We finished sending the response.
   * @ingroup request
   */
  MHD_REQUEST_TERMINATED_COMPLETED_OK = 0,

  /**
   * Error handling the connection (resources
   * exhausted, other side closed connection,
   * application error accepting request, etc.)
   * @ingroup request
   */
  MHD_REQUEST_TERMINATED_WITH_ERROR = 1,

  /**
   * No activity on the connection for the number
   * of seconds specified using
   * #MHD_OPTION_CONNECTION_TIMEOUT.
   * @ingroup request
   */
  MHD_REQUEST_TERMINATED_TIMEOUT_REACHED = 2,

  /**
   * We had to close the session since MHD was being
   * shut down.
   * @ingroup request
   */
  MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN = 3,

  /**
   * We tried to read additional data, but the other side closed the
   * connection.  This error is similar to
   * #MHD_REQUEST_TERMINATED_WITH_ERROR, but specific to the case where
   * the connection died because the other side did not send expected
   * data.
   * @ingroup request
   */
  MHD_REQUEST_TERMINATED_READ_ERROR = 4,

  /**
   * The client terminated the connection by closing the socket
   * for writing (TCP half-closed); MHD aborted sending the
   * response according to RFC 2616, section 8.1.4.
   * @ingroup request
   */
  MHD_REQUEST_TERMINATED_CLIENT_ABORT = 5

};


/**
 * The `enum MHD_ConnectionNotificationCode` specifies types
 * of connection notifications.
 * @ingroup request
 */
enum MHD_ConnectionNotificationCode
{

  /**
   * A new connection has been started.
   * @ingroup request
   */
  MHD_CONNECTION_NOTIFY_STARTED = 0,

  /**
   * A connection is closed.
   * @ingroup request
   */
  MHD_CONNECTION_NOTIFY_CLOSED = 1

};


/**
 * Information about a connection.
 */
union MHD_ConnectionInfo
{

  /**
   * Cipher algorithm used, of type "enum gnutls_cipher_algorithm".
   */
  int /* enum gnutls_cipher_algorithm */ cipher_algorithm;

  /**
   * Protocol used, of type "enum gnutls_protocol".
   */
  int /* enum gnutls_protocol */ protocol;

  /**
   * Connect socket
   */
  MHD_socket connect_fd;

  /**
   * GNUtls session handle, of type "gnutls_session_t".
   */
  void * /* gnutls_session_t */ tls_session;

  /**
   * GNUtls client certificate handle, of type "gnutls_x509_crt_t".
   */
  void * /* gnutls_x509_crt_t */ client_cert;

  /**
   * Address information for the client.
   */
  struct sockaddr *client_addr;

  /**
   * Which daemon manages this connection (useful in case there are many
   * daemons running).
   */
  struct MHD_Daemon *daemon;

  /**
   * Socket-specific client context.  Points to the same address as
   * the "socket_context" of the #MHD_NotifyConnectionCallback.
   */
  void **socket_context;
};


/**
 * Values of this enum are used to specify what
 * information about a connection is desired.
 * @ingroup request
 */
enum MHD_ConnectionInfoType
{
  /**
   * What cipher algorithm is being used.
   * Takes no extra arguments.
   * @ingroup request
   */
  MHD_CONNECTION_INFO_CIPHER_ALGO,

  /**
   *
   * Takes no extra arguments.
   * @ingroup request
   */
  MHD_CONNECTION_INFO_PROTOCOL,

  /**
   * Obtain IP address of the client.  Takes no extra arguments.
   * Returns essentially a `struct sockaddr **` (since the API returns
   * a `union MHD_ConnectionInfo *` and that union contains a `struct
   * sockaddr *`).
   * @ingroup request
   */
  MHD_CONNECTION_INFO_CLIENT_ADDRESS,

  /**
   * Get the gnuTLS session handle.
   * @ingroup request
   */
  MHD_CONNECTION_INFO_GNUTLS_SESSION,

  /**
   * Get the gnuTLS client certificate handle.  Dysfunctional (never
   * implemented, deprecated).  Use #MHD_CONNECTION_INFO_GNUTLS_SESSION
   * to get the `gnutls_session_t` and then call
   * gnutls_certificate_get_peers().
   */
  MHD_CONNECTION_INFO_GNUTLS_CLIENT_CERT,

  /**
   * Get the `struct MHD_Daemon *` responsible for managing this connection.
   * @ingroup request
   */
  MHD_CONNECTION_INFO_DAEMON,

  /**
   * Request the file descriptor for the listening socket.
   * No extra arguments should be passed.
   * @ingroup request
   */
  MHD_CONNECTION_INFO_CONNECTION_FD,

  /**
   * Returns the client-specific pointer to a `void *` that was (possibly)
   * set during a #MHD_NotifyConnectionCallback when the socket was
   * first accepted.  Note that this is NOT the same as the "con_cls"
   * argument of the #MHD_AccessHandlerCallback.  The "con_cls" is
   * fresh for each HTTP request, while the "socket_context" is fresh
   * for each socket.
   */
  MHD_CONNECTION_INFO_SOCKET_CONTEXT

};


/**
 * Values of this enum are used to specify what
 * information about a deamon is desired.
 */
enum MHD_DaemonInfoType
{
  /**
   * No longer supported (will return NULL).
   */
  MHD_DAEMON_INFO_KEY_SIZE,

  /**
   * No longer supported (will return NULL).
   */
  MHD_DAEMON_INFO_MAC_KEY_SIZE,

  /**
   * Request the file descriptor for the listening socket.
   * No extra arguments should be passed.
   */
  MHD_DAEMON_INFO_LISTEN_FD,

  /**
   * Request the file descriptor for the external epoll.
   * No extra arguments should be passed.
   */
  MHD_DAEMON_INFO_EPOLL_FD_LINUX_ONLY,

  /**
   * Request the number of current connections handled by the daemon.
   * No extra arguments should be passed.
   */
  MHD_DAEMON_INFO_CURRENT_CONNECTIONS
};


/**
 * Callback for serious error condition. The default action is to print
 * an error message and `abort()`.
 *
 * @param cls user specified value
 * @param file where the error occured
 * @param line where the error occured
 * @param reason error detail, may be NULL
 * @ingroup logging
 */
typedef void
(*MHD_PanicCallback) (void *cls,
                      const char *file,
                      unsigned int line,
                      const char *reason);

/**
 * Allow or deny a client to connect.
 *
 * @param cls closure
 * @param addr address information from the client
 * @param addrlen length of @a addr
 * @return #MHD_YES if connection is allowed, #MHD_NO if not
 */
typedef int
(*MHD_AcceptPolicyCallback) (void *cls,
                             const struct sockaddr *addr,
                             socklen_t addrlen);


/**
 * A client has requested the given url using the given method
 * (#MHD_HTTP_METHOD_GET, #MHD_HTTP_METHOD_PUT,
 * #MHD_HTTP_METHOD_DELETE, #MHD_HTTP_METHOD_POST, etc).  The callback
 * must call MHD callbacks to provide content to give back to the
 * client and return an HTTP status code (i.e. #MHD_HTTP_OK,
 * #MHD_HTTP_NOT_FOUND, etc.).
 *
 * @param cls argument given together with the function
 *        pointer when the handler was registered with MHD
 * @param url the requested url
 * @param method the HTTP method used (#MHD_HTTP_METHOD_GET,
 *        #MHD_HTTP_METHOD_PUT, etc.)
 * @param version the HTTP version string (i.e.
 *        #MHD_HTTP_VERSION_1_1)
 * @param upload_data the data being uploaded (excluding HEADERS,
 *        for a POST that fits into memory and that is encoded
 *        with a supported encoding, the POST data will NOT be
 *        given in upload_data and is instead available as
 *        part of #MHD_get_connection_values; very large POST
 *        data *will* be made available incrementally in
 *        @a upload_data)
 * @param upload_data_size set initially to the size of the
 *        @a upload_data provided; the method must update this
 *        value to the number of bytes NOT processed;
 * @param con_cls pointer that the callback can set to some
 *        address and that will be preserved by MHD for future
 *        calls for this request; since the access handler may
 *        be called many times (i.e., for a PUT/POST operation
 *        with plenty of upload data) this allows the application
 *        to easily associate some request-specific state.
 *        If necessary, this state can be cleaned up in the
 *        global #MHD_RequestCompletedCallback (which
 *        can be set with the #MHD_OPTION_NOTIFY_COMPLETED).
 *        Initially, `*con_cls` will be NULL.
 * @return #MHD_YES if the connection was handled successfully,
 *         #MHD_NO if the socket must be closed due to a serios
 *         error while handling the request
 */
typedef int
(*MHD_AccessHandlerCallback) (void *cls,
                              struct MHD_Connection *connection,
                              const char *url,
                              const char *method,
                              const char *version,
                              const char *upload_data,
                              size_t *upload_data_size,
                              void **con_cls);


/**
 * Signature of the callback used by MHD to notify the
 * application about completed requests.
 *
 * @param cls client-defined closure
 * @param connection connection handle
 * @param con_cls value as set by the last call to
 *        the #MHD_AccessHandlerCallback
 * @param toe reason for request termination
 * @see #MHD_OPTION_NOTIFY_COMPLETED
 * @ingroup request
 */
typedef void
(*MHD_RequestCompletedCallback) (void *cls,
                                 struct MHD_Connection *connection,
                                 void **con_cls,
                                 enum MHD_RequestTerminationCode toe);

/**
 * Signature of the callback used by MHD to notify the
 * application about started/stopped connections
 *
 * @param cls client-defined closure
 * @param connection connection handle
 * @param socket_context socket-specific pointer where the
 *                       client can associate some state specific
 *                       to the TCP connection; note that this is
 *                       different from the "con_cls" which is per
 *                       HTTP request.  The client can initialize
 *                       during #MHD_CONNECTION_NOTIFY_STARTED and
 *                       cleanup during #MHD_CONNECTION_NOTIFY_CLOSED
 *                       and access in the meantime using
 *                       #MHD_CONNECTION_INFO_SOCKET_CONTEXT.
 * @param toe reason for connection notification
 * @see #MHD_OPTION_NOTIFY_CONNECTION
 * @ingroup request
 */
typedef void
(*MHD_NotifyConnectionCallback) (void *cls,
                                 struct MHD_Connection *connection,
                                 void **socket_context,
                                 enum MHD_ConnectionNotificationCode toe);


/**
 * Iterator over key-value pairs.  This iterator
 * can be used to iterate over all of the cookies,
 * headers, or POST-data fields of a request, and
 * also to iterate over the headers that have been
 * added to a response.
 *
 * @param cls closure
 * @param kind kind of the header we are looking at
 * @param key key for the value, can be an empty string
 * @param value corresponding value, can be NULL
 * @return #MHD_YES to continue iterating,
 *         #MHD_NO to abort the iteration
 * @ingroup request
 */
typedef int
(*MHD_KeyValueIterator) (void *cls,
                         enum MHD_ValueKind kind,
                         const char *key,
                         const char *value);


/**
 * Callback used by libmicrohttpd in order to obtain content.  The
 * callback is to copy at most @a max bytes of content into @a buf.  The
 * total number of bytes that has been placed into @a buf should be
 * returned.
 *
 * Note that returning zero will cause libmicrohttpd to try again.
 * Thus, returning zero should only be used in conjunction
 * with MHD_suspend_connection() to avoid busy waiting.
 *
 * @param cls extra argument to the callback
 * @param pos position in the datastream to access;
 *        note that if a `struct MHD_Response` object is re-used,
 *        it is possible for the same content reader to
 *        be queried multiple times for the same data;
 *        however, if a `struct MHD_Response` is not re-used,
 *        libmicrohttpd guarantees that "pos" will be
 *        the sum of all non-negative return values
 *        obtained from the content reader so far.
 * @param buf where to copy the data
 * @param max maximum number of bytes to copy to @a buf (size of @a buf)
 * @return number of bytes written to @a buf;
 *  0 is legal unless we are running in internal select mode (since
 *    this would cause busy-waiting); 0 in external select mode
 *    will cause this function to be called again once the external
 *    select calls MHD again;
 *  #MHD_CONTENT_READER_END_OF_STREAM (-1) for the regular
 *    end of transmission (with chunked encoding, MHD will then
 *    terminate the chunk and send any HTTP footers that might be
 *    present; without chunked encoding and given an unknown
 *    response size, MHD will simply close the connection; note
 *    that while returning #MHD_CONTENT_READER_END_OF_STREAM is not technically
 *    legal if a response size was specified, MHD accepts this
 *    and treats it just as #MHD_CONTENT_READER_END_WITH_ERROR;
 *  #MHD_CONTENT_READER_END_WITH_ERROR (-2) to indicate a server
 *    error generating the response; this will cause MHD to simply
 *    close the connection immediately.  If a response size was
 *    given or if chunked encoding is in use, this will indicate
 *    an error to the client.  Note, however, that if the client
 *    does not know a response size and chunked encoding is not in
 *    use, then clients will not be able to tell the difference between
 *    #MHD_CONTENT_READER_END_WITH_ERROR and #MHD_CONTENT_READER_END_OF_STREAM.
 *    This is not a limitation of MHD but rather of the HTTP protocol.
 */
typedef ssize_t
(*MHD_ContentReaderCallback) (void *cls,
                              uint64_t pos,
                              char *buf,
                              size_t max);


/**
 * This method is called by libmicrohttpd if we
 * are done with a content reader.  It should
 * be used to free resources associated with the
 * content reader.
 *
 * @param cls closure
 * @ingroup response
 */
typedef void
(*MHD_ContentReaderFreeCallback) (void *cls);


/**
 * Iterator over key-value pairs where the value
 * maybe made available in increments and/or may
 * not be zero-terminated.  Used for processing
 * POST data.
 *
 * @param cls user-specified closure
 * @param kind type of the value, always #MHD_POSTDATA_KIND when called from MHD
 * @param key 0-terminated key for the value
 * @param filename name of the uploaded file, NULL if not known
 * @param content_type mime-type of the data, NULL if not known
 * @param transfer_encoding encoding of the data, NULL if not known
 * @param data pointer to @a size bytes of data at the
 *              specified offset
 * @param off offset of data in the overall value
 * @param size number of bytes in @a data available
 * @return #MHD_YES to continue iterating,
 *         #MHD_NO to abort the iteration
 */
typedef int
(*MHD_PostDataIterator) (void *cls,
                         enum MHD_ValueKind kind,
                         const char *key,
                         const char *filename,
                         const char *content_type,
                         const char *transfer_encoding,
                         const char *data,
                         uint64_t off,
                         size_t size);

/* **************** Daemon handling functions ***************** */

/**
 * Start a webserver on the given port.
 *
 * @param flags combination of `enum MHD_FLAG` values
 * @param port port to bind to (in host byte order)
 * @param apc callback to call to check which clients
 *        will be allowed to connect; you can pass NULL
 *        in which case connections from any IP will be
 *        accepted
 * @param apc_cls extra argument to apc
 * @param dh handler called for all requests (repeatedly)
 * @param dh_cls extra argument to @a dh
 * @param ap list of options (type-value pairs,
 *        terminated with #MHD_OPTION_END).
 * @return NULL on error, handle to daemon on success
 * @ingroup event
 */
_MHD_EXTERN struct MHD_Daemon *
MHD_start_daemon_va (unsigned int flags,
		     uint16_t port,
		     MHD_AcceptPolicyCallback apc, void *apc_cls,
		     MHD_AccessHandlerCallback dh, void *dh_cls,
		     va_list ap);


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
 * @param apc_cls extra argument to apc
 * @param dh handler called for all requests (repeatedly)
 * @param dh_cls extra argument to @a dh
 * @return NULL on error, handle to daemon on success
 * @ingroup event
 */
_MHD_EXTERN struct MHD_Daemon *
MHD_start_daemon (unsigned int flags,
		  uint16_t port,
		  MHD_AcceptPolicyCallback apc, void *apc_cls,
		  MHD_AccessHandlerCallback dh, void *dh_cls,
		  ...);


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
_MHD_EXTERN MHD_socket
MHD_quiesce_daemon (struct MHD_Daemon *daemon);


/**
 * Shutdown an HTTP daemon.
 *
 * @param daemon daemon to stop
 * @ingroup event
 */
_MHD_EXTERN void
MHD_stop_daemon (struct MHD_Daemon *daemon);


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
 *        not handle the connection (i.e. `malloc()` failed, etc).
 *        The socket will be closed in any case; `errno` is
 *        set to indicate further details about the error.
 * @ingroup specialized
 */
_MHD_EXTERN int
MHD_add_connection (struct MHD_Daemon *daemon,
		    MHD_socket client_socket,
		    const struct sockaddr *addr,
		    socklen_t addrlen);


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
_MHD_EXTERN int
MHD_get_fdset (struct MHD_Daemon *daemon,
               fd_set *read_fd_set,
               fd_set *write_fd_set,
	       fd_set *except_fd_set,
	       MHD_socket *max_fd);


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
_MHD_EXTERN int
MHD_get_fdset2 (struct MHD_Daemon *daemon,
               fd_set *read_fd_set,
               fd_set *write_fd_set,
               fd_set *except_fd_set,
               MHD_socket *max_fd,
               unsigned int fd_setsize);


/**
 * Obtain the `select()` sets for this daemon.
 * Daemon's FDs will be added to fd_sets. To get only
 * daemon FDs in fd_sets, call FD_ZERO for each fd_set
 * before calling this function. Size of fd_set is
 * determined by current value of FD_SETSIZE.
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
#define MHD_get_fdset(daemon,read_fd_set,write_fd_set,except_fd_set,max_fd) \
  MHD_get_fdset2((daemon),(read_fd_set),(write_fd_set),(except_fd_set),(max_fd),FD_SETSIZE)


/**
 * Obtain timeout value for `select()` for this daemon (only needed if
 * connection timeout is used).  The returned value is how many milliseconds
 * `select()` or `poll()` should at most block, not the timeout value set for
 * connections.  This function MUST NOT be called if MHD is running with
 * #MHD_USE_THREAD_PER_CONNECTION.
 *
 * @param daemon daemon to query for timeout
 * @param timeout set to the timeout (in milliseconds)
 * @return #MHD_YES on success, #MHD_NO if timeouts are
 *        not used (or no connections exist that would
 *        necessiate the use of a timeout right now).
 * @ingroup event
 */
_MHD_EXTERN int
MHD_get_timeout (struct MHD_Daemon *daemon,
		 MHD_UNSIGNED_LONG_LONG *timeout);


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
_MHD_EXTERN int
MHD_run (struct MHD_Daemon *daemon);


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
_MHD_EXTERN int
MHD_run_from_select (struct MHD_Daemon *daemon,
		     const fd_set *read_fd_set,
		     const fd_set *write_fd_set,
		     const fd_set *except_fd_set);




/* **************** Connection handling functions ***************** */

/**
 * Get all of the headers from the request.
 *
 * @param connection connection to get values from
 * @param kind types of values to iterate over, can be a bitmask
 * @param iterator callback to call on each header;
 *        maybe NULL (then just count headers)
 * @param iterator_cls extra argument to @a iterator
 * @return number of entries iterated over
 * @ingroup request
 */
_MHD_EXTERN int
MHD_get_connection_values (struct MHD_Connection *connection,
                           enum MHD_ValueKind kind,
                           MHD_KeyValueIterator iterator,
                           void *iterator_cls);


/**
 * This function can be used to add an entry to the HTTP headers of a
 * connection (so that the #MHD_get_connection_values function will
 * return them -- and the `struct MHD_PostProcessor` will also see
 * them).  This maybe required in certain situations (see Mantis
 * #1399) where (broken) HTTP implementations fail to supply values
 * needed by the post processor (or other parts of the application).
 *
 * This function MUST only be called from within the
 * #MHD_AccessHandlerCallback (otherwise, access maybe improperly
 * synchronized).  Furthermore, the client must guarantee that the key
 * and value arguments are 0-terminated strings that are NOT freed
 * until the connection is closed.  (The easiest way to do this is by
 * passing only arguments to permanently allocated strings.).
 *
 * @param connection the connection for which a
 *  value should be set
 * @param kind kind of the value
 * @param key key for the value
 * @param value the value itself
 * @return #MHD_NO if the operation could not be
 *         performed due to insufficient memory;
 *         #MHD_YES on success
 * @ingroup request
 */
_MHD_EXTERN int
MHD_set_connection_value (struct MHD_Connection *connection,
                          enum MHD_ValueKind kind,
                          const char *key,
			  const char *value);


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
_MHD_EXTERN void
MHD_set_panic_func (MHD_PanicCallback cb, void *cls);


/**
 * Process escape sequences ('%HH') Updates val in place; the
 * result should be UTF-8 encoded and cannot be larger than the input.
 * The result must also still be 0-terminated.
 *
 * @param val value to unescape (modified in the process)
 * @return length of the resulting val (`strlen(val)` may be
 *  shorter afterwards due to elimination of escape sequences)
 */
_MHD_EXTERN size_t
MHD_http_unescape (char *val);


/**
 * Get a particular header value.  If multiple
 * values match the kind, return any one of them.
 *
 * @param connection connection to get values from
 * @param kind what kind of value are we looking for
 * @param key the header to look for, NULL to lookup 'trailing' value without a key
 * @return NULL if no such item was found
 * @ingroup request
 */
_MHD_EXTERN const char *
MHD_lookup_connection_value (struct MHD_Connection *connection,
			     enum MHD_ValueKind kind,
			     const char *key);


/**
 * Queue a response to be transmitted to the client (as soon as
 * possible but after #MHD_AccessHandlerCallback returns).
 *
 * @param connection the connection identifying the client
 * @param status_code HTTP status code (i.e. #MHD_HTTP_OK)
 * @param response response to transmit
 * @return #MHD_NO on error (i.e. reply already sent),
 *         #MHD_YES on success or if message has been queued
 * @ingroup response
 */
_MHD_EXTERN int
MHD_queue_response (struct MHD_Connection *connection,
                    unsigned int status_code,
		    struct MHD_Response *response);


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
_MHD_EXTERN void
MHD_suspend_connection (struct MHD_Connection *connection);


/**
 * Resume handling of network data for suspended connection.  It is
 * safe to resume a suspended connection at any time.  Calling this
 * function on a connection that was not previously suspended will
 * result in undefined behavior.
 *
 * @param connection the connection to resume
 */
_MHD_EXTERN void
MHD_resume_connection (struct MHD_Connection *connection);


/* **************** Response manipulation functions ***************** */


/**
 * Flags for special handling of responses.
 */
enum MHD_ResponseFlags
{
  /**
   * Default: no special flags.
   */
  MHD_RF_NONE = 0,

  /**
   * Only respond in conservative HTTP 1.0-mode.   In particular,
   * do not (automatically) sent "Connection" headers and always
   * close the connection after generating the response.
   */
  MHD_RF_HTTP_VERSION_1_0_ONLY = 1

};


/**
 * MHD options (for future extensions).
 */
enum MHD_ResponseOptions
{
  /**
   * End of the list of options.
   */
  MHD_RO_END = 0
};


/**
 * Set special flags and options for a response.
 *
 * @param response the response to modify
 * @param flags to set for the response
 * @param ... #MHD_RO_END terminated list of options
 * @return #MHD_YES on success, #MHD_NO on error
 */
_MHD_EXTERN int
MHD_set_response_options (struct MHD_Response *response,
                          enum MHD_ResponseFlags flags,
                          ...);


/**
 * Create a response object.  The response object can be extended with
 * header information and then be used any number of times.
 *
 * @param size size of the data portion of the response, #MHD_SIZE_UNKNOWN for unknown
 * @param block_size preferred block size for querying crc (advisory only,
 *                   MHD may still call @a crc using smaller chunks); this
 *                   is essentially the buffer size used for IO, clients
 *                   should pick a value that is appropriate for IO and
 *                   memory performance requirements
 * @param crc callback to use to obtain response data
 * @param crc_cls extra argument to @a crc
 * @param crfc callback to call to free @a crc_cls resources
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_callback (uint64_t size,
				   size_t block_size,
				   MHD_ContentReaderCallback crc, void *crc_cls,
				   MHD_ContentReaderFreeCallback crfc);


/**
 * Create a response object.  The response object can be extended with
 * header information and then be used any number of times.
 *
 * @param size size of the @a data portion of the response
 * @param data the data itself
 * @param must_free libmicrohttpd should free data when done
 * @param must_copy libmicrohttpd must make a copy of @a data
 *        right away, the data maybe released anytime after
 *        this call returns
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @deprecated use #MHD_create_response_from_buffer instead
 * @ingroup response
 */
_MHD_DEPR_FUNC("MHD_create_response_from_data() is deprecated, use MHD_create_response_from_buffer()") \
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_data (size_t size,
			       void *data,
			       int must_free,
			       int must_copy);


/**
 * Specification for how MHD should treat the memory buffer
 * given for the response.
 * @ingroup response
 */
enum MHD_ResponseMemoryMode
{

  /**
   * Buffer is a persistent (static/global) buffer that won't change
   * for at least the lifetime of the response, MHD should just use
   * it, not free it, not copy it, just keep an alias to it.
   * @ingroup response
   */
  MHD_RESPMEM_PERSISTENT,

  /**
   * Buffer is heap-allocated with `malloc()` (or equivalent) and
   * should be freed by MHD after processing the response has
   * concluded (response reference counter reaches zero).
   * @ingroup response
   */
  MHD_RESPMEM_MUST_FREE,

  /**
   * Buffer is in transient memory, but not on the heap (for example,
   * on the stack or non-`malloc()` allocated) and only valid during the
   * call to #MHD_create_response_from_buffer.  MHD must make its
   * own private copy of the data for processing.
   * @ingroup response
   */
  MHD_RESPMEM_MUST_COPY

};


/**
 * Create a response object.  The response object can be extended with
 * header information and then be used any number of times.
 *
 * @param size size of the data portion of the response
 * @param buffer size bytes containing the response's data portion
 * @param mode flags for buffer management
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_buffer (size_t size,
				 void *buffer,
				 enum MHD_ResponseMemoryMode mode);


/**
 * Create a response object.  The response object can be extended with
 * header information and then be used any number of times.
 *
 * @param size size of the data portion of the response
 * @param fd file descriptor referring to a file on disk with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd (size_t size,
                               int fd);


/**
 * Create a response object.  The response object can be extended with
 * header information and then be used any number of times.
 *
 * @param size size of the data portion of the response;
 *        sizes larger than 2 GiB may be not supported by OS or
 *        MHD build; see ::MHD_FEATURE_LARGE_FILE
 * @param fd file descriptor referring to a file on disk with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd64 (uint64_t size,
                               int fd);


/**
 * Create a response object.  The response object can be extended with
 * header information and then be used any number of times.
 *
 * @param size size of the data portion of the response
 * @param fd file descriptor referring to a file on disk with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @param offset offset to start reading from in the file;
 *        Be careful! `off_t` may have been compiled to be a
 *        64-bit variable for MHD, in which case your application
 *        also has to be compiled using the same options! Read
 *        the MHD manual for more details.
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_DEPR_FUNC("Function MHD_create_response_from_fd_at_offset() is deprecated, use MHD_create_response_from_fd_at_offset64()") \
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd_at_offset (size_t size,
                                       int fd,
                                       off_t offset);

#ifndef _MHD_NO_DEPR_IN_MACRO
/* Substitute MHD_create_response_from_fd_at_offset64() instead of MHD_create_response_from_fd_at_offset()
   to minimize possible problems with different off_t options */
#define MHD_create_response_from_fd_at_offset(size,fd,offset) \
  _MHD_DEPR_IN_MACRO("Usage of MHD_create_response_from_fd_at_offset() is deprecated, use MHD_create_response_from_fd_at_offset64()") \
  MHD_create_response_from_fd_at_offset64((size),(fd),(offset))
#endif /* ! _MHD_NO_DEPR_IN_MACRO */


/**
 * Create a response object.  The response object can be extended with
 * header information and then be used any number of times.
 *
 * @param size size of the data portion of the response;
 *        sizes larger than 2 GiB may be not supported by OS or
 *        MHD build; see ::MHD_FEATURE_LARGE_FILE
 * @param fd file descriptor referring to a file on disk with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @param offset offset to start reading from in the file;
 *        reading file beyond 2 GiB may be not supported by OS or
 *        MHD build; see ::MHD_FEATURE_LARGE_FILE
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd_at_offset64 (uint64_t size,
                                         int fd,
                                         uint64_t offset);


#if 0
/**
 * Enumeration for actions MHD should perform on the underlying socket
 * of the upgrade.  This API is not finalized, and in particular
 * the final set of actions is yet to be decided. This is just an
 * idea for what we might want.
 */
enum MHD_UpgradeAction
{

  /**
   * Close the socket, the application is done with it.
   *
   * Takes no extra arguments.
   *
   * NOTE: it is unclear if we want to have this in the
   * "final" API, this is all just ideas.
   */
  MHD_UPGRADE_ACTION_CLOSE = 0,

  /**
   * Uncork the TCP write buffer (that is, tell the OS to transmit all
   * bytes in the buffer now, and to not use TCP-CORKing).
   *
   * Takes no extra arguments.
   *
   * NOTE: it is unclear if we want to have this in the
   * "final" API, this is all just ideas.
   */
  MHD_UPGRADE_ACTION_CORK

};


/**
 * This connection-specific callback is provided by MHD to
 * applications (unusual) during the #MHD_UpgradeHandler.
 * It allows applications to perform 'special' actions on
 * the underlying socket from the upgrade.
 *
 * @param cls the closure (from `upgrade_action_cls`)
 * @param action which action should be performed
 * @param ... arguments to the action (depends on the action)
 * @return #MHD_NO on error, #MHD_YES on success
 */
typedef int
(*MHD_UpgradeActionCallback)(void *cls,
                             enum MHD_UpgradeAction action,
                             ...);

/**
 * Function called after a protocol "upgrade" response was sent
 * successfully and the socket should now be controlled by some
 * protocol other than HTTP.
 *
 * Any data received on the socket will be made available in
 * 'data_in'.  The function should update 'data_in_size' to
 * reflect the number of bytes consumed from 'data_in' (the remaining
 * bytes will be made available in the next call to the handler).
 *
 * Any data that should be transmitted on the socket should be
 * stored in 'data_out'.  '*data_out_size' is initially set to
 * the available buffer space in 'data_out'.  It should be set to
 * the number of bytes stored in 'data_out' (which can be zero).
 *
 * The return value is a BITMASK that indicates how the function
 * intends to interact with the event loop.  It can request to be
 * notified for reading, writing, request to UNCORK the send buffer
 * (which MHD is allowed to ignore, if it is not possible to uncork on
 * the local platform), to wait for the 'external' select loop to
 * trigger another round.  It is also possible to specify "no events"
 * to terminate the connection; in this case, the
 * #MHD_RequestCompletedCallback will be called and all resources of
 * the connection will be released.
 *
 * Except when in 'thread-per-connection' mode, implementations
 * of this function should never block (as it will still be called
 * from within the main event loop).
 *
 * @param cls closure
 * @param connection original HTTP connection handle,
 *                   giving the function a last chance
 *                   to inspect the original HTTP request
 * @param sock socket to use for bi-directional communication
 *        with the client.  For HTTPS, this may not be a socket
 *        that is directly connected to the client and thus certain
 *        operations (TCP-specific setsockopt(), getsockopt(), etc.)
 *        may not work as expected (as the socket could be from a
 *        socketpair() or a TCP-loopback)
 * @param upgrade_action function that can be used to perform actions
 *        on the @a sock (like those that cannot be done explicitly).
 *        Applications must use this callback to perform the
 *        close() action on the @a sock.
 * @param upgrade_action_cls closure that must be passed to @a upgrade_action
 */
typedef void
(*MHD_UpgradeHandler)(void *cls,
                      struct MHD_Connection *connection,
                      MHD_SOCKET sock,
                      MHD_UpgradeActionCallback upgrade_action,
                      void *upgrade_action_cls);


/**
 * Create a response object that can be used for 101 UPGRADE
 * responses, for example to implement WebSockets.  After sending the
 * response, control over the data stream is given to the callback (which
 * can then, for example, start some bi-directional communication).
 * If the response is queued for multiple connections, the callback
 * will be called for each connection.  The callback
 * will ONLY be called after the response header was successfully passed
 * to the OS; if there are communication errors before, the usual MHD
 * connection error handling code will be performed.
 *
 * Setting the correct HTTP code (i.e. MHD_HTTP_SWITCHING_PROTOCOLS)
 * and setting correct HTTP headers for the upgrade must be done
 * manually (this way, it is possible to implement most existing
 * WebSocket versions using this API; in fact, this API might be useful
 * for any protocol switch, not just WebSockets).  Note that
 * draft-ietf-hybi-thewebsocketprotocol-00 cannot be implemented this
 * way as the header "HTTP/1.1 101 WebSocket Protocol Handshake"
 * cannot be generated; instead, MHD will always produce "HTTP/1.1 101
 * Switching Protocols" (if the response code 101 is used).
 *
 * As usual, the response object can be extended with header
 * information and then be used any number of times (as long as the
 * header information is not connection-specific).
 *
 * @param upgrade_handler function to call with the 'upgraded' socket
 * @param upgrade_handler_cls closure for @a upgrade_handler
 * @return NULL on error (i.e. invalid arguments, out of memory)
 */
struct MHD_Response *
MHD_create_response_for_upgrade (MHD_UpgradeHandler upgrade_handler,
				 void *upgrade_handler_cls);
#endif

/**
 * Destroy a response object and associated resources.  Note that
 * libmicrohttpd may keep some of the resources around if the response
 * is still in the queue for some clients, so the memory may not
 * necessarily be freed immediatley.
 *
 * @param response response to destroy
 * @ingroup response
 */
_MHD_EXTERN void
MHD_destroy_response (struct MHD_Response *response);


/**
 * Add a header line to the response.
 *
 * @param response response to add a header to
 * @param header the header to add
 * @param content value to add
 * @return #MHD_NO on error (i.e. invalid header or content format),
 *         or out of memory
 * @ingroup response
 */
_MHD_EXTERN int
MHD_add_response_header (struct MHD_Response *response,
                         const char *header,
			 const char *content);


/**
 * Add a footer line to the response.
 *
 * @param response response to remove a header from
 * @param footer the footer to delete
 * @param content value to delete
 * @return #MHD_NO on error (i.e. invalid footer or content format).
 * @ingroup response
 */
_MHD_EXTERN int
MHD_add_response_footer (struct MHD_Response *response,
                         const char *footer,
			 const char *content);


/**
 * Delete a header (or footer) line from the response.
 *
 * @param response response to remove a header from
 * @param header the header to delete
 * @param content value to delete
 * @return #MHD_NO on error (no such header known)
 * @ingroup response
 */
_MHD_EXTERN int
MHD_del_response_header (struct MHD_Response *response,
                         const char *header,
			 const char *content);


/**
 * Get all of the headers (and footers) added to a response.
 *
 * @param response response to query
 * @param iterator callback to call on each header;
 *        maybe NULL (then just count headers)
 * @param iterator_cls extra argument to @a iterator
 * @return number of entries iterated over
 * @ingroup response
 */
_MHD_EXTERN int
MHD_get_response_headers (struct MHD_Response *response,
                          MHD_KeyValueIterator iterator, void *iterator_cls);


/**
 * Get a particular header (or footer) from the response.
 *
 * @param response response to query
 * @param key which header to get
 * @return NULL if header does not exist
 * @ingroup response
 */
_MHD_EXTERN const char *
MHD_get_response_header (struct MHD_Response *response,
			 const char *key);


/* ********************** PostProcessor functions ********************** */

/**
 * Create a `struct MHD_PostProcessor`.
 *
 * A `struct MHD_PostProcessor` can be used to (incrementally) parse
 * the data portion of a POST request.  Note that some buggy browsers
 * fail to set the encoding type.  If you want to support those, you
 * may have to call #MHD_set_connection_value with the proper encoding
 * type before creating a post processor (if no supported encoding
 * type is set, this function will fail).
 *
 * @param connection the connection on which the POST is
 *        happening (used to determine the POST format)
 * @param buffer_size maximum number of bytes to use for
 *        internal buffering (used only for the parsing,
 *        specifically the parsing of the keys).  A
 *        tiny value (256-1024) should be sufficient.
 *        Do NOT use a value smaller than 256.  For good
 *        performance, use 32 or 64k (i.e. 65536).
 * @param iter iterator to be called with the parsed data,
 *        Must NOT be NULL.
 * @param iter_cls first argument to @a iter
 * @return NULL on error (out of memory, unsupported encoding),
 *         otherwise a PP handle
 * @ingroup request
 */
_MHD_EXTERN struct MHD_PostProcessor *
MHD_create_post_processor (struct MHD_Connection *connection,
			   size_t buffer_size,
			   MHD_PostDataIterator iter, void *iter_cls);


/**
 * Parse and process POST data.  Call this function when POST data is
 * available (usually during an #MHD_AccessHandlerCallback) with the
 * "upload_data" and "upload_data_size".  Whenever possible, this will
 * then cause calls to the #MHD_PostDataIterator.
 *
 * @param pp the post processor
 * @param post_data @a post_data_len bytes of POST data
 * @param post_data_len length of @a post_data
 * @return #MHD_YES on success, #MHD_NO on error
 *         (out-of-memory, iterator aborted, parse error)
 * @ingroup request
 */
_MHD_EXTERN int
MHD_post_process (struct MHD_PostProcessor *pp,
                  const char *post_data, size_t post_data_len);


/**
 * Release PostProcessor resources.
 *
 * @param pp the PostProcessor to destroy
 * @return #MHD_YES if processing completed nicely,
 *         #MHD_NO if there were spurious characters / formatting
 *                problems; it is common to ignore the return
 *                value of this function
 * @ingroup request
 */
_MHD_EXTERN int
MHD_destroy_post_processor (struct MHD_PostProcessor *pp);


/* ********************* Digest Authentication functions *************** */


/**
 * Constant to indicate that the nonce of the provided
 * authentication code was wrong.
 * @ingroup authentication
 */
#define MHD_INVALID_NONCE -1


/**
 * Get the username from the authorization header sent by the client
 *
 * @param connection The MHD connection structure
 * @return NULL if no username could be found, a pointer
 * 			to the username if found
 * @ingroup authentication
 */
_MHD_EXTERN char *
MHD_digest_auth_get_username (struct MHD_Connection *connection);


/**
 * Authenticates the authorization header sent by the client
 *
 * @param connection The MHD connection structure
 * @param realm The realm presented to the client
 * @param username The username needs to be authenticated
 * @param password The password used in the authentication
 * @param nonce_timeout The amount of time for a nonce to be
 * 			invalid in seconds
 * @return #MHD_YES if authenticated, #MHD_NO if not,
 * 			#MHD_INVALID_NONCE if nonce is invalid
 * @ingroup authentication
 */
_MHD_EXTERN int
MHD_digest_auth_check (struct MHD_Connection *connection,
		       const char *realm,
		       const char *username,
		       const char *password,
		       unsigned int nonce_timeout);


/**
 * Queues a response to request authentication from the client
 *
 * @param connection The MHD connection structure
 * @param realm The realm presented to the client
 * @param opaque string to user for opaque value
 * @param response reply to send; should contain the "access denied"
 *        body; note that this function will set the "WWW Authenticate"
 *        header and that the caller should not do this
 * @param signal_stale #MHD_YES if the nonce is invalid to add
 * 			'stale=true' to the authentication header
 * @return #MHD_YES on success, #MHD_NO otherwise
 * @ingroup authentication
 */
_MHD_EXTERN int
MHD_queue_auth_fail_response (struct MHD_Connection *connection,
			      const char *realm,
			      const char *opaque,
			      struct MHD_Response *response,
			      int signal_stale);


/**
 * Get the username and password from the basic authorization header sent by the client
 *
 * @param connection The MHD connection structure
 * @param password a pointer for the password
 * @return NULL if no username could be found, a pointer
 * 			to the username if found
 * @ingroup authentication
 */
_MHD_EXTERN char *
MHD_basic_auth_get_username_password (struct MHD_Connection *connection,
				      char** password);


/**
 * Queues a response to request basic authentication from the client
 * The given response object is expected to include the payload for
 * the response; the "WWW-Authenticate" header will be added and the
 * response queued with the 'UNAUTHORIZED' status code.
 *
 * @param connection The MHD connection structure
 * @param realm the realm presented to the client
 * @param response response object to modify and queue
 * @return #MHD_YES on success, #MHD_NO otherwise
 * @ingroup authentication
 */
_MHD_EXTERN int
MHD_queue_basic_auth_fail_response (struct MHD_Connection *connection,
				    const char *realm,
				    struct MHD_Response *response);

/* ********************** generic query functions ********************** */


/**
 * Obtain information about the given connection.
 *
 * @param connection what connection to get information about
 * @param info_type what information is desired?
 * @param ... depends on @a info_type
 * @return NULL if this information is not available
 *         (or if the @a info_type is unknown)
 * @ingroup specialized
 */
_MHD_EXTERN const union MHD_ConnectionInfo *
MHD_get_connection_info (struct MHD_Connection *connection,
			 enum MHD_ConnectionInfoType info_type,
			 ...);


/**
 * MHD connection options.  Given to #MHD_set_connection_option to
 * set custom options for a particular connection.
 */
enum MHD_CONNECTION_OPTION
{

  /**
   * Set a custom timeout for the given connection.  Specified
   * as the number of seconds, given as an `unsigned int`.  Use
   * zero for no timeout.
   */
  MHD_CONNECTION_OPTION_TIMEOUT

};


/**
 * Set a custom option for the given connection, overriding defaults.
 *
 * @param connection connection to modify
 * @param option option to set
 * @param ... arguments to the option, depending on the option type
 * @return #MHD_YES on success, #MHD_NO if setting the option failed
 * @ingroup specialized
 */
_MHD_EXTERN int
MHD_set_connection_option (struct MHD_Connection *connection,
			   enum MHD_CONNECTION_OPTION option,
			   ...);


/**
 * Information about an MHD daemon.
 */
union MHD_DaemonInfo
{
  /**
   * Size of the key, no longer supported.
   * @deprecated
   */
  size_t key_size;

  /**
   * Size of the mac key, no longer supported.
   * @deprecated
   */
  size_t mac_key_size;

  /**
   * Listen socket file descriptor, for #MHD_DAEMON_INFO_EPOLL_FD_LINUX_ONLY
   * and #MHD_DAEMON_INFO_LISTEN_FD.
   */
  MHD_socket listen_fd;

  /**
   * Number of active connections, for #MHD_DAEMON_INFO_CURRENT_CONNECTIONS.
   */
  unsigned int num_connections;
};


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
_MHD_EXTERN const union MHD_DaemonInfo *
MHD_get_daemon_info (struct MHD_Daemon *daemon,
		     enum MHD_DaemonInfoType info_type,
		     ...);


/**
 * Obtain the version of this library
 *
 * @return static version string, e.g. "0.9.9"
 * @ingroup specialized
 */
_MHD_EXTERN const char*
MHD_get_version (void);


/**
 * Types of information about MHD features,
 * used by #MHD_is_feature_supported().
 */
enum MHD_FEATURE
{
  /**
   * Get whether messages are supported. If supported then in debug
   * mode messages can be printed to stderr or to external logger.
   */
  MHD_FEATURE_MESSGES = 1,

  /**
   * Get whether HTTPS is supported.  If supported then flag
   * #MHD_USE_SSL and options #MHD_OPTION_HTTPS_MEM_KEY,
   * #MHD_OPTION_HTTPS_MEM_CERT, #MHD_OPTION_HTTPS_MEM_TRUST,
   * #MHD_OPTION_HTTPS_MEM_DHPARAMS, #MHD_OPTION_HTTPS_CRED_TYPE,
   * #MHD_OPTION_HTTPS_PRIORITIES can be used.
   */
  MHD_FEATURE_SSL = 2,

  /**
   * Get whether option #MHD_OPTION_HTTPS_CERT_CALLBACK is
   * supported.
   */
  MHD_FEATURE_HTTPS_CERT_CALLBACK = 3,

  /**
   * Get whether IPv6 is supported. If supported then flag
   * #MHD_USE_IPv6 can be used.
   */
  MHD_FEATURE_IPv6 = 4,

  /**
   * Get whether IPv6 without IPv4 is supported. If not supported
   * then IPv4 is always enabled in IPv6 sockets and
   * flag #MHD_USE_DUAL_STACK if always used when #MHD_USE_IPv6 is
   * specified.
   */
  MHD_FEATURE_IPv6_ONLY = 5,

  /**
   * Get whether `poll()` is supported. If supported then flag
   * #MHD_USE_POLL can be used.
   */
  MHD_FEATURE_POLL = 6,

  /**
   * Get whether `epoll()` is supported. If supported then Flags
   * #MHD_USE_EPOLL_LINUX_ONLY and
   * #MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY can be used.
   */
  MHD_FEATURE_EPOLL = 7,

  /**
   * Get whether shutdown on listen socket to signal other
   * threads is supported. If not supported flag
   * #MHD_USE_PIPE_FOR_SHUTDOWN is automatically forced.
   */
  MHD_FEATURE_SHUTDOWN_LISTEN_SOCKET = 8,

  /**
   * Get whether socketpair is used internally instead of pipe to
   * signal other threads.
   */
  MHD_FEATURE_SOCKETPAIR = 9,

  /**
   * Get whether TCP Fast Open is supported. If supported then
   * flag #MHD_USE_TCP_FASTOPEN and option
   * #MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE can be used.
   */
  MHD_FEATURE_TCP_FASTOPEN = 10,

  /**
   * Get whether HTTP Basic authorization is supported. If supported
   * then functions #MHD_basic_auth_get_username_password and
   * #MHD_queue_basic_auth_fail_response can be used.
   */
  MHD_FEATURE_BASIC_AUTH = 11,

  /**
   * Get whether HTTP Digest authorization is supported. If
   * supported then options #MHD_OPTION_DIGEST_AUTH_RANDOM,
   * #MHD_OPTION_NONCE_NC_SIZE and
   * #MHD_digest_auth_check() can be used.
   */
  MHD_FEATURE_DIGEST_AUTH = 12,

  /**
   * Get whether postprocessor is supported. If supported then
   * functions #MHD_create_post_processor(), #MHD_post_process() and
   * #MHD_destroy_post_processor() can
   * be used.
   */
  MHD_FEATURE_POSTPROCESSOR = 13,

  /**
  * Get whether password encrypted private key for HTTPS daemon is
  * supported. If supported then option
  * ::MHD_OPTION_HTTPS_KEY_PASSWORD can be used.
  */
  MHD_FEATURE_HTTPS_KEY_PASSWORD = 14,

  /**
   * Get whether reading files beyond 2 GiB boundary is supported.
   * If supported then #MHD_create_response_from_fd(),
   * #MHD_create_response_from_fd64 #MHD_create_response_from_fd_at_offset()
   * and #MHD_create_response_from_fd_at_offset64() can be used with sizes and
   * offsets larger than 2 GiB. If not supported value of size+offset is
   * limited to 2 GiB.
   */
  MHD_FEATURE_LARGE_FILE = 15
};


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
MHD_is_feature_supported(enum MHD_FEATURE feature);


#if 0                           /* keep Emacsens' auto-indent happy */
{
#endif
#ifdef __cplusplus
}
#endif

#endif
