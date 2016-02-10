# This file is part of libmicrohttpd.
# (C) 2006-2015 Christian Grothoff (and other contributing authors)
#
# libmicrohttpd is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation; either version 3, or (at your
# option) any later version.
#
# libmicrohttpd is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libmicrohttpd; see the file COPYING.  If not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.
#
#
# Process this file with autoconf to produce a configure script.
#
#
AC_PREREQ([2.60])
LT_PREREQ([2.4.0])
AC_INIT([libmicrohttpd],[0.9.48],[libmicrohttpd@gnu.org])
AM_INIT_AUTOMAKE([silent-rules] [subdir-objects])
AC_CONFIG_HEADERS([MHD_config.h])
AC_CONFIG_MACRO_DIR([m4])
AH_TOP([#define _GNU_SOURCE  1])

LIB_VERSION_CURRENT=48
LIB_VERSION_REVISION=0
LIB_VERSION_AGE=36
AC_SUBST(LIB_VERSION_CURRENT)
AC_SUBST(LIB_VERSION_REVISION)
AC_SUBST(LIB_VERSION_AGE)


AC_MSG_CHECKING([[whether z/OS special settings are required]])
if test `uname -s` = "OS/390"
then
# configure binaries for z/OS
  if test -z "$CC"
  then
    CC=`pwd`"/contrib/xcc"
    chmod +x $CC || true
  fi
  if test -z "$CPP"
  then
    CPP="c89 -E"
  fi
  if test -z "$CXXCPP"
  then
    CXXCPP="c++ -E -+"
  fi
  AC_MSG_RESULT([[yes]])
#  _CCC_CCMODE=1
#  _C89_CCMODE=1
else
  AC_MSG_RESULT([[no]])
fi

# Checks for programs.
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_CANONICAL_HOST
AC_PROG_CC_STDC
AS_IF([[test -n "$ac_cv_prog_cc_stdc" && test "x$ac_cv_prog_cc_stdc" != "xno"]],
      [[CFLAGS="$CFLAGS $ac_cv_prog_cc_stdc"]])
LT_INIT([win32-dll])
LT_LANG([Windows Resource])

PACKAGE_VERSION_MAJOR=${PACKAGE_VERSION%.*.*}
PACKAGE_VERSION_MINOR=${PACKAGE_VERSION%.*}; PACKAGE_VERSION_MINOR=${PACKAGE_VERSION_MINOR#*.}
PACKAGE_VERSION_SUBMINOR=${PACKAGE_VERSION#*.*.}
AC_SUBST([PACKAGE_VERSION_MAJOR])
AC_SUBST([PACKAGE_VERSION_MINOR])
AC_SUBST([PACKAGE_VERSION_SUBMINOR])
AC_CONFIG_FILES([src/microhttpd/microhttpd_dll_res.rc])

MHD_LIB_CPPFLAGS=""
MHD_LIB_CFLAGS=""
MHD_LIB_LDFLAGS=""
MHD_LIBDEPS=""
# for pkg-config
MHD_REQ_PRIVATE=''
MHD_LIBDEPS_PKGCFG=''

AC_ARG_WITH([threads],
   [AS_HELP_STRING([--with-threads=LIB],[choose threading library (posix, w32, auto) [auto]])],
   [], [with_threads='auto'])
test "x$with_threads" = "xwin32" && with_threads='w32'
test "x$with_threads" = "xpthreads" && with_threads='posix'

# Check for posix threads support
AX_PTHREAD([HAVE_POSIX_THREADS='yes'],[HAVE_POSIX_THREADS='no'])
AM_CONDITIONAL([HAVE_POSIX_THREADS],[test "x$HAVE_POSIX_THREADS" = "xyes"])
# Simple check for W32 threads support
AC_CHECK_HEADER([windows.h],
  [
    AC_MSG_CHECKING([for CreateThread()])
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([#include <windows.h>], [ HANDLE h = CreateThread(NULL, 0, NULL, NULL, 0, NULL);])],
      [
        AC_MSG_RESULT([yes])
        HAVE_W32_THREADS='yes'
      ],
      [
        AC_MSG_RESULT([no])
        HAVE_W32_THREADS='no'
      ])
  ],
  [HAVE_W32_THREADS='no'])

# Check system type
AC_MSG_CHECKING([[for target host OS]])
case "$host_os" in
*darwin* | *rhapsody* | *macosx*)
     AC_DEFINE_UNQUOTED(OSX,1,[This is an OS X system])
     CFLAGS="-no-cpp-precomp -fno-common $CFLAGS"
     mhd_host_os='Darwin'
     AC_MSG_RESULT([[$mhd_host_os]])
     ;;
freebsd*)
     AC_DEFINE_UNQUOTED(SOMEBSD,1,[This is a BSD system])
     AC_DEFINE_UNQUOTED(FREEBSD,1,[This is a FreeBSD system])
     mhd_host_os='FreeBSD'
     AC_MSG_RESULT([[$mhd_host_os]])
     ;;
openbsd*)
     AC_DEFINE_UNQUOTED(SOMEBSD,1,[This is a BSD system])
     AC_DEFINE_UNQUOTED(OPENBSD,1,[This is an OpenBSD system])
     mhd_host_os='OpenBSD'
     AC_MSG_RESULT([[$mhd_host_os]])
     ;;
netbsd*)
     AC_DEFINE_UNQUOTED(SOMEBSD,1,[This is a BSD system])
     AC_DEFINE_UNQUOTED(NETBSD,1,[This is a NetBSD system])
     mhd_host_os='NetBSD'
     AC_MSG_RESULT([[$mhd_host_os]])
     ;;
*solaris*)
     AC_DEFINE_UNQUOTED(SOLARIS,1,[This is a Solaris system])
     AC_DEFINE_UNQUOTED(_REENTRANT,1,[Need with solaris or errno doesnt work])
     mhd_host_os='Solaris'
     AC_MSG_RESULT([[$mhd_host_os]])
     AC_SEARCH_LIBS(gethostbyname, nsl)
     AC_SEARCH_LIBS(socket, socket)
     ;;
