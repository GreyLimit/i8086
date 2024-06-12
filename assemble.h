/*
 *	assemble
 *	========
 *
 *	Conversion from tokens to assembled instruction
 */

#ifndef _ASSEMBLE_H_
#define _ASSEMBLE_H_


/*
 *	Define the data structure in which the instruction is
 *	built up before being sent to the output system.
 */
typedef struct {
	opcode_prefix	prefixes;
	byte		coded,
			code[ MAX_CODE_BYTES ];
	boolean		segment_overriden,		/* TRUE when the segment prefix is necessary (actually different from default) */
			byte_data,
			word_data,
			near_data,			/* Near and Far are used when fixed immediate */
			far_data,			/* addresses in the CODE stack are implied */
			signed_data,
			unsigned_data,
			reg_is_dest;
} instruction;


/*
 *	Assign a segment and offset to a label.
 *
 *	Return TRUE if the assignment is consistent with data already
 *	assigned to the label, FALSE otherwise.
 */
extern boolean set_label_here( id_record *label, segment_record *seg );

/*
 *	Take an opcode description and bunch of encoded arguments
 *	and produce some output to suit.
 *
 *	inst	Record detailing the instruction
 *	prefs	The set of prefixes requested
 *	arg	an array of records providing the arguments
 *
 *	Number of arguments is provided in the inst data.
 */
extern boolean assemble_inst( opcode *inst, opcode_prefix prefs, ea_breakdown *arg, instruction *mc );

/*
 *	Conversion of an opcode and a series of arguments into
 *	a recognised assembly instruction.
 */
extern boolean process_opcode( opcode_prefix prefs, modifier mods, component op, int args, token_record **arg, int *len );

#endif

/*
 *	EOF
 */
