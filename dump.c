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
 *	dump
 *	====
 *
 *	Routines associated with the extended verification mode to
 *	facilitate dumping of internal opcode tables.
 */
 
#include "os.h"
#include "includes.h"

#ifdef VERIFICATION

/*
 *	These routines are provided to display the content of the opcode table
 *	in a manner that permits verification that the tabel correctly identifies
 *	and encodes instructions.  This is not a required component of the
 *	assembler in normal use.
 */

static const char *display_sign( word v ) {
	switch( v ) {
		case SIGN_IGNORED:	return( "ignore" );
		case SIGN_UNSIGNED:	return( "unsigned" );
		case SIGN_SIGNED:	return( "signed" );
		default:		break;
	}
	return( "Unknown" );
}

static const char *display_sizing( word v ) {
	switch( v ) {
		case DATA_SIZE_BYTE:	return( "byte" );
		case DATA_SIZE_WORD:	return( "word" );
		case DATA_SIZE_NEAR:	return( "near" );
		case DATA_SIZE_FAR:	return( "far" );
		default:		break;
	}
	return( "Unknown" );
}

static const char *display_range( word v ) {
	switch( v ) {
		case RANGE_BYTE:	return( "byte" );
		case RANGE_WORD:	return( "word" );
		case RANGE_BOTH:	return( "byte/word" );
		default:		break;
	}
	return( "Unknown" );
}

static const char *display_direct( word v ) {
	switch( v ) {
		case DIRECT_TO_EA:	return( "ea=reg" );
		case DIRECT_TO_REG:	return( "reg=ea" );
		default:		break;
	}
	return( "Unknown" );
}

static const char *display_subop( word v ) {
	static char subop[ 5 ];
	if( v > 7 ) return( "Unknown" );
	subop[ 0 ] = '%';
	subop[ 1 ] = BOOL( v & 4 )? '1': '0';
	subop[ 2 ] = BOOL( v & 2 )? '1': '0';
	subop[ 3 ] = BOOL( v & 1 )? '1': '0';
	subop[ 4 ] = EOS;
	return( subop );
}

