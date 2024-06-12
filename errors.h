/*
 *	Errors
 *	======
 *
 *	Error handling routines.
 */

#ifndef _ERRORS_H_
#define _ERRORS_H_

extern void log_error( const char *msg );
extern void log_error_i( const char *msg, integer i );
extern void log_error_c( const char *msg, char c );
extern void log_error_s( const char *msg, char *s );
extern void log_error_si( const char *msg, char *s, integer i );

#endif

/*
 *	EOF
 */
