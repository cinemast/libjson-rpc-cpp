prefix=${FULL_PATH_INSTALL_PREFIX}
exec_prefix=${FULL_PATH_EXEC_PREFIX}
includedir=${FULL_PATH_INCLUDEDIR}
libdir=${FULL_PATH_LIBDIR}

Name: libjson-rpc-cpp
Description: A C++ implementation of json-rpc.
Version: ${PROJECT_VERSION}
Requires: ${PKG_CONFIG_REQUIRES}
Libs: -L${FULL_PATH_LIBDIR} -ljsonrpccpp-common -ljsonrpccpp-server -ljsonrpccpp-client
Cflags: -I${FULL_PATH_INCLUDEDIR}