static void display_definition( opcode *op ) {
	component	mods[ MAXIMUM_MODIFIERS ], *m;
	int		i;
	word		w;

	/*
	 *	Simply dump the content of the encoding table.
	 */
	expand_modifier( op->mods, mods, MAXIMUM_MODIFIERS );
	for( m = mods; *m != nothing; printf( "%s ", component_text( *m++ )));
	printf( "%s, ", component_text( op->op ));
	for( i = 0; i < op->args; i++ ) {
		printf( "," );
		show_ea_bitmap( op->arg[ i ]);
	}
	for( i = 0; i < op->encoded; i++ ) {
		w = op->encode[ i ];
		switch( GET_ACT( w )) {
			case SB_ACT: {
				/*
				 *	Set Byte (Action 0)
				 *
				 *	Provide static data forming the basic for the machine
				 *	code instruction.
				 *
				 *	SB(v)		v = value to add to instruction
				 */
				printf( ", SB(val=$%02X)", SB_VALUE( w ));
				break;
			}
			case IDS_ACT: {
				/*
				 *	Identify Data Size (Action 1)
				 *
				 *	Determine, from the indicated argument, the size of the
				 *	data to be handled (byte or word).
				 *
				 *	IDS(a,g)	a = Argument Number
				 *			g = Sign,	0:Ignore
				 *					1:Unsigned
				 *					2:Signed
				 */
				printf( ", IDS(arg=%d,sign=%s)", IDS_ARG( w ), display_sign( IDS_SIGN( w )));
				break;
			}
			case FDS_ACT: {
				/*
				 *	Fix Data Size (Action 2)
				 *
				 *	Set the internal structure for working on Bytes, Words or
				 *	Double Words (only used in FAR calculation).
				 *
				 *	FDS(s,g)	s = Size,	0:Byte
				 *					1:Word
				 *					2:Near
				 *					3:Far
				 *			g = Sign,	0:Ignore
				 *					1:Unsigned
				 *					2:Signed
				 */
				printf( ", FDS(size=%s,sign=%s)", display_sizing( FDS_SIZE( w )), display_sign( FDS_SIGN( w )));
				break;
			}
			case IMM_ACT: {
				/*
				 *	Immediate Data (Action 3)
				 *
				 *	Encode immediate data into the instruction from the
				 *	indicated argument.  Data size to be taken from the
				 *	data gathered by IDS or FDS defined above.
				 *
				 *	IMM(a)		a = Argument Number
				 */
				printf( ", IMM(arg=%d)", IMM_ARG( w ));
				break;
			}
			case EA_ACT: {
				/*
				 *	Effective Address (Action 4)
				 *
				 *	Generate the effective address byte (mod reg rm) in the
				 *	instruction.
				 *
				 *	EA(r,a)		r = argument that is the register component
				 *			a = argument number where the EA can be found
				 */
				printf( ", EA(reg_arg=%d,ea_arg=%d)", EA_REG( w ), EA_EADRS( w ));
				break;
			}
			case EAO_ACT: {
				/*
				 *	Effective Address Operator (Action 5)
				 *
				 *	Generate the effective address byte (mod opcode rm) in the
				 *	instruction.
				 *
				 *	EAO(o,a)	o = 3 bit opcode to insert into the byte
				 *			a = argument number where the EA can be found
				 */
				printf( ", EAO(opcode=%s,ea_arg=%d)", display_subop( EAO_OPCODE( w )), EAO_EADRS( w ));
				break;
			}
			case SDS_ACT: {
				/*
				 *	Save Data Size (Action 6)
				 *
				 *	Set data size location providing a byte index and 'bit in byte'
				 *	position where a 0 (for bytes) or 1 (for words) will be placed.
				 *
				 *	SDS(i,b)	i = index into machine instruction (0..7)
				 *			b = bit number in byte (0..7)
				 */
				printf( ", SDS(byte=%d,bit=%d)", SDS_INDEX( w ), SDS_BIT( w ));
				break;
			}
			case SDR_ACT: {
				/*
				 *	Set DiRection (Action 7)
				 *
				 *	Set data direction providing a byte index and 'bit in byte'
				 *	position where a 0 (EA is Destination) or 1 (EA is Source).
				 *
				 *	SDR(d,i,b)	d = direction,	0 = Reg->Source, EA->Destination
				 *					1 = EA->Source, Reg->Destination
				 *			i = index into machine instruction (0..7)
				 *			b = bit number in byte (0..7)
				 */
				printf( ", SDR(dir:%s,byte=%d,bit=%d)", display_direct( SDR_DIR( w )), SDR_INDEX( w ), SDR_BIT( w ));
				break;
			}
			case REG_ACT: {
				/*
				 *	Register (Action 8)
				 *
				 *	Take the register number from the argument a (three bit
				 *	value 0..7) and place this value into instruction byte i
				 *	at bit offset b.
				 *
				 *	REG(a,i,b)	a = Argument number (0..7)
				 *			i = index into machine instruction (0..7)
				 *			b = bit number in byte (0..7)
				 */
				printf( ", REG(arg=%d,byte=%d,bit=%d)", REG_ARG( w ), REG_INDEX( w ), REG_BIT( w ));
				break;
			}
			case ESC_ACT: {
				/*
				 *	ESCape Data (Action 9)
				 *
				 *	Provides a mechanism to capture the co-processor opcode and
				 *	position it within the output code generated.
				 *
				 *	ESC(a)		a = Argument number of immediate data
				 */
				printf( ", ESC(arg=%d)", ESC_ARG( w ));
				break;
			}
			case REL_ACT: {
				/*
				 *	RELative reference (Action 10)
				 *
				 *	Handles the conversion of an immediate label reference into
				 *	a relative IP offset value.
				 *
				 *	REL(a,s,i,b)	a = Argument number of immediate data
				 *			s = Size/Range of relative reference allowed
				 *				0 - invalid
				 *				1 - Byte only
				 *				2 - Word only
				 *				3 - Byte or word (if range is invalid for byte)
				 *			i = index of code to adjust for word disp
				 *			b = bit in index byte to flip.
				 */
				printf( ", REL(arg=%d,range=%s,byte=%d,bit=%d)", REL_ARG( w ), display_range( REL_RANGE( w )), REL_INDEX( w ), REL_BIT( w ));
				break;
			}
			case TER_ACT: {
				/*
				 *	TEst Register (Action 11)
				 *
				 *	Check that the argument specified is, (or is not) a specified
				 *	register.
				 *
				 *	TER(a,p,r)	a = Argument number of immediate data
				 *			p = condition result required to pass
				 *			r = register specification
				 */
				printf( ", TER(arg=%d,pass=%d,reg=%d)", TER_ARG( w ), TER_PASS( w ), TER_REG( w ));
				break;
			}
			case VDS_ACT: {
				/*
				 *	Verify Data Size (Action 11)
				 *
				 *	Check that the argument specified has the compatible size
				 *	configuration as the size data recorded in the constructed
				 *	instruction data.
				 *
				 *	VDS(a)		a = Argument number of immediate data
				 */
				printf( ", VDS(arg=%d)", VDS_ARG( w ));
				break;
			}
			default: {
				printf( ", Unknown($%04X)", w );
				break;
			}
		}
	}
	printf( "\n" );
}

