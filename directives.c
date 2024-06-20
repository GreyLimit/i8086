/*
 *	directives
 *	==========
 *
 *	Provide the various assembler directives
 */

#include "os.h"
#include "includes.h"

/*
 *	End processing on the current source file.
 *
 *		END
 *
 *	If the input is a result of an 'INCLUDE' directive
 *	then processing will continue from the line after
 *	the INCLUDE.
 */
static boolean process_dir_end( int args, token_record **arg, int *len ) {
	if( args ) {
		log_error( "END has no arguments" );
		return( FALSE );
	}
	return( skip_to_end());
}

/*
 *	Assign static value to a label, possibly as a result of
 *	an expression evaluation.
 *
 *	{label}	EQU	{expression}
 */
static boolean process_dir_equ( id_record *label, int args, token_record **arg, int *len ) {
	constant_value	val;
	int		used;

	if( label == NIL( id_record )) {
		log_error( "EQU requires target label" );
		return( FALSE );
	}
	if( args != 1 ) {
		log_error( "EQU requires a target value" );
		return( FALSE );
	}
	if(( label->type != class_unknown )&&( label->type != class_const )) {
		log_error( "EQU target label in use" );
		return( FALSE );
	}
	/*
	 *	Evaluate the expression provided to get the
	 *	value of the label.
	 */
	if( !evaluate( arg[ 0 ], len[ 0 ], &used, &val, FALSE )) {
		log_error( "Error in EQU expression" );
		return( FALSE );
	}
	if( used != len[ 0 ]) {
		log_error( "Invalid EQU expression" );
		return( FALSE );
	}
	/*
	 *	Update the label record.
	 */
	if( label->type == class_unknown ) {
		label->type = class_const;
		label->var.value = val;
		this_jiggle++;
	}
	else {
		if( label->var.value.segment != val.segment ) {
			log_error( "Inconsistent segment in EQU expression" );
			return( FALSE );
		}
		if( label->var.value.value != val.value ) {
			if( BOOL( command_flags & more_verbose )) printf( "%s: %04x -> %04x\n", label->id, label->var.value.value, val.value );
			label->var.value.value = val.value;
			this_jiggle++;
		}
	}
	return( TRUE );
}

/*
 *	Define a generic 'place static data into segment' routine.
 */
static boolean process_dir_data( int size, value_scope scope, int args, token_record **arg, int *len ) {
	byte	buffer[ 4 ];
	int	i;
	boolean	ret;

	ASSERT(( size == 1 )||( size == 2 )||( size == 4 ));

	ret = TRUE;
	for( i = 0; i < args; i++ ) {

		ASSERT( len[ i ] > 0 );
		ASSERT( arg[ i ] != NIL( token_record ));

		if(( len[ i ] == 1 )&&( arg[ i ]->id == tok_string )) {
			int	l;

			/*
			 *	This is character string.  We will output
			 *	it a byte at a time to the data size we
			 *	were called with.
			 */
			if(( l = arg[ i ]->var.block.len )) {
				if( size == 1 ) {
					/*
					 *	We will do this the easy way.
					 */
					ret &= output_data( arg[ i ]->var.block.ptr, l );
				}
				else {
					int	k, v;
					byte	*s;

					s = arg[ i ]->var.block.ptr;
					while( l-- ) {
						v = *s++;
						for( k = 0; k < size; k++ ) {
							buffer[ k ] = v & 0xff;
							v >>= 8;
						}
						ret &= output_data( buffer, size );
					}
				}
			}
		}
		else {
			/*
			 *	This is, probably, an expression which can
			 *	be evaluated for a static result.
			 */
			constant_value	cv;
			int		k, l, v;

			if( !evaluate( arg[ i ], len[ i ], &l, &cv, FALSE )) {
				log_error( "Expression error in static data" );
				return( FALSE );
			}
			if( l < len[ i ]) {
				log_error( "Invalid expression in static data" );
				return( FALSE );
			}
			if( !BOOL( cv.scope & scope )) {
				log_error( "Expression result outside data range" );
				return( FALSE );
			}
			if(( cv.segment != NIL( segment_record )) && !BOOL( scope & scope_address )) {
				log_error( "Data does not support segment references" );
				return( FALSE );
			}
			/*
			 *	We're good to go
			 */
			v = cv.value;
			for( k = 0; k < size; k++ ) {
				buffer[ k ] = v & 0xff;
				v >>= 8;
			}
			ret &= output_data( buffer, size );
		}
	}
	return( ret );
}

