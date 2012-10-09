ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES = libx10.la
noinst_PROGRAMS = example raw

libx10_la_CFLAGS = -Wall -g ${DARWIN_CFLAGS}
libx10_la_LDFLAGS = -lusb ${DARWIN_CFLAGS}
libx10_la_SOURCES = \
	src/cm15a.c \
	src/libx10.c \
	src/libx10.h \
	src/libx10int.h

library_includedir="$(includedir)/${PACKAGE}"
library_include_HEADERS = \
	src/libx10.h

example_CFLAGS = -Wall -g
example_LDADD = libx10.la
example_SOURCES = \
	src/example.c

raw_CFLAGS = -Wall -g
raw_LDADD = libx10.la
raw_SOURCES = \
	src/raw.c

if X10_BUILD_ERLANG

erlanglib_LTLIBRARIES = erlx10.la

erlx10_la_LDFLAGS = -shared -module -avoid-version ${DARWIN_CFLAGS}
erlx10_la_CFLAGS = -Wall -g ${ERLANG_CFLAGS} ${DARWIN_CFLAGS}
erlx10_la_LIBADD = libx10.la
erlx10_la_SOURCES = \
	src/erlang/erlx10.c

erlanglib_DATA = src/erlang/erlx10.beam
CLEANFILES = $(beam_DATA) defines.hrl

src/erlang/erlx10.erl: defines.hrl

.erl.beam:
	$(ERLC) -W -o $(shell dirname $@) $<

defines.hrl: src/erlang/defines.h
	cat src/erlang/defines.h | grep \#define | awk '{print "-define(" $$2 ", " $$3 ")."}' > defines.hrl

endif

EXTRA_DIST = \
	src/erlang/erlx10.c \
	src/erlang/erlx10.erl
