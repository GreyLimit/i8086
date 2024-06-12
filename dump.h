/*
 *	dump
 *	====
 *
 *	Routines associated with the extended verification mode to
 *	facilitate dumping of internal opcode tables.
 */
 
#ifndef _DUMP_H_
#define _DUMP_H_

#ifdef VERIFICATION

/*
 *	Full dump of everything
 */
extern void dump_opcode_list( boolean show_more );

/*
 *	Basic dump of each entry in the opcode table.
 */
extern void dump_opcode_table( void );

#endif

#endif

/*
 *	EOF
 */