/*
 *	Define a set of binary BYTE values into the current segment
 */
static boolean process_dir_db( int args, token_record **arg, int *len ) {
	return( process_dir_data( 1, scope_byte, args, arg, len ));
}

/*
 *	Define a set of binary WORD values into the current segment
 */
static boolean process_dir_dw( int args, token_record **arg, int *len ) {
	return( process_dir_data( 2, scope_word, args, arg, len ));
}

/*
 *	Define a set of binary BYTE values into the current segment
 */
static boolean process_dir_reserve( int args, token_record **arg, int *len ) {
	constant_value	val;
	int		used;

	if( args != 1 ) {
		log_error( "RESERVE requires a number of bytes to reserve" );
		return( FALSE );
	}
	/*
	 *	Evaluate the expression provided to get the
	 *	value of the label.
	 */
	if( !evaluate( arg[ 0 ], len[ 0 ], &used, &val, FALSE )) {
		log_error( "Error in RESERVE expression" );
		return( FALSE );
	}
	if( used != len[ 0 ]) {
		log_error( "Invalid RESERVE expression" );
		return( FALSE );
	}
	/*
	 *	Validate the result of the RESERVE expression.
	 */
	if( val.segment != NIL( segment_record )) {
		log_error( "RESERVE expression contains segment reference" );
		return( FALSE );
	}
	if( val.value < 0 ) {
		log_error( "RESERVE expression is negative" );
		return( FALSE );
	}
	/*
	 *	All good.
	 */
	return( output_space( val.value ));
}

/*
 *	Define a set of binary BYTE values into the current segment
 *
 *		ALIGN	{expression}|{modifier}
 *
 *	where modifier is one of BYTE, WORD, DWORD or PTR.
 */
static boolean process_dir_align( int args, token_record **arg, int *len ) {
	integer	alignment, gap;

	if( args != 1 ) {
		log_error( "ALIGN requires single argument" );
		return( FALSE );
	}
	alignment = 0;
	if( is_modifier( arg[ 0 ]->id )) {
		if( len[ 0 ] != 1 ) {
			log_error( "Invalid size modifier in ALIGN" );
			return( FALSE );
		}
		switch( arg[ 0 ]->id ) {
			case mod_byte: {
				alignment = 1;
				break;
			}
			case mod_word: {
				alignment = 2;
				break;
			}
			case mod_ptr: {
				alignment = BOOL( scope_address & scope_uword )? 2: 4;
				break;
			}
			default: break;
		}
	}
	else {
		constant_value	cv;
		int		used;

		if( !evaluate( arg[0], len[0], &used, &cv, FALSE )) {
			log_error( "Expression error in ALIGN" );
			return( FALSE );
		}
		if( used < len[0] ) {
			log_error( "Incomplete expression in ALIGN" );
			return( FALSE );
		}
		if( cv.segment ) {
			log_error( "Segment reference invalid in ALIGN" );
			return( FALSE );
		}
		alignment = cv.value;
	}
	if( this_segment == NIL( segment_record )) {
		log_error( "Segment not set before ALIGN" );
		return( FALSE );
	}
	if( alignment <= 0 ) {
		log_error( "Invalid ALIGN specification" );
		return( FALSE );
	}
	/*
	 *	Work out the gap and return if it is zero.
	 */
	if(( gap = this_segment->posn % alignment ) == 0 ) return( TRUE );
	/*
	 *	so we are not (by luck) on a suitable
	 *	alignment, add in necessary space.
	 */
	return( output_space( alignment - gap ));
}

