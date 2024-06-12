/*
 *	token
 *	=====
 *
 *	module relating to token capture and handling.
 */

#ifndef _TOKEN_H_
#define _TOKEN_H_


/*
 *	Capture what we need to keep track of with a block
 *	of memory.
 */
typedef struct {
	byte		*ptr;
	int		len;
} constant_block;

/*
 *	Define the structure used to capture a token.
 */
typedef struct _token_record {
	component		id;
	union {
		id_record		*label;
		constant_value		constant;
		constant_block		block;
	} var;
	struct _token_record	*next;
} token_record;


/*
 *	Delete a token list
 */
extern void delete_tokens( token_record *list );

/*
 *	Find the N'th item in a token list, where the
 *	list is indexed from 0.
 */
extern token_record *index_token( token_record *list, int index );


#endif

/*
 *	EOF
 */
