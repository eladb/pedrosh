EXTRA_DIST =

# a list of programs to run in order to do the testing
TESTS = params_test
check_SCRIPTS = 
check_PROGRAMS = utcompl params_test test_client

bin_PROGRAMS = retester

AM_CFLAGS =
AM_CFLAGS += -I..
AM_CFLAGS += -Wall

retester_SOURCES = retester.c

params_files = \
	../params.cc \
	../params-int.cc \
	../params-string.cc \
	../params-ipv4.cc \
	../params-enum.cc \
	../params-ipv6.cc \
	../params-datetime.cc

utcompl_SOURCES = utcompl.cc ${params_files}
utcompl_LDADD = \
	../pedrosh-getopt.o \
	../pedrosh-compl.o \
	../named-pipes.o \
	../plugin-ipc.o \
	../plugin-stub.o \
	../plugins.o \
	../utils.o

params_test_SOURCES = \
	params_test.cc \
	../utils.cc \
	${params_files}

test_client_SOURCES = test_client.cc
test_client_LDADD = ../libpedro.a
