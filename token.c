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
