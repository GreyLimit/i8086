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
 *	identifiers
 *	===========
 *
 *	System for classification and storage of labels with values.
 */

#include "os.h"
#include "includes.h"

/*
 *	Sabed labels are all located below this variable.
 */
static id_record *saved_labels = NIL( id_record );

/*
 *	The 'Uniqueness' number that tracks labels as they are defined
 *	allowing the creation of "localised" labels.
 */
static int uniqueness = 0;


/*
 *	Reset identifier system to "top of input" for the start
 *	of a new pass.
 */
void restart_identifiers( void ) {
	uniqueness = 0;
}

/*
 *	UNIQUENESS_SIZE is a possible cause for inexplicable crashes
 *	in the assembler, if the value of uniqueness exceeds a 16 bit
 *	unsigned value.
 */
#define UNIQUENESS_SIZE		6
#define UNIQUENESS_PREFIX	"L%04X_"
#define MAXIMUM_UNIQUENESS	MAX_UWORD

/*
 *	Save/Find a label record.
 */
id_record *find_label( char *label, boolean definition ) {
	id_record	**adrs,
			*look;

	ASSERT( label != NIL( char ));

	/*
	 *	If the label starts with a PERIOD then this is
	 *	a locally referenced label.
	 */
	if( *label == PERIOD ) {
		char	*temp;
		int	len;

		/*
		 *	If there is a period at the start then we modify
		 *	the label to include the 'uniqueness' of the
		 *	last normal label defined.
		 */
		len = strlen( label ) + UNIQUENESS_SIZE + 1;
		temp = STACK_ARRAY( char, len );
		sprintf( temp, UNIQUENESS_PREFIX "%s", uniqueness, label+1 );
		if( BOOL( command_flags & more_verbose )) printf( "Localise %s -> %s\n", label, temp );
		label = temp;
	}
	else if( definition ) {
		/*
		 *	We are defining a label, we need to establish
		 *	a uniqueness tag for this symbol to be applied
		 *	to locally referential label used after it.
		 */
		if( uniqueness < MAXIMUM_UNIQUENESS ) {
			uniqueness++;
		}
		else {
			log_error( "Uniqueness counter exceeds maximum" );
		}
	}
	/*
	 *	Find/Insert the label into the list of all
	 *	known labels.
	 */
	adrs = &( saved_labels );
	if( BOOL( command_flags & ignore_label_case )) {
		while(( look = *adrs )) {
			if( strcasecmp( look->id, label ) == 0 ) {
				return( look );
			}
			adrs = &( look->next );
		}
	}
	else {
		while(( look = *adrs )) {
			if( strcmp( look->id, label ) == 0 ) {
				return( look );
			}
			adrs = &( look->next );
		}
	}
	look = NEW( id_record );
	look->id = strdup( label );
	look->type = class_unknown;
	look->next = NIL( id_record );
	*adrs = look;
	return( look );
}

/*
 *	Dump the content of a constant value record, used
 *	as part of the verbose/debug output.
 */
void dump_value( constant_value *v ) {

#	define BUFFER_FOR_SCOPE 32

	char	scope[ BUFFER_FOR_SCOPE ];

	ASSERT( v != NIL( constant_value ));

	if( v->segment ) printf( " %s:", v->segment->name );
	scope[ convert_scope_to_text( v->scope, scope, BUFFER_FOR_SCOPE-1 )] = EOS;
	printf( " %d($%04x)%s", (int)v->value, (unsigned int)v->value, scope );
}


/*
 *	To support verbose-ness and debugging this routine can be called
 *	to produce a dump of the label held by the assembler at this point
 *	in time.
 */
void dump_labels( void ) {
	static char *segment_names[ SEGMENT_REGISTERS ] = { "CS", "DS", "SS", "ES" };

	id_record	*look;

	printf( "Symbols:\n" );
	for( look = saved_labels; look; look = look->next ) {
		printf( "\t%s: ", look->id );
		switch( look->type ) {
			case class_unknown: {
				printf( "Undefined.\n" );
				break;
			}
			case class_label: {
				printf( "label:" );
				dump_value( &( look->var.value ));
				printf( ".\n" );
				break;
			}
			case class_const: {
				printf( "const:" );
				dump_value( &( look->var.value ));
				printf( ".\n" );
				break;
			}
			case class_group: {
				segment_record	*seg;

				ASSERT( look->var.group != NIL( segment_group ));

				printf( "group:" );
				for( seg = look->var.group->segments; seg; seg = seg->next ) printf( " %s", seg->name );
				printf( ".\n" );
				break;
			}
			case class_segment: {
				segment_record	*seg;

				ASSERT( look->var.segment != NIL( segment_record ));

				seg = look->var.segment;
				printf( "segment: " );
				if( seg->seg_reg < SEGMENT_REGISTERS ) {
					printf( "%s:", segment_names[ seg->seg_reg ]);
				}
				else {
					printf( "%d:", seg->seg_reg );
				}
				printf( " Start $%04x, Size %d.\n", (unsigned int)seg->start, (int)seg->size );
				break;
			}
			default: {
				ABORT( "Programmer Error!" );
				break;
			}
		}
	}
}


/*
 *	EOF
 */
