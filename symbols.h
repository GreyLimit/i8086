/*
 *	symbols
 *	=======
 */

#ifndef _SYMBOLS_H_
#define _SYMBOLS_H_



/*
 *	Conversion of component ID back to printable
 *	text.
 */
extern const char *component_text( component comp );

/*
 *	Component identification routines..
 */
extern int match_identifier( char *search );
extern int digit_value( char d );
extern boolean isoctal( char o );
extern boolean ishex( char h );
extern int character_constant( char *string, char *value );
extern int string_constant( char quote, char *search, char *value, int max, int *fill, boolean *errors );
extern int match_constant( char *search, integer *value, boolean *errors );

/*
 *	Keyword and symbol identification
 */
extern int find_best_keyword( char *search, component *found );
extern int find_best_symbol( char *search, component *found );


#endif

/*
 *	EOF
 */
