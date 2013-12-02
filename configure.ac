AC_PREREQ(2.57)
AC_INIT(libjsonrpccpp, 0.2.1, https://github.com/cinemast/libjson-rpc-cpp/issues)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([src/jsonrpc/rpc.h])
AC_PATH_PROG([BASH], [bash])
AC_PROG_CXX
AC_PROG_CC

AM_INIT_AUTOMAKE(foreign tar-ustar)
AM_CONFIG_HEADER([config.h])
LT_INIT


AC_ARG_ENABLE(dep-check,
  AS_HELP_STRING([--disable-dep-check],
  [Do not check for dependencies other then those required for dist target]))

AC_DEFUN([SF_MISSING_DEP],[
  if test "x$enable_dep_check" != "xno"; then
    AC_MSG_ERROR([$1])
  fi
])

AC_SUBST(MAJOR_VERSION, "0")
AC_SUBST(MINOR_VERSION, "2")

PACKAGE_REVISION=`cd ${srcdir} && ./revision-gen ${PACKAGE_VERSION}`
AC_DEFINE_UNQUOTED([PACKAGE_REVISION], "${PACKAGE_REVISION}", [Revion number including git SHA])
AC_SUBST(PACKAGE_REVISION)

AC_CHECK_HEADERS([curl/curl.h], [], [SF_MISSING_DEP("Curl library not found.  Install libcurl-devel.")])
AC_CHECK_LIB(curl, main, [CURL_LIBS="-lcurl"], [SF_MISSING_DEP("Curl library not found.  Install libcurl-devel.")])
AC_SUBST(CURL_LIBS)


AC_ARG_ENABLE([build-examples],
[  --enable-build-examples    Build Examples],
[case "${enableval}" in
  yes) build_example_src=true ;;
  no)  build_example_src=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-build-examples]) ;;
esac],[build_example_src=false])
AM_CONDITIONAL([BUILD_EXAMPLES], [test x$build_example_src = xtrue])


AC_CONFIG_FILES([
	src/Makefile
	src/test/Makefile
	src/example/Makefile
	src/stubgenerator/Makefile
	Makefile
	src/jsonrpc/version.h
	libjsonrpccpp.spec
])

AC_OUTPUT
