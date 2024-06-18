/*
 *	assemble
 *	========
 *
 *	Conversion from tokens to assembled instruction
 */

#include "os.h"
#include "includes.h"

/*
 *	Assign a segment and offset to a label.
 *
 *	Return TRUE if the assignment is consistent with data already
 *	assigned to the label, FALSE otherwise.
 */
boolean set_label_here( id_record *label, segment_record *seg ) {
	if( seg == NIL( segment_record )) {
		log_error_s( "No segment set for label", label->id );
		return( FALSE );
	}
	switch( label->type ) {
		case class_unknown: {
			label->type = class_label;
			label->var.value.segment = seg;
			label->var.value.value = seg->posn;
			label->var.value.scope = scope_address;
			this_jiggle++;
			return( TRUE );
		}
		case class_label: {

			ASSERT( label->var.value.scope == scope_address );

			if( label->var.value.segment != seg ) break;
			if( label->var.value.value != seg->posn ) {
				if( BOOL( command_flags & more_verbose )) printf( "%s:%s %04x -> %04x\n", seg->name, label->id, label->var.value.value, seg->posn );
				label->var.value.value = seg->posn;
				this_jiggle++;
			}
			return( TRUE );
		}
		default: break;
	}
	log_error_s( "Redefinition of label", label->id );
	return( FALSE );
}



/*
 *	Routine interprets the sign information provided and
 *	set flags appropriately.
 */
static void set_sign_flags( instruction *mc, byte sign ) {

	ASSERT( mc != NIL( instruction ));
	
	switch( sign ) {
		case SIGN_IGNORED: {
			DPRINT(( "Data sign ignored.\n" ));
			mc->unsigned_data = FALSE;
			mc->signed_data = FALSE;
			break;
		}
		case SIGN_UNSIGNED: {
			DPRINT(( "Unsigned data.\n" ));
			mc->unsigned_data = TRUE;
			mc->signed_data = FALSE;
			break;
		}
		case SIGN_SIGNED: {
			DPRINT(( "Signed data.\n" ));
			mc->unsigned_data = FALSE;
			mc->signed_data = TRUE;
			break;
		}
		default: {
			ABORT( "Programmer coding error" );
			break;
		}
	}
	/*
	 *	Simpler coding option (but less easy when debugging).
	 *
	 *	mc->unsigned_data = ( sign == SIGN_UNSIGNED );
	 *	mc->signed_data = ( sign == SIGN_SIGNED );
	 */
}

/*
 *	Take an argument and append the mod_reg_rm byte to
 *	the instruction being created.
 *
 *	This routine will serve dual use encode both the generic
 *	'mode reg adrs' version of an EA byte and also the 'mode
 *	opcode adrs' version (used where immediate data is preset)
 *
 *	if 'reg' is NIL then op_code is valid (second format)
 *	otherwise 'reg' points to a register (first format).
 */
