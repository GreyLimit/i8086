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
