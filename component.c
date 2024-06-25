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
 *	component
 *	=========
 *
 *	Define symbolic values for opcodes and syntactic elements.
 */

#include "os.h"
#include "includes.h"

/*
 *	Component testing and filtering macros/routines
 */
boolean is_opcode( component comp ) {
	return(( comp >= op_aaa )&&( comp <= op_xor ));
}

boolean is_directive( component comp ) {
	return(( comp >= asm_segment )&&( comp <= asm_end ));
}

boolean is_prefix( component comp ) {
	return(( comp >= pref_lock )&&( comp <= pref_repnz ));
}

boolean is_modifier( component comp ) {
	return(( comp >= mod_byte )&&( comp <= mod_far ));
}


/*
 *	EOF
 */
