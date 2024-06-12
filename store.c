/*
 *	Store
 *	=====
 *
 *	Mechanism for caching blobs of memory to (try to)
 *	optimise overall memory utilisation.
 */

#include "os.h"
#include "includes.h"

/*
 *	Memory management used to consolidate blocks
 *	to reduce storage (hopefully).
 */
typedef struct _block_record {
	byte			*blk;
	int			len;
	struct _block_record	*next;
} block_record;

/*
 *	The head of the stored adta.
 */
static block_record *saved_blocks = NIL( block_record );

/*
 *	Add/Extract a block from the saved blocks.  Should probably
 *	use something better than a list.
 *
 *	TODO	Even a simple binary tree stands a better chance of
 *		being faster, but at the cost of additional pointers.
 */
byte *save_block( byte *block, int len ) {
	block_record	**adrs,
			*look;

	adrs = &saved_blocks;
	while(( look = *adrs )) {
		if(( look->len == len )&&( memcmp( look->blk, block, len ) == 0 )) return( look->blk );
		adrs = &( look->next );
	}
	look = NEW( block_record );
	look->blk = NEW_ARRAY( byte, len );
	memcpy( look->blk, block, len );
	look->next = NIL( block_record );
	*adrs = look;
	return( look->blk );
}
char *save_string( char *string ) {
	return( (char *)save_block( (byte *)string, strlen( string )+1 ));
}

/*
 *	EOF
 */
