/*
 *	opcodes
 *	=======
 * 
 *	Structures, definitions and tables that capture the whole
 *	mess that is the 16-bit x86 instruction set.
 */
 
#include "os.h"
#include "includes.h"

/************************
 *			*
 *	MODIFIERS	*
 *			*
 ************************/

/*
 *	Define structure and routines that convert a component into
 *	its corresponding modifier value and a matching routine which
 *	performs the reverse operation.
 */
typedef struct {
	component		comp;
	modifier		mod;
} modifier_entry;

static modifier_entry modifier_lookup[] = {
	{ mod_byte,		byte_modifier		},
	{ mod_word,		word_modifier		},
	{ mod_ptr,		ptr_modifier		},
	{ mod_near,		near_modifier		},
	{ mod_far,		far_modifier		},
	{ nothing,		no_modifier		}
};

modifier map_modifier( component m ) {
	modifier_entry	*look;
	
	for( look = modifier_lookup; look->comp != nothing; look++ ) {
		if( look->comp == m ) return( look->mod );
	}
	return( no_modifier );
}

void expand_modifier( modifier input, component *output, int max ) {
	modifier_entry	*look;
	int		i;
	
	ASSERT( max > 0 );

	i = 0;
	max--;
	for( look = modifier_lookup; ( i < max )&&( look->mod != no_modifier ); look++ ) {
		if( BOOL( input & look->mod )) output[ i++ ] = look->comp;
	}
	output[ i ] = nothing;
}

/************************
 *			*
 *	PREFIXES	*
 *			*
 ************************/



/*
 *	Define a table which captures the basic relationships between
 *	each of the prefixes and the byte code it should generate.
 */
typedef struct {
	opcode_prefix		pref;
	char			*name;
	byte			code;
	mnemonic_flags		cpu;
	opcode_prefix		exclude;
} opcode_prefix_data;

static opcode_prefix_data prefix_data[] = {
	{ lock_prefix,		"lock",		0xF0,	cpu_8086,	no_prefix				},
	{ rep_prefix,		"rep",		0xF3,	cpu_8086,	rep_eq_prefix|rep_ne_prefix		},
	{ rep_eq_prefix,	"repeq",	0xF3,	cpu_8086,	rep_prefix|rep_ne_prefix		},
	{ rep_ne_prefix,	"repne",	0xF2,	cpu_8086,	rep_prefix|rep_eq_prefix		},
	{ CS_prefix,		"CS",		0x2E,	cpu_8086,	DS_prefix|SS_prefix|ES_prefix		},
	{ DS_prefix,		"DS",		0x3E,	cpu_8086,	CS_prefix|SS_prefix|ES_prefix		},
	{ SS_prefix,		"SS",		0x36,	cpu_8086,	CS_prefix|DS_prefix|ES_prefix		},
	{ ES_prefix,		"ES",		0x26,	cpu_8086,	CS_prefix|DS_prefix|SS_prefix		},
	{ branch_ignored_prefix,"bra_no",	0x2E,	cpu_8086,	branch_taken_prefix			},
	{ branch_taken_prefix,	"bra_yes",	0x3E,	cpu_8086,	branch_ignored_prefix			},
	{ operand_size_prefix,	"data_sz",	0x66,	cpu_8086,	no_prefix				},
	{ address_size_prefix,	"adrs_sz",	0x67,	cpu_8086,	no_prefix				},
	{ no_prefix }
};

/*
 *	Convert a prefix bitmap into a byte stream.  Return number of bytes
 *	placed into the buffer or ERROR if there has been an error.
 */
int encode_prefix_bytes( opcode_prefix prefs, byte *buffer, int max ) {
	opcode_prefix_data	*look;
	int			used;

	used = 0;
	for( look = prefix_data; look->pref != no_prefix; look++ ) {
		if( BOOL( prefs & look->pref )) {
			if( BOOL( prefs & look->exclude )) {
				log_error_s( "Invalid prefix combination", look->name );
				return( ERROR );
			}
			if( !BOOL( assembler_parameters & look->cpu )) {
				log_error_s( "Prefix invalid on target CPU", look->name );
				return( ERROR );
			}
			if( used == max ) {
				log_error( "Too many prefix bytes" );
				return( ERROR );
			}
			buffer[ used++ ] = look->code;
		}
	}
	return( used );
}

/*
 *	Convert component prefix identifiers to an internal
 *	bit mapped value.
 */
opcode_prefix map_prefix( component pref ) {
	/*
	 *	This code needs to align with is_prefix().
	 */
	switch( pref ) {
		case pref_lock:		return( lock_prefix );
		case pref_rep:		return( rep_prefix );
		case pref_repe:
		case pref_repz:		return( rep_eq_prefix );
		case pref_repne:
		case pref_repnz:	return( rep_ne_prefix );
		default: {
			ABORT( "Programmer Error" );
			break;
		}
	}
	return( no_prefix );
}

/*
 *	Convert a segment register number into a prefix bitmap bit
 */
static opcode_prefix segment_prefix_map[ SEGMENT_REGISTERS ] = { CS_prefix, DS_prefix, SS_prefix, ES_prefix };

opcode_prefix map_segment_prefix( byte segment_reg ) {

	if( segment_reg < SEGMENT_REGISTERS ) return( segment_prefix_map[ segment_reg ]);
	return( no_prefix );
}

/********************************
 *				*
 *	EFFECTIVE ADDRESS	*
 *				*
 ********************************/


