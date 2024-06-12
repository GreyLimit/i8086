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
	boolean	FUNC( open_file )( char *name );
	void	FUNC( close_file )( void );
	void	FUNC( output_data )( byte *data, int len );
	void	FUNC( output_space )( int count );
} output_api;

/*
 *	Flag that controls the output of the binary
 *	files as either raw binary data or cooked ASCII
 *	hexadecimal data dump.
 */
extern boolean target_hex;

/*
 *	The file handle where the output is being directed.
 */
extern FILE *target_file;

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
extern output_api *target_api;

extern boolean open_file( char *name );
extern void close_file( void );
extern void output_data( byte *data, int len );
extern void output_space( int count );




#endif

/*
 *	EOF
 */
