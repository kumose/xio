AUTOMAKE_OPTIONS = subdir-objects

if SEPARATE_COMPILATION
noinst_LIBRARIES = libxio.a
libxio_a_SOURCES = ../../xio.cpp
if HAVE_OPENSSL
libxio_a_SOURCES += ../../xio_ssl.cpp
endif
LDADD = libxio.a
endif

if HAVE_COROUTINES
noinst_PROGRAMS = \
	coroutines_ts/chat_server \
	coroutines_ts/echo_server \
	coroutines_ts/echo_server_with_default \
	coroutines_ts/echo_server_with_as_single_default \
	coroutines_ts/echo_server_with_as_tuple_default \
	coroutines_ts/refactored_echo_server
endif

AM_CXXFLAGS = -I$(srcdir)/../../../include

if HAVE_COROUTINES
coroutines_ts_chat_server_SOURCES = coroutines_ts/chat_server.cpp
coroutines_ts_echo_server_SOURCES = coroutines_ts/echo_server.cpp
coroutines_ts_echo_server_with_default_SOURCES = coroutines_ts/echo_server_with_default.cpp
coroutines_ts_echo_server_with_as_single_default_SOURCES = coroutines_ts/echo_server_with_as_single_default.cpp
coroutines_ts_echo_server_with_as_tuple_default_SOURCES = coroutines_ts/echo_server_with_as_tuple_default.cpp
coroutines_ts_refactored_echo_server_SOURCES = coroutines_ts/refactored_echo_server.cpp
endif

EXTRA_DIST = \
	coroutines_ts/range_based_for.cpp

MAINTAINERCLEANFILES = \
	$(srcdir)/Makefile.in