#if defined( VERIFICATION )||defined( DEBUG )

	/*
	 *	Debugging code to output an AC or EA bit map in text to stdout.
	 */
	typedef struct {
		arg_component	ac;
		char		*name;
	} ac_translation;
	static ac_translation trans_ac[] = {
		{ ac_brackets,		"indirect"	},
		{ ac_byte_reg,		"byte"		},
		{ ac_word_reg,		"word"		},
		{ ac_acc_reg,		"acc"		},
		{ ac_pointer_reg,	"pointer"	},
		{ ac_base_reg,		"base"		},
		{ ac_index_reg,		"index"		},
		{ ac_segment_reg,	"segment"	},
		{ ac_immediate,		"immediate"	},
		{ ac_seg_override,	"seg_override"	},
		{ ac_empty }
	};
	
	void show_ac_bitmap( arg_component ac ) {
		ac_translation *look;
		char		lead;

		lead = SPACE;
		for( look = trans_ac; look->ac; look++ ) {
			if( BOOL( ac & look->ac )) {
				printf( "%c%s", lead, look->name );
				lead = '|';
			}
		}
		if( lead == SPACE ) printf( " empty" );
	}

	typedef struct {
		effective_address	ea;
		char			*name;
	} ea_translation;

	static ea_translation trans_ea[] = {
		{ ea_byte_acc,			"byte_acc"		},
		{ ea_word_acc,			"word_acc"		},
		{ ea_byte_reg,			"byte_reg"		},
		{ ea_word_reg,			"word_reg"		},
		{ ea_immediate,			"immediate"		},
		{ ea_indirect,			"indirect"		},
		{ ea_pointer_reg,		"pointer_reg"		},
		{ ea_base_disp,			"base_disp"		},
		{ ea_index_disp,		"index_disp"		},
		{ ea_base_index_disp,		"base_index_disp"	},
		{ ea_segment_reg,		"segment_reg"		},
		{ ea_far_immediate,		"far_immediate"		},
		{ ea_far_indirect,		"far_indirect"		},
		{ ea_far_pointer_reg,		"far_pointer_reg"	},
		{ ea_far_base_disp,		"far_base_disp"		},
		{ ea_far_index_disp,		"far_index_disp"	},
		{ ea_far_base_index_disp,	"far_base_index_disp"	},
		{ ea_empty }
	};
	
	void show_ea_bitmap( effective_address ea ) {
		ea_translation *look;
		char		lead;

		lead = SPACE;
		for( look = trans_ea; look->ea; look++ ) {
			if( BOOL( ea & look->ea )) {
				printf( "%c%s", lead, look->name );
				lead = '|';
			}
		}
		if( lead == SPACE ) printf( " empty" );
	}

#endif

static register_data component_eas[] = {
/*	  Register	E Adrs						Reg		Ptr	B/I	Implied Seg	*/
/*	  --------	------	----					---		---	---	-----------	*/
	{ reg_al,	ac_byte_reg|ac_acc_reg,				REG_AL,		0,	0,	UNREQUIRED_SEG	},
	{ reg_cl,	ac_byte_reg,					REG_CL,		0,	0,	UNREQUIRED_SEG	},
	{ reg_dl,	ac_byte_reg,					REG_DL,		0,	0,	UNREQUIRED_SEG	},
	{ reg_bl,	ac_byte_reg,					REG_BL,		0,	0,	UNREQUIRED_SEG	},
	{ reg_ah,	ac_byte_reg,					REG_AH,		0,	0,	UNREQUIRED_SEG	},
	{ reg_ch,	ac_byte_reg,					REG_CH,		0,	0,	UNREQUIRED_SEG	},
	{ reg_dh,	ac_byte_reg,					REG_DH,		0,	0,	UNREQUIRED_SEG	},
	{ reg_bh,	ac_byte_reg,					REG_BH,		0,	0,	UNREQUIRED_SEG	},
	{ reg_ax,	ac_word_reg|ac_acc_reg,				REG_AX,		0,	0,	UNREQUIRED_SEG	},
	{ reg_cx,	ac_word_reg,					REG_CX,		0,	0,	UNREQUIRED_SEG	},
	{ reg_dx,	ac_word_reg,					REG_DX,		0,	0,	UNREQUIRED_SEG	},
	{ reg_bx,	ac_word_reg|ac_pointer_reg|ac_base_reg,		REG_BX,		B111,	B000,	REG_DS	},
	{ reg_sp,	ac_word_reg,					REG_SP,		0,	0,	UNREQUIRED_SEG	},
	{ reg_bp,	ac_word_reg|ac_pointer_reg|ac_base_reg,		REG_BP,		B110,	B010,	REG_SS	},
	{ reg_si,	ac_word_reg|ac_pointer_reg|ac_index_reg,	REG_SI,		B100,	B000,	REG_DS	},
	{ reg_di,	ac_word_reg|ac_pointer_reg|ac_index_reg,	REG_DI,		B101,	B001,	UNREQUIRED_SEG	},
	{ reg_cs,	ac_segment_reg,					REG_CS,		0,	0,	UNREQUIRED_SEG	},
	{ reg_ds,	ac_segment_reg,					REG_DS,		0,	0,	UNREQUIRED_SEG	},
	{ reg_ss,	ac_segment_reg,					REG_SS,		0,	0,	UNREQUIRED_SEG	},
	{ reg_es,	ac_segment_reg,					REG_ES,		0,	0,	UNREQUIRED_SEG	},
	{ nothing,	ac_empty,					0,		0,	0,	UNREQUIRED_SEG	}
};

/*
 *	A routine that returns the address of the register_data
 * 	record for a specific register.
 *
 *	Yes, this is really slow.
 */
register_data *register_component( component comp ) {
	register_data	*look;

	for( look = component_eas; look->comp != nothing; look++ )
		if( look->comp == comp ) return( look );
	return( NIL( register_data ));
}


/*
 *	Here be dragons (be warned)
 *
 *	If VERIFICATION is defined then this table is accessed
 *	by the "dump" module to facilitate confirmation of the
 *	tables content.
 *
 *	So, if not defined, then the data is static.
 */
#ifndef VERIFICATION
static
#endif

opcode opcodes[] = {
/* Programming_the_8086_8088
 * Page 68
 *
 *	ASCII Adjust for Addition
 *	0011 0111
 */
	{ op_aaa,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x37) }},

/* Programming_the_8086_8088
 * Page 69
 *
 *	ASCII Adjust for Division
 *	1101 0101, 0000 1010
 */
	{ op_aad,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			2,	{ SB(0xD5),SB(0x0A) }},

/* Programming_the_8086_8088
 * Page 70
 *
 *	ASCII Adjust for Multiply
 *	1101 0100, 0000 1010
 */
	{ op_aam,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			2,	{ SB(0xD4),SB(0x0A) }},

/* Programming_the_8086_8088
 * Page 71
 *
 *	ASCII Adjust for Subtraction
 *	0011 1111
 */
	{ op_aas,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x3F) }},

/* Programming_the_8086_8088
 * Page 72
 *
 *	Add with Carry memory or Register Operand with Register Operand
 * A	0001 00dw, mod reg r/m
 * 
 *	Add with Carry immediate Operand to Memory or Register Operand
 * B	1000 00sw, mod 010 r/m, data, data if sw = 01
 *
 *	Add with Carry immediate Operand to Accumulator
 * C	0001 010w, data, data if w =1
 */