static boolean encode_ea( instruction *mc, ea_breakdown *reg, byte op_code, ea_breakdown *eadrs ) {
	byte		middle;
	opcode_prefix	sp;		/* Used for segment prefix bit */

	ASSERT( mc != NIL( instruction ));
	ASSERT( eadrs != NIL( ea_breakdown ));

	/*
	 *	+1	mmrrraaa	mm: Mode	00: Memory Mode (no disp)
	 *						01: Memory Mode (8 bit disp)
	 *						10: Memory Mode (16 bit disp)
	 *						11: Register Mode (aaa is a register)
	 *
	 *				rrr: Register		w=0	w=1
	 *						000:	AL	AX
	 * 						001:	CL	CX
	 * 						010:	DL	DX
	 * 						011:	BL	BX
	 * 						100:	AH	SP
	 * 						101:	CH	BP
	 * 						110:	DH	SI
	 * 						111:	BH	DI
	 *
	 * 				rrr:	Gets replaced by an 'opcode' when the EA is the target
	 *					of an operation with an immediate value.
	 *
	 *				aaa: EAdrs		m=00	m=01	m=10	m=11
	 *										w=0/w=1
	 *						000:	BX+SI	+D8	+D16	AL/AX
	 *						001:	BX+DI	+D8	+D16	CL/CX
	 *						010:	BP+SI	+D8	+D16	DL/DX
	 *						011:	BP+DI	+D8	+D16	BL/BX
	 *
	 *						100:	SI	+D8	+D16	AH/SP
	 *						101:	DI	+D8	+D16	CH/BP
	 *						110:	Direct	BP+D8	BP+D16	DH/SI
	 *						111:	BX	+D8	+D16	BH/DI
	 *
	 *	+2			Low Disp/Data
	 *	+3			High Disp/Data
	 *	+4			Low Data
	 *	+5			High Data
	 */

	/*
	 *	Simplify building EA bytes content.
	 */
#	define BUILD_EA_BYTE(m,r,a) (((m)<<6)|((r)<<3)|(a))

	if( reg ) {
		/*
		 *	First format of the EA byte.
		 */
		DPRINT(( "Encode EA: reg =" ));
		DCODE( show_ea_bitmap( reg->ea ));
		DPRINT(( ", EAdrs =" ));
		DCODE( show_ea_bitmap( eadrs->ea ));
		DPRINT(( "\n" ));

		ASSERT( BOOL( reg->ea & ea_all_reg ));
		ASSERT( reg->registers == 1 );

		middle = reg->reg[0]->reg_no;
	}
	else {
		/*
		 *	Second format of the EA byte.
		 */
		DPRINT(( "Encode EA: opcode = %d", op_code ));
		DPRINT(( ", EAdrs =" ));
		DCODE( show_ea_bitmap( eadrs->ea ));
		DPRINT(( "\n" ));

		ASSERT( op_code <= B111 );

		middle = op_code;
	}

	/*
	 *	In the following code the value of displacement
	 *	effectively selects the type of EA used.
	 *
	 *	Note that (until better information is obtained) all
	 *	displacements are considered SIGNED values.  Therefore
	 *	8 bit displacements are sign extended to 16 bits.
	 *
	 *	As an example a displacement of between 128 to 255 is
	 *	considered a 16 bit displacement.
	 *
	 *	TODO:  What 'do I do' if the constant value is 'scoped'
	 *	as an unsigned 16 bit value only?  This is not technically
	 *	an error, thought might be indicative of a actual error
	 *	inn the assembly source code.  Hmm.
	 */

	if( BOOL( eadrs->ea & ( ea_base_index_disp | ea_far_base_index_disp ))) {
		register_data	*bp;

		ASSERT( eadrs->registers == 2 );
		ASSERT( BOOL( eadrs->reg[ 0 ]->ac & ac_base_reg ) || BOOL( eadrs->reg[ 1 ]->ac & ac_base_reg ));
		ASSERT( BOOL( eadrs->reg[ 0 ]->ac & ac_index_reg ) || BOOL( eadrs->reg[ 1 ]->ac & ac_index_reg ));

		DPRINT(( "[Base+Index+Disp]\n" ));

		bp = eadrs->reg[0];
		if( !BOOL( bp->ac & ac_base_reg )) bp = eadrs->reg[1];

		ASSERT( BOOL( bp->ac & ac_base_reg ));
		ASSERT( bp->segment != UNREQUIRED_SEG );

		if(( sp = map_segment_prefix( eadrs->segment_override )) != no_prefix ) {
			mc->prefixes |= sp;
			mc->segment_overriden |= ( eadrs->segment_override != bp->segment );
		}
		if( eadrs->immediate_arg.value == 0 ) {
			ASSERT( mc->coded <= MAX_CODE_BYTES-1 );

			mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B00, middle, ( eadrs->reg[0]->base_index_reg_no + eadrs->reg[1]->base_index_reg_no ));
		}
		else if( BOOL( eadrs->immediate_arg.scope & scope_sbyte )) {
			ASSERT( mc->coded <= MAX_CODE_BYTES-2 );

			mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B01, middle, ( eadrs->reg[0]->base_index_reg_no + eadrs->reg[1]->base_index_reg_no ));
			mc->code[ mc->coded++ ] = eadrs->immediate_arg.value;
		}
		else {
			ASSERT( mc->coded <= MAX_CODE_BYTES-3 );

			mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B10, middle, ( eadrs->reg[0]->base_index_reg_no + eadrs->reg[1]->base_index_reg_no ));
			mc->code[ mc->coded++ ] = L( eadrs->immediate_arg.value );
			mc->code[ mc->coded++ ] = H( eadrs->immediate_arg.value );
		}
		return( TRUE );
	}
	if( BOOL( eadrs->ea & ( ea_index_disp | ea_base_disp | ea_far_index_disp | ea_far_base_disp ))) {
		ASSERT( eadrs->registers == 1 );
		ASSERT( BOOL( eadrs->reg[ 0 ]->ac & ac_word_reg ));

		DPRINT(( BOOL( eadrs->ea & ea_index_disp )? "[Index+Disp]\n": "[Base+Disp]\n" ));

		if(( sp = map_segment_prefix( eadrs->segment_override )) != no_prefix ) {
			mc->prefixes |=sp;
			mc->segment_overriden |= ( eadrs->segment_override != eadrs->reg[ 0 ]->segment );
		}
		if( eadrs->immediate_arg.value == 0 ) {
			if( eadrs->reg[0]->ptr_reg_no == B110 ) {
				/*
				 *	Note EA description; This is where [BP] is replaced
				 *	by the [immediate] action.  See later in this routine.
				 *	We will implement the byte disp version to achieve
				 *	the desired effect.
				 */
				ASSERT( mc->coded <= MAX_CODE_BYTES-2 );

				mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B01, middle, eadrs->reg[0]->ptr_reg_no );
				mc->code[ mc->coded++ ] = 0;
			}
			else {
				ASSERT( mc->coded <= MAX_CODE_BYTES-1 );

				mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B00, middle, eadrs->reg[0]->ptr_reg_no );
			}
		}
		else if( BOOL( eadrs->immediate_arg.scope & scope_sbyte )) {
			ASSERT( mc->coded <= MAX_CODE_BYTES-2 );

			mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B01, middle, eadrs->reg[0]->ptr_reg_no );
			mc->code[ mc->coded++ ] = eadrs->immediate_arg.value;
		}
		else {
			ASSERT( mc->coded <= MAX_CODE_BYTES-3 );

			mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B10, middle, eadrs->reg[0]->ptr_reg_no );
			mc->code[ mc->coded++ ] = L( eadrs->immediate_arg.value );
			mc->code[ mc->coded++ ] = H( eadrs->immediate_arg.value );
		}
		return( TRUE );
	}
	if( BOOL( eadrs->ea & ( ea_pointer_reg | ea_far_pointer_reg ))) {
		ASSERT( eadrs->registers == 1 );
		ASSERT( BOOL( eadrs->reg[ 0 ]->ac & ac_pointer_reg ));

		DPRINT(( "[Pointer]\n" ));

		if(( sp = map_segment_prefix( eadrs->segment_override )) != no_prefix ) {
			mc->prefixes |= sp;
			mc->segment_overriden |= ( eadrs->segment_override != eadrs->reg[ 0 ]->segment );
		}
		if( eadrs->reg[0]->ptr_reg_no == B110 ) {	/* See above */
			ASSERT( mc->coded <= MAX_CODE_BYTES-2 );

			mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B01, middle, eadrs->reg[0]->ptr_reg_no );
			mc->code[ mc->coded++ ] = 0;
		}
		else {
			ASSERT( mc->coded <= MAX_CODE_BYTES-1 );

			mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B00, middle, eadrs->reg[0]->ptr_reg_no );
		}
		return( TRUE );
	}
	if( BOOL( eadrs->ea & ( ea_indirect | ea_far_indirect ))) {
		DPRINT(( "[Disp]\n" ));
		ASSERT( mc->coded <= MAX_CODE_BYTES-3 );

		mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B00, middle, B110 );
		mc->code[ mc->coded++ ] = L( eadrs->immediate_arg.value );
		mc->code[ mc->coded++ ] = H( eadrs->immediate_arg.value );
		return( TRUE );
	}
	if( BOOL( eadrs->ea & ea_all_reg )) {
		ASSERT( eadrs->registers == 1 );
		ASSERT( BOOL( eadrs->reg[ 0 ]->ac & ( ac_byte_reg | ac_word_reg )));

		DPRINT(( "Register\n" ));
		ASSERT( mc->coded <= MAX_CODE_BYTES-1 );

		mc->code[ mc->coded++ ] = BUILD_EA_BYTE( B11, middle, eadrs->reg[ 0 ]->reg_no );
		return( TRUE );
	}
	if( BOOL( eadrs->ea & ( ea_immediate | ea_far_immediate ))) {
		DPRINT(( "Immediate\n" ));
		ABORT( "Uncoded" ); /* TODO??? Is this even right? */
		return( TRUE );
	}
	log_error( "Unrecognised Effective Address" );
	return( FALSE );
}

