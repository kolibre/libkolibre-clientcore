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

check_PROGRAMS = mediasourcemanager sleeptimer getset

TESTS = mediasourcemanager sleeptimer getset

mediasourcemanager_SOURCES = mediasourcemanager.cpp
sleeptimer_SOURCES = sleeptimer.cpp
getset_SOURCES = getset.cpp

LDADD = -lkolibre-clientcore -lboost_regex
AM_LDFLAGS = -L$(top_builddir)/src
AM_CPPFLAGS = -I$(top_srcdir)/src
