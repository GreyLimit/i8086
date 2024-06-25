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
 *	symbols
 *	=======
 */

#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_



/*
 *	Conversion of component ID back to printable
 *	text.
 */
extern const char *component_text( component comp );

/*
 *	Component identification routines..
 */
extern int match_identifier( char *search );
extern int digit_value( char d );
extern boolean isoctal( char o );
extern boolean ishex( char h );
extern int character_constant( char *string, char *value );
extern int string_constant( char quote, char *search, char *value, int max, int *fill, boolean *errors );
extern int match_constant( char *search, integer *value, boolean *errors );

/*
 *	Keyword and symbol identification
 */
extern int find_best_keyword( char *search, component *found );
extern int find_best_symbol( char *search, component *found );


#endif

/*
 *	EOF
 */
