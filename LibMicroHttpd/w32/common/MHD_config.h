/* MHD_config.h for W32 */
/* Created manually. */

/* *** Basic OS/compiler information *** */

/* This is a Windows system */
#define WINDOWS 1

/* Define if MS VC compiler is used */
#define MSVC 1

/* *** MHD configuration *** */
/* Undef to disable feature */

/* Enable basic Auth support */
#define BAUTH_SUPPORT 1

/* Enable digest Auth support */
#define DAUTH_SUPPORT 1

/* Enable postprocessor.c */
#define HAVE_POSTPROCESSOR 1

/* Enable error messages */
#define HAVE_MESSAGES 1

/* Disable HTTPS support */
#define HTTPS_SUPPORT 0


/* *** OS features *** */

/* Provides IPv6 headers */
#define HAVE_INET6 1

/* Define to use pair of sockets instead of pipes for signaling */
#define MHD_DONT_USE_PIPES 1

/* define to use W32 threads */
#define MHD_USE_W32_THREADS 1

#ifndef _WIN32_WINNT
/* MHD supports Windows XP and later W32 systems*/
#define _WIN32_WINNT 0x0501
#endif /* _WIN32_WINNT */

/* winsock poll is available only on Vista and later */
#if _WIN32_WINNT >= 0x0600
#define HAVE_POLL 1
#endif /* _WIN32_WINNT >= 0x0600 */

/* define to 0 to disable epoll support */
#define EPOLL_SUPPORT 0

/* Define to 1 if you have the <winsock2.h> header file. */
#define HAVE_WINSOCK2_H 1

/* Define to 1 if you have the <ws2tcpip.h> header file. */
#define HAVE_WS2TCPIP_H 1

/* Define to 1 if you have the declaration of `SOCK_NONBLOCK', and to 0 if you
   don't. */
#define HAVE_DECL_SOCK_NONBLOCK 0

/* Define to 1 if you have the `_lseeki64' function. */
#define HAVE___LSEEKI64 1

/* Define to 1 if you have the `gmtime_s' function in W32 form. */
#define HAVE_W32_GMTIME_S 1

#if _MSC_VER >= 1900 /* snprintf() supported natively since VS2015 */
/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1
#endif


/* *** Headers information *** */
/* Not really important as not used by code currently */

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 0

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1


/* *** Other useful staff *** */

#define _GNU_SOURCE  1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1


/* End of MHD_config.h */
