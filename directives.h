/*
 *	directives
 *	==========
 *
 *	Provide the various assembler directives
 */

#ifndef _DIRECTIVES_H_
#define _DIRECTIVES_H_

/*
 *	Process directives.
 */
extern boolean process_directive( id_record *label, component dir, int args, token_record **arg, int *len );

#endif

/*
 *	EOF
 */
