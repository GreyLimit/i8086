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
static boolean listing_api_openfile( char *name ) {
	return( TRUE );
}

static void listing_api_closefile( void ) {
}

static void listing_api_output_data( byte *data, int len ) {
	printf( "%04x", this_segment->posn );
	while( len-- ) printf( " %02x", (unsigned int)( *data++ ));
	printf( "\n" );
}

static void listing_api_output_space( int count ) {
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
}



output_api listing_output_api = {
	listing_api_openfile, listing_api_closefile, listing_api_output_data, listing_api_output_space
};


/*
 *	EOF
 */
