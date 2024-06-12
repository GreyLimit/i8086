/*
 *	Source
 *	======
 *
 *	Simplified, nesting, source line input mechanism.
 */

#ifndef _SOURCE_H_
#define _SOURCE_H_

extern boolean include_file( char *name );
extern boolean next_line( char *buffer, int len );
extern boolean skip_to_end( void );
extern void error_is_at( FILE *to );

#endif

/*
 *	EOF
 */