/*
 *	Take an argument and determine the data size being
 *	operated on from it.
 */
static boolean encode_ids( instruction *mc, ea_breakdown *arg ) {

	ASSERT( mc != NIL( instruction ));
	ASSERT( arg != NIL( ea_breakdown ));

	DPRINT(( "Encode IDS for EA" ));
	DCODE( show_ea_bitmap( arg->ea ));
	DPRINT(( "\n" ));

	if( BOOL( arg->ea & ea_all_reg )) {
		
		ASSERT( arg->registers == 1 );

		DPRINT(( "Register components" ));
		DCODE( show_ac_bitmap( arg->reg[ 0 ]->ac ));
		DPRINT(( "\n" ));

		if( arg->mod != no_modifier ) {
			log_error( "Register sizes cannot be modified" );
			return( FALSE );
		}
		mc->byte_data = BOOL( arg->reg[ 0 ]->ac & ac_byte_reg );
		mc->word_data = !mc->byte_data;
		mc->near_data = FALSE;
		mc->far_data = FALSE;
	}
	else {
		mc->byte_data = BOOL( arg->mod & byte_modifier );
		mc->word_data = BOOL( arg->mod & word_modifier );
		mc->near_data = BOOL( arg->mod & near_modifier );
		mc->far_data = BOOL( arg->mod & far_modifier );
	}
	DPRINT(( "Data size set to '%s'\n", ( mc->far_data? "far": ( mc->near_data? "near": ( mc->word_data? "word": ( mc->byte_data? "byte": "Unknown" ))))));
	return( TRUE );
}

/*
 *	Handle the conversion from label based addresses to
 *	relative (to IP) based distances.
 */
static boolean encode_rel( instruction *mc, constant_value *v, byte w, byte i, byte b ) {
	integer	d;
	
	
	ASSERT( mc != NIL( instruction ));
	ASSERT( v != NIL( constant_value ));
	ASSERT( w != 0 );
	ASSERT( this_pass != no_pass );

#ifdef VERIFICATION
	if(( this_pass == pass_label_gathering )||( this_pass == data_verification ))
#else
	if( this_pass == pass_label_gathering )
#endif
	{
		/*
		 *	In the label gathering pass we will assert an
		 *	displacement of 0 (zero).  This is because it
		 *	is possible that the displacement is positive
		 *	(ie forwards) and the label has not yet been
		 *	identified.
		 */
		DPRINT(( "Assuming 0 byte displacement.\n" ));

		if( BOOL( w & RANGE_BYTE )) {
			/*
			 *	Byte relative allowed.
			 */
			ASSERT( mc->coded <= MAX_CODE_BYTES - sizeof( byte ));

			mc->code[ mc->coded++ ] = 0;
			return( TRUE );
		}

		/*
		 *	Default to word relative.
		 */
		ASSERT( BOOL( w & RANGE_WORD ));
		ASSERT( mc->coded <= MAX_CODE_BYTES-sizeof( word ));

		mc->code[ mc->coded++ ] = 0;
		mc->code[ mc->coded++ ] = 0;
		return( TRUE );
	}
	/*
	 *	If the segment pointer is empty at this point then the
	 *	immediate value is not a label.  We consider this an
	 *	error.
	 */
	if( v->segment == NIL( segment_record )) {
		log_error( "Invalid target for relative location calculation" );
		return( FALSE );
	}
	/*
	 *	If we are trying to produce a relative distance to a
	 *	location in another segment then this is also a mess.
	 */
	if( v->segment != this_segment ) {
		log_error( "Relative location target in different segment" );
		return( FALSE );
	}
	/*
	 *	The value in the constant is an offset in the current
	 *	segment we are working in so all that's required is to
	 *	work out if its an 8 bit or 16 bit displacement.
	 */
	if( BOOL( w & RANGE_BYTE )) {
		/*
		 *	If we can do it in a byte then calculate it and
		 *	check that it fits.
		 */
		d = v->value - ( this_segment->posn + mc->coded + sizeof( byte ));

		DPRINT(( "Byte displacement is %d.\n", (int)d ));
	
		if( BOOL( get_scope( d ) & scope_sbyte )) {
			/*
			 *	Everything is in range .. do it.
			 */
			DPRINT(( "Relative displacement in byte range.\n" ));
			
			ASSERT( mc->coded <= MAX_CODE_BYTES - sizeof( byte ));

			mc->code[ mc->coded++ ] = (byte)d;
			return( TRUE );
		}
		if( w == RANGE_BYTE ) {
			log_error_i( "Displacement out of range (signed byte)", d );
			return( FALSE );
		}
	}
	/*
	 *	Word displacement required.
	 */
	ASSERT( BOOL( w & RANGE_WORD ));
	
	d = v->value - ( this_segment->posn + mc->coded + sizeof( word ));

	DPRINT(( "Word displacement is %d.\n", (int)d ));
	
	if( BOOL( get_scope( d ) & scope_sword )) {
		/*
		 *	Everything is in range .. do it.
		 */
		DPRINT(( "Relative displacement in word range.\n" ));
		
		/*
		 *	Fix up opcode if (and only if) a byte
		 *	displacement was an option.
		 */
		if( BOOL( w & RANGE_BYTE )) {
			ASSERT( i < mc->coded );
			ASSERT( b < 8 );

			mc->code[ i ] ^= 1 << b;
		}

		ASSERT( mc->coded <= MAX_CODE_BYTES - sizeof( word ));

		mc->code[ mc->coded++ ] = L( d );
		mc->code[ mc->coded++ ] = H( d );

		/*
		 *	Huzzar!
		 */
		return( TRUE );
	}
	/*
	 *	This is a real mess!
	 */
	log_error_i( "Displacement out of range (signed word)", d );
	return( FALSE );
}