/*C*/	{ op_adc,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_accumulators, ea_immediate },	4,	{ SB(0x14),IDS(0,SIGN_IGNORED),IMM(1),SDS(0,0) }},
/*B*/	{ op_adc,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	5,	{ SB(0x80),IDS(0,SIGN_IGNORED),EAO(B010,0),IMM(1),SDS(0,0) }},
/*A*/	{ op_adc,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_all_reg, ea_mod_reg_adrs },	6,	{ SB(0x10),SDR(DIRECT_TO_REG,0,1),IDS(0,SIGN_IGNORED),EA(0,1),SDS(0,0),VDS(1) }},
/*A*/	{ op_adc,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_all_reg },	6,	{ SB(0x10),SDR(DIRECT_TO_EA,0,1),IDS(1,SIGN_IGNORED),EA(1,0),SDS(0,0),VDS(0) }},

/* Programming_the_8086_8088
 * Page 73
 *
 *	Add memory or Register Operand with Register Operand
 * A	0000 00dw, mod reg r/m
 * 
 *	Add immediate Operand to Memory or Register Operand
 * B	1000 00sw, mod 000 r/m, data, data if sw = 01
 *
 *	Add immediate Operand to Accumulator
 * C	0000 010w, data, data if w =1
 */
/*C*/	{ op_add,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_accumulators, ea_immediate },	4,	{ SB(0x04),IDS(0,SIGN_IGNORED),IMM(1),SDS(0,0) }},
/*B*/	{ op_add,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	5,	{ SB(0x80),IDS(0,SIGN_IGNORED),EAO(B000,0),IMM(1),SDS(0,0) }},
/*A*/	{ op_add,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_all_reg, ea_mod_reg_adrs },	6,	{ SB(0x00),SDR(DIRECT_TO_REG,0,1),IDS(0,SIGN_IGNORED),EA(0,1),SDS(0,0),VDS(1) }},
/*A*/	{ op_add,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_all_reg },	6,	{ SB(0x00),SDR(DIRECT_TO_EA,0,1),IDS(1,SIGN_IGNORED),EA(1,0),SDS(0,0),VDS(0) }},

/* Programming_the_8086_8088
 * Pages 74,75
 *
 *	And memory or Register Operand with Register Operand
 * A	0001 00dw, mod reg r/m
 * 
 *	And immediate Operand to Memory or Register Operand
 * B	1000 00sw, mod 100 r/m, data, data if sw = 01
 *
 *	And immediate Operand to Accumulator
 * C	0010 010w, data, data if w =1
 */
/*C*/	{ op_and,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_accumulators, ea_immediate },	4,	{ SB(0x24),IDS(0,SIGN_IGNORED),IMM(1),SDS(0,0) }},
/*B*/	{ op_and,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	5,	{ SB(0x80),IDS(0,SIGN_IGNORED),EAO(B100,0),IMM(1),SDS(0,0) }},
/*A*/	{ op_and,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_all_reg, ea_mod_reg_adrs },	6,	{ SB(0x20),SDR(DIRECT_TO_REG,0,1),IDS(0,SIGN_IGNORED),EA(0,1),SDS(0,0),VDS(1) }},
/*A*/	{ op_and,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_all_reg },	6,	{ SB(0x20),SDR(DIRECT_TO_EA,0,1),IDS(1,SIGN_IGNORED),EA(1,0),SDS(0,0),VDS(0) }},

/* iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-53,54
 *
 *	Bound
 *	0110 0010, mod req r/m
 */
	{ op_bound,	flag_186,	lock_n_segments,	no_modifier,	2,	{ ea_all_reg, ea_mod_reg_adrs },	4,	{ SB(0x20),SDR(DIRECT_TO_REG,0,1),FDS(DATA_SIZE_WORD,SIGN_IGNORED),EA(0,1) }},

/* Programming_the_8086_8088
 * Pages 76,77
 *
 *	Call near (intra segment) direct
 * A	1110 1000, disp-low, disp-high
 * 
 *	Call near (intra segment) indirect
 * B	1111 1111, mod 010 r/m
 *
 *	Call far (inter segment) direct
 * C	1001 1010, offset-low, offset-high, seg-low, seg-high
 * 
 * 	Call far (inter segment) indirect
 * D	1111 1111, mod 011 r/m
 */
/*A*/	{ op_call,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_immediate, ea_empty },		4,	{ SB(0xE8),FDS(DATA_SIZE_NEAR,SIGN_UNSIGNED),VDS(0),REL(0,RANGE_WORD,0,0) }},
/*B*/	{ op_call,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_wreg_adrs, ea_empty },		4,	{ SB(0xFF),FDS(DATA_SIZE_NEAR,SIGN_UNSIGNED),VDS(0),EAO(B010,0) }},
/*C*/	{ op_call,	flag_086|flag_abs,lock_n_segments,	no_modifier,	1,	{ ea_far_immediate, ea_empty },		4,	{ SB(0x9A),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),VDS(0),IMM(0) }},
/*C*/	{ op_call,	flag_086|flag_abs,lock_n_segments,	far_modifier,	1,	{ ea_immediate, ea_empty },		4,	{ SB(0x9A),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),VDS(0),IMM(0) }},
/*D*/	{ op_call,	flag_086|flag_abs,lock_n_segments,	no_modifier,	1,	{ ea_far_mod_reg_adrs, ea_empty },	4,	{ SB(0xFF),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),VDS(0),EAO(B011,0) }},
/*D*/	{ op_call,	flag_086|flag_abs,lock_n_segments,	far_modifier,	1,	{ ea_mod_wreg_adrs, ea_empty },		4,	{ SB(0xFF),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),VDS(0),EAO(B011,0) }},
/*C*/	{ op_lcall,	flag_086|flag_abs,lock_n_segments,	no_modifier,	1,	{ ea_immediate, ea_empty },		4,	{ SB(0x9A),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),VDS(0),IMM(0) }},
/*D*/	{ op_lcall,	flag_086|flag_abs,lock_n_segments,	no_modifier,	1,	{ ea_mod_wreg_adrs, ea_empty },		4,	{ SB(0xFF),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),VDS(0),EAO(B011,0) }},

/* Programming_the_8086_8088
 * Page 78
 *
 *	Convert byte to word
 *	1001 1000
 */
	{ op_cbw,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x98) }},

/* Programming_the_8086_8088
 * Page 79
 *
 *	Clear carry flag
 *	1111 1000
 */
	{ op_clc,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xF8) }},

/* Programming_the_8086_8088
 * Page 80
 *
 *	Clear Direction flag
 *	1111 1100
 */
	{ op_cld,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xFC) }},

