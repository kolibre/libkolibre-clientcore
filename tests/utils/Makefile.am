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

check_PROGRAMS = datapath trim isdir isfile search

TESTS = datapath.sh trim isdir isfile search.sh

datapath_SOURCES = datapath.cpp
trim_SOURCES = trim.cpp
isdir_SOURCES = isdir.cpp
isfile_SOURCES = isfile.cpp
search_SOURCES = search.cpp

AM_CPPFLAGS = -I$(top_srcdir)/src
AM_LDFLAGS = @LOG4CXX_LIBS@ -L$(top_builddir)/src -lboost_regex -lboost_filesystem

EXTRA_DIST = datapath.sh search.sh searchData
