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