/* Programming_the_8086_8088
 * Page 81
 *
 *	Clear Interrupt enable flag
 *	1111 1010
 */
	{ op_cli,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xFA) }},

/* Programming_the_8086_8088
 * Page 82
 *
 *	Complement carry flag
 *	1111 0101
 */
	{ op_cmc,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xF5) }},

/* Programming_the_8086_8088
 * Pages 83
 *
 *	Compare memory or Register Operand with Register Operand
 * A	0011 10dw, mod reg r/m
 * 
 *	Compare immediate Operand with Memory or Register Operand
 * B	1000 00sw, mod 111 r/m, data, data if sw = 01
 *
 *	Compare immediate Operand with Accumulator
 * C	0011 110w, data, data if w =1
 */
/*C*/	{ op_cmp,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_accumulators, ea_immediate },	4,	{ SB(0x3C),IDS(0,SIGN_IGNORED),IMM(1),SDS(0,0) }},
/*B*/	{ op_cmp,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	5,	{ SB(0x80),IDS(0,SIGN_IGNORED),EAO(B111,0),IMM(1),SDS(0,0) }},
/*A*/	{ op_cmp,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_all_reg, ea_mod_reg_adrs },	6,	{ SB(0x38),SDR(DIRECT_TO_REG,0,1),IDS(0,SIGN_IGNORED),EA(0,1),SDS(0,0),VDS(1) }},
/*A*/	{ op_cmp,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_all_reg },	6,	{ SB(0x38),SDR(DIRECT_TO_EA,0,1),IDS(1,SIGN_IGNORED),EA(1,0),SDS(0,0),VDS(0) }},

/* Programming_the_8086_8088
 * Page 84
 *
 *	Compare string (Byte or word)
 *	1010 011w
 */
	{ op_cmps,	flag_086,	rep_test_prefix,	no_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0xA6),FDS(DATA_SIZE_BYTE,SIGN_IGNORED),SDS(0,0) }},
	{ op_cmps,	flag_086,	rep_test_prefix,	byte_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0xA6),FDS(DATA_SIZE_BYTE,SIGN_IGNORED),SDS(0,0) }},
	{ op_cmps,	flag_086,	rep_test_prefix,	word_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0xA6),FDS(DATA_SIZE_WORD,SIGN_IGNORED),SDS(0,0) }},

/* Programming_the_8086_8088
 * Page 85
 *
 *	Convert word to double word
 *	1001 1001
 */
	{ op_cwd,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x99) }},

/* Programming_the_8086_8088
 * Page 86
 *
 *	Decimal adjust for addition
 *	0010 0111
 */
	{ op_daa,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x27) }},

/* Programming_the_8086_8088
 * Page 87
 *
 *	Decimal adjust for subtraction
 *	0010 1111
 */
	{ op_das,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x2F) }},

/* Programming_the_8086_8088
 * Page 88
 *
 *	Decrement memory or register
 * A	1111 111w, mod 001 r/m
 * 
 *	Decrement Register (word)
 * B	0100 1reg
 */
/*B*/	{ op_dec,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_word_registers, ea_empty },	2,	{ SB(0x48),REG(0,0,0) }},
/*A*/	{ op_dec,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xFE),IDS(0,SIGN_IGNORED),EAO(B001,0),SDS(0,0) }},

/* Programming_the_8086_8088
 * Page 89,90
 *
 *	Divide 
 *	1111 011w, mod 110 r/m, 2 to 4 data bytes
 */
	{ op_div,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xF6),IDS(0,SIGN_IGNORED),EAO(B110,0),SDS(0,0) }},

/* iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-71,72
 *
 *	Enter
 *	1100 1000, data-low, data-high
 */
	{ op_enter,	flag_186,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xC8),FDS(DATA_SIZE_WORD,SIGN_UNSIGNED),IMM(0) }},

/* Programming_the_8086_8088
 * Page 91
 *
 *	Escape 
 *	1101 1xxx, mod xxx r/m
 */
	{ op_esc,	flag_086,	segment_prefixes,	no_modifier,	2,	{ ea_immediate, ea_mem_mod_adrs },	4,	{ SB(0xD8),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),EAO(B000,1),ESC(0) }},

/* Programming_the_8086_8088
 * Page 92
 *
 *	Halt 
 *	1111 0100
 */
	{ op_hlt,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xF4) }},

/* Programming_the_8086_8088
 * Page 93,94
 *
 *	Interger divide 
 *	1111 011w, mod 111 r/m
 */
	{ op_idiv,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xF6),IDS(0,SIGN_IGNORED),EAO(B111,0),SDS(0,0) }},

/* Programming_the_8086_8088
 * Page 95
 *
 *	Interger multiply 
 *	1111 011w, mod 101 r/m
 */
	{ op_imul,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xF6),IDS(0,SIGN_IGNORED),EAO(B101,0),SDS(0,0) }},

/* Programming_the_8086_8088
 * Page 96
 *
 *	Input byte or word (fixed port) 
 * A	1110 010w, port
 *
 *	Input byte or word (indirect port, DX)
 * B	1110 110w
 */
/*B*/	{ op_in,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_accumulators, ea_empty },		3,	{ SB(0xEC),IDS(0,SIGN_IGNORED),SDS(0,0) }},
/*A*/	{ op_in,	flag_086,	no_prefix,		no_modifier,	2,	{ ea_accumulators, ea_immediate },	5,	{ SB(0xE4),IDS(0,SIGN_IGNORED),SDS(0,0),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),IMM(1) }},

/* Programming_the_8086_8088
 * Page 97
 *
 *	Increment memory or register
 * A	1111 111w, mod 000 r/m
 * 
 *	Increment Register (word)
 * B	0100 0reg
 */
/*B*/	{ op_inc,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_word_registers, ea_empty },	2,	{ SB(0x40),REG(0,0,0) }},
/*A*/	{ op_inc,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xFE),IDS(0,SIGN_IGNORED),EAO(B000,0),SDS(0,0) }},

/* iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-80
 *
 *	Input string (bytes or words)
 *	0110 110w
 */
	{ op_ins,	flag_186,	rep_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0x6C),FDS(DATA_SIZE_BYTE,SIGN_IGNORED),SDS(0,0) }},
	{ op_ins,	flag_186,	rep_prefix,		byte_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0x6C),FDS(DATA_SIZE_BYTE,SIGN_IGNORED),SDS(0,0) }},
	{ op_ins,	flag_186,	rep_prefix,		word_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0x6C),FDS(DATA_SIZE_WORD,SIGN_IGNORED),SDS(0,0) }},

