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
 *	Numerics
 *	========
 *
 *	Definitions and routines to deal with handling
 *	numerical values of various sortes.
 */

#include "os.h"
#include "includes.h"

/*
 *	Provide some basic data conversion mechanisms.
 */
word w( byte h, byte l ) { return((((word)h)<<8)|((word)l)); }
byte l( word w ) { return( (byte)w ); }
byte h( word w ) { return( (byte)( w >> 8 )); }
word se( byte b ) { return( ((word)b )|( BOOL( b & 0x80 )? 0xff00: 0x0000 )); }


/*
 *	Scope manipulation routines.
 */
value_scope get_scope( integer v ) {
	value_scope	s;

	s = scope_none;

	if(( v >= MIN_UBYTE )&&( v <= MAX_UBYTE )) s |= scope_ubyte;
	if(( v >= MIN_SBYTE )&&( v <= MAX_SBYTE )) s |= scope_sbyte;

	if(( v >= MIN_UWORD )&&( v <= MAX_UWORD )) s |= scope_uword;
	if(( v >= MIN_SWORD )&&( v <= MAX_SWORD )) s |= scope_sword;

	return( s );
}

boolean numeric_scope( value_scope s ) {
	return( BOOL( s & scope_number ));
}

boolean address_scope( value_scope s ) {
	return( BOOL( s & scope_address ));
}

/*
 *	Convert a scope into a human readable buffer
 */
typedef struct {
	value_scope	scope;
	int		len;
	char		*name;
} scope_name;
static scope_name scope_names[] = {
	{ scope_ubyte,		6,	" ubyte"	},
	{ scope_sbyte,		6,	" sbyte"	},
	{ scope_uword,		6,	" uword"	},
	{ scope_sword,		6,	" sword"	},
	{ scope_address,	8,	" address"	},
	{ scope_none,		0,	NIL( char )	}
};

int convert_scope_to_text( value_scope scope, char *buffer, int max ) {
	scope_name	*sp;
	int		left;

	left = max;
	for( sp = scope_names; sp->scope != scope_none; sp++ ) {
		if( BOOL( scope & sp->scope )) {
			if( sp->len <= left ) {
				memcpy( buffer, sp->name, sp->len );
				left -= sp->len;
				buffer += sp->len;
			}
		}
	}
	return( max - left );
}


/*
 *	EOF
 */
