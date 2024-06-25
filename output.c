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
 
#include "os.h"
#include "includes.h"

/*
 *	We will default to the testing API for the moment so that
 *	the output remains clearly available.
 *
 *	TODO: Sort out the output selection and file open/close
 *	code.
 */
static output_api *target_api = NIL( output_api );

/*
 *	The file handle where the output is being directed.
 */
static FILE *target_file = NIL( FILE );

/*
 *	Flag that controls the output of the binary
 *	files as either raw binary data or cooked ASCII
 *	hexadecimal data dump.
 */
static boolean target_hex = FALSE;


/*
 *	The call to initialise the output API
 */
void initialise_output( output_api *api, boolean hex ) {

	ASSERT( target_api == NIL( output_api ));
	ASSERT( api != NIL( output_api ));
	
	target_api = api;
	target_hex = hex;
}

/*
 *	Generic Output API
 *	==================
 *
 *	Need to provide an API that provides a consistent interface
 *	for the creation of the output data
 */

boolean open_file( char *name ) {

	ASSERT( target_api != NIL( output_api ));
	ASSERT( target_file == NIL( FILE ));

	return( FUNC( target_api->open_file )( &target_file, target_hex, name ));
}

boolean close_file( void ) {

	ASSERT( target_api != NIL( output_api ));
	ASSERT( target_file != NIL( FILE ));
	
	return( FUNC( target_api->close_file )( target_file, target_hex ));
}

boolean output_data( byte *data, int len ) {
	boolean	ret;
	
	ASSERT( target_api != NIL( output_api ));
	ASSERT( target_file != NIL( FILE ));
	
	ASSERT( this_segment != NIL( segment_record ));
	ASSERT( data != NIL( byte ));
	ASSERT( len >= 0 );

	ret = ( this_segment == codegen_segment )? FUNC( target_api->output_data )( target_file, target_hex, data, len ): TRUE;
	
	this_segment->posn += len;
	return( ret );
}

boolean output_space( int count ) {
	boolean	ret;
	
	ASSERT( target_api != NIL( output_api ));
	ASSERT( target_file != NIL( FILE ));
	
	ASSERT( this_segment != NIL( segment_record ));
	ASSERT( count >= 0 );

	ret = ( this_segment == codegen_segment )? FUNC( target_api->output_space )( target_file, target_hex, count ): TRUE;
	
	this_segment->posn += count;
	return( ret );
}


/*
 *	EOF
 */
