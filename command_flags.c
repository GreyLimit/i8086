/*
 *	command_flags
 *	=============
 *
 *	Define the set of flags that modify the function
 *	of the command.
 */

#include "os.h"
#include "includes.h"

/*
 *	Declare the global holding the flags current in force.
 */
command_flag command_flags = no_command_flags;

/*
 *	Return TRUE if the output format matches the configuration of
 *	the source code being assembled.
 */
boolean output_format_valid( void ) {
	if( BOOL( command_flags & generate_dot_com )) {
		/*
		 *	Verification for '.COM' creation.
		 *
		 *	For this file type we specifically must have
		 *	only a single group and there must be no loose
		 *	segments left over.
		 */
		if( loose_segments ) {
			log_error( "Ungrouped segments not permitted in .COM file" );
			return( FALSE );
		}
		if(( all_groups == NIL( segment_group ))||( all_groups->next != NIL( segment_group ))) {
			log_error( "Only single group permitted in .COM file" );
			return( FALSE );
		}
	}
	return( TRUE );
}

/*
 *	EOF
 */