static void display_instruction( boolean show_more, char *flags, component *mods, opcode *op, ea_breakdown *arg, instruction *mc ) {
	int	i, j;
	
	/*
	 *	Dump the hexidecimal code first
	 */
	j = HEX_DUMP_COLS;
	for( i = 0; i < mc->coded; i++ ) {
		printf( "%02X ", mc->code[ i ]);
		j -= 3;
	}
	while( j-- > 0 ) printf( " " );
	printf( ";" );
	if( show_more ) printf( "[%s]\t", flags );
	while( *mods != nothing ) printf( "%s ", component_text( *mods++ ));
	printf( "%s ", component_text( op->op ));
	for( i = 0; i < op->args; i++ ) {
		component	arg_mods[ MAXIMUM_MODIFIERS ];
		boolean		has_imm,
				has_ind,
				has_far,
				add_add;
		
		if( i ) printf( ", " );
		expand_modifier( arg[ i ].mod, arg_mods, MAXIMUM_MODIFIERS );
		for( j = 0; arg_mods[ j ] != nothing; printf( "%s ", component_text( arg_mods[ j++ ])));
		
		has_imm = BOOL( arg[ i ].ea & (	ea_immediate |		ea_far_immediate |
						ea_indirect |		ea_far_indirect |
						ea_base_disp |		ea_far_base_disp |
						ea_index_disp |		ea_far_index_disp |
						ea_base_index_disp |	ea_far_base_index_disp ));

		has_ind = BOOL( arg[ i ].ea & (	ea_indirect |		ea_far_indirect |
						ea_pointer_reg | 	ea_far_pointer_reg |
						ea_base_disp |		ea_far_base_disp |
						ea_index_disp |		ea_far_index_disp |
						ea_base_index_disp |	ea_far_base_index_disp ));

		has_far = BOOL( arg[ i ].ea & (	ea_far_immediate |
						ea_far_indirect |
						ea_far_pointer_reg |
						ea_far_base_disp |
						ea_far_index_disp |
						ea_far_base_index_disp ));

		add_add = BOOL( arg[ i ].ea & (	ea_base_disp |		ea_far_base_disp |
						ea_index_disp |		ea_far_index_disp |
						ea_base_index_disp |	ea_far_base_index_disp ));

		if( has_far ) printf( "far " );
		if( has_ind ) printf( "[" );
		for( j = 0; j < arg[ i ].registers; j++ ) {
			if( j ) printf( "+" );
			printf( "%s", component_text( arg[ i ].reg[ j ]->comp ));
		}
		if( has_imm ) {
			char	s[ BUFFER_FOR_SCOPE ];
			
			s[ convert_scope_to_text( TRUE, arg[ i ].immediate_arg.scope, s, BUFFER_FOR_SCOPE-1 )] = EOS;
			if( add_add ) {
				printf( "+{%s}", s );
			}
			else {
				printf( "{%s}", s );
			}
		}
		if( has_ind ) printf( "]" );
	}
	printf( "\n" );
}