/*
 *	Provide a list of labels which are to be exported
 *	to an external source.
 *
 *		EXPORT	{label}[,{label}]*
 *
 *	There is no requirement to worry about segment association
 *	as this is all known by the assembler.
 */
static boolean process_dir_export( int args, token_record **arg, int *len ) {
	int	i;

	if( args < 1 ) {
		log_error( "Label names expected after export" );
		return( FALSE );
	}
	for( i = 0; i < args; i++ ) {
		if(( len[ i ] != 1 )||( arg[ i ]->id != tok_label )) {
			log_error_i( "Label names expected after export", i+1 );
			return( FALSE );
		}
	}
	/*TODO*/
	return( TRUE );
}

/*
 *	Provide a list of labels which are to be imported
 *	from an external source.
 *
 *		IMPORT	{label}[,{label}]*
 *
 *	A key issue with this facility is that the basic
 *	syntax does not provide for any associated segment
 *	information with the label.  As a result the
 *	assembler will have to infer which segment a label
 *	is relative to from its use.
 *
 *	TODO
 *
 *	Perhaps include the option for the segment to be included
 *	in the label specification:
 *
 *		IMPORT	[{register}:]?{label}[,[{register}:]?{label}]*
 *
 *	Thought required.
 */
static boolean process_dir_import( int args, token_record **arg, int *len ) {
	int	i;

	if( args < 1 ) {
		log_error( "Label names expected after import" );
		return( FALSE );
	}
	for( i = 0; i < args; i++ ) {
		if(( len[ i ] != 1 )||( arg[ i ]->id != tok_label )) {
			log_error_i( "Label names expected after import", i+1 );
			return( FALSE );
		}
	}
	/*TODO*/
	return( TRUE );
}

/*
 *	Define how a group of declared segments are grouped together
 *	in memory.
 *
 *	The segments listed can belong to only a single group and while
 *	different segments can reference through different segment registers
 *	all the segment registers will point to the same paragraph that
 *	represents the start off the group.
 *
 *		{label}	GROUP	{segment}[,{segment}]*
 */
