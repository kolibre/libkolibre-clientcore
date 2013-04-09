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

SUBDIRS = src tests $(subdirs)

if SAMPLES
SUBDIRS += samples
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkolibre-clientcore.pc

include doxygen.am

EXTRA_DIST = samples
AM_DISTCHECK_CONFIGURE_FLAGS = "PKG_CONFIG_PATH=${PKG_CONFIG_PATH}"
