/*
 * Copyright (C) 2012 Kolibre
 *
 * This file is part of kolibre-clientcore.
 *
 * Kolibre-clientcore is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-clientcore is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DEFINES_H
#define _DEFINES_H

// inform the compiler to use thread safe (i.e. re-entrant) versions of several functions in the C library.
#ifndef _REENTRANT
#define _REENTRANT
#endif

/**
 * \def _(string)
 * \brief A macro for managing translations with gettext.
 */
#define _(string) dgettext (PACKAGE, string)

/**
 * \def _N(string)
 * \brief A macro to surround prompt strings.
 *
 * All prompt strings should be surrounded by this macro.
 * Prompt finder shell scripts included in libkolibre look for this macro solely.
 */
#define _N(string) string

#endif // _DEFINES_H
