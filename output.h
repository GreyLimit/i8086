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
