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

#ifndef _NUMERICS_H_
#define _NUMERICS_H_

/*
 *	Provide some basic data conversion mechanisms.
 */
extern word w( byte h, byte l );
extern byte l( word w );
extern byte h( word w );
extern word se( byte b );

/*
 *	Wrapper for above to offer some flexibility in how
 *	this is actually implemented (in the future).
 */
#define W(h,l)		w(h,l)
#define L(w)		l(w)
#define H(w)		h(w)
#define SE(b)		se(b)

/*
 *	The following type provides a bitmap mechanism for tracking
 *	a values potential scope.  These are ORd together to define
 *	what any specific value might validly be used as.
 */
typedef enum {
	/*
	 *	No scope, no understanding.
	 */
	scope_none	= 0,
	/*
	 *	Scopes for numerical ranges.
	 */
	scope_ubyte	= 000001,
	scope_sbyte	= 000002,
	scope_uword	= 000004,
	scope_sword	= 000010,
	scope_address	= 000020,
	/*
	 *	Scope by size
	 */
	scope_byte	= ( scope_ubyte | scope_sbyte ),
	scope_word	= ( scope_ubyte | scope_sbyte | scope_uword | scope_sword ),
	scope_number	= ( scope_byte | scope_word )
} value_scope;


/*
 *	Scope manipulation routines.
 */
extern value_scope get_scope( integer v );
extern boolean numeric_scope( value_scope s );
extern boolean address_scope( value_scope s );

/*
 *	Convert a scope into a human readable buffer
 */
extern int convert_scope_to_text( value_scope scope, char *buffer, int max );

#endif

/*
 *	EOF
 */