*arm-linux*)
     AC_DEFINE_UNQUOTED(LINUX,1,[This is a Linux kernel])
     AC_DEFINE_UNQUOTED(HAVE_LISTEN_SHUTDOWN,1,[can use shutdown on listen sockets])
     mhd_host_os='ARM Linux'
     AC_MSG_RESULT([[$mhd_host_os]])
     CFLAGS="-fPIC -pipe $CFLAGS"
     ;;
*linux*)
     AC_DEFINE_UNQUOTED(LINUX,1,[This is a Linux kernel])
     AC_DEFINE_UNQUOTED(HAVE_LISTEN_SHUTDOWN,1,[can use shutdown on listen sockets])
     mhd_host_os='Linux'
     AC_MSG_RESULT([[$mhd_host_os]])
     ;;
*cygwin*)
     AC_DEFINE_UNQUOTED(CYGWIN,1,[This is a Cygwin system])
     mhd_host_os='Windows (Cygwin)'
     AC_MSG_RESULT([[$mhd_host_os]])
     os_is_windows=yes
     ;;
*mingw*)
     AC_DEFINE_UNQUOTED(MINGW,1,[This is a MinGW system])
     AC_DEFINE_UNQUOTED(WINDOWS,1,[This is a Windows system])
     mhd_host_os='Windows (MinGW)'
     AC_MSG_RESULT([[$mhd_host_os]])
     LIBS="$LIBS -lws2_32"
     AC_CHECK_HEADERS([winsock2.h ws2tcpip.h],, AC_MSG_ERROR([[Winsock2 headers are required for W32]]))
     AC_CACHE_CHECK([for MS lib utility], [ac_cv_use_ms_lib_tool],
       [[mslibcheck=`lib 2>&1`
        if [[ $mslibcheck = "Microsoft (R) Library Manager"* ]]; then
          ac_cv_use_ms_lib_tool=yes
        else
          ac_cv_use_ms_lib_tool=no
        fi
         ]])
     if test "x$ac_cv_use_ms_lib_tool" = "xyes"; then
       AC_SUBST([MS_LIB_TOOL], [[lib]])
     fi
     AC_SUBST([lt_cv_objdir])
     os_is_windows=yes
     os_is_native_w32=yes
     ;;
*openedition*)
     AC_DEFINE_UNQUOTED(OS390,1,[This is a OS/390 system])
     mhd_host_os='OS/390'
     AC_MSG_RESULT([[$mhd_host_os]])
    ;;
*)
     mhd_host_os='unrecognised OS'
     AC_MSG_RESULT([[$mhd_host_os]])
     AC_MSG_WARN([Unrecognised OS $host_os])
     AC_DEFINE_UNQUOTED(OTHEROS,1,[Some strange OS])
#    You might want to find out if your OS supports shutdown on listen sockets,
#    and extend the switch statement; if we do not have 'HAVE_LISTEN_SHUTDOWN',
#    pipes are used instead to signal 'select'.
#    AC_DEFINE_UNQUOTED(HAVE_LISTEN_SHUTDOWN,1,[can use shutdown on listen sockets])
;;
esac

if test "x$with_threads" = "xposix"; then
# forced posix threads
  if test "x$HAVE_POSIX_THREADS" = "xyes"; then
    USE_THREADS='posix'
  else
    if test "x$HAVE_W32_THREADS" = "xyes"; then
      AC_MSG_ERROR([[Posix threads are not available. Try to configure --with-threads=auto]])
    else
      AC_MSG_ERROR([[Posix threads are not available]])
    fi
  fi
elif test "x$with_threads" = "xw32"; then
# forced w32 threads
  if test "x$HAVE_W32_THREADS" = "xyes"; then
    USE_THREADS='w32'
  else
    if test "x$HAVE_POSIX_THREADS" = "xyes"; then
      AC_MSG_ERROR([[W32 threads are not available. Try to configure --with-threads=auto]])
    else
      AC_MSG_ERROR([[W32 threads are not available]])
    fi
  fi
else
# automatic threads lib selection
  if test "x$HAVE_POSIX_THREADS" = "xyes" && test "x$HAVE_W32_THREADS" = "xyes"; then
    if test "x$os_is_native_w32" = "xyes"; then
      USE_THREADS='w32'
    else
      USE_THREADS='posix'
    fi
  elif test "x$HAVE_POSIX_THREADS" = "xyes"; then
    USE_THREADS='posix'
  elif test "x$HAVE_W32_THREADS" = "xyes"; then
    USE_THREADS='w32'
  else
    AC_MSG_ERROR([[No threading lib is available. Cosider installing pthreads]])
  fi
fi