/* Programming_the_8086_8088
 * Page 98
 *
 *	Break point interrupt 
 * A	1100 1100
 * 
 *	General interrupt
 * B	1100 1101, number
 */
/*A*/	{ op_break,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xCC) }},
/*B*/	{ op_int,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xCD),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),IMM(0) }},

/* Programming_the_8086_8088
 * Page 99
 *
 *	Interrupt on overflow 
 *	1100 1110
 */
	{ op_into,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xCE) }},

/* Programming_the_8086_8088
 * Page 100
 *
 *	Return from interrupt 
 *	1100 1111
 */
	{ op_iret,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xCF) }},

/* Programming_the_8086_8088
 * Page 101
 *
 *	Jump on Above
 * 	Jump on Not Below or Equal 
 *	0111 0111, disp
 */
	{ op_ja,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x77),REL(0,RANGE_BYTE,0,0) }},
	{ op_jnbe,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x77),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 102
 *
 *	Jump on Above or Equal
 * 	Jump on Not Below 
 *	0111 0011, disp
 */
	{ op_jae,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x73),REL(0,RANGE_BYTE,0,0) }},
	{ op_jnb,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x73),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 103
 *
 *	Jump on Below
 * 	Jump on Not Above or Equal 
 *	0111 0010, disp
 */
	{ op_jb,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x72),REL(0,RANGE_BYTE,0,0) }},
	{ op_jnae,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x72),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 104
 *
 *	Jump on Below or Equal
 * 	Jump on Not Above 
 *	0111 0110, disp
 */
	{ op_jbe,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x76),REL(0,RANGE_BYTE,0,0) }},
	{ op_jna,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x76),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 105
 *
 *	Jump on Carry
 *	0111 0010, disp
 */
	{ op_jc,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x72),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 106
 *
 *	Jump on CX is Zero
 *	1110 0011, disp
 */
	{ op_jcxz,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xE3),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 107
 *
 *	Jump on Equal
 * 	Jump on Zero 
 *	0111 0100, disp
 */
	{ op_je,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x74),REL(0,RANGE_BYTE,0,0) }},
	{ op_jz,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x74),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 108
 *
 *	Jump on Greater than
 * 	Jump on Not Less than or Equal 
 *	0111 1111, disp
 */
	{ op_jg,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7F),REL(0,RANGE_BYTE,0,0) }},
	{ op_jnle,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7F),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 109
 *
 *	Jump on Greater than or Equal
 * 	Jump on Not Less than 
 *	0111 1101, disp
 */
	{ op_jge,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7D),REL(0,RANGE_BYTE,0,0) }},
	{ op_jnl,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7D),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 110
 *
 *	Jump on Less than
 * 	Jump on Not Greater than or Equal
 *	0111 1100, disp
 */
	{ op_jl,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7C),REL(0,RANGE_BYTE,0,0) }},
	{ op_jnge,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7C),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 111
 *
 *	Jump on Less than or Equal
 * 	Jump on Not Greater than
 *	0111 1110, disp
 */
	{ op_jle,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7E),REL(0,RANGE_BYTE,0,0) }},
	{ op_jng,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7E),REL(0,RANGE_BYTE,0,0) }},


/* Programming_the_8086_8088
 * Page 112,113
 *
 *	Jump near relative (signed 16 bit)
 * A	1110 1001, disp-low, disp-high
 * 
 *	Jump near relative (signed 8-bit sign extended to 16 bits)
 * B	1110 1011, disp
 * 
 *	Jump near indirect (absolute 16 bit offset)
 * C	1111 1111, mod 100 r/m
 * 
 *	Jump far absolute
 * D	1110 1010, offset-low, offset-high, seg-low, seg-high
 * 
 * 	Jump far indirect (absolute 16 offset and 16 bit segment page)
 * E	1111 1111, mod 101 r/m
 */
/*B*/	{ op_jmp,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xEB),REL(0,RANGE_BOTH,0,1) }},
/*B*/	{ op_jmp,	flag_086,	no_prefix,		near_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xEB),REL(0,RANGE_BOTH,0,1) }},
/*D*/	{ op_jmp,	flag_086|flag_abs,no_prefix,		no_modifier,	1,	{ ea_far_immediate, ea_empty },		3,	{ SB(0xEA),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),IMM(0) }},
/*D*/	{ op_jmp,	flag_086|flag_abs,no_prefix,		far_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xEA),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),IMM(0) }},
/*C*/	{ op_jmp,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_wreg_adrs, ea_empty },		3,	{ SB(0xFF),FDS(DATA_SIZE_NEAR,SIGN_UNSIGNED),EAO(B100,0) }},
/*E*/	{ op_jmp,	flag_086|flag_abs,lock_n_segments,	no_modifier,	1,	{ ea_far_mod_reg_adrs, ea_empty },	3,	{ SB(0xFF),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),EAO(B101,0) }},
/*E*/	{ op_jmp,	flag_086|flag_abs,lock_n_segments,	far_modifier,	1,	{ ea_mod_wreg_adrs, ea_empty },		3,	{ SB(0xFF),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),EAO(B101,0) }},
/*D*/	{ op_ljmp,	flag_086|flag_abs,no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xEA),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),IMM(0) }},
/*E*/	{ op_ljmp,	flag_086|flag_abs,lock_n_segments,	no_modifier,	1,	{ ea_mod_wreg_adrs, ea_empty },		3,	{ SB(0xFF),FDS(DATA_SIZE_FAR,SIGN_UNSIGNED),EAO(B101,0) }},


/* Programming_the_8086_8088
 * Page 114
 *
 *	Jump on Not Carry
 *	0111 0011, disp
 */
	{ op_jnc,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x73),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 115
 *
 *	Jump on Not Equal
 * 	Jump on Not Zero
 *	0111 0101, disp
 */
	{ op_jne,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x75),REL(0,RANGE_BYTE,0,0) }},
	{ op_jnz,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x75),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 116
 *
 *	Jump on Not Overflow
 *	0111 0001, disp
 */
	{ op_jno,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x71),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 117
 *
 *	Jump on Not Sign
 *	0111 1001, disp
 */
	{ op_jns,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x79),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 118
 *
 *	Jump on Not Parity
 *	Jump on Parity Odd
 *	0111 1011, disp
 */
	{ op_jnp,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7B),REL(0,RANGE_BYTE,0,0) }},
	{ op_jpo,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7B),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 119
 *
 *	Jump on Overflow
 *	0111 0000, disp
 */
	{ op_jo,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x70),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 120
 *
 *	Jump on Parity Even
 *	Jump on Parity Equal
 *	0111 1010, disp
 */
	{ op_jp,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7A),REL(0,RANGE_BYTE,0,0) }},
	{ op_jpe,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x7A),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 121
 *
 *	Jump on Sign
 *	0111 1000, disp
 */
	{ op_js,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0x78),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 122
 *
 *	Load register AH from flags
 *	1001 1111
 */
	{ op_lahf,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xAF) }},