/*
 *	Encode an Immediate value into the opcode.
 */
static boolean encode_imm( instruction *mc, constant_value *v ) {
	ASSERT( mc != NIL( instruction ));
	ASSERT( v != NIL( constant_value ));
	
	if( mc->far_data ) {
		/*
		 *	This is a reference to a specific FAR address
		 *	part of a JuMP or CALL to a subroutine.
		 */
		ASSERT( !mc->word_data );
		ASSERT( !mc->signed_data );
		ASSERT( mc->unsigned_data );
		ASSERT( !mc->near_data );
		
		ASSERT(( mc->coded + sizeof( dword )) <= MAX_CODE_BYTES );
		
#ifdef VERIFICATION
		if( this_pass == data_verification ) {
			mc->code[ mc->coded++ ] = L( v->value );
			mc->code[ mc->coded++ ] = H( v->value );
			mc->code[ mc->coded++ ] = 0;
			mc->code[ mc->coded++ ] = 0;
			return( TRUE );
		}
#endif

		if( this_pass == pass_label_gathering ) {
			/*
			 *	In this pass there will be times that the
			 *	label is undefined, and hence has no segment.
			 *	We will fake some data to get through this
			 *	pass while creating the right volume of code.
			 */

			DPRINT(( "Incomplete immediate far address = %04x\n", v->value ));

			mc->code[ mc->coded++ ] = L( v->value );
			mc->code[ mc->coded++ ] = H( v->value );
			mc->code[ mc->coded++ ] = 0;
			mc->code[ mc->coded++ ] = 0;
		}
		else {

			if( !BOOL( v->scope & scope_address )) {
				log_error( "Invalid immediate value (far address)." );
				return( FALSE );
			}
			if( v->segment == NIL( segment_record )) {
				log_error( "Far label has no segment" );
				return( FALSE );
			}

			DPRINT(( "Immediate far address = %04x:%04x\n", v->segment->start, v->value ));

			mc->code[ mc->coded++ ] = L( v->value );
			mc->code[ mc->coded++ ] = H( v->value );
			if( v->segment->group ) {
				mc->code[ mc->coded++ ] = L( v->segment->group->page );
				mc->code[ mc->coded++ ] = H( v->segment->group->page );
			}
			else {
				mc->code[ mc->coded++ ] = 0;
				mc->code[ mc->coded++ ] = 0;
			}
		}
		return( TRUE );
	}
	if( mc->near_data ) {
		/*
		 *	This is a reference to a specific NEAR address
		 *	part of a JuMP or CALL to a subroutine.
		 */
		ASSERT( !mc->word_data );
		ASSERT( !mc->signed_data );
		ASSERT( mc->unsigned_data );
		ASSERT( !mc->far_data );
		
		ASSERT(( mc->coded + sizeof( word )) <= MAX_CODE_BYTES );
		
#ifdef VERIFICATION
		if( this_pass == data_verification ) {
			mc->code[ mc->coded++ ] = L( v->value );
			mc->code[ mc->coded++ ] = H( v->value );
			return( TRUE );
		}
#endif

		if( this_pass == pass_label_gathering ) {
			/*
			 *	In this pass there will be times that the
			 *	label is undefined, and hence has no segment.
			 *	We will fake some data to get through this
			 *	pass while creating the right volume of code.
			 */

			DPRINT(( "Incomplete immediate near address = %04x\n", v->value ));

			mc->code[ mc->coded++ ] = L( v->value );
			mc->code[ mc->coded++ ] = H( v->value );
		}
		else {
			if( !BOOL( v->scope & scope_address )) {
				log_error( "Invalid immediate value (near address)." );
				return( FALSE );
			}
			if( v->segment == NIL( segment_record )) {
				log_error( "Near label has no segment" );
				return( FALSE );
			}
			if( v->segment != this_segment ) {
				log_error( "Near label in different segment" );
				return( FALSE );
			}

			DPRINT(( "Immediate near address = %04x\n", v->value ));

			mc->code[ mc->coded++ ] = L( v->value );
			mc->code[ mc->coded++ ] = H( v->value );
		}
		return( TRUE );
	}
	if( mc->word_data ) {
		/*
		 *	Word sized immediate
		 */

#ifdef VERIFICATION
		if( this_pass == data_verification ) {
			mc->code[ mc->coded++ ] = L( v->value );
			mc->code[ mc->coded++ ] = H( v->value );
			return( TRUE );
		}
#endif

		if( !BOOL( v->scope & scope_address )) {
			if( mc->signed_data && !BOOL( v->scope & scope_sword )) {
				log_error( "Immediate value out of range (signed word)." );
				return( FALSE );
			}
			if( mc->unsigned_data && !BOOL( v->scope & scope_uword )) {
				log_error( "Immediate value out of range (unsigned word)." );
				return( FALSE );
			}
			if( !BOOL( v->scope & scope_word )) {
				log_error( "Immediate value out of range (word)." );
				return( FALSE );
			}
		}
		
		ASSERT(( mc->coded + sizeof( word )) <= MAX_CODE_BYTES );

		DPRINT(( "Immediate word = %04x\n", v->value ));

		mc->code[ mc->coded++ ] = L( v->value );
		mc->code[ mc->coded++ ] = H( v->value );
		return( TRUE );
	}
	/*
	 *	Byte sized immediate.
	 */
	ASSERT( mc->byte_data );
	
#ifdef VERIFICATION
	if( this_pass == data_verification ) {
		mc->code[ mc->coded++ ] = v->value;
		return( TRUE );
	}
#endif

	if( mc->signed_data && !BOOL( v->scope & scope_sbyte )) {
		log_error( "Immediate value out of range (signed byte)." );
		return( FALSE );
	}
	if( mc->unsigned_data && !BOOL( v->scope & scope_ubyte )) {
		log_error( "Immediate value out of range (unsigned byte)." );
		return( FALSE );
	}
	if( !BOOL( v->scope & scope_byte )) {
		log_error( "Immediate value out of range (byte)." );
		return( FALSE );
	}
	
	ASSERT(( mc->coded + sizeof( byte )) <= MAX_CODE_BYTES );

	DPRINT(( "Immediate byte = %02x\n", v->value ));

	if( !BOOL( v->scope & scope_byte )) {
		log_error( "Immediate value out of scope" );
		return( FALSE );
	}
	mc->code[ mc->coded++ ] = v->value;
	return( TRUE );
}

