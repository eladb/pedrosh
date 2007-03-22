#!/bin/sh

# this script splits the source files into a directory tree more convenient to corrigent.
# please run this script from the source directory.

##################################
libpedro_src="client.c"
libpedro_inc=""
libpedro_ext_api="pedrosh.h"

pedrosh_src="pedrosh.c    \
	pedrosh-compl.c  \
	named-pipes.c    \
	pedrosh-getopt.c \
	plugins.c        \
	plugin-stub.c    \
	plugin-ipc.c     \
	plugin-globals.c"
pedrosh_inc="config.h     \
	debug.h          \
	pedrosh-i.h      \
	pedrosh-compl.h  \
	plugins.h        \
	plugin-stub.h    \
	plugin-ipc.h     \
	plugin-globals.h \
	named-pipes.h    \
	pedrosh-getopt.h"
pedrosh_ext_api=""
##################################

if [ ! -f pedrosh.c ]
then
	echo "please run this script from the source directory"
	exit -1
fi

echo "creating subdirectory 'corrigent'..."

###### pedrosh ######

files="$libpedro_src"
dst="corrigent/pedrosh/src"
if [ "x" != "x$files" ]; then
	mkdir -p  $dst
	cp $files $dst
fi

files="$libpedro_inc"
dst="corrigent/pedrosh/inc"
if [ "x" != "x$files" ]; then
	mkdir -p  $dst
	cp $files $dst
fi

files="$libpedro_ext_api"
dst="corrigent/pedrosh/ext_api"
if [ "x" != "x$files" ]; then
	mkdir -p  $dst
	cp $files $dst
fi


###### libpedro ######

files="$pedrosh_src"
dst="corrigent/libpedro/src"
if [ "x" != "x$files" ]; then
	mkdir -p  $dst
	cp $files $dst
fi

files="$pedrosh_inc"
dst="corrigent/libpedro/inc"
if [ "x" != "x$files" ]; then
	mkdir -p  $dst
	cp $files $dst
fi

files="$pedrosh_ext_api"
dst="corrigent/libpedro/ext_api"
if [ "x" != "x$files" ]; then
	mkdir -p  $dst
	cp $files $dst
fi


###### libtecla ######
##mkdir -p corrigent/libtecla
