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
 *	Flag that controls the output of the binary
 *	files as either raw binary data or cooked ASCII
 *	hexadecimal data dump.
 */
boolean target_hex = FALSE;

/*
 *	The file handle where the output is being directed.
 */
FILE *target_file = NIL( FILE );


/*
 *	Generic Output API
 *	==================
 *
 *	Need to provide an API that provides a consistent interface
 *	for the creation of the output data
 */

/*
 *	We will default to the testing API for the moment so that
 *	the output remains clearly available.
 *
 *	TODO: Sort out the output selection and file open/close
 *	code.
 */
output_api *target_api = NIL( output_api );

boolean open_file( char *name ) {
	ASSERT( target_api != NIL( output_api ));
	return( FUNC( target_api->open_file )( name ));
}
void close_file( void ) {
	ASSERT( target_api != NIL( output_api ));
	FUNC( target_api->close_file )();
}
void output_data( byte *data, int len ) {
	ASSERT( target_api != NIL( output_api ));
	ASSERT( this_segment != NIL( segment_record ));
	ASSERT( data != NIL( byte ));
	ASSERT( len >= 0 );

	if( this_segment == codegen_segment ) FUNC( target_api->output_data )( data, len );
	this_segment->posn += len;
}
void output_space( int count ) {
	ASSERT( target_api != NIL( output_api ));
	ASSERT( this_segment != NIL( segment_record ));
	ASSERT( count >= 0 );

	if( this_segment == codegen_segment ) FUNC( target_api->output_space )( count );
	this_segment->posn += count;
}


/*
 *	EOF
 */
