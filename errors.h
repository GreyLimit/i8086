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

#ifndef _ERRORS_H_
#define _ERRORS_H_

extern void log_error( const char *msg );
extern void log_error_i( const char *msg, integer i );
extern void log_error_c( const char *msg, char c );
extern void log_error_s( const char *msg, char *s );
extern void log_error_si( const char *msg, char *s, integer i );

#endif

/*
 *	EOF
 */
