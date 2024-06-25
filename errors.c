/**
 **	"i8086" An assembler for the 16-bit Intel x86 CPUs
 **
 **	Copyright (C) 2024  Jeff Penfold (jeff.penfold@googlemail.com)
 **
 **	This program is free software: you can redistribute it and/or modify
 **	it under the terms of the GNU General Public License as published by
 **	the Free Software Foundation, either version 3 of the License, or
 **	(at your option) any later version.
 **
 **	This program is distributed in the hope that it will be useful,
 **	but WITHOUT ANY WARRANTY; without even the implied warranty of
 **	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **	GNU General Public License for more details.
 **
 **	You should have received a copy of the GNU General Public License
 **	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/
/*
 *	Errors
 *	======
 *
 *	Error handling routines.
 */

#include "os.h"
#include "includes.h"

void log_error( const char *msg ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s\n", msg );
}

void log_error_i( const char *msg, integer i ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s (%d)\n", msg, (int)i );
}

void log_error_c( const char *msg, char c ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s ('%c')\n", msg, c );
}

void log_error_s( const char *msg, char *s ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s (%s)\n", msg, s );
}

void log_error_si( const char *msg, char *s, integer i ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s (%s,%d)\n", msg, s, (int)i );
}

/*
 *	EOF
 */