/* Programming_the_8086_8088
 * Page 123
 *
 *	Load reg (16 bit) from memory+0,1
 * 	Load DS from memory+2,3
 *	1100 0101, mod reg r/m
 */
	{ op_lds,	flag_086|flag_abs,lock_n_segments,	no_modifier,	2,	{ ea_word_registers, ea_mem_mod_adrs },	2,	{ SB(0xC5),FDS(DATA_SIZE_WORD,SIGN_IGNORED),EA(0,1) }},

/* Programming_the_8086_8088
 * Page 124
 *
 *	Load reg (16-bit) with offset of memory argument
 *	1000 1101, mod reg r/m
 */
	{ op_lea,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_word_registers, ea_mem_mod_adrs },	2,	{ SB(0x8D),FDS(DATA_SIZE_WORD,SIGN_IGNORED),EA(0,1) }},

/* iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-109
 *
 *	Return from subroutine deallocating the stack frame
 *	1100 1001
 */
	{ op_leave,	flag_186,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xC9),FDS(DATA_SIZE_WORD,SIGN_UNSIGNED),IMM(0) }},

/* Programming_the_8086_8088
 * Page 125
 *
 *	Load reg (16 bit) from memory+0,1
 * 	Load ES from memory+2,3
 *	1100 0100, mod reg r/m
 */
	{ op_les,	flag_086|flag_abs,lock_n_segments,	no_modifier,	2,	{ ea_word_registers, ea_mem_mod_adrs },	2,	{ SB(0xC4),FDS(DATA_SIZE_WORD,SIGN_IGNORED),EA(0,1) }},

/* Programming_the_8086_8088
 * Page 127
 *
 *	Load String (Byte or word)
 *	1010 110w
 */
	{ op_lods,	flag_086,	repeat_n_segments,	no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xAC) }},
	{ op_lods,	flag_086,	repeat_n_segments,	byte_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xAC) }},
	{ op_lods,	flag_086,	repeat_n_segments,	word_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xAD) }},

/* Programming_the_8086_8088
 * Page 128
 *
 *	Decrement CX, Loop if CX != 0
 *	1110 0010, disp
 */
	{ op_loop,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xE2),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 129
 *
 *	Decrement CX, Loop if ( CX != 0 )&&( ZF == 1 )
 *	1110 0010, disp
 */
	{ op_looppe,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xE1),REL(0,RANGE_BYTE,0,0) }},
	{ op_looppz,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xE1),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Page 130
 *
 *	Decrement CX, Loop if ( CX != 0 )&&( ZF == 0 )
 *	1110 0010, disp
 */
	{ op_loopne,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xE0),REL(0,RANGE_BYTE,0,0) }},
	{ op_loopnz,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		2,	{ SB(0xE0),REL(0,RANGE_BYTE,0,0) }},

/* Programming_the_8086_8088
 * Pages 131,132
 * 
 * 	Move memory or Register Operand to/from Register Operand:
 * A	1000 10dw, mod reg r/m
 *
 *	Move immediate Operand to Memory or Register Operand:
 * B	1100 011w, mod 000 r/m, data, dataifw=l
 *
 *	Move immediate Operand to Register:
 * C	1011 wreg, data, data if w = 1
 *
 *	Move memory Operand to Accumulator:
 * D	1010 000w, addr-low, addr-high
 *
 *	Move accumulator to Memory Operand:
 * E	1010 001w, addr-low, addr-high
 *
 *	Move memory or Register Operand to Segment Register
 * F	1000 1110, mod 0+reg r/m
 * 
 *	Move segment Register to Memory or Register Operand
 * G	1000 1100, mod 0+reg r/m
 */
/*C*/	{ op_mov,	flag_086,	no_prefix,		no_modifier,	2,	{ ea_all_reg, ea_immediate },		5,	{ SB(0xB0),IDS(0,SIGN_IGNORED),REG(0,0,0),IMM(1),SDS(0,3) }},
/*B*/	{ op_mov,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	5,	{ SB(0xC6),IDS(0,SIGN_IGNORED),EAO(B000,0),IMM(1),SDS(0,0) }},
/*D*/	{ op_mov,	flag_086,	no_prefix,		no_modifier,	2,	{ ea_accumulators, ea_indirect },	4,	{ SB(0xA0),IDS(0,SIGN_IGNORED),IMM(1),SDS(0,0) }},
/*E*/	{ op_mov,	flag_086,	no_prefix,		no_modifier,	2,	{ ea_indirect, ea_accumulators },	4,	{ SB(0xA0),IDS(1,SIGN_IGNORED),IMM(0),SDS(0,0) }},
/*A*/	{ op_mov,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_all_reg, ea_mod_reg_adrs },	6,	{ SB(0x88),SDR(DIRECT_TO_REG,0,1),IDS(0,SIGN_IGNORED),EA(0,1),SDS(0,0),VDS(1) }},
/*A*/	{ op_mov,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_all_reg },	6,	{ SB(0x88),SDR(DIRECT_TO_EA,0,1),IDS(1,SIGN_IGNORED),EA(1,0),SDS(0,0),VDS(0) }},
/*F*/	{ op_mov,	flag_086|flag_abs,lock_n_segments,	no_modifier,	2,	{ ea_segment_reg, ea_mod_wreg_adrs },	3,	{ SB(0x8E),FDS(DATA_SIZE_WORD,SIGN_UNSIGNED),EA(0,1) }},
/*G*/	{ op_mov,	flag_086|flag_abs,lock_n_segments,	no_modifier,	2,	{ ea_mod_wreg_adrs, ea_segment_reg },	3,	{ SB(0x8C),FDS(DATA_SIZE_WORD,SIGN_UNSIGNED),EA(1,0) }},

/* Programming_the_8086_8088
 * Pages 133
 *
 * 	MOVe String (from SI to DI)
 *	1010 010w
 */
	{ op_movs,	flag_086,	repeat_n_segments,	no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xA4) }},
	{ op_movs,	flag_086,	repeat_n_segments,	byte_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xA4) }},
	{ op_movs,	flag_086,	repeat_n_segments,	word_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xA5) }},