/*
 *	Confirm that the size of the argument provided is compatible
 *	with the size data already gathered in the instruction data.
 */
static boolean perform_vds( instruction *mc, ea_breakdown *arg ) {
	ASSERT( mc != NIL( instruction ));
	ASSERT( arg != NIL( ea_breakdown ));

	if( this_pass == pass_label_gathering ) return( TRUE );

	DPRINT(( "Signed data = %d\n", mc->signed_data ));
	DPRINT(( "Unsigned data = %d\n", mc->unsigned_data ));
	DPRINT(( "Byte data = %d\n", mc->byte_data ));
	DPRINT(( "Word data = %d\n", mc->word_data ));
	DPRINT(( "Near data = %d\n", mc->near_data ));
	DPRINT(( "Far data = %d\n", mc->far_data ));
	
	switch( arg->ea ) {
		case ea_byte_acc:
		case ea_byte_reg: {
			return( mc->byte_data );
		}
		case ea_word_acc:
		case ea_word_reg: {
			return( mc->word_data || mc->near_data );
		}
		case ea_immediate: {
			DPRINT(( "Immediate value =" ));
			DCODE( dump_value( &( arg->immediate_arg )));
			DPRINT(( "\n" ));

			if( mc->byte_data ) return( BOOL( arg->immediate_arg.scope & scope_byte ));
			if( mc->word_data ) return( BOOL( arg->immediate_arg.scope & scope_word ));
			if( mc->near_data || mc->far_data ) return( BOOL( arg->immediate_arg.scope & scope_address ));
			return( FALSE );
		}
		case ea_indirect:
		case ea_pointer_reg:
		case ea_base_disp:
		case ea_index_disp:
		case ea_base_index_disp: {
			if( BOOL( arg->mod & byte_modifier )) return( mc->byte_data );
			if( BOOL( arg->mod & word_modifier )) return( mc->word_data );
			if( BOOL( arg->mod & near_modifier )) return( mc->near_data );
			if( BOOL( arg->mod & far_modifier )) return( mc->far_data );
			return( TRUE );
		}
		case ea_segment_reg: {
			return( mc->word_data || mc->near_data );
		}
		case ea_far_immediate:
		case ea_far_indirect:
		case ea_far_pointer_reg:
		case ea_far_base_disp:
		case ea_far_index_disp:
		case ea_far_base_index_disp: {
			return( mc->far_data );
		}
		default: {
			DPRINT(( "Unrecognised EA" ));
			DCODE( show_ea_bitmap( arg->ea ));
			DPRINT(( "\n" ));
			ABORT( "Programmer error" );
			break;
		}
	}
	/*
	 *	Well, what do we do here?  If we have not
	 *	answered the question by the time we get here
	 *	then its probably broken!
	 */
	return( FALSE );
}

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
boolean assemble_inst( opcode *inst, opcode_prefix prefs, ea_breakdown *arg, instruction *mc ) {
	
	int		i;

	ASSERT( inst != NIL( opcode ));
	ASSERT( arg != NIL( ea_breakdown ));
	
	DPRINT(( "Assemble op '%s (%d args)", component_text( inst->op ), inst->args ));
	DCODE( for( int a = 0; a < inst->args; a++ ) show_ea_bitmap( inst->arg[ a ] ));
	DPRINT(( "'\n" ));

	/*
	 *	Verify prefix data against instruction criteria.
	 */
	if( BOOL( prefs & ~inst->prefs )) {
		log_error( "Invalid prefix for instruction" );
		return( FALSE );
	}
	/*
	 *	Clear instruction.
	 */
	mc->prefixes = prefs;		/* Gathered from before the opcode */
	mc->coded = 0;
	mc->segment_overriden = FALSE;
	mc->byte_data = FALSE;
	mc->word_data = FALSE;
	mc->near_data = FALSE;
	mc->far_data = FALSE;
	mc->unsigned_data = FALSE;
	mc->signed_data = FALSE;
	mc->reg_is_dest = TRUE;
	/*
	 *	Step through the encoding instructions.
	 */
	for( i = 0; i < inst->encoded; i++ ) {
		word	e;

		e = inst->encode[ i ];
		switch( GET_ACT( e )) {
			case SB_ACT: {
				/*
				 *	Set Byte
				 *
				 *	Provide static data forming the basic for the machine
				 *	code instruction.
				 *
				 *	SB(v)		v = value to add to instruction
				 */

				ASSERT( mc->coded < MAX_CODE_BYTES );

				DPRINT(( "Set Byte %d -> %0X\n", mc->coded, SB_VALUE( e )));

				mc->code[ mc->coded++ ] = SB_VALUE( e );
				break;
			}
			case EA_ACT: {
				ASSERT( EA_REG( e ) < inst->args );
				ASSERT( EA_EADRS( e ) < inst->args );
				ASSERT( EA_EADRS( e ) != EA_REG( e ));
				
				DPRINT(( "Effective Address (reg @ %d, EAdrs @ %d).\n", EA_REG( e ), EA_EADRS( e )));

				if( !encode_ea( mc, &( arg[ EA_REG( e )]), 0, &( arg[ EA_EADRS( e )]))) return( FALSE );
				break;
			}
			case EAO_ACT: {
				ASSERT( EAO_EADRS( e ) < inst->args );
				ASSERT( EAO_EADRS( e ) != EA_REG( e ));
				
				DPRINT(( "Effective Address Opcode (OP:%d, EAdrs @ %d).\n", EAO_OPCODE( e ), EAO_EADRS( e )));

				if( !encode_ea( mc, NIL( ea_breakdown ), EAO_OPCODE( e ), &( arg[ EA_EADRS( e )]))) return( FALSE );
				break;
			}
			case IMM_ACT: {
				ASSERT( IMM_ARG( e ) < inst->args );
				ASSERT( BOOL( arg[ IMM_ARG( e )].ea & ( ea_immediate | ea_far_immediate )));
				
				DPRINT(( "Immediate value (arg %d).\n", IMM_ARG( e )+1 ));
				
				if( !encode_imm( mc, &( arg[ IMM_ARG( e )].immediate_arg ))) return( FALSE );
				break;
			}
			case IDS_ACT: {
				/*
				 *	Identify Data Size.
				 *
				 *	Determine, from the indicated argument, the size of the
				 *	data to be handled (byte or word).
				 *
				 *	IDS(a,g)	a = Argument Number
				 *			g = Sign,	0:Ignore
				 *					1:Unsigned
				 *					2:Signed
				 */
				 
				ASSERT( IDS_ARG( e ) < inst->args );
				
				DPRINT(( "Identify Data Size (arg %d).\n", IDS_ARG( e )));
				DPRINT(( "Identify Data Sign (%d).\n", IDS_SIGN( e )));

				set_sign_flags( mc, IDS_SIGN( e ));
				if( !encode_ids( mc, &( arg[ IDS_ARG( e )]))) return( FALSE );
				break;
			}
			case FDS_ACT: {
				DPRINT(( "Fix Data Size (%d).\n", FDS_SIZE( e )));
				DPRINT(( "Fix Data Sign (%d).\n", FDS_SIGN( e )));

				set_sign_flags( mc, FDS_SIGN( e ));
				switch( FDS_SIZE( e )) {
					case DATA_SIZE_BYTE: {
						mc->byte_data = TRUE;
						break;
					}
					case DATA_SIZE_WORD: {
						mc->word_data = TRUE;
						break;
					}
					case DATA_SIZE_NEAR: {
						mc->near_data = TRUE;
						break;
					}
					case DATA_SIZE_FAR: {
						mc->far_data = TRUE;
						break;
					}
					default: {
						ABORT( "Programmer Error" );
						break;
					}
				}
				break;
			}
			case SDS_ACT: {
				byte	b;

				ASSERT( SDS_INDEX( e ) < mc->coded );
				ASSERT( SDS_BIT( e ) < 8 );
				ASSERT( !mc->far_data );
				
				DPRINT(( "Set Data Size (byte %d, bit %d) to '%s'.\n", SDS_INDEX( e ), SDS_BIT( e ), ( mc->word_data? "word": "byte" )));

				b = BIT( SDS_BIT( e ));
				if( mc->word_data ) {
					mc->code[ SDS_INDEX( e )] |= b;
				}
				else {
					mc->code[ SDS_INDEX( e )] &= ~b;
				}
				break;
			}
			case SDR_ACT: {
				byte	b;

				ASSERT( SDR_INDEX( e ) < mc->coded );
				ASSERT( SDR_BIT( e ) < 8 );
				DPRINT(( "Set Direction (byte %d, bit %d) to '%s'.\n", SDS_INDEX( e ), SDS_BIT( e ), ( SDR_DIR( e )? "Reg <- EA": "EA <- Reg" )));

				b = BIT( SDR_BIT( e ));
				if(( mc->reg_is_dest = SDR_DIR( e ))) {
					mc->code[ SDR_INDEX( e )] |= b;
				}
				else {
					mc->code[ SDR_INDEX( e )] &= ~b;
				}
				break;
			}
			case REG_ACT: {
				byte	r;

				ASSERT( REG_ARG( e ) < inst->args );
				ASSERT( REG_INDEX( e ) < mc->coded );
				ASSERT( REG_BIT( e ) < 8 );
				ASSERT( BOOL( arg[ REG_ARG( e )].ea & ea_all_reg ));
				ASSERT( arg[ REG_ARG( e )].registers == 1 );
				ASSERT( arg[ REG_ARG( e ) ].reg[ 0 ] != NIL( register_data ));

				r = arg[ REG_ARG( e ) ].reg[ 0 ]->reg_no;

				ASSERT( r < 8 );

				DPRINT(( "Set Register (byte %d, bit %d) to %d.\n", REG_INDEX( e ), REG_BIT( e ), r ));

				mc->code[ REG_INDEX( e )] |= r << REG_BIT( e );
				
				break;
			}
			case ESC_ACT: {
				/* ESC, The Co-Processor Escape Mechanism.	*/
				/* Syntax:					*/
				/*	ESC imm, effective_address		*/
				/* Encoding:					*/
				/*	[11 011 xxx][mm xxx aaa]		*/
				/* Where:					*/
				/*	xxx xxx	:Co-Processor opcode/inst'n	*/
				/*	mm	:Addressing mode (11 is ignored)*/
				/*	aaa	:Address Specifics		*/
				constant_value	*v;

				ASSERT( ESC_ARG( e ) < inst->args );
				ASSERT( !mc->far_data );
				ASSERT( !mc->word_data );
				ASSERT( mc->coded >= 2 );

				v = &( arg[ ESC_ARG( e )].immediate_arg );

#ifdef VERIFICATION
				/*
				 *	If, and ONLY if, we are in data verification mode
				 *	then we will explicitly round off any supplied
				 *	immediate value to be in the "correct range" and
				 *	so ensure the range check passes.
				 */
				if( this_pass == data_verification ) v->value &= 0x3F;
#endif

				if(( v->value < 0 )||( v->value > 63 )) {
					log_error_i( "Co-processor opcode out of range", v->value );
					return( FALSE );
				}

				DPRINT(( "Co-processor opcode = %d\n", v->value ));

				mc->code[ 0 ] |= ( v->value >> 3 ) & 7;		/* High order bits in first byte */
				mc->code[ 1 ] |= ( v->value & 7 ) << 3;		/* Low order bits in second byte */
				break;
			}
			case REL_ACT: {
				constant_value	*v;

				ASSERT( REL_ARG( e ) < inst->args );
				ASSERT( BOOL( arg[ REL_ARG( e )].ea & ea_immediate ));

				v = &( arg[ REL_ARG( e )].immediate_arg );

				DPRINT(( "Relative address conversion.\n" ));

				if( !encode_rel( mc, v, REL_RANGE( e ), REL_INDEX( e ), REL_BIT( e ))) return( FALSE );
				break;
			}
			case VDS_ACT: {
				ASSERT( VDS_ARG( e ) < inst->args );
				
				DPRINT(( "Verify data size (arg %d).\n", VDS_ARG( e )+1 ));
				
				if( !perform_vds( mc, &( arg[ VDS_ARG( e )]))) {

#ifdef VERIFICATION
					/*
					 *	The error is only conditional if VERIFICATION has
					 *	been compiled in and *and* we are outputting table
					 *	verification data.
					 */
					if( this_pass != data_verification )
#endif

						/*
						 *	Error being (optionally) conditionally generated.
						 */
						log_error_i( "Argument incompatible with data size", VDS_ARG( e )+1 );
					return( FALSE );
				}
				break;
			}
			default: {
				ABORT( "Programmer Error!" );
			}
		}
	}
	/*
	 *	Instruction assembled successfully.
	 */
	return( TRUE );
}