if test "x$USE_THREADS" = "xposix"; then
  CC="$PTHREAD_CC"
  AC_DEFINE([MHD_USE_POSIX_THREADS],[1],[define to use pthreads])
  MHD_LIB_CFLAGS="$MHD_LIB_CFLAGS $PTHREAD_CFLAGS"
  MHD_LIBDEPS="$PTHREAD_LIBS $MHD_LIBDEPS"
  MHD_LIBDEPS_PKGCFG="$PTHREAD_LIBS $MHD_LIBDEPS_PKGCFG"
elif test "x$USE_THREADS" = "xw32"; then
  AC_DEFINE([MHD_USE_W32_THREADS],[1],[define to use W32 threads])
fi
AM_CONDITIONAL([USE_POSIX_THREADS], [test "x$USE_THREADS" = "xposix"])
AM_CONDITIONAL([USE_W32_THREADS], [test "x$USE_THREADS" = "xw32"])


AM_CONDITIONAL(HAVE_W32, [test "x$os_is_native_w32" = "xyes"])
w32_shared_lib_exp=no
if test "x$enable_shared" = "xyes" && test "x$os_is_native_w32" = "xyes"; then
  if test "x$ac_cv_use_ms_lib_tool" = "xyes" || test -n "$DLLTOOL"; then
    w32_shared_lib_exp=yes
  else
    AC_MSG_WARN([[GNU dlltool or MS lib.exe is required for creating shared library export on W32]])
    AC_MSG_WARN([[Export library libmicrohttpd.lib will not be created]])
  fi
fi
AM_CONDITIONAL(W32_SHARED_LIB_EXP, [test "x$w32_shared_lib_exp" = "xyes"])
AM_CONDITIONAL(USE_MS_LIB_TOOL, [test "x$ac_cv_use_ms_lib_tool" = "xyes"])

# set GCC options
# use '-fno-strict-aliasing', but only if the compiler
# and linker can take it
AX_CHECK_LINK_FLAG([-fno-strict-aliasing],
  [AX_APPEND_COMPILE_FLAGS([-fno-strict-aliasing])])

AC_C_BIGENDIAN

AC_CHECK_PROG([HAVE_CURL_BINARY],[curl],[yes],[no])
AM_CONDITIONAL([HAVE_CURL_BINARY],[test "x$HAVE_CURL_BINARY" = "xyes"])
AC_CHECK_PROG([HAVE_MAKEINFO_BINARY],[makeinfo],[yes],[no])
AM_CONDITIONAL([HAVE_MAKEINFO_BINARY],[test "x$HAVE_MAKEINFO_BINARY" = "xyes"])
AM_CONDITIONAL(W32_STATIC_LIB, [test "x$os_is_native_w32" = "xyes"  && test "x$enable_static" = "xyes"])


AC_ARG_ENABLE([[doc]],
  [AS_HELP_STRING([[--disable-doc]], [do not build any documentation])], ,
    [enable_doc=yes])
test "x$enable_doc" = "xno" || enable_doc=yes
AM_CONDITIONAL([BUILD_DOC], [test "x$enable_doc" = "xyes"])

AC_ARG_ENABLE([[examples]],
  [AS_HELP_STRING([[--disable-examples]], [do not build any examples])], ,
    [enable_examples=yes])
test "x$enable_examples" = "xno" || enable_examples=yes
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x$enable_examples" = "xyes"])

AC_ARG_ENABLE([[poll]],
  [AS_HELP_STRING([[--enable-poll[=ARG]]], [enable poll support (yes, no, auto) [auto]])],
    [enable_poll=${enableval}],
    [enable_poll='auto']
  )

if test "$enable_poll" != "no"; then
  if test "$os_is_native_w32" != "yes"; then
    AC_CHECK_HEADERS([poll.h],
      [
        AC_CHECK_FUNCS([poll], [have_poll='yes'], [have_poll='no'])
      ])
  else
    AC_MSG_CHECKING([for WSAPoll()])
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[#include <winsock2.h>]], [[
WSAPOLLFD fda[2];
WSAPoll(fda, 2, 0);]])],
        [
          have_poll='yes'
          AC_DEFINE([HAVE_POLL],[1])
        ], [have_poll='no'])
    AC_MSG_RESULT([$have_poll])
  fi
  if test "$enable_poll" = "yes" && test "$have_poll" != "yes"; then
    AC_MSG_ERROR([[Support for poll was explicitly requested but cannot be enabled on this platform.]])
  fi
  enable_poll="$have_poll"
fi

AC_ARG_ENABLE([[epoll]],
  [AS_HELP_STRING([[--enable-epoll[=ARG]]], [enable epoll support (yes, no, auto) [auto]])],
    [enable_epoll=${enableval}],
    [enable_epoll='auto']
  )

if test "$enable_epoll" != "no"; then
  AX_HAVE_EPOLL
  if test "${ax_cv_have_epoll}" = "yes"; then
    AC_DEFINE([EPOLL_SUPPORT],[1],[define to 1 to enable epoll support])
    enable_epoll='yes'
  else
    AC_DEFINE([EPOLL_SUPPORT],[0],[define to 0 to disable epoll support])
    if test "$enable_epoll" = "yes"; then
      AC_MSG_ERROR([[Support for epoll was explicitly requested but cannot be enabled on this platform.]])
    fi
    enable_epoll='no'
  fi
fi

