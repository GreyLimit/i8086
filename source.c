/*
 *	Source
 *	======
 *
 *	Simplified, nesting, source line input mechanism.
 */

#include "os.h"
#include "includes.h"

/*
 *	Provide a stream like system to access nested file
 *	includes.
 *
 *	Define data structures used to track this.
 */
typedef struct {
	char		*fname;
	int		line;
	FILE		*fd;
} file_record;

/*
 *	The 'stack' of opened source files.
 */
static file_record file_io[ MAX_FILE_NESTING ];
static int nested_files = 0;

/*
 *	Declare a routine called to insert a new file into the stream.
 *	This file will provide the next line of text to be processed
 *	by the assembler.  Return TRUE if it's all worked.
 */
boolean include_file( char *name ) {
	file_record	*fr;

	if( nested_files == MAX_FILE_NESTING ) {
		log_error_i( "Maximum file nesting reached", MAX_FILE_NESTING );
		return( FALSE );
	}
	fr = &( file_io[ nested_files ]);
	if(( fr->fd = fopen( name, "r" )) == NIL( FILE )) {
		log_error_s( "Unable to read file", name );
		return( FALSE );
	}
	fr->fname = save_string( name );
	fr->line = 0;
	nested_files++;
	return( TRUE );
}

/*
 *	Pull off the next line for the input stream.
 */
boolean next_line( char *buffer, int len ) {
	while( nested_files ) {
		file_record	*fr;

		fr = &( file_io[ nested_files-1 ]);
		if(( fgets( buffer, len, fr->fd )) != NIL( char )) {
			buffer[ len-1 ] = EOS;
			fr->line += 1;
			return( TRUE );
		}
		fclose( fr->fd );
		nested_files--;
	}
	return( FALSE );
}

/*
 *	Skip to end closes the current file. If this is the
 *	last file open (ie the file from the command line)
 *	then the next call to "next_line" to return FALSE
 *	indicating the end of the input data.
 */
boolean skip_to_end( void ) {
	if( nested_files ) {
		fclose( file_io[ --nested_files ].fd );
		return( TRUE );
	}
	return( FALSE );
}

/*
 *	Send a description of where we are to a FILE.
 *
 *	This works even if we are not in a file through
 *	the simple expedient of not outputting anything.
 */
void error_is_at( FILE *to ) {
	int	i;

	for( i = nested_files; i; i-- ) {
		file_record	*fr;

		fr = &( file_io[ i-1 ]);
		fprintf( to, "%s:%d,", fr->fname, fr->line );
	}
}

/*
 *	EOF
 */



