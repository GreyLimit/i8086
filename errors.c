/*
 *	Errors
 *	======
 *
 *	Error handling routines.
 */

#include "os.h"
#include "includes.h"

void log_error( const char *msg ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s\n", msg );
}

void log_error_i( const char *msg, integer i ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s (%d)\n", msg, (int)i );
}

void log_error_c( const char *msg, char c ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s ('%c')\n", msg, c );
}

void log_error_s( const char *msg, char *s ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s (%s)\n", msg, s );
}

void log_error_si( const char *msg, char *s, integer i ) {
	error_is_at( stderr );
	fprintf( stderr, "E: %s (%s,%d)\n", msg, s, (int)i );
}

/*
 *	EOF
 */
