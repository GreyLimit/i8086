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
 *	output_listing
 *	==============
 *
 *	An output format for listing purposes.
 */
 
#include "os.h"
#include "includes.h"


/*
 *	Listing Output API
 * 	==================
 */
static boolean listing_api_openfile( FILE **file, boolean hex, char *name ) {
	return( TRUE );
}

static boolean listing_api_closefile( FILE *file, boolean hex ) {
	return( TRUE );
}

static boolean listing_api_output_data( FILE *file, boolean hex, byte *data, int len ) {
	printf( "%04x", this_segment->posn );
	while( len-- ) printf( " %02x", (unsigned int)( *data++ ));
	printf( "\n" );
	return( TRUE );
}

static boolean listing_api_output_space( FILE *file, boolean hex, int count ) {
	if( count > 0 ) {
		word	here;
		int	i;

		here = this_segment->posn;
		for( i = 0; i < count; i++ ) {
			if( !BOOL( i & 0x000f )) {
				if( i ) printf( "\n" );
				printf( "%04x", here );
			}
			here++;
			printf( " 00" );
		}
		printf( "\n" );
	}
	return( TRUE );
}



output_api listing_output_api = {
	listing_api_openfile, listing_api_closefile, listing_api_output_data, listing_api_output_space
};


/*
 *	EOF
 */
