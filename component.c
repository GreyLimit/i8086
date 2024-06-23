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
