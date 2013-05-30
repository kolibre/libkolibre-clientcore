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

AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = KolibreSampleClient
KolibreSampleClient_SOURCES = Main.cpp Input.cpp

KolibreSampleClient_LDFLAGS = @LOG4CXX_LIBS@ @GLIB_LIBS@ -L$(top_builddir)/src/.libs -lkolibre-clientcore -lboost_regex
KolibreSampleClient_CPPFLAGS = @LOG4CXX_CFLAGS@ @GLIB_CFLAGS@ -I$(top_srcdir)/src

all-local:
	cp $(top_srcdir)/samples/client/run $(top_builddir)/samples/client
	cp $(top_srcdir)/samples/client/run.sh $(top_builddir)/samples/client
	cp $(top_srcdir)/samples/client/log4cxx.conf $(top_builddir)/samples/client

EXTRA_DIST = run.sh \
			 Input.h