/*
 *	Output the machine code data from the supplied instruction record.
 */
static boolean generate_inst( instruction *mc ) {
	/*
	 *	Output the machine instruction generated, we will assume that
	 *	if no coded bytes are generated then any prefix is unnecessary.
	 */
	if( mc->coded > 0 ) {
		byte	*ops,
			*fill;
		int	i, j;

		DPRINT(( "Output %d code bytes\n", mc->coded ));

		ops = STACK_ARRAY( byte, mc->coded + MAX_PREFIX_BYTES );
		if(( j = encode_prefix_bytes( mc->prefixes, ops, MAX_PREFIX_BYTES )) == ERROR ) return( FALSE );
		fill = &( ops[ j ]);
		for( i = 0; i < mc->coded; *fill++ = mc->code[ i++ ]);
		output_data( ops, j + i );
		return( TRUE );
	}
	log_error( "No code generated" );
	return( FALSE );
}

/*
 *	Conversion lookup table from arg_components to an actual
 *	effective address value.
 */
typedef struct {
	arg_component		ac;
	effective_address	ea,
				far_ea;
} ac_conversion;

static ac_conversion convert_ac_to_ea[] = {
	{ ac_brackets|ac_base_reg|ac_index_reg|ac_immediate,	ea_base_index_disp,	ea_far_base_index_disp	},
	{ ac_brackets|ac_base_reg|ac_immediate,			ea_base_disp,		ea_far_base_disp	},
	{ ac_brackets|ac_index_reg|ac_immediate,		ea_index_disp,		ea_far_index_disp	},
	{ ac_brackets|ac_pointer_reg,				ea_pointer_reg,		ea_far_pointer_reg	},
	{ ac_brackets|ac_immediate,				ea_indirect,		ea_far_indirect		},
	{ ac_acc_reg|ac_byte_reg,				ea_byte_acc,		ea_empty		},
	{ ac_acc_reg|ac_word_reg,				ea_word_acc,		ea_empty		},
	{ ac_byte_reg,						ea_byte_reg,		ea_empty		},
	{ ac_word_reg,						ea_word_reg,		ea_empty		},
	{ ac_segment_reg,					ea_segment_reg,		ea_empty		},
	{ ac_immediate,						ea_immediate,		ea_far_immediate	},
	{ ac_empty,						ea_empty,		ea_empty		}
};

