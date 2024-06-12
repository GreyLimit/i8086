/*
 *	output_com
 * 	==========
 */
 
#include "os.h"
#include "includes.h"


static boolean com_api_openfile( char *name ) {
	char	*s, *t;

	ASSERT( name != NIL( char ));
	ASSERT( target_file == NIL( FILE ));

	s = strcpy( STACK_ARRAY( char, strlen( name ) + 5 ), name );
	if(( t = strrchr( s, PERIOD ))) *t = EOS;
	strcat( s, ".com" );
	if(!( target_file = fopen( s, "w" ))) {
		log_error_s( "Failed to open file for write", s );
		return( FALSE );
	}
	return( TRUE );
}

static void com_api_closefile( void ) {
	ASSERT( target_file != NIL( FILE ));

	fclose( target_file );
}

static void com_api_output_data( byte *data, int len ) {
	if( len > 0 ) {
		int	i;
		
		if( BOOL( command_flags & generate_hex )) {
			for( i = 0; i < len; i++ ) {
				fprintf( target_file, " %02X", data[ i ]);
				if(( i & 0x07 ) == 0x07 ) fprintf( target_file, "\n" );
			}
			if( BOOL( i & 0x0f )) fprintf( target_file, "\n" );
		}
		else {
			for( i = 0; i < len; i++ ) fputc( data[ i ], target_file );
		}
	}
}

static void com_api_output_space( int count ) {
	if( count > 0 ) {
		int	i;
		
		if( BOOL( command_flags & generate_hex )) {
			for( i = 0; i < count; i++ ) {
				fprintf( target_file, " 00" );
				if(( i & 0x07 ) == 0x07 ) fprintf( target_file, "\n" );
			}
			if( BOOL( i & 0x0f )) fprintf( target_file, "\n" );
		}
		else {
			for( i = 0; i < count; i++ ) fputc( 0, target_file );
		}
	}
}



output_api com_output_api = {
	com_api_openfile, com_api_closefile, com_api_output_data, com_api_output_space
};


/*
 *	EOF
 */
