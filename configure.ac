## Copyright (C) 2012 Kolibre
#
# This file is part of kolibre-clientcore.
#
# Kolibre-clientcore is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2.1 of the License, or
# (at your option) any later version.
#
# Kolibre-clientcore is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
#

# Process this file with autoconf to produce a configure script.

dnl -----------------------------------------------
dnl  Release instructions
dnl -----------------------------------------------
dnl
dnl  When you are about to release the software
dnl  either as a stable release or an unstable release
dnl  you should update the *_VERSION variables below
dnl  according to the following rules:
dnl
dnl  1. If any interfaces has been removed or changed,
dnl     increment MAJOR_VERSION and set others to 0
dnl  2. If any interface has been added,
dnl     increment MINOR_VERSION and set PATCH_VERSION to 0
dnl  3. If only the source code has changed,
dnl     increment PATCH_VERSION
dnl
dnl  If you are releasing a stable version,
dnl  set package version number in AC_INIT to
dnl  [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION]
dnl
dnl  If you are releasing an unstable version,
dnl  add a string (i.e -alpha or -beta or -rc1) to the
dnl  package version in AC_INIT after $PATCH_VERSION
dnl  e.g. [$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION-rc1]

# Setup version here:
m4_define([MAJOR_VERSION], [0])
m4_define([MINOR_VERSION], [0])
m4_define([PATCH_VERSION], [1])
m4_define([EXTRA_VERSION], [])

AC_PREREQ([2.67])
AC_INIT([kolibre-clientcore],[MAJOR_VERSION.MINOR_VERSION.PATCH_VERSION[]EXTRA_VERSION],[BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(tar-pax color-tests parallel-tests)

VERSION_INFO=m4_eval(MAJOR_VERSION + MINOR_VERSION):PATCH_VERSION:MINOR_VERSION
AC_SUBST(VERSION_INFO)

# Checks for programs.
LT_INIT
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

dnl -----------------------------------------------
dnl Doxygen settings
dnl -----------------------------------------------

DX_DOXYGEN_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_PDF_FEATURE(ON)
DX_INIT_DOXYGEN([kolibre-clientcore], doxygen.cfg, [doxygen-doc])

# Checks for libraries.

dnl -----------------------------------------------
dnl check for boost libraries
dnl -----------------------------------------------

AX_BOOST_BASE
AX_BOOST_SIGNALS
AC_CHECK_DECL([HAVE_BOOST_SIGNALS], [], [AC_MSG_ERROR([Library requirement (Boost::Signals) were not met])])
AX_BOOST_REGEX
AC_CHECK_DECL([HAVE_BOOST_REGEX], [], [AC_MSG_ERROR([Library requirement (Boost::Regex) were not met])])

dnl -----------------------------------------------
dnl check for log4cxx
dnl -----------------------------------------------

PKG_CHECK_MODULES(LOG4CXX, liblog4cxx >= 0.10.0)

AC_SUBST(LOG4CXX_CFLAGS)
AC_SUBST(LOG4CXX_LIBS)

dnl -----------------------------------------------
dnl Checks for kolibre naviengine
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBRENAVIENGINE, libkolibre-naviengine >= 0.0.1)

AC_SUBST(LIBKOLIBRENAVIENGINE_CFLAGS)
AC_SUBST(LIBKOLIBRENAVIENGINE_LIBS)

dnl -----------------------------------------------
dnl Checks for kolibre player
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBREPLAYER, libkolibre-player >= 0.0.1)

AC_SUBST(LIBKOLIBREPLAYER_CFLAGS)
AC_SUBST(LIBKOLIBREPLAYER_LIBS)

dnl -----------------------------------------------
dnl Checks for kolibre narrator
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBRENARRATOR, libkolibre-narrator >= 0.0.1)

AC_SUBST(LIBKOLIBRENARRATOR_CFLAGS)
AC_SUBST(LIBKOLIBRENARRATOR_LIBS)

dnl -----------------------------------------------
dnl Checks for kolibre daisyonline
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBREDAISYONLINE, libkolibre-daisyonline >= 0.0.1)

AC_SUBST(LIBKOLIBREDAISYONLINE_CFLAGS)
AC_SUBST(LIBKOLIBREDAISYONLINE_LIBS)

dnl -----------------------------------------------
dnl Checks for kolibre xmlreader
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBREXMLREADER, libkolibre-xmlreader >= 0.0.1)

AC_SUBST(LIBKOLIBREXMLREADER_CFLAGS)
AC_SUBST(LIBKOLIBREXMLREADER_LIBS)

dnl -----------------------------------------------
dnl Checks for kolibre amis
dnl -----------------------------------------------

PKG_CHECK_MODULES(LIBKOLIBREAMIS, libkolibre-amis >= 0.0.1)

AC_SUBST(LIBKOLIBREAMIS_CFLAGS)
AC_SUBST(LIBKOLIBREAMIS_LIBS)

# Create kolibre prfix
KOLIBRE_PREFIX=${prefix}
AC_SUBST(KOLIBRE_PREFIX)

# Checks for header files.
AC_CHECK_HEADERS([libintl.h stdint.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_CHECK_FUNCS([pow select strstr strtol])

dnl-------------------------------------------
dnl Configure samples
dnl--------------------------------------------

AC_ARG_WITH([samples], AS_HELP_STRING([--with-samples], [build applications in samples folder]))
AM_CONDITIONAL(SAMPLES, test "x$with_samples" = "xyes")

dnl-------------------------------------------
dnl Check for sample dependencies
dnl--------------------------------------------
AM_COND_IF(SAMPLES,
           [
            PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0)
            AC_SUBST(GLIB_CFLAGS)
            AC_SUBST(GLIB_LIBS)
            ])

AC_CONFIG_FILES([libkolibre-clientcore.pc
                 src/version.h
                 Makefile
                 src/Makefile
                 src/Settings/Makefile
                 tests/Makefile
                 tests/settings/Makefile
                 tests/commandqueue2/Makefile
                 tests/utils/Makefile
                 tests/nodes/Makefile
                 tests/interface/Makefile
                 samples/Makefile
                 samples/client/Makefile
                 samples/client/kolibre_prefix.sh])
AC_OUTPUT