/* Programming_the_8086_8088
 * Pages 134
 *
 * 	MULtiply source
 *	1111 011w, mod 100 r/m
 */
	{ op_mul,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xF6),IDS(0,SIGN_UNSIGNED),EAO(B100,0),SDS(0,0) }},

/* Programming_the_8086_8088
 * Pages 135
 *
 * 	NEGate distination (2s complement)
 *	1111 011w, mod 011 r/m
 */
	{ op_neg,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xF6),IDS(0,SIGN_IGNORED),EAO(B011,0),SDS(0,0) }},

/* Programming_the_8086_8088
 * Pages 136
 *
 * 	No OPeration
 *	1001 0000
 */
	{ op_nop,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x90) }},

/* Programming_the_8086_8088
 * Pages 137
 *
 * 	NOT distination (1s complement)
 *	1111 011w, mod 010 r/m
 */
	{ op_neg,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xF6),IDS(0,SIGN_IGNORED),EAO(B010,0),SDS(0,0) }},


/* Programming_the_8086_8088
 * Pages 138,139
 *
 *	Or memory or Register Operand with Register Operand
 * A	0000 10dw, mod reg r/m
 * 
 *	Or immediate Operand to Memory or Register Operand
 * B	1000 00sw, mod 001 r/m, data, data if sw = 01
 *
 *	Or immediate Operand to Accumulator
 * C	0000 110w, data, data if w =1
 */
/*C*/	{ op_or,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_accumulators, ea_immediate },	4,	{ SB(0x0C),IDS(0,SIGN_IGNORED),IMM(1),SDS(0,0) }},
/*B*/	{ op_or,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	5,	{ SB(0x80),IDS(0,SIGN_IGNORED),EAO(B001,0),IMM(1),SDS(0,0) }},
/*A*/	{ op_or,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_all_reg, ea_mod_reg_adrs },	6,	{ SB(0x08),SDR(DIRECT_TO_REG,0,1),IDS(0,SIGN_IGNORED),EA(0,1),SDS(0,0),VDS(1) }},
/*A*/	{ op_or,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_all_reg },	6,	{ SB(0x08),SDR(DIRECT_TO_EA,0,1),IDS(1,SIGN_IGNORED),EA(1,0),SDS(0,0),VDS(0) }},

/* Programming_the_8086_8088
 * Page 140
 *
 *	Output byte or word (fixed port) 
 * A	1110 011w, port
 *
 *	Output byte or word (indirect port, DX)
 * B	1110 111w
 */
/*B*/	{ op_out,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_accumulators, ea_empty },		3,	{ SB(0xEE),IDS(0,SIGN_IGNORED),SDS(0,0) }},
/*A*/	{ op_out,	flag_086,	no_prefix,		no_modifier,	2,	{ ea_immediate, ea_accumulators },	5,	{ SB(0xE6),IDS(1,SIGN_UNSIGNED),SDS(0,0),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),IMM(0) }},

/* iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-126
 *
 *	Output string (bytes or words)
 *	0110 111w
 */
	{ op_outs,	flag_186,	rep_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0x6E),FDS(DATA_SIZE_BYTE,SIGN_IGNORED),SDS(0,0) }},
	{ op_outs,	flag_186,	rep_prefix,		byte_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0x6E),FDS(DATA_SIZE_BYTE,SIGN_IGNORED),SDS(0,0) }},
	{ op_outs,	flag_186,	rep_prefix,		word_modifier,	0,	{ ea_empty, ea_empty },			3,	{ SB(0x6E),FDS(DATA_SIZE_WORD,SIGN_IGNORED),SDS(0,0) }},

/* Programming_the_8086_8088
 * Page 141
 *
 *	Pop memory or register operand
 * A	1000 1111, mod 000 r/m
 *
 *	Pop register operand
 * B	0101 1reg
 *
 * C	Pop segment register (except CS)
 * 	000r eg111
 */
/*C*/	{ op_pop,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_segment_reg, ea_empty },		3,	{ SB(0x07),TER(0,MATCH_FALSE,REG_CS),REG(0,0,3) }},
/*B*/	{ op_pop,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_word_registers, ea_empty },	2,	{ SB(0x58),REG(0,0,0) }},
/*A*/	{ op_pop,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		2,	{ SB(0x8F),EAO(B000,0) }},

/* iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-129,130
 *
 *	Pop all registers
 *	0110 0001
 */
	{ op_popa,	flag_186,	pref_lock,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x61) }},

/* Programming_the_8086_8088
 * Page 141
 * iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-131
 * 
 *	Pop flags
 *	1001 1101
 *
 *	NOTE:	The 8086 manual specifically has the opcode as "1001 1100".
 *		However the 80186 manual show this code as "1001 1101".
 *		Confirmation via an on-line assembler indicates the 80186
 *		manual is correct.
 * 	NOTE:	Machine coding in original 8086 programming manual had the
 *		binary values of popf and pushf reversed.
 */
	{ op_popf,	flag_086,	pref_lock,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x9D) }},

/* Programming_the_8086_8088
 * Page 143
 *
 *	Push memory or register operand
 * A	1111 1111, mod 110 r/m
 *
 *	Push register operand
 * B	0101 0reg
 *
 * C	Push segment register
 * 	000r eg110
 */
/*C*/	{ op_push,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_segment_reg, ea_empty },		2,	{ SB(0x06),REG(0,0,3) }},
/*B*/	{ op_push,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_word_registers, ea_empty },	2,	{ SB(0x50),REG(0,0,0) }},
/*A*/	{ op_push,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		2,	{ SB(0xFF),EAO(B110,0) }},

/* iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-134,135
 *
 *	Push all registers
 *	0110 0000
 */
	{ op_pusha,	flag_186,	pref_lock,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x60) }},

/* Programming_the_8086_8088
 * Page 144
 * iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-136
 * 
 *	Pop flags
 *	1001 1100
 *
 *	NOTE:	The 8086 manual specifically has the opcode as "1001 1101".
 *		However the 80186 manual show this code as "1001 1100".
 *		Confirmation via an on-line assembler indicates the 80186
 *		manual is correct.
 * 	NOTE:	Machine coding in original 8086 programming manual had the
 *		binary values of popf and pushf reversed.
 */
	{ op_popf,	flag_086,	pref_lock,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0x9C) }},

