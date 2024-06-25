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
 *	output
 *	======
 *
 *	Output system responcible for creating the linkable or
 *	executable files.
 */
 
#ifndef _OUTPUT_H_
#define _OUTPUT_H_

/*
 *	Define the methods data structure through which the
 *	output routines are routed, depending on the output
 *	selected.
 */
typedef struct {
	boolean	FUNC( open_file )( FILE **file, boolean hex, char *name );
	boolean	FUNC( close_file )( FILE *file, boolean hex );
	boolean	FUNC( output_data )( FILE *file, boolean hex, byte *data, int len );
	boolean	FUNC( output_space )( FILE *file, boolean hex, int count );
} output_api;

/*
 *	Generic Output API
 *	==================
 *
 *	Need to provide an API that provides a consistent interface
 *	for the creation of the output data
 */

/*
 *	The call to initialise the output API
 */
extern void initialise_output( output_api *api, boolean hex );

/*
 *	We will default to the testing API for the moment so that
 *	the output remains clearly available.
 */
extern boolean open_file( char *name );
extern boolean close_file( void );
extern boolean output_data( byte *data, int len );
extern boolean output_space( int count );




#endif

/*
 *	EOF
 */
