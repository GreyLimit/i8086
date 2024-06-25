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
 *	Source
 *	======
 *
 *	Simplified, nesting, source line input mechanism.
 */

#ifndef _SOURCE_H_
#define _SOURCE_H_

extern boolean include_file( char *name );
extern boolean next_line( char *buffer, int len );
extern boolean skip_to_end( void );
extern void error_is_at( FILE *to );

#endif

/*
 *	EOF
 */
