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

SRCS = \
ClientCore.cpp \
DaisyNavi.cpp \
DaisyOnlineNode.cpp \
DaisyBookNode.cpp \
DaisyOnlineBookNode.cpp \
FileSystemNode.cpp \
MediaSourceManager.cpp \
Navi.cpp \
NaviListImpl.cpp \
RootNode.cpp \
Menu/ContextMenuNode.cpp \
Menu/VirtualContextMenuNode.cpp \
Menu/BookInfoNode.cpp \
Menu/TempoNode.cpp \
Menu/SleepTimerNode.cpp \
Menu/NarratedNode.cpp \
Menu/GotoPageNode.cpp \
Menu/GotoPercentNode.cpp \
Menu/GotoTimeNode.cpp

SUBDIRS = Settings

# Install the headers in a versioned directory - e.g. examplelib-1.0:
library_includedir=$(includedir)/libkolibre/clientcore-$(PACKAGE_VERSION)
library_include_HEADERS = ClientCore.h NaviList.h NaviListItem.h

lib_LTLIBRARIES = libkolibre-clientcore.la

libkolibre_clientcore_la_SOURCES = $(SRCS)
libkolibre_clientcore_la_LIBADD = $(top_builddir)/src/Settings/libsettings.la
libkolibre_clientcore_la_LDFLAGS = -version-info $(VERSION_INFO) @LOG4CXX_LIBS@ @LIBKOLIBRENAVIENGINE_LIBS@ @LIBKOLIBREPLAYER_LIBS@ @LIBKOLIBRENARRATOR_LIBS@ @LIBKOLIBREDAISYONLINE_LIBS@ @LIBKOLIBREXMLREADER_LIBS@ @LIBKOLIBREAMIS_LIBS@
libkolibre_clientcore_la_CPPFLAGS = @LOG4CXX_CFLAGS@ @LIBKOLIBRENAVIENGINE_CFLAGS@ @LIBKOLIBREPLAYER_CFLAGS@ @LIBKOLIBRENARRATOR_CFLAGS@ @LIBKOLIBREDAISYONLINE_CFLAGS@ @LIBKOLIBREXMLREADER_CFLAGS@ @LIBKOLIBREAMIS_CFLAGS@

EXTRA_DIST = CommandQueue2/CommandQueue.h \
			 CommandQueue2/ScopeLock.h \
			 Commands/InternalCommands.h \
			 Commands/JumpCommand.h \
			 Commands/NotifyCommands.h \
			 DaisyNavi.h \
			 DaisyBookNode.h \
			 DaisyOnlineBookNode.h \
			 DaisyOnlineNode.h \
			 Defines.h \
			 FileSystemNode.h \
			 MediaSourceManager.h \
			 Navi.h \
			 NaviListImpl.h \
			 Utils.h \
			 RootNode.h \
			 Menu/ContextMenuNode.h \
			 Menu/VirtualContextMenuNode.h \
			 Menu/BookInfoNode.h \
			 Menu/NarratedNode.h \
			 Menu/GotoPageNode.h \
			 Menu/GotoTimeNode.h \
			 Menu/GotoPercentNode.h \
			 Menu/TempoNode.h \
			 Menu/SleepTimerNode.h
