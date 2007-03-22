#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT([embrix-pedro], [1.1], [https://launchpad.net/products/pedro/+bugs])
AC_CONFIG_SRCDIR(pedrosh.h)
# The new AM_INIT_AUTOMAKE, requires no parameters. It gets the the package
# name and version from variables set by AC_INIT.
# So the next line should have been:
#   AM_INIT_AUTOMAKE
# The problem is that I'm using automake 1.4 - quite old. Its AC_INIT_AUTOMAKE
#  does not do this, and if I give it no params, PACKAGE and VERSION will
#  not be defined in Makefile and in config.h. This will cause "make dist" to fail.
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

# Don't put in Makefile rules to check if configure.ac, Makefile.am, etc. files have changed,
# unless the user requests this by the running:
#   ./configure --enable-maintainer-mode
##AM_MAINTAINER_MODE

AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_RANLIB

  # gengetopt is used to create code that parses the argc/argv stuff passed to main()
AC_CHECK_TOOL(GENGETOPT, gengetopt)
AM_CONDITIONAL(GENGETOPT_EXISTS, test x$GENGETOPT != x)

# Check for Libtecla
AC_CHECK_LIB([tecla], [libtecla_version], [], [
        echo "error! You need to have libtecla installed."
	echo "to download, visit http://www.astro.caltech.edu/~mcs/tecla/libtecla-1.6.1.tar.gz"
        exit -1
        ])

# This does NOT define HAVE_LIBTCELA_H
AC_CHECK_HEADER([libtecla.h],[], [
        echo "error: libtecla.h not in include path."
        exit -1
        ])

# Check for Libipc
AC_CHECK_LIB([ipc], [IPC_connect], [], [
        echo "error: libipc not found."
	echo "to download, http://www.cs.cmu.edu/afs/cs/project/TCA/ftp/ipc-3.7.10.tar.gz"
        exit -1
        ])

# This does NOT define HAVE_LIBIPC_H
AC_CHECK_HEADER([ipc.h],[], [
        echo "error: ipc.h is not available."
	echo "to download please visit http://www.cs.cmu.edu/afs/cs/project/TCA/ftp/"
        exit -1
        ])


# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([locale.h stdlib.h string.h unistd.h])


######## handle getopt_long ################
# Check for getopt_long support, and if not available, use: getopt.c getopt1.c getopt.h 
############################################
#AC_CHECK_HEADERS([getopt.h])
#AC_CHECK_FUNCS([getopt_long],,
#    [# FreeBSD has a gnugetopt library for this
#     AC_CHECK_LIB([gnugetopt],[getopt_long],[AC_DEFINE([HAVE_GETOPT_LONG])],
#          [# use the GNU replacement
#	   AC_LIBOBJ(getopt)
#	   AC_LIBOBJ(getopt1)
#	   AC_CONFIG_LINKS([src/getopt.h:src/gnugetopt.h])])])
##############################################

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_STAT
AC_CHECK_FUNCS([setlocale strerror strrchr strstr])


#AC_CANONICAL_BUILD
#AC_CANONICAL_HOST

AC_CONFIG_FILES([Makefile tests/Makefile])
#AC_CONFIG_FILES([tests/Makefile])
AC_OUTPUT
