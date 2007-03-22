#
# Makefile.am
# automake input file.
#
# copyright (c) 2006 by embrix.
# all rights reserved.
# developed by embrix for corrigent systems ltd.
#
# @author talba@embrix.com
# @since ??
#

SUBDIRS = tests

# example-mgmtfs contains recoursive symbolic links which fails "make dist"
#EXTRA_DIST = example-mgmtfs pedrosh.ggo
EXTRA_DIST = pedrosh.ggo \
	testsuite/config/unix.exp \
	testsuite/pedrosh.test/pedrosh.exp

params = \
	params-int.cc    params-int.h \
	params-string.cc params-string.h \
	params-ipv4.cc   params-ipv4.h \
	params-enum.cc   params-enum.h \
	params-ipv6.cc   params-ipv6.h \
	params-datetime.cc params-datetime.h

bin_PROGRAMS = pedrosh
pedrosh_SOURCES = \
	pedrosh.cc \
	pedrosh.h \
	pedrosh-i.h \
	named-pipes.cc \
	named-pipes.h \
	pedrosh-compl.cc \
	pedrosh-compl.h \
	pedrosh-getopt.cc \
	pedrosh-getopt.h \
	plugins.h \
	plugins.cc \
	debug.h \
	\
	plugin-stub.cc    plugin-stub.h \
	plugin-ipc.cc     plugin-ipc.h \
	plugin-globals.cc plugin-globals.h \
	\
	params.cc params.h \
	${params} \
	\
	utils.cc utils.h
	


include_HEADERS = pedrosh.h

lib_LIBRARIES = libpedro.a
libpedro_a_SOURCES = client.cc pedrosh.h pedrosh-i.h \
	utils.cc utils.h \
	${params} \
	params.cc   # TODO: this should not be here! but for now, every params-xxx.cc needs it.

noinst_PROGRAMS = example_task2

example_task2_SOURCES = example_task2.cc pedrosh.h
example_task2_LDADD = libpedro.a




# default localtion in the filesystem for the menu definitions
AM_CFLAGS =
AM_CFLAGS += -Wall
AM_CFLAGS += -DDEF_MGMTFS_PATH="\"$(localstatedir)/pedfs\""

AM_CXXFLAGS =
AM_CXXFLAGS += -Wall
AM_CXXFLAGS += -DDEF_MGMTFS_PATH="\"$(localstatedir)/pedfs\""

#AM_CXXFLAGS += -DPEDRO_DEBUG

if GENGETOPT_EXISTS
pedrosh-getopt.c: pedrosh.ggo
	$(GENGETOPT) --input=$< --file-name=pedrosh-getopt --unamed-opts --no-handle-help
endif

#
### DejaGNU tests ###
#

AUTOMAKE_OPTIONS = dejagnu
DEJATOOL = pedrosh
RUNTESTDEFAULTFLAGS = --tool $$tool  --srcdir $$srcdir/testsuite

