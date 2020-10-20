#!/bin/bash -x
export LDFLAGS=-L$(pwd)/../build/lib -static
export LIBS=-lwolfssl
export CFLAGS=-static
export LT_SYS_LIBRARY_PATH=$(pwd)/../build/lib
./configure --prefix=$(pwd)/../build \
			--libdir=$(pwd)/../build/lib \
			--includedir=$(pwd)/../build/include \
			--datadir=$(pwd)/../build/docs \
			--datarootdir=$(pwd)/../build/docs \
			--disable-shared \
			--enable-static \
			--disable-dependency-tracking \
			--disable-ipv6 \
			--disable-ftp \
			--disable-file \
			--disable-ldap \
			--disable-ldaps \
			--disable-rtsp \
			--disable-proxy \
			--disable-dict \
			--disable-telnet \
			--disable-tftp \
			--disable-pop3 \
			--disable-imap \
			--disable-smtp \
			--disable-gopher \
			--disable-sspi \
			--disable-manual \
			--disable-zlib \
			--without-zlib \
			--disable-thread \
			--disable-threaded-resolver \
			--without-ssl \
			--disable-optimize \
			--disable-cookies \
			--disable-crypto-auth \
			--disable-manul \
			--disable-proxy \
			--disable-unix-sockets \
			--disable-doh \
			--without-libidn \
			--without-librtmp \
			--disable-dnsshuffle \
			--with-wolfssl=$(pwd)/../build \
			--disable-verbose

make curl_LDFLAGS=-all-static -j $(nproc)
make install