/* Programming_the_8086_8088
 * Page 145
 *
 *	Rotate EA Left through CF 1 bit
 * A 	1101 000w, mod 010 r/m
 *
 *	Rotate EA Left through CF 'CL' bits
 * B	1101 001w, mod 010 r/m
 *
 * iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-137,138
 *
 *	Rotate EA Left through CF imm8 bits
 * C	1100 000w, mod 010 r/m, imm8
 *	
 */
/*A*/	{ op_rcl,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xD0),IDS(0,SIGN_IGNORED),EAO(B010,0),SDS(0,0) }},
/*B*/	{ op_rcl,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_byte_reg },	5,	{ SB(0xD2),IDS(0,SIGN_IGNORED),TER(1,MATCH_TRUE,REG_CL),EAO(B010,0),SDS(0,0) }},
/*C*/	{ op_rcl,	flag_186,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	6,	{ SB(0xC0),IDS(0,SIGN_IGNORED),EAO(B010,0),SDS(0,0),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),IMM(1) }},

/* Programming_the_8086_8088
 * Page 146
 *
 *	Rotate EA Right through CF 1 bit
 * A 	1101 000w, mod 011 r/m
 *
 *	Rotate EA Right through CF 'CL' bits
 * B	1101 001w, mod 011 r/m
 *
 * iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-137,138
 *
 *	Rotate EA Right through CF imm8 bits
 * C	1100 000w, mod 011 r/m, imm8
 *	
 */
/*A*/	{ op_rcr,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xD0),IDS(0,SIGN_IGNORED),EAO(B011,0),SDS(0,0) }},
/*B*/	{ op_rcr,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_byte_reg },	5,	{ SB(0xD2),IDS(0,SIGN_IGNORED),TER(1,MATCH_TRUE,REG_CL),EAO(B011,0),SDS(0,0) }},
/*C*/	{ op_rcr,	flag_186,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	6,	{ SB(0xC0),IDS(0,SIGN_IGNORED),EAO(B011,0),SDS(0,0),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),IMM(1) }},

/* Programming_the_8086_8088
 * Page 149,150
 *
 *	Return from subroutne (near)
 * A 	1100 0011
 *
 *	Return from subroutine (near) with 16-bit stack correction
 * B	1100 0010, imm-l, imm-h
 *
 *	Return from subroutne (far)
 * C	1100 1011
 *
 *	Return from subroutine (far) with 16-bit stack correction
 * D	1100 1010, imm-l, imm-h
 */
/*B*/	{ op_ret,	flag_086,	no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xC2),FDS(DATA_SIZE_WORD,SIGN_UNSIGNED),IMM(0) }},
/*A*/	{ op_ret,	flag_086,	no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xC3) }},
/*D*/	{ op_ret,	flag_086|flag_abs,no_prefix,		far_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xCA),FDS(DATA_SIZE_WORD,SIGN_UNSIGNED),IMM(0) }},
/*C*/	{ op_ret,	flag_086|flag_abs,no_prefix,		far_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xCB) }},
/*D*/	{ op_lret,	flag_086|flag_abs,no_prefix,		no_modifier,	1,	{ ea_immediate, ea_empty },		3,	{ SB(0xCA),FDS(DATA_SIZE_WORD,SIGN_UNSIGNED),IMM(0) }},
/*C*/	{ op_lret,	flag_086|flag_abs,no_prefix,		no_modifier,	0,	{ ea_empty, ea_empty },			1,	{ SB(0xCB) }},

/* Programming_the_8086_8088
 * Page 151
 *
 *	Rotate EA Left 1 bit
 * A 	1101 000w, mod 010 r/m
 *
 *	Rotate EA Left 'CL' bits
 * B	1101 001w, mod 010 r/m
 *
 * iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-145
 *
 *	Rotate EA Left imm8 bits
 * C	1100 000w, mod 010 r/m, imm8
 *	
 */
/*A*/	{ op_rol,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xD0),IDS(0,SIGN_IGNORED),EAO(B000,0),SDS(0,0) }},
/*B*/	{ op_rol,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_byte_reg },	5,	{ SB(0xD2),IDS(0,SIGN_IGNORED),TER(1,MATCH_TRUE,REG_CL),EAO(B000,0),SDS(0,0) }},
/*C*/	{ op_rol,	flag_186,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	6,	{ SB(0xC0),IDS(0,SIGN_IGNORED),EAO(B000,0),SDS(0,0),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),IMM(1) }},

/* Programming_the_8086_8088
 * Page 152
 *
 *	Rotate EA Right 1 bit
 * A 	1101 000w, mod 011 r/m
 *
 *	Rotate EA Right 'CL' bits
 * B	1101 001w, mod 011 r/m
 *
 * iAPX86_88_186_188_Programmers_Reference (1983)
 * Page 3-146
 *
 *	Rotate EA Right imm8 bits
 * C	1100 000w, mod 011 r/m, imm8
 *	
 */
/*A*/	{ op_ror,	flag_086,	lock_n_segments,	no_modifier,	1,	{ ea_mod_reg_adrs, ea_empty },		4,	{ SB(0xD0),IDS(0,SIGN_IGNORED),EAO(B001,0),SDS(0,0) }},
/*B*/	{ op_ror,	flag_086,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_byte_reg },	5,	{ SB(0xD2),IDS(0,SIGN_IGNORED),TER(1,MATCH_TRUE,REG_CL),EAO(B001,0),SDS(0,0) }},
/*C*/	{ op_ror,	flag_186,	lock_n_segments,	no_modifier,	2,	{ ea_mod_reg_adrs, ea_immediate },	6,	{ SB(0xC0),IDS(0,SIGN_IGNORED),EAO(B001,0),SDS(0,0),FDS(DATA_SIZE_BYTE,SIGN_UNSIGNED),IMM(1) }},

/* SHAF */

	{ nothing }
};

/*
 *	Look for an instruction definition given the details provided.
 */
opcode *find_opcode( modifier mods, component op, int args, ea_breakdown *format ) {
	opcode	*search;
	int	a;
	
	/*
	 *	So .. at this point we have gathered everything together
	 *	all the bits and pieces which will allow us to identify
	 *	what actual opcode the programmer was trying to code.
	 *
	 *	In theory.
	 */
	for( search = opcodes; search->op != nothing; search++ ) {
		if(( search->op == op )&&( search->args == args )&&( search->mods == mods )) {
			for( a = 0; a < args; a++ ) if(( search->arg[ a ] & format[ a ].ea ) != format[ a ].ea ) break;
			if( a == args ) return( search );
		}
	}
	return( NIL( opcode ));
}


/*
 *	EOF
 */
