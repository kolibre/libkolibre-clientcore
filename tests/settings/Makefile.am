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

check_PROGRAMS = settings

TESTS = settings

settings_SOURCES = main.cpp
settings_LDADD = -L$(top_builddir)/src/Settings -lsettings -lsqlite3
settings_CPPFLAGS = @LIBKOLIBREXMLREADER_CFLAGS@

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/Settings

clean-local: clean-local-check
.PHONY: clean-local-check

clean-local-check:
	-rm -rf *.db