static boolean process_dir_group( id_record *label, int args, token_record **arg, int *len ) {
	segment_group	*gp;
	int		i, j, pages;

	if( label == NIL( id_record )) {
		log_error( "GROUP definition requires a label" );
		return( FALSE );
	}
	if(( label->type != class_unknown )&&( label->type != class_group )) {
		log_error_s( "Redefinition of GROUP label", label->id );
		return( FALSE );
	}
	/*
	 *	Create the group if it didn't already exist.
	 */
	if(( gp = label->var.group ) == NIL( segment_group )) {
		/*
		 *	We are creating a group from scratch.
		 */
		ASSERT( label->type == class_unknown );

		gp = NEW( segment_group );
		gp->name = label->id;
		gp->page = 0;			/* Default Page for group */
		gp->segments = NIL( segment_record );
		gp->next = NIL( segment_group );
		*tail_all_groups = gp;
		tail_all_groups = &( gp->next );

		label->type = class_group;
		label->var.group = gp;
	}
	/*
	 *	Now step through the arguments which should all be
	 *	segment names.
	 */
	pages = 0;
	for( i = 0; i < args; i++ ) {
		id_record	*ip;
		segment_record	*ts, *sp, **asp;

		if( len[ i ] != 1 ) {
			log_error_i( "Invalid GROUP argument", i+1 );
			return( FALSE );
		}
		if( arg[ i ]->id == tok_label ) {
			/*
			 *	Labels should be references to segments.
			 */
			ip = arg[ i ]->var.label;

			ASSERT( ip != NIL( id_record ));

			if( ip->type != class_segment ) {
				log_error_i( "GROUP argument not a segment name", i+1 );
				return( FALSE );
			}
			ts = ip->var.segment;

			ASSERT( ts != NIL( segment_record ));

			if(( ts->group != NIL( segment_group ))&&( ts->group != gp )) {
				log_error_s( "GROUP segment belongs to another group", ip->id );
				return( FALSE );
			}

			j = 0;
			asp = &( gp->segments );
			while(( sp = *asp )) {
				if( sp == ts ) break;
				asp = &( sp->next );
				j++;
			}
			if( sp == NIL( segment_record )) {
				/*
				 *	This is a new segment for this group so it is
				 *	appended to the end of the group.  This however
				 *	is not a feature - this only happens in PASS 1
				 *	of the assembler.  In other passes sp should
				 *	be set and the segment MUST be in the correct
				 *	place in the group.
				 */

				ASSERT( ts->group == NIL( segment_group ));

				ts->group = gp;
				if(( *( ts->link ) = ts->next )) {
					ts->next->link = ts->link;
					ts->next = NIL( segment_record );
				}
				*( ts->link = asp ) = ts;
			}
		}
		else if( arg[ i ]->id == tok_immediate ) {
			/*
			 *	A numerical argument is a page number to associate
			 *	with this group.
			 */
			if( pages ) {
				log_error_i( "GROUP page index can only be set once", i+1 );
				return( FALSE );
			}
			/*
			 *	TODO - shouldn't there be some sort of numerical
			 *	validation on the constant provided?  Shoudn't it
			 *	be, at the very least, in uword_scope?
			 */
			 if( !BOOL( arg[ i ]->var.constant.scope & scope_uword )) {
				log_error_i( "GROUP page index must be an unsigned word", i+1 );
				return( FALSE );
			}
			gp->page = arg[ i ]->var.constant.value;
			pages++;
		}
		else {
			log_error_i( "Invalid GROUP argument", i+1 );
			return( FALSE );
		}
	}
	/*
	 *	If we get here without an error we are all good.
	 */
	return( TRUE );
}

/*
 *	Introduce a new segment or continue with an existing segment.
 *	If the segment does not already exist then a segment register
 *	must be assigned to it at this point.
 *
 *	To define a new segment:
 *
 * 	{name}	SEGMENT	{segment_reg}
 *
 * 	To return to an existing segment
 *
 *		SEGMENT	{name}
 *
 *	TODO:
 *		The segment code has the begininnings of support for
 *		defining the 'nature' of a segment (read-write, read-only,
 *		empty etc).  The current concept is that (to avoid the
 *		definition of even more keywords) a string follows the
 *		segment register and the content of the string offers
 *		the necessary extra information.  That being said, it's
 *		not /that/ /many/ new keywords.
 */
