/**
 **	"i8086" An assembler for the 16-bit Intel x86 CPUs
 **
 **	Copyright (C) 2024  Jeff Penfold (jeff.penfold@googlemail.com)
 **
 **	This program is free software: you can redistribute it and/or modify
 **	it under the terms of the GNU General Public License as published by
 **	the Free Software Foundation, either version 3 of the License, or
 **	(at your option) any later version.
 **
 **	This program is distributed in the hope that it will be useful,
 **	but WITHOUT ANY WARRANTY; without even the implied warranty of
 **	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **	GNU General Public License for more details.
 **
 **	You should have received a copy of the GNU General Public License
 **	along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/
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
