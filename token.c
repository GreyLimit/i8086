/*
 *	token
 *	=====
 *
 *	module relating to token capture and handling.
 */

#include "os.h"
#include "includes.h"

/*
 *	Delete a token list
 */
void delete_tokens( token_record *list ) {
	token_record	*look;

	while(( look = list )) {
		list = list->next;
		FREE( look );
	}
}

/*
 *	Find the N'th item in a token list, where the
 *	list is indexed from 0.
 */
token_record *index_token( token_record *list, int index ) {

	ASSERT( index >= 0 );

	while( index && list ) {
		list = list->next;
		index--;
	}
	return( list );
}



/*
 *	EOF
 */