if test "x$enable_epoll" = "xyes"; then
  AC_CACHE_CHECK([for epoll_create1()], [mhd_cv_have_epoll_create1], [
    AC_LINK_IFELSE([
      AC_LANG_PROGRAM([[#include <sys/epoll.h>]], [[
int fd;
fd = epoll_create1(EPOLL_CLOEXEC);]])],
      [mhd_cv_have_epoll_create1=yes],
      [mhd_cv_have_epoll_create1=no])])
  AS_IF([test "x$mhd_cv_have_epoll_create1" = "xyes"],[
    AC_DEFINE([[HAVE_EPOLL_CREATE1]], [[1]], [Define if you have epoll_create1 function.])])
fi

if test "x$HAVE_POSIX_THREADS" = "xyes"; then
  # Check for pthread_setname_np()
  SAVE_LIBS="$LIBS"
  SAVE_CFLAGS="$CFLAGS"
  LIBS="$PTHREAD_LIBS $LIBS"
  CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
  AC_MSG_CHECKING([[for pthread_setname_np]])
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([[#include <pthread.h>]], [[  pthread_setname_np(pthread_self(), "name")]])],
    [AC_DEFINE([[HAVE_PTHREAD_SETNAME_NP]], [[1]], [Define if you have pthread_setname_np function.])
     AC_MSG_RESULT([[yes]])],
    [AC_MSG_RESULT([[no]])] )
  LIBS="$SAVE_LIBS"
  CFLAGS="$SAVE_CFLAGS"
fi

# Check for headers that are ALWAYS required
AC_CHECK_HEADERS([fcntl.h math.h errno.h limits.h stdio.h locale.h sys/stat.h sys/types.h pthread.h],,AC_MSG_ERROR([Compiling libmicrohttpd requires standard UNIX headers files]))

# Check for optional headers
AC_CHECK_HEADERS([sys/types.h sys/time.h sys/msg.h netdb.h netinet/in.h netinet/tcp.h time.h sys/socket.h sys/mman.h arpa/inet.h sys/select.h search.h endian.h machine/endian.h sys/endian.h sys/param.h sys/machine.h sys/byteorder.h machine/param.h sys/isa_defs.h])
AM_CONDITIONAL([HAVE_TSEARCH], [test "x$ac_cv_header_search_h" = "xyes"])

AC_CHECK_MEMBER([struct sockaddr_in.sin_len],
   [ AC_DEFINE(HAVE_SOCKADDR_IN_SIN_LEN, 1, [Do we have sockaddr_in.sin_len?])
   ],
   [],
   [
    #ifdef HAVE_SYS_TYPES_H
      #include <sys/types.h>
    #endif
    #ifdef HAVE_SYS_SOCKET_H
      #include <sys/socket.h>
    #endif
    #ifdef HAVE_NETINET_IN_H
      #include <netinet/in.h>
    #endif
   ])


# Check for pipe/socketpair signaling
AC_MSG_CHECKING([[whether to enable signaling by socketpair]])

AC_ARG_ENABLE([[socketpair]],
	[AS_HELP_STRING([[--enable-socketpair[=ARG]]], [disable internal singalling by pipes and use socket pair instead (yes, no, try) [no]])], ,
	[AS_IF([[test "x$os_is_windows" = "xyes"]], [enable_socketpair=yes], [enable_socketpair=no])]
  )

AS_IF(
       [[test "x$enable_socketpair" != "xno"]],
         [AS_IF([[test "x$os_is_windows" = "xyes"]],
           [ AC_MSG_RESULT([[yes, forced on W32]]) ],
           [ AC_LINK_IFELSE(
             [ AC_LANG_PROGRAM([[
				#ifdef HAVE_SYS_TYPES_H
				#include <sys/types.h>
				#endif
				#ifdef HAVE_SYS_SOCKET_H
				#include <sys/socket.h>
				#endif
				]],[[
				  int sv[2];
				  if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) != 0) return 1
				]])
             ],
             [ AC_MSG_RESULT([[yes, socketpair in available]]) ],
             [ AC_MSG_RESULT([[no, socketpair in not available]])
              AS_IF([[test "x$enable_socketpair" = "xyes"]], [ AC_MSG_ERROR([[socketpair signalling cannot be enabled.]]) ])
             ]
             )
           ]
          )
         ],
       [
        AC_MSG_RESULT([[no]])
        AS_IF([[test "x$os_is_windows" = "xyes"]], [ AC_MSG_ERROR([[socketpair must be enabled on W32]]) ])
       ]
     )
if test "x$enable_socketpair" = "xyes"; then
	AC_DEFINE([[MHD_DONT_USE_PIPES]], [[1]], [Define to use pair of sockets instead of pipes for signaling])
fi

