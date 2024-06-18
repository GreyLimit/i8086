/*
 *	state
 *	=====
 *
 *	Track the current state (operating mode/position) the assembler
 *	is in.  This controls the way the assembler handles discovery
 *	of labels and assignment of values before finally falling into
 *	code generation mode.
 */
 
#include "os.h"
#include "includes.h"

/*
 *	The following variables are used to track the
 *	"state" of the assembly process as it passes through
 *	the source file multiple times.
 */
segment_record		*this_segment = NIL( segment_record );

/*
 *		Track number of times we jiggle labels etc. and also
 *		remember the previous value (for comparison purposes).
 */
int			this_jiggle = 0,
			prev_jiggle = 0;
/*
 *	State variables used during the code generation phase.
 */
segment_group		*codegen_group = NIL( segment_group );
segment_record		*codegen_segment = NIL( segment_record );

/*
 *	What phase of the assembler processing are we in.
 */
assembler_phase this_pass = no_pass;

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
boolean reset_state( void ) {
	
#ifdef VERIFICATION
	ASSERT( this_pass != data_verification );
#endif

	/*
	 *	The mandatory new pass actions:
	 */
	this_segment = NIL( segment_record );
	restart_identifiers();
	/*
	 *	Now state specific actions.
	 */
	switch( this_pass ) {
		case no_pass: {
			/*
			 *	Pass/Phase 1: Data Gathering.
			 *
			 *	Collect all of the labels referenced and
			 *	gather initial values for them.  These will
			 *	contain errors, which the confirmation
			 *	pass (repeated as necessary) will resolve.
			 */
			this_pass = pass_label_gathering;
			prev_jiggle = 0;
			this_jiggle = 0;
			break;
		}
		case pass_label_gathering: {
			/*
			 *	Pass/Phase 2: Value Confirmation.
			 *
			 *	Have gathered all the labels, we now
			 *	check all the values and see what moves
			 *	as a result of 'settling'.
			 */
			if( !reset_segments()) {
				log_error( "Inconsistent segment configuration" );
				return( FALSE );
			}
			this_pass = pass_value_confirmation;
			prev_jiggle = this_jiggle+1;	/* +1 forces value confirmation to cycle once */
			this_jiggle = 0;
			break;
		}
		case pass_value_confirmation: {
			/*
			 *	Pass/Phase 2: Value Confirmation (repeated)
			 * or
			 * 	Pass/Phase 3: Code Generation.
			 *
			 *	If the jiggle count is not zero then we need
			 *	to repeat Phase 2.  If, however, the jiggle
			 *	count is the same as the previous cycle then
			 *	we abort the assembler.
			 *
			 * 	If the Jiggle count is zero, we roll into
			 * 	the Code generation.
			 */
			if( !reset_segments()) {
				log_error( "Inconsistent segment configuration" );
				return( FALSE );
			}
			if( this_jiggle == 0 ) {
				/*
				 *	Before launching into code generation we need to
				 *	verify that the output format selected is
				 *	compatible with the set of Groups and Segments
				 *	defined.
				 */
				if( !output_format_valid()) {
					log_error( "Output format does not support this memory configuration" );
					return( FALSE );
				}
				/*
				 *	We are good for code generation, so initialise
				 *	the codegen state variables to handle the first
				 *	group available, or the first loose segment
				 *	if no group has been defined.
				 *
				 *	This is the FIRST element of the code generation
				 *	code.  These section under the pass_code_generation
				 *	case handles the switch over between segments.
				 */
				if(( codegen_group = all_groups )) {
					codegen_segment = codegen_group->segments;

					ASSERT( codegen_segment != NIL( segment_record ));

					if( BOOL( command_flags & be_verbose )) printf( "Codegen: Group %s, Segment %s\n", codegen_group->name, codegen_segment->name );
				}
				else {
					codegen_segment = loose_segments;

					ASSERT( codegen_segment != NIL( segment_record ));

					if( BOOL( command_flags & be_verbose )) printf( "Codegen: Segment %s\n", codegen_segment->name );
				}
				this_pass = pass_code_generation;
				prev_jiggle = this_jiggle;
				this_jiggle = 0;
			}
			else {
				if( this_jiggle == prev_jiggle ) {
					log_error( "Unstable Label values in source" );
					return( FALSE );
				}
				prev_jiggle = this_jiggle;
				this_jiggle = 0;
			}
			break;
		}
		case pass_code_generation: {
			/*
			 *	We are back here having generated code for the
			 *	segment (indicated by codegen_segment) which may
			 *	be part of a group (indicated by codegen_group).
			 *	Our role here is to move onto the next segment
			 *	or, if nothing is left to do, exit.
			 */
			if( !reset_segments()) {
				log_error( "Inconsistent segment configuration" );
				return( FALSE );
			}
			/*
			 *	Move on the segment pointer to the next in the list.
			 *	If this is NIL then either move to the next group,
			 *	of if the group pointer is NIL then end the assembler.
			 */
			if(( codegen_segment = codegen_segment->next )) {
				if( codegen_group ) {
					if( BOOL( command_flags & be_verbose )) printf( "Codegen: Group %s, Segment %s\n", codegen_group->name, codegen_segment->name );
				}
				else {
					if( BOOL( command_flags & be_verbose )) printf( "Codegen: Segment %s\n", codegen_segment->name );
				}
			}
			else {
				if( codegen_group ) {
					if(( codegen_group = codegen_group->next )) {
						codegen_segment = codegen_group->segments;

						ASSERT( codegen_segment != NIL( segment_record ));

						if( BOOL( command_flags & be_verbose )) printf( "Codegen: Group %s, Segment %s\n", codegen_group->name, codegen_segment->name );
					}
					else {
						if(( codegen_segment = loose_segments )) {
							if( BOOL( command_flags & be_verbose )) printf( "Codegen: Segment %s\n", codegen_segment->name );
						}
						else {
							/*
							 *	No more groups, no more
							 *	segments then we must be done.
							 */
							this_pass = no_pass;
						}
					}
				}
				else {
					/*
					 *	No more groups, no more
					 *	segments then we must be done.
					 */
					this_pass = no_pass;
				}
			}
			break;
		}
		default: {
			ABORT( "Coding Error" );
			this_pass = no_pass;
			break;
		}
	}
	this_jiggle = 0;

	return( this_pass != no_pass );
}


/*
 *	EOF
 */
