/*
 * Codepage functions
 *
 * Copyright (c) 2009-2012, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <types.h>

#include "pyevt_codepage.h"
#include "pyevt_libevt.h"

/* Returns a string representation of the codepage
 * Codecs and aliases are defined: http://docs.python.org/library/codecs.html#standard-encodings
 * Returns 1 if successful or NULL if codepage is not supported
 */
const char *pyevt_codepage_to_string(
             int codepage )
{
	switch( codepage )
	{
		case LIBEVT_CODEPAGE_ASCII:
			return( "ascii" );

		case LIBEVT_CODEPAGE_ISO_8859_1:
			return( "iso-8859-1" );

		case LIBEVT_CODEPAGE_ISO_8859_2:
			return( "iso-8859-2" );

		case LIBEVT_CODEPAGE_ISO_8859_3:
			return( "iso-8859-3" );

		case LIBEVT_CODEPAGE_ISO_8859_4:
			return( "iso-8859-4" );

		case LIBEVT_CODEPAGE_ISO_8859_5:
			return( "iso-8859-5" );

		case LIBEVT_CODEPAGE_ISO_8859_6:
			return( "iso-8859-6" );

		case LIBEVT_CODEPAGE_ISO_8859_7:
			return( "iso-8859-7" );

		case LIBEVT_CODEPAGE_ISO_8859_8:
			return( "iso-8859-8" );

		case LIBEVT_CODEPAGE_ISO_8859_9:
			return( "iso-8859-9" );

		case LIBEVT_CODEPAGE_ISO_8859_10:
			return( "iso-8859-10" );

		case LIBEVT_CODEPAGE_ISO_8859_11:
			return( "iso-8859-11" );

		case LIBEVT_CODEPAGE_ISO_8859_13:
			return( "iso-8859-13" );

		case LIBEVT_CODEPAGE_ISO_8859_14:
			return( "iso-8859-14" );

		case LIBEVT_CODEPAGE_ISO_8859_15:
			return( "iso-8859-15" );

		case LIBEVT_CODEPAGE_ISO_8859_16:
			return( "iso-8859-16" );

		case LIBEVT_CODEPAGE_KOI8_R:
			return( "koi8_r" );

		case LIBEVT_CODEPAGE_KOI8_U:
			return( "koi8_u" );

		case LIBEVT_CODEPAGE_WINDOWS_874:
			return( "cp874" );

		case LIBEVT_CODEPAGE_WINDOWS_932:
			return( "cp932" );

		case LIBEVT_CODEPAGE_WINDOWS_936:
			return( "cp936" );

		case LIBEVT_CODEPAGE_WINDOWS_949:
			return( "cp949" );

		case LIBEVT_CODEPAGE_WINDOWS_950:
			return( "cp950" );

		case LIBEVT_CODEPAGE_WINDOWS_1250:
			return( "cp1250" );

		case LIBEVT_CODEPAGE_WINDOWS_1251:
			return( "cp1251" );

		case LIBEVT_CODEPAGE_WINDOWS_1252:
			return( "cp1252" );

		case LIBEVT_CODEPAGE_WINDOWS_1253:
			return( "cp1253" );

		case LIBEVT_CODEPAGE_WINDOWS_1254:
			return( "cp1254" );

		case LIBEVT_CODEPAGE_WINDOWS_1255:
			return( "cp1255" );

		case LIBEVT_CODEPAGE_WINDOWS_1256:
			return( "cp1256" );

		case LIBEVT_CODEPAGE_WINDOWS_1257:
			return( "cp1257" );

		case LIBEVT_CODEPAGE_WINDOWS_1258:
			return( "cp1258" );

		default:
			break;
	}
	return( NULL );
}