static boolean process_dir_segment( id_record *label, int args, token_record **arg, int *len ) {
	register_data	*rd;
	segment_record	*sp;

	if( args != 1 ) {
		log_error( "SEGMENT requires a single argument" );
		return( FALSE );
	}
	if( len[ 0 ] != 1 ) {
		log_error( "SEGMENT invalid argument size" );
		return( FALSE );
	}
	if( label ) {
		/*
		 *	Defining a new segment.  The argument must be a
		 *	segment register.
		 */
		if(( label->type != class_unknown )&&( label->type != class_segment )) {
			log_error( "SEGMENT name already in use" );
			return( FALSE );
		}
		if((( rd = register_component( arg[ 0 ]->id )) == NIL( register_data )) || !BOOL( rd->ac & ac_segment_reg )) {
			log_error( "SEGMENT expecting a segment register" );
			return( FALSE );
		}
		if( label->type == class_unknown ) {
			sp = NEW( segment_record );
			sp->name = label->id;
			sp->seg_reg = rd->reg_no;
			sp->access = segment_undefined_access;
			sp->fixed = FALSE;
			sp->start = 0;
			sp->posn = 0;
			sp->size = 0;
			sp->group = NIL( segment_group );

			*tail_loose_segments = sp;
			sp->link = tail_loose_segments;
			tail_loose_segments = &( sp->next );
			sp->next = NIL( segment_record );

			label->type = class_segment;
			label->var.segment = sp;
		}
		else {
			ASSERT( label->type == class_segment );

			sp = label->var.segment;

			ASSERT( sp != NIL( segment_record ));

			if( sp->seg_reg == UNKNOWN_SEG ) {
				sp->seg_reg = rd->reg_no;
			}
			else {
				if( sp->seg_reg != rd->reg_no ) {
					log_error( "Inconsistent SEGMENT register" );
					return( FALSE );
				}
			}
		}
	}
	else {
		id_record	*ip;

		if( arg[ 0 ]->id != tok_label ) {
			log_error( "SEGMENT expecting segment name" );
			return( FALSE );
		}
		ip = arg[ 0 ]->var.label;

		ASSERT( ip != NIL( id_record ));

		if(( ip->type != class_unknown )&&( ip->type != class_segment )) {
			log_error( "Invalid SEGMENT name" );
			return( FALSE );
		}
		if( ip->type == class_unknown ) {
			sp = NEW( segment_record );
			sp->name = ip->id;
			sp->seg_reg = UNKNOWN_SEG;
			sp->access = segment_undefined_access;
			sp->fixed = FALSE;
			sp->start = 0;
			sp->posn = 0;
			sp->size = 0;
			sp->group = NIL( segment_group );

			*tail_loose_segments = sp;
			sp->link = tail_loose_segments;
			tail_loose_segments = &( sp->next );
			sp->next = NIL( segment_record );

			ip->type = class_segment;
			ip->var.segment = sp;
		}
		else {
			ASSERT( ip->type == class_segment );

			sp = ip->var.segment;

			ASSERT( sp != NIL( segment_record ));
		}
	}
	/*
	 *	Finally we change our segment.
	 */
	this_segment = sp;
	return( TRUE );
}

/*
 *	Set a fixed static 'address' for which the assembly language
 *	is the targeted at.
 *
 *		ORG	{address}
 */
static boolean process_dir_org( int args, token_record **arg, int *len ) {
	/*
	 *	Statically set the offset within the currently set
	 *	segment.  However, be warned, this DOES NOT set a
	 *	static address, nor does it set any part of the segment
	 *	register.
	 *
	 *	What does this do?
	 *
	 *	This sets the 'start' offset of a segment if, and only
	 *	if the following conditions are met:
	 *
	 *	o	The segment must be the first segment in a
	 *		defined group.
	 *
	 *	o	The segment must not have been used: The start
	 *		offset and current position offset must be
	 *		both be 0.
	 */
	if( args != 1 ) {
		log_error( "ORG incorrect number of arguments" );
		return( FALSE );
	}
	if(( len[ 0 ] != 1 )||( arg[ 0 ]->id != tok_immediate )) {
		log_error( "ORG expecting fixed offset" );
		return( FALSE );
	}
	if( !BOOL( arg[ 0 ]->var.constant.scope & scope_uword )) {
		log_error_i( "ORG invalid offset value", arg[ 0 ]->var.constant.value );
		return( FALSE );
	}
	if( this_segment == NIL( segment_record )) {
		log_error( "ORG requires current segment is set" );
		return( FALSE );
	}
	if( this_segment->group != NIL( segment_group )) {
		if( this_segment->group->segments != this_segment ) {
			log_error_s( "ORG segment is not first in group", this_segment->group->name );
			return( FALSE );
		}
	}
	if( this_segment->posn != this_segment->start ) {
		log_error_s( "ORG segment already in use", this_segment->name );
		return( FALSE );
	}
	if( this_segment->fixed ) {
		if( this_segment->start != arg[ 0 ]->var.constant.value ) {
			log_error_s( "ORG segment offset inconsistent", this_segment->name );
			return( FALSE );
		}
	}
	else {
		this_segment->fixed = TRUE;
		this_segment->start = arg[ 0 ]->var.constant.value;
		this_segment->posn = this_segment->start;
	}
	return( TRUE );
}

