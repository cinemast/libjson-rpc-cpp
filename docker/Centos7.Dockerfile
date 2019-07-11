FROM centos:7

ENV OS=centos7
RUN yum -y install epel-release
RUN yum -y install jsoncpp-devel libcurl-devel hiredis-devel redis cmake3 argtable-devel gcc-c++ wget gnutls-devel git gnutls-devel libgcrypt-devel
ENV MICROHTTPD_VERSION=0.9.37
RUN wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-${MICROHTTPD_VERSION}.tar.gz && tar xvz -f libmicrohttpd-${MICROHTTPD_VERSION}.tar.gz \
    && cd libmicrohttpd-${MICROHTTPD_VERSION} && ./configure && make -j$(nproc) && make install
RUN ln -sf /usr/bin/cmake3 /usr/bin/cmake
RUN mkdir /app
COPY docker/build_test_install.sh /app
COPY . /app
RUN chmod a+x /app/build_test_install.sh
RUN cd /app && ./build_test_install.sh