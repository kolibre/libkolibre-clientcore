What is Kolibre?
---------------------------------
Kolibre is a Finnish non-profit association whose purpose is to promote
information systems that aid people with reading disabilities. The software
which Kolibre develops is published under open source and made available to all
stakeholders at github.com/kolibre.

Kolibre is committed to broad cooperation between organizations, businesses and
individuals around the innovative development of custom information systems for
people with different needs. More information about Kolibres activities, association 
membership and contact information can be found at http://www.kolibre.org/


What is libkolibre-clientcore?
---------------------------------
Libkolibre-clientcore is a core library for listing accessible media, i.e. digital talking 
books, from online and offline sources. The library also supports playback, 
navigation and bookmark management for supported content formats. 
The library uses voice narration and can thus easily be utilized to create 
an easy-to-use playback client for visually impaired.


Documentation
---------------------------------
Kolibre client developer documentation is available at 
https://github.com/kolibre/libkolibre-builder/wiki

This library is documented using doxygen. Generate documentation by executing

    $ ./configure
    $ make doxygen-doc

Platforms
---------------------------------
Libkolibre-clientcore has been tested with Linux Debian Squeeze and can be built
using dev packages from apt repositories.

Dependencies
---------------------------------
Major dependencies for libkolibre-clientcore:

* libboost-signals
* libboost-regex
* libboost-filesystem
* libdbus-glib
* liblog4cxx
* libkolibre-naviengine
* libkolibre-daisyonline
* libkolibre-amis
* libkolibre-xmlreader
* libkolibre-player
* libkolibre-narrator

Building from source
---------------------------------
If building from GIT sources, first do a:

    $ autoreconf -fi

If building from a release tarball you can skip the above step.

    $ ./configure
    $ make
    $ make install

see INSTALL for detailed instructions.

Licensing
---------------------------------
Copyright (C) 2012 Kolibre

This file is part of libkolibre-clientcore.

Libkolibre-clientcore is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libkolibre-clientcore is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libkolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.

[![githalytics.com alpha](https://cruel-carlota.pagodabox.com/74210a6e048b9545d378303cd5fe029d "githalytics.com")](http://githalytics.com/kolibre/libkolibre-clientcore)
