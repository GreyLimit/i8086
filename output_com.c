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
 *	output_com
 * 	==========
 */
 
#include "os.h"
#include "includes.h"


static boolean com_api_openfile( FILE **file, boolean hex, char *name ) {
	char	*s, *t;

	ASSERT( name != NIL( char ));
	ASSERT( file != NIL( FILE * ));
	ASSERT( *file == NIL( FILE ));

	s = strcpy( STACK_ARRAY( char, strlen( name ) + 5 ), name );
	if(( t = strrchr( s, PERIOD ))) *t = EOS;
	strcat( s, ".com" );

	if(( *file = fopen( s, "w" )) == NIL( FILE )) {
		log_error_s( "Failed to open file for write", s );
		return( FALSE );
	}
	return( TRUE );
}

static boolean com_api_closefile( FILE *file, boolean hex ) {
	
	ASSERT( file != NIL( FILE ));

	return( TRUE );
}

static boolean com_api_output_data( FILE *file, boolean hex, byte *data, int len ) {
	
	ASSERT( file != NIL( FILE ));
	ASSERT( data != NIL( byte ));
	ASSERT( len >= 0 );
	
	if( len > 0 ) {
		int	i;
		
		if( hex ) {
			for( i = 0; i < len; i++ ) {
				fprintf( file, " %02X", data[ i ]);
				if(( i & 0x07 ) == 0x07 ) fprintf( file, "\n" );
			}
			if( BOOL( i & 0x0f )) fprintf( file, "\n" );
		}
		else {
			for( i = 0; i < len; i++ ) fputc( data[ i ], file );
		}
	}
	return( TRUE );
}

static boolean com_api_output_space( FILE *file, boolean hex, int count ) {
	
	ASSERT( file != NIL( FILE ));
	
	if( count > 0 ) {
		int	i;
		
		if( hex ) {
			for( i = 0; i < count; i++ ) {
				fprintf( file, " 00" );
				if(( i & 0x07 ) == 0x07 ) fprintf( file, "\n" );
			}
			if( BOOL( i & 0x0f )) fprintf( file, "\n" );
		}
		else {
			for( i = 0; i < count; i++ ) fputc( 0, file );
		}
	}
	return( TRUE );
}

/*
 *	This is the external presentation of this API
 */
output_api com_output_api = {
	com_api_openfile, com_api_closefile, com_api_output_data, com_api_output_space
};


/*
 *	EOF
 */
