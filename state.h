/*
 *	state
 *	=====
 *
 *	Track the current state (operating mode/position) the assembler
 *	is in.  This controls the way the assembler handles discovery
 *	of labels and assignment of values before finally falling into
 *	code generation mode.
 */
 
#ifndef _STATE_H_
#define _STATE_H_

/*
 *	Define the various phases that the assembler operates in.
 */
typedef enum {
	no_pass = 0,					/* The default start position for the program. */

#ifdef VERIFICATION
	data_verification,				/* Routines being called as part of a table */
							/* generation, debug and testing process. */
#endif

	pass_label_gathering,				/* Key phase for name/label gathering */
	pass_value_confirmation,			/* Label value and position confirmation (repeatable) */
	pass_code_generation				/* Output file generation (repeatable) */
} assembler_phase;


/*
 *	The following variables are used to track the
 *	"state" of the assembly process as it passes through
 *	the source file multiple times.
 */
extern segment_record		*this_segment;
extern int			this_jiggle,		/* Track number of times we jiggle labels etc. */
				prev_jiggle;		/* Jiggle count of previous pass */

/*
 *	State variables used during the code generation phase.
 */
extern segment_group		*codegen_group;
extern segment_record		*codegen_segment;

/*
 *	What phase of the assembler processing are we in.
 */
extern assembler_phase		this_pass;

/*
 *	The "reset_state" call is made before each pass through the
 *	source file(s).  The routine is responsible for setting
 *	everything up appropiately for each pass.  So, what are the
 *	passes?  Traditionally (for me at least) these are the
 *	following:
 *			1	Effectively label gathering phase.
 *			2	Value stabalisation phase.
 *			3	Code/Object generation phase.
 *
 *	Phase 2 and 3 are repeated to facilitate them achieveing the
 *	correct results under the following conditions:
 *
 *	Phase 2		Repeated until all labels adopt a consistent
 *			and stable set of values.  A limit is placed on
 *			the number of times this happens based on the
 *			number of value tweaks not reducing on a per
 *			iteration basis.
 *
 *	Phase 3		Is repeated for each segment, targeting the
 *			segments in the order that they should be
 *			placed into memory.
 */
extern boolean reset_state( void );


#endif

/*
 *	EOF
 */