/*
 *	Bring in a nested source file to assemble 'in-line' with the
 *	current file.
 *
 *		INCLUDE	"filename"
 */
static boolean process_dir_include( int args, token_record **arg, int *len ) {
	char	*fname;

	if( args != 1 ) {
		log_error( "INCLUDE requires filename argument" );
		return( FALSE );
	}
	if( arg[ 0 ]->id != tok_string ) {
		log_error( "INCLUDE expects quoted filename" );
		return( FALSE );
	}
	/*
	 *	Having elected to used a parsed assembly string
	 *	as a filename we need to convert it into a C
	 *	string so that all 'normal' calls can use it.
	 */
	fname = STACK_ARRAY( char, arg[ 0 ]->var.block.len+1 );
	memcpy( fname, arg[ 0 ]->var.block.ptr, arg[ 0 ]->var.block.len );
	fname[ arg[ 0 ]->var.block.len ] = EOS;
	/*
	 *	Now go for it.
	 */
	return( include_file( fname ));
}

/*
 *	Process directives.
 */
boolean process_directive( id_record *label, component dir, int args, token_record **arg, int *len ) {
	switch( dir ) {
		case asm_end: {
			if( !process_dir_end( args, arg, len )) return( FALSE );
			if( label ) {
				log_error_s( "Invalid label on END", label->id );
				return( FALSE );
			}
			return( TRUE );
		}
		case asm_db: {
			if( label ) if( !set_label_here( label, this_segment )) return( FALSE );
			return( process_dir_db( args, arg, len ));
		}
		case asm_dw: {
			if( label ) if( !set_label_here( label, this_segment )) return( FALSE );
			return( process_dir_dw( args, arg, len ));
		}
		case asm_reserve: {
			if( label ) if( !set_label_here( label, this_segment )) return( FALSE );
			return( process_dir_reserve( args, arg, len ));
		}
		case asm_align: {
			if( !process_dir_align( args, arg, len )) return( FALSE );
			if( label ) return( set_label_here( label, this_segment ));
			return( TRUE );
		}
		case asm_equ: {
			/*
			 *	Assign a value to a label
			 */
			return( process_dir_equ( label, args, arg, len ));
		}
		case asm_export: {
			/*
			 *	Provide a list of labels which are to be exported
			 *	to an external source.
			 */
			if(!( process_dir_export( args, arg, len ))) return( FALSE );
			if( label ) {
				log_error_s( "Invalid label on EXPORT", label->id );
				return( FALSE );
			}
			return( TRUE );
		}
		case asm_import: {
			/*
			 *	Provide a list of labels which are to be imported
			 *	from an external source.
			 */
			if(!( process_dir_import( args, arg, len ))) return( FALSE );
			if( label ) {
				log_error_s( "Invalid label on IMPORT", label->id );
				return( FALSE );
			}
			return( TRUE );
		}
		case asm_org: {
			/*
			 *	Define a fixed address required by
			 *	the assembler when generating position
			 *	dependent machine code.
			 */
			if( !process_dir_org( args, arg, len )) return( FALSE );
			if( label ) return( set_label_here( label, this_segment ));
			return( TRUE );
		}
		case asm_include: {
			/*
			 *	Include another files content into the
			 *	streamed assembly source code.
			 */
			if( !process_dir_include( args, arg, len )) return( FALSE );
			if( label ) {
				log_error_s( "Invalid label on INCLUDE", label->id );
				return( FALSE );
			}
			return( TRUE );
		}
		case asm_segment: {
			/*
			 *	Declare or change the current target segment.
			 */
			return( process_dir_segment( label, args, arg, len ));
		}
		case asm_group: {
			/*
			 *	Process a group of segments into one memory area.
			 */
			return( process_dir_group( label, args, arg, len ));
		}
		default: break;
	}
	log_error( "Directive not implemented" );
	return( FALSE );
}

/*
 *	EOF
 */