static effective_address convert_ac( arg_component ac, boolean has_far ) {
	ac_conversion	*look;

	DPRINT(( "Convert AC" ));
	DCODE( show_ac_bitmap( ac ));
	DPRINT(( "\n" ));

	for( look = convert_ac_to_ea; look->ac != ac_empty; look++ ) if(( ac & look->ac ) == look->ac ) break;

	DPRINT(( "To EA" ));
	DCODE( show_ea_bitmap( look->ea ));
	DPRINT(( "\n" ));

	return( has_far? look->far_ea: look->ea );
}

/*
 *	Conversion of an opcode and a series of arguments into
 *	a recognised assembly instruction.
 */
boolean process_opcode( opcode_prefix prefs, modifier mods, component op, int args, token_record **arg, int *len ) {
	ea_breakdown	*format,
			*fill;
	int		a;
	opcode		*search;

	/*
	 *	Keep in mind that, during the work of this routine
	 *	the array of argument pointers are links into the
	 *	middle of a linked list.  Only the len array tells the
	 *	routine how many tokens make up a single argument.
	 *
	 * 	Also note that the line of tokens has an 'end_of_line'
	 *	token at the end of it.  This means that all argument
	 *	tokens always have a valid next pointer, even if (as
	 *	with the last token in an argument) it points to a
	 *	token outside the argument being examined.
	 */
	format = STACK_ARRAY( ea_breakdown, args+1 );
	format[ args ].ea = ac_empty;
	/*
	 *	Identify the nature of each argument (what each
	 *	argument effective "is") then with the opcode
	 *	we can properly identify which instruction is
	 *	required.
	 */
	a = 0;
	fill = format;
	while( a < args ) {
		arg_component	ac;
		token_record	*look;
		int		left;
		boolean		separator_rqd,
				negative_sep;

		DPRINT(( "ARG %d\n", a ));

		ac = ac_empty;
		fill->ea = ea_empty;
		fill->mod = no_modifier;
		fill->registers = 0;
		fill->segment_override = UNKNOWN_SEG;
		look = arg[ a ];
		left = len[ a ];
		/*
		 *	Any modifiers preceding the argument?  With this
		 *	assembler (for the moment) only a single modifier
		 *	can be applied.
		 */
		while( left && is_modifier( look->id )) {
			DPRINT(( "'%s' modified\n", component_text( look->id )));

			if( look->id == mod_ptr ) {
				if( !BOOL( fill->mod & ( range_modifiers | size_modifiers ))) {
					log_error( "PTR must follow size or range modifier" );
					return( FALSE );
				}
				if( BOOL( fill->mod & ptr_modifier )) {
					log_error( "Use PTR modifier only once" );
					return( FALSE );
				}
			}
			else {
				if( BOOL( fill->mod & ( range_modifiers | size_modifiers ))) {
					log_error( "Only one size or range modifier allowed" );
					return( FALSE );
				}
			}
			fill->mod |= map_modifier( look->id );
			left--;
			look = look->next;
		}
		/*
		 *	Is this a memory access argument?
		 */
		if(( left > 0 )&&( look->id == tok_obracket )) {
			token_record	*last;
			/*
			 *	An OPEN BRACKET, so some sort of indirection.
			 *	Need to find the CLOSE BRACKET (at the end)
			 *	and ignore that for the rest of the scan.
			 */
			DPRINT(( "BRACKET symbols\n" ));

			last = index_token( look, left-1 );

			ASSERT( last != NIL( token_record ));

			if( last->id != tok_cbracket ) {
				log_error( "Unmatched open bracket" );
				return( FALSE );
			}
			ac |= ac_brackets;
			left -= 2;
			look = look->next;
		}
		/*
		 *	What is left now is a series of '+' separated elements
		 *	which we can loop through and fill out in the effective
		 *	address record for this argument.
		 */
		negative_sep = FALSE;
		separator_rqd = FALSE;
		while( left ) {
			register_data	*rd;

			if( separator_rqd ) {
				if(( look->id != tok_plus )&&( look->id != tok_minus )) {
					log_error_i( "Expecting a separator (+/-) within argument %d.", a+1 );
					return( FALSE );
				}
				DPRINT(( "Separator (+/-)\n" ));
				negative_sep = ( look->id == tok_minus );
				left--;
				look = look->next;
				separator_rqd = FALSE;
			}
			else {
				if(( rd = register_component( look->id ))) {

					DPRINT(( "Recognised register '%s'\n", component_text( look->id )));
					DPRINT(( "Register components" ));
					DCODE( show_ac_bitmap( rd->ac ));
					DPRINT(( "\n" ));

					/*
					 *	Before going full on adding a register lets see if this
					 *	is a segment override definition (look for the colon).
					 */
 
					ASSERT( look->next != NIL( token_record ));

					if( BOOL( rd->ac & ac_segment_reg ) && ( look->next->id == tok_colon )) {
						if( BOOL( fill->ea & ac_seg_override )) {
							log_error( "Multiple segments specified" );
							return( FALSE );
						}
						ac |= ac_seg_override;
						fill->segment_override = rd->reg_no;
						left -= 2;
						look = look->next->next;
					}
					else {
						if( fill->registers == MAX_REGISTERS ) {
							log_error( "Too many registers specified" );
							return( FALSE );
						}
						if( negative_sep ) {
							log_error( "Registers can only be added" );
							return( FALSE );
						}
						fill->reg[ fill->registers++ ] = rd;
						ac |= rd->ac;
						left--;
						look = look->next;
						separator_rqd = TRUE;
					}
				}
				else if(( look->id == tok_label )&&( look->var.label->type == class_segment )) {

					DPRINT(( "SEGMENT identifier\n" ));

					if( BOOL( fill->ea & ac_seg_override )) {
						log_error( "Multiple segments specified" );
						return( FALSE );
					}
					ac |= ac_seg_override;
					fill->segment_override = look->var.label->var.segment->seg_reg;
					left--;
					look = look->next;
					if( look->id != tok_colon ) {
						log_error( "Colon missing after segment specification" );
						return( FALSE );
					}
					left--;
					look = look->next;
				}
				else {
					int		used;
					constant_value	val;

					/*
					 *	At this point we (probably) should have
					 *	an expression of some sort.
					 */
					if( !evaluate( look, left, &used, &val, negative_sep )) {
						log_error( "Error detected in constant expression" );
						return( FALSE );
					}

					DPRINT(( "EXPRESSION identified\n" ));

					look = index_token( look, used );
					left -= used;
					if( BOOL( fill->ea & ac_immediate )) {
						log_error( "Multiple constant expressions" );
						return( FALSE );
					}
					ac |= ac_immediate;
					fill->immediate_arg = val;
					separator_rqd = TRUE;
				}
			}
		}

		DPRINT(( "Arg %d =", a ));
		DCODE( show_ac_bitmap( ac ));
		DPRINT(( "\n" ));

		/*
		 *	Convert to a proper EA value.
		 */
		if(( fill->ea = convert_ac( ac, BOOL( fill->mod & far_modifier ))) == ea_empty ) {
			log_error_i( "Unrecognised opcode argument.", a+1 );
			return( FALSE );
		}
		/*
		 *	To the next argument.
		 */
		a++;
		fill++;
	}

	/*
	 *	So .. at this point we have gathered everything together
	 *	all the bits and pieces which will allow us to identify
	 *	what actual opcode the programmer was trying to code.
	 *
	 *	In theory.
	 */
	if(( search = find_opcode( mods, op, args, format ))) {
		instruction mc;
				
		if( !assemble_inst( search, prefs, format, &mc )) return( FALSE );
		return( generate_inst( &mc ));
	}
	/*
	 *	Getting here means we have not found what we are
	 *	looking for.
	 */
	log_error( "Assembler instruction not identified" );
	return( FALSE );
}

/*
 *	EOF
 */