/*
 *	Arbitary conversion from index to components (registers)
 */
static component byte_register[ BYTE_REGISTERS ] = {
	reg_al, reg_ah, reg_bl, reg_bh, reg_cl, reg_ch, reg_dl, reg_dh
};
static component word_register[ WORD_REGISTERS ] = {
	reg_ax, reg_cx, reg_dx, reg_bx, reg_sp, reg_bp, reg_si, reg_di
};
static component pointer_register[ POINTER_REGISTERS ] = {
	reg_bx, reg_si, reg_di
};
static component base_register[ BASE_REGISTERS ] = {
	reg_bx, reg_bp
};
static component index_register[ INDEX_REGISTERS ] = {
	reg_si, reg_di
};
static component segment_register[ SEGMENT_REGISTERS ] = {
	reg_cs, reg_ds, reg_ss, reg_es
};


typedef struct {
	effective_address	map,
				pick;
	byte			step;
} ea_state;

static boolean init_ea_state( ea_state *state, effective_address source ) {

	ASSERT( state != NIL( ea_state ));

	state->map = source;
	state->pick = (effective_address)1;	/* don't care what it is really, see below. */
	state->step = 0;
	return( state->map != ea_empty );
}

static boolean next_ea_state( ea_state *state, ea_breakdown *target ) {

	ASSERT( state != NIL( ea_state ));
	ASSERT( target != NIL( ea_breakdown ));

	while( state->map ) {
		if( BOOL( state->map & state->pick )) {
			switch( state->pick ) {
				case ea_byte_acc: {
					if( state->step == 0 ) {
						target->ea = state->pick;
						target->mod = no_modifier;
						target->registers = 1;
						target->reg[ 0 ] = register_component( byte_register[ state->step ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = 0;
						target->immediate_arg.scope = scope_none;
						target->immediate_arg.segment = NIL( segment_record );
						state->step++;
						return( TRUE );
					}
					break;
				}
				case ea_byte_reg: {
					state->step++;	/* pre-inc to force step over accumulator */
					if( state->step < BYTE_REGISTERS ) {
						target->ea = state->pick;
						target->mod = no_modifier;
						target->registers = 1;
						target->reg[ 0 ] = register_component( byte_register[ state->step ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = 0;
						target->immediate_arg.scope = scope_none;
						target->immediate_arg.segment = NIL( segment_record );
						return( TRUE );
					}
					break;
				}
				case ea_word_acc: {
					if( state->step == 0 ) {
						target->ea = state->pick;
						target->mod = no_modifier;
						target->registers = 1;
						target->reg[ 0 ] = register_component( word_register[ state->step ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = 0;
						target->immediate_arg.scope = scope_none;
						target->immediate_arg.segment = NIL( segment_record );
						state->step++;
						return( TRUE );
					}
					break;
				}
				case ea_word_reg: {
					state->step++;	/* pre-inc to force step over accumulator */
					if( state->step < WORD_REGISTERS ) {
						target->ea = state->pick;
						target->mod = no_modifier;
						target->registers = 1;
						target->reg[ 0 ] = register_component( word_register[ state->step ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = 0;
						target->immediate_arg.scope = scope_none;
						target->immediate_arg.segment = NIL( segment_record );
						return( TRUE );
					}
					break;
				}
				case ea_immediate:
				case ea_far_immediate: {
					target->ea = state->pick;
					target->mod = no_modifier;
					target->registers = 0;
					target->segment_override = UNKNOWN_SEG;
					target->immediate_arg.segment = NIL( segment_record );
					switch( state->step ) {
						case 0: {
							target->immediate_arg.value = 0x5;
							target->immediate_arg.scope = scope_byte_only;
							state->step++;
							return( TRUE );
						}
						case 1: {
							target->immediate_arg.value = 0x555;
							target->immediate_arg.scope = scope_word_only;
							state->step++;
							return( TRUE );
						}
						case 2: {
							target->immediate_arg.value = 0x5555;
							target->immediate_arg.scope = scope_address;
							state->step++;
							return( TRUE );
						}
						default: {
							break;
						}
					}
					break;
				}
				case ea_indirect:
				case ea_far_indirect: {
					target->ea = state->pick;
					target->registers = 0;
					target->segment_override = UNKNOWN_SEG;
					target->immediate_arg.value = 0xAAAA;
					target->immediate_arg.scope = scope_address;
					target->immediate_arg.segment = NIL( segment_record );
					switch( state->step ) {
						case 0: {
							target->mod = no_modifier;
							state->step++;
							return( TRUE );
						}
						case 1: {
							target->mod = byte_modifier | ptr_modifier;
							state->step++;
							return( TRUE );
						}
						case 2: {
							target->mod = word_modifier | ptr_modifier;
							state->step++;
							return( TRUE );
						}
					}
					break;
				}
				case ea_pointer_reg:
				case ea_far_pointer_reg: {
					if( state->step < POINTER_REGISTERS ) {
						target->ea = state->pick;
						target->mod = no_modifier;
						target->registers = 1;
						target->reg[ 0 ] = register_component( pointer_register[ state->step ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = 0;
						target->immediate_arg.scope = scope_none;
						target->immediate_arg.segment = NIL( segment_record );
						state->step++;
						return( TRUE );
					}
					break;
				}
				case ea_base_disp:
				case ea_far_base_disp: {
					boolean	iw;
					int	md,
						br;

					br = state->step;
					iw = BOOL( br & 1 );		br >>= 1;
					md = br % 3;			br /= 3;
					
					if( br < BASE_REGISTERS ) {
						target->ea = state->pick;
						switch( md ) {
							case 1: {
								target->mod = byte_modifier | ptr_modifier;
								break;
							}
							case 2: {
								target->mod = word_modifier | ptr_modifier;
								break;
							}
							default: {
								target->mod = no_modifier;
								break;
							}
						}
						target->registers = 1;
						target->reg[ 0 ] = register_component( base_register[ br ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = iw? 0xDDDD: 0xDD;
						target->immediate_arg.scope = iw? scope_word_only: scope_byte_only;
						target->immediate_arg.segment = NIL( segment_record );
						state->step++;
						return( TRUE );
					}
					break;
				}
				case ea_index_disp:
				case ea_far_index_disp: {
					boolean	iw;
					int	md,
						ir;

					ir = state->step;
					iw = BOOL( state->step & 1 );		ir >>= 1;
					md = ir % 3;				ir /= 3;

					if( ir < INDEX_REGISTERS ) {
						target->ea = state->pick;
						switch( md ) {
							case 1: {
								target->mod = byte_modifier | ptr_modifier;
								break;
							}
							case 2: {
								target->mod = word_modifier | ptr_modifier;
								break;
							}
							default: {
								target->mod = no_modifier;
								break;
							}
						}
						target->registers = 1;
						target->reg[ 0 ] = register_component( index_register[ ir ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = iw? 0xDDDD: 0xDD;
						target->immediate_arg.scope = iw? scope_word_only: scope_byte_only;
						target->immediate_arg.segment = NIL( segment_record );
						state->step++;
						return( TRUE );
					}
					break;
				}
				case ea_base_index_disp:
				case ea_far_base_index_disp: {
					boolean	iw;
					int	md,
						br,
						ir;

					/*
					 *	Break out the three value
					 *	from the step value.  We
					 *	use 'ir' as the temp variable
					 *	so it can be used as the
					 *	control to end this code.
					 */
					ir = state->step;
					iw = BOOL( ir & 1 );		ir >>= 1;
					md = ir % 3;			ir /= 3;
					br = ir % BASE_REGISTERS;	ir /= BASE_REGISTERS;

					if( ir < INDEX_REGISTERS ) {
						target->ea = state->pick;
						switch( md ) {
							case 1: {
								target->mod = byte_modifier | ptr_modifier;
								break;
							}
							case 2: {
								target->mod = word_modifier | ptr_modifier;
								break;
							}
							default: {
								target->mod = no_modifier;
								break;
							}
						}
						target->registers = 2;
						target->reg[ 0 ] = register_component( base_register[ br ]);
						target->reg[ 1 ] = register_component( index_register[ ir ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = iw? 0xDDDD: 0xDD;
						target->immediate_arg.scope = iw? scope_word_only: scope_byte_only;
						target->immediate_arg.segment = NIL( segment_record );
						state->step++;
						return( TRUE );
					}
					break;
				}
				case ea_segment_reg: {
					if( state->step < SEGMENT_REGISTERS ) {
						target->ea = state->pick;
						target->mod = no_modifier;
						target->registers = 1;
						target->reg[ 0 ] = register_component( segment_register[ state->step ]);
						target->segment_override = UNKNOWN_SEG;
						target->immediate_arg.value = 0;
						target->immediate_arg.scope = scope_none;
						target->immediate_arg.segment = NIL( segment_record );
						state->step++;
						return( TRUE );
					}
					break;
				}
				default: {
					log_error( "Unrecognised EA" );
					break;
				}
			}
		}
		state->map &= ~state->pick;
		state->pick <<= 1;		/* See? we're just pushing the bit along */
		state->step = 0;
	}
	return( FALSE );
}



/*
 *	Full dump of everything
 */
void dump_opcode_list( boolean show_more ) {
	opcode		*op;
	component	mods[ MAXIMUM_MODIFIERS ];
	char		opflags[ MAXIMUM_FLAGS ];
	int		i;
	instruction	mc;
	ea_breakdown	arg[ MAX_OPCODE_ARGS ];

	printf( "Opcode List:-\n" );
	for( op = opcodes; op->op != nothing; op++ ) {
		for( i = 0; i < HEX_DUMP_COLS; i++ ) printf( " " );
		printf( "; " );
		display_definition( op );
		/*
		 *	Gather common elements of all instructions.
		 */
		expand_mnemonic_flags( op->flags, opflags, MAXIMUM_FLAGS );
		expand_modifier( op->mods, mods, MAXIMUM_MODIFIERS );
		/*
		 *	Encode the instruction described
		 */
		switch( op->args ) {
			case 0: {
				/*
				 *	No arguments.
				 */
				if( assemble_inst( op, no_prefix, arg, &mc )) display_instruction( show_more, opflags, mods, op, arg, &mc );
				break;
			}
			case 1: {
				ea_state	foreach;
				
				if( init_ea_state( &foreach, op->arg[ 0 ])) {
					while( next_ea_state( &foreach, &( arg[ 0 ]))) {
						if( assemble_inst( op, no_prefix, arg, &mc )) display_instruction( show_more, opflags, mods, op, arg, &mc );
					}
				}
				break;
			}
			case 2: {
				ea_state	foreach_1,
						foreach_2;
				
				if( init_ea_state( &foreach_1, op->arg[ 0 ])) {
					while( next_ea_state( &foreach_1, &( arg[ 0 ]))) {
						if( init_ea_state( &foreach_2, op->arg[ 1 ])) {
							while( next_ea_state( &foreach_2, &( arg[ 1 ]))) {
								if( assemble_inst( op, no_prefix, arg, &mc )) display_instruction( show_more, opflags, mods, op, arg, &mc );
							}
						}
					}
				}
				break;
			}
			default: {
				log_error( "Argument count error" );
				break;
			}
		}
	}
}

/*
 *	Basic dump of each entry in the opcode table.
 */
void dump_opcode_table( void ) {
	/*
	 *	Simply dump the content of the encoding table.
	 */
	printf( "Opcode Table:-\n" );
	for( opcode *op = opcodes; op->op != nothing; display_definition( op++ ));
}

#endif

/*
 *	EOF
 */