AC_CHECK_FUNCS_ONCE([accept4 gmtime_r memmem snprintf])
AC_CHECK_DECL([gmtime_s],
  [
    AC_MSG_CHECKING([[whether gmtime_s is in C11 form]])
    AC_LINK_IFELSE(
        [ AC_LANG_PROGRAM(
          [[ #define __STDC_WANT_LIB_EXT1__ 1
             #include <time.h>
             #ifdef __cplusplus
             extern "C"
             #endif
             struct tm* gmtime_s(const time_t* time, struct tm* result);
           ]], [[ 
             struct tm res;
             time_t t;
             gmtime_s (&t, &res);
          ]])
        ],
        [
          AC_DEFINE([HAVE_C11_GMTIME_S], [1], [Define to 1 if you have the `gmtime_s' function in C11 form.])
          AC_MSG_RESULT([[yes]])
        ],
        [
          AC_MSG_RESULT([[no]])
          AC_MSG_CHECKING([[whether gmtime_s is in W32 form]])
          AC_LINK_IFELSE(
            [ AC_LANG_PROGRAM(
              [[ #include <time.h>
                 #ifdef __cplusplus
                 extern "C"
                 #endif
                 errno_t gmtime_s(struct tm* _tm, const time_t* time);
              ]], [[ 
                 struct tm res;
                 time_t t;
                 gmtime_s (&res, &t);
              ]])
            ],
            [
              AC_DEFINE([HAVE_W32_GMTIME_S], [1], [Define to 1 if you have the `gmtime_s' function in W32 form.])
              AC_MSG_RESULT([[yes]])
            ],
            [AC_MSG_RESULT([[no]])
            ])
        ])
  ], [], 
  [[#define __STDC_WANT_LIB_EXT1__ 1
    #include<time.h>]])


AC_CHECK_DECLS([SOCK_NONBLOCK], [AC_DEFINE([HAVE_SOCK_NONBLOCK], [1], [SOCK_NONBLOCK is defined in a socket header])], [],
                   [
                    #if defined HAVE_SYS_TYPES_H
                    #  include <sys/types.h>
                    #endif
                    #if defined HAVE_SYS_SOCKET_H
                    #  include <sys/socket.h>
                    #elif defined HAVE_WINSOCK2_H
                    #  include <winsock2.h>
                    #endif
                   ])


AC_SEARCH_LIBS([clock_gettime], [rt], [
				AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Have clock_gettime])
])

AC_MSG_CHECKING([[for clock_get_time]])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(
    [[
      #include <mach/clock.h>
      #include <mach/mach.h>
    ]],
    [[
      clock_serv_t cs;
      mach_timespec_t mt;
      host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cs);
      clock_get_time(cs, &mt);
      mach_port_deallocate(mach_task_self(), cs);
    ]])
  ],
  [
    AC_DEFINE([HAVE_CLOCK_GET_TIME], [1], [Define to 1 if you have `clock_get_time', `host_get_clock_service' and `mach_port_deallocate' functions.])
    AC_MSG_RESULT([[yes]])
  ],
  [AC_MSG_RESULT([[no]])
  ])

