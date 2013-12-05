#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([scuttlebutt], [0.1], [darelf@me.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/scuttle.cpp])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.
PKG_CHECK_MODULES([JANSSON], [jansson])
AC_SEARCH_LIBS([clock_gettime], [rt], [], [AC_MSG_ERROR([unable to find clock_gettime() function])])

# Checks for header files.
AC_CHECK_HEADERS([sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memset])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