AC_MSG_CHECKING([[for gethrtime]])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(
    [[#include <sys/time.h>]], [[hrtime_t hrt = gethrtime(); ]])
  ],
  [
    AC_DEFINE([HAVE_GETHRTIME], [1], [Define to 1 if you have `gethrtime' function.])
    AC_MSG_RESULT([[yes]])
  ],
  [AC_MSG_RESULT([[no]])
  ])

# IPv6
AC_MSG_CHECKING(for IPv6)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
#if HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
#endif
]], [[
int af=AF_INET6;
int pf=PF_INET6;
struct sockaddr_in6 sa;
printf("%d %d %p\n", af, pf, &sa);
]])],[
have_inet6=yes;
AC_DEFINE([HAVE_INET6], [1], [Provides IPv6 headers])
],[
have_inet6=no
])
AC_MSG_RESULT($have_inet6)

HIDDEN_VISIBILITY_CFLAGS=""
case "$host" in
  *-*-mingw*)
    dnl on mingw32 we do -fvisibility=hidden and __declspec(dllexport)
    AC_DEFINE([_MHD_EXTERN], [__attribute__((visibility("default"))) __declspec(dllexport) extern],
              [defines how to decorate public symbols while building])
    HIDDEN_VISIBILITY_CFLAGS="-fvisibility=hidden"
    ;;
  *)
    dnl on other compilers, check if we can do -fvisibility=hidden
    AX_CHECK_LINK_FLAG([-fvisibility=hidden],
      [AX_CHECK_COMPILE_FLAG([-fvisibility=hidden],
                           [AC_DEFINE([_MHD_EXTERN], [__attribute__((visibility("default"))) extern],
                                       [defines how to decorate public symbols while building])
                            HIDDEN_VISIBILITY_CFLAGS="-fvisibility=hidden"])])
    ;;
esac
AC_SUBST(HIDDEN_VISIBILITY_CFLAGS)

# libcurl (required for testing)
AC_ARG_ENABLE([curl],
  [AS_HELP_STRING([--disable-curl],[disable cURL based testcases])],
  [enable_curl=${enableval}])
curl=0
if test "$enable_curl" != "no"
then
  LIBCURL_CHECK_CONFIG([yes],[7.16.4],[enable_curl=yes],
    [
      if test "x$enable_curl" = "xyes"; then
        AC_MSG_WARN([[cURL-based tests cannot be enabled because libcurl is missing]])
      fi
      enable_curl=no
    ])
fi
if test "$enable_curl" != "no"
then
# Lib cURL & cURL - OpenSSL versions
  AC_DEFINE([MHD_REQ_CURL_VERSION], ["7.16.4"], [required cURL version to run tests])
  AC_DEFINE([MHD_REQ_CURL_OPENSSL_VERSION], ["0.9.8"], [required cURL SSL version to run tests])
  AC_DEFINE([MHD_REQ_CURL_GNUTLS_VERSION], ["2.8.6"], [gnuTLS lib version - used in conjunction with cURL])
  AC_DEFINE([MHD_REQ_CURL_NSS_VERSION], ["3.12.0"], [NSS lib version - used in conjunction with cURL])
fi
AM_CONDITIONAL([HAVE_CURL], [test "x$enable_curl" = "xyes"])

AC_CHECK_LIB([[magic]], [[magic_open]],
  [AC_CHECK_HEADERS([magic.h],
   AM_CONDITIONAL(HAVE_MAGIC, true),
   AM_CONDITIONAL(HAVE_MAGIC, false))],
  AM_CONDITIONAL(HAVE_MAGIC, false))


# large file support (> 4 GB)
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_CHECK_FUNCS([_lseeki64 lseek64 sendfile64])

# optional: have error messages ?
AC_MSG_CHECKING(whether to generate error messages)
AC_ARG_ENABLE([messages],
   [AS_HELP_STRING([--disable-messages],
               [disable MHD error messages])],
   [enable_messages=${enableval}],
   [enable_messages=yes])
AC_MSG_RESULT($enable_messages)
if test "$enable_messages" = "yes"
then
 AC_DEFINE([HAVE_MESSAGES],[1],[Include error messages])
else
 AC_DEFINE([HAVE_MESSAGES],[0],[Disable error messages])
fi


# optional: have postprocessor?
AC_MSG_CHECKING(whether to enable postprocessor)
AC_ARG_ENABLE([postprocessor],
   [AS_HELP_STRING([--disable-postprocessor],
               [disable MHD PostProcessor functionality])],
   [enable_postprocessor=${enableval}],
   [enable_postprocessor=yes])
test "x$enable_postprocessor" = "xno" || enable_postprocessor=yes
AC_MSG_RESULT([[$enable_postprocessor]])
AM_CONDITIONAL([HAVE_POSTPROCESSOR],test "x$enable_postprocessor" != "xno")
if test "x$enable_postprocessor" != "xno"
then
 AC_DEFINE([HAVE_POSTPROCESSOR],[1],[define to 1 if MHD was build with postprocessor.c])
fi


# optional: have zzuf, socat?
AC_CHECK_PROG([have_zzuf],[zzuf], [yes], [no])
AC_CHECK_PROG([have_socat],[socat], [yes], [no])
AM_CONDITIONAL([HAVE_ZZUF], [test "x$have_zzuf" = "xyes"])
AM_CONDITIONAL([HAVE_SOCAT], [test "x$have_socat" = "xyes"])


# libgcrypt linkage: required for HTTPS support
AM_PATH_LIBGCRYPT([1.2.2], [have_gcrypt=yes], [have_gcrypt=no])
if test "x$have_gcrypt" = "xyes"
then
  SAVE_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS $LIBGCRYPT_CFLAGS"
  # LIBGCRYPT_CFLAGS can be actually a CPPFLAGS, so check them both
  SAVE_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $LIBGCRYPT_CFLAGS"
  AC_CHECK_HEADERS([gcrypt.h], [], [have_gcrypt=no])
  CFLAGS="$SAVE_CFLAGS"
  CPPFLAGS="$SAVE_CPPFLAGS"
fi

# gnutls
GNUTLS_CPPFLAGS=""
GNUTLS_LDFLAGS=""
have_gnutls=no
have_gnutls_sni=no
have_gnutls_pkgcfg=no
AC_MSG_CHECKING([[how to find GnuTLS library]])
AC_ARG_WITH([[gnutls]],
   [AS_HELP_STRING([[--with-gnutls[=PFX]]],[use GnuTLS for HTTPS support, optional PFX overrides pkg-config data for GnuTLS headers (PFX/include) and libs (PFX/lib)])],
   [
    case $with_gnutls in
      no)
        AC_MSG_RESULT([[GnuTLS disabled]])
        ;;
      yes)
        AC_MSG_RESULT([[automatically, forced]])
        ;;
      *)
        AC_MSG_RESULT([[-I$with_gnutls/include -L$with_gnutls/lib -lgnutls]])
        SAVE_LDFLAGS="$LDFLAGS"
        SAVE_CPPFLAGS="$CPPFLAGS"
        LDFLAGS="-L$with_gnutls/lib $LDFLAGS"
        CPPFLAGS="-I$with_gnutls/include $CPPFLAGS"
        have_gnutls_pkgcfg=no
          AC_CHECK_HEADERS([gnutls/gnutls.h],
            [AC_CHECK_LIB([gnutls], [gnutls_priority_set],
              [
                GNUTLS_CPPFLAGS="-I$with_gnutls/include"
                GNUTLS_LDFLAGS="-L$with_gnutls/lib"
                GNUTLS_LIBS="-lgnutls"
                AC_CHECK_LIB([gnutls], [gnutls_load_file], [AC_CHECK_LIB([gnutls], [gnutls_privkey_import_x509_raw], [have_gnutls_sni=yes])])
                have_gnutls=yes
              ])])
        AS_IF([test "x$have_gnutls" != "xyes"], [AC_MSG_ERROR([can't find usable libgnutls at specified prefix $with_gnutls])])
        LDFLAGS="$SAVE_LDFLAGS"
        CPPFLAGS="$SAVE_CPPFLAGS"
        ;;
    esac
   ],
   [AC_MSG_RESULT([[automatically]])
   ])

AS_IF([test "x$with_gnutls" != "xno" && test "x$have_gnutls" != "xyes"],
  [
    PKG_CHECK_MODULES(GNUTLS, [[gnutls]],
      [
       have_gnutls_pkgcfg='yes'
       SAVE_CPPFLAGS="$CPPFLAGS"
       SAVE_CFLAGS="$CFLAGS"
       SAVE_LDFLAGS="$LDFLAGS"
       SAVE_LIBS="$LIBS"
       CPPFLAGS="$GNUTLS_CFLAGS $CPPFLAGS"
       CFLAGS="$GNUTLS_CFLAGS $CFLAGS"
       LDFLAGS="$GNUTLS_LIBS $LDFLAGS"
       LIBS="$LIBS $GNUTLS_LIBS"
       AC_MSG_CHECKING([[whether GnuTLS is usable]])
       AC_LINK_IFELSE([
         AC_LANG_PROGRAM([[#include <gnutls/gnutls.h>]], [[
                gnutls_session_t session;
                gnutls_priority_t priorities;
                gnutls_global_init();
                gnutls_priority_init(&priorities, "NORMAL", NULL);
                gnutls_init(&session, GNUTLS_SERVER);
                gnutls_priority_set(session, priorities);
          ]])],
          [
           AC_MSG_RESULT([[yes]])
           have_gnutls=yes
           GNUTLS_CPPLAGS="$GNUTLS_CFLAGS"
           GNUTLS_LDFLAGS="$GNUTLS_LIBS"
           AC_MSG_CHECKING([[for gnutls_privkey_import_x509_raw()]])
           AC_LINK_IFELSE([
             AC_LANG_PROGRAM([[#include <gnutls/gnutls.h>]], [[
                    gnutls_datum_t data;
                    gnutls_privkey_t key;
                    gnutls_load_file("key.pem", &data);
                    gnutls_privkey_import_x509_raw(key, &data, GNUTLS_X509_FMT_PEM, NULL, 0);
                    gnutls_free(data.data);
                  ]])], [[have_gnutls_sni=yes]], [[have_gnutls_sni=no]])
           AC_MSG_RESULT([[$have_gnutls_sni]])
          ],
          [
           AC_MSG_RESULT([[no]])
           have_gnutls=no
          ])

       AS_IF([test "x$have_gnutls" != "xyes"], [AC_MSG_WARN([pkg-config reports that GnuTLS is present, but GnuTLS can't be used])])
       CPPFLAGS="$SAVE_CPPFLAGS"
       CFLAGS="$SAVE_CFLAGS"
       LDFLAGS="$SAVE_LDFLAGS"
       LIBS="$SAVE_LIBS"
      ],
      [
       have_gnutls_pkgcfg='no'
       AC_CHECK_HEADERS([gnutls/gnutls.h],
        [AC_CHECK_LIB([gnutls], [gnutls_priority_set],
          [
            GNUTLS_LIBS="-lgnutls"
            AC_CHECK_LIB([gnutls], [gnutls_load_file], [AC_CHECK_LIB([gnutls], [gnutls_privkey_import_x509_raw], [have_gnutls_sni=yes])])
            have_gnutls=yes
          ])])
      ])
  ])

AS_IF([test "x$have_gnutls" != "xyes" && test "x$with_gnutls" = "xyes"], [AC_MSG_ERROR([[can't find usable libgnutls]])])

AM_CONDITIONAL(HAVE_GNUTLS, test "x$have_gnutls" = "xyes")
AM_CONDITIONAL([HAVE_GNUTLS_SNI], [test "x$have_gnutls_sni" = "xyes"])

AC_SUBST([GNUTLS_CPPFLAGS])
AC_SUBST([GNUTLS_CFLAGS])
AC_SUBST([GNUTLS_LDFLAGS])
AC_SUBST([GNUTLS_LIBS])

# optional: HTTPS support.  Enabled by default
AC_MSG_CHECKING(whether to support HTTPS)
AC_ARG_ENABLE([https],
   [AS_HELP_STRING([--enable-https],
               [enable HTTPS support (yes, no, auto)[auto]])],
   [enable_https=${enableval}])
if test "x$enable_https" != "xno"
then
  AS_IF([test "x$have_gnutls" = "xyes" && test "x$have_gcrypt" = "xyes"], [
          AC_DEFINE([HTTPS_SUPPORT],[1],[include HTTPS support])
          enable_https=yes
          MSG_HTTPS="yes (using libgnutls and libgcrypt)"
          MHD_LIB_CPPFLAGS="$MHD_LIB_CPPFLAGS $LIBGCRYPT_CFLAGS $GNUTLS_CPPFLAGS"
          MHD_LIB_CFLAGS="$MHD_LIB_CFLAGS $LIBGCRYPT_CFLAGS $GNUTLS_CFLAGS"
          MHD_LIB_LDFLAGS="$MHD_LIB_LDFLAGS $GNUTLS_LDFLAGS"
          MHD_LIBDEPS="$GNUTLS_LIBS $LIBGCRYPT_LIBS $MHD_LIBDEPS"
          AS_IF([[ test "x$have_gnutls_pkgcfg" = "xyes" ]],
            [ # remove GnuTLS from private libs in .pc file as it defined in Requires.private
              MHD_REQ_PRIVATE='gnutls'
              MHD_LIBDEPS_PKGCFG="$LIBGCRYPT_LIBS $MHD_LIBDEPS_PKGCFG"
            ],
            [
              MHD_REQ_PRIVATE=''
              MHD_LIBDEPS_PKGCFG="$GNUTLS_LIBS $LIBGCRYPT_LIBS $MHD_LIBDEPS_PKGCFG"
          ])
        ], [
          AS_IF([test "x$have_gnutls" = "xyes"], [crypt_missing="libgrypt"],
                [test "x$have_gcrypt" = "xyes"], [crypt_missing="libgnutls"],
                                                 [crypt_missing="libgrypt and libgnutls"])
          AS_IF([[test "x$enable_https" = "xyes" ]], [AC_MSG_ERROR([[HTTPS support cannot be enabled without $crypt_missing.]])])
          AC_DEFINE([HTTPS_SUPPORT],[0],[no libgcrypt or libgnutls])
          enable_https=no
          MSG_HTTPS="no (lacking $crypt_missing)"
        ])
else
  AC_DEFINE([HTTPS_SUPPORT],[0],[disable HTTPS support])
  MSG_HTTPS="no (disabled)"
fi
AC_MSG_RESULT([$MSG_HTTPS])

AM_CONDITIONAL([ENABLE_HTTPS], [test "x$enable_https" = "xyes"])

# optional: HTTP Basic Auth support. Enabled by default
AC_MSG_CHECKING(whether to support HTTP basic authentication)
AC_ARG_ENABLE([bauth],
		AS_HELP_STRING([--disable-bauth],
			[disable HTTP basic Auth support]),
		[enable_bauth=${enableval}],
		[enable_bauth=yes])
if test "x$enable_bauth" != "xno"
then
 enable_bauth=yes
 AC_DEFINE([BAUTH_SUPPORT],[1],[include basic Auth support])
else
 AC_DEFINE([BAUTH_SUPPORT],[0],[disable basic Auth support])
fi
AC_MSG_RESULT($enable_bauth)
AM_CONDITIONAL(ENABLE_BAUTH, [test "x$enable_bauth" != "xno"])

# optional: HTTP Digest Auth support. Enabled by default
AC_MSG_CHECKING(whether to support HTTP digest authentication)
AC_ARG_ENABLE([dauth],
		AS_HELP_STRING([--disable-dauth],
			[disable HTTP basic and digest Auth support]),
		[enable_dauth=${enableval}],
		[enable_dauth=yes])
if test "x$enable_dauth" != "xno"
then
 enable_dauth=yes
 AC_DEFINE([DAUTH_SUPPORT],[1],[include digest Auth support])
else
 AC_DEFINE([DAUTH_SUPPORT],[0],[disable digest Auth support])
fi
AC_MSG_RESULT($enable_dauth)
AM_CONDITIONAL(ENABLE_DAUTH, [test "x$enable_dauth" != "xno"])



MHD_LIB_LDFLAGS="$MHD_LIB_LDFLAGS -export-dynamic -no-undefined"

# gcov compilation
AC_MSG_CHECKING(whether to compile with support for code coverage analysis)
AC_ARG_ENABLE([coverage],
              AS_HELP_STRING([--enable-coverage],
                             [compile the library with code coverage support]),
              [use_gcov=${enableval}],
              [use_gcov=no])
AC_MSG_RESULT($use_gcov)
AM_CONDITIONAL([USE_COVERAGE], [test "x$use_gcov" = "xyes"])

AX_COUNT_CPUS
AC_SUBST([CPU_COUNT])

AC_SUBST(MHD_LIB_CPPFLAGS)
AC_SUBST(MHD_LIB_CFLAGS)
AC_SUBST(MHD_LIB_LDFLAGS)
AC_SUBST(MHD_LIBDEPS)

# for pkg-config
AC_SUBST([MHD_REQ_PRIVATE])
AC_SUBST([MHD_LIBDEPS_PKGCFG])

AC_SUBST(CPPFLAGS)
AC_SUBST(LIBS)
AC_SUBST(LDFLAGS)

AC_CONFIG_FILES([
libmicrohttpd.pc
w32/common/microhttpd_dll_res_vc.rc
Makefile
contrib/Makefile
doc/Makefile
doc/doxygen/Makefile
doc/examples/Makefile
m4/Makefile
src/Makefile
src/include/Makefile
src/platform/Makefile
src/microhttpd/Makefile
src/examples/Makefile
src/testcurl/Makefile
src/testcurl/https/Makefile
src/testzzuf/Makefile])
AC_OUTPUT

# Finally: summary
if test "x$enable_curl" != "xyes"; then
 MSG_CURL="no, many unit tests will not run"
else
 MSG_CURL="yes"
fi

AC_MSG_NOTICE([libmicrohttpd ${PACKAGE_VERSION} Configuration Summary:
  Cross-compiling:   ${cross_compiling}
  Operating System:  ${host_os}
  Threading lib:     ${USE_THREADS}
  libcurl (testing): ${MSG_CURL}
  Target directory:  ${prefix}
  Messages:          ${enable_messages}
  Basic auth.:       ${enable_bauth}
  Digest auth.:      ${enable_dauth}
  Postproc:          ${enable_postprocessor}
  HTTPS support:     ${MSG_HTTPS}
  poll support:      ${enable_poll=no}
  epoll support:     ${enable_epoll=no}
  build docs:        ${enable_doc}
  build examples:    ${enable_examples}
])

if test "x$enable_https" = "xyes"
then
 AC_MSG_NOTICE([HTTPS subsystem configuration:
  License         :  LGPL only
 ])
else
 AC_MSG_NOTICE([
  License         :  LGPL or eCos
])
fi

if test "x$enable_bauth" != "xyes" -o \
        "x$enable_dauth" != "xyes" -o \
        "x$enable_postprocessor" != "xyes"
then
 AC_MSG_NOTICE([WARNING: This will be a custom build with missing symbols. Do NOT use this build in a distribution. Building with these kinds of configure options is only for custom builds for embedded systems.])
fi
