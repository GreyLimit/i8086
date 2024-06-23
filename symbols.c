/*
 *	symbols
 *	=======
 *
 *	Define human symbolic values for the components.
 */

#include "os.h"
#include "includes.h"

/*
 *	Define a universal structure for capturing relationships
 *	between human symbolic labels and internal representation.
 */
typedef struct {
	component	id;
	char		*text;
} match;

/*
 *	Define the table of keywords and opcodes.
 */
static match all_keywords[] = {
	/*
	 *	Opcodes.
	 */
	{ op_aaa,	"aaa"	}, { op_aad,	"aad"	}, { op_aam,	"aam"	}, { op_aas,	"aas"	},
	{ op_adc,	"adc"	}, { op_add,	"add"	}, { op_and,	"and"	}, { op_bound,	"bound"	},
	{ op_break,	"break"	}, { op_call,	"call"	}, { op_lcall,	"lcall"	},
	{ op_cbw,	"cbw"	}, { op_clc,	"clc"	}, { op_cld,	"cld"	}, { op_cli,	"cli"	},
	{ op_cmc,	"cmc"	}, { op_cmp,	"cmp"	}, { op_cmps,	"cmps"	}, { op_cwd,	"cwd"	},
	{ op_daa,	"daa"	}, { op_das,	"das"	}, { op_dec,	"dec"	}, { op_div,	"div"	},
	{ op_esc,	"esc"	}, { op_enter,	"enter"	}, { op_hlt,	"hlt"	},
	{ op_idiv,	"idiv"	}, { op_imul,	"imul"	},
	{ op_in,	"in"	}, { op_inc,	"inc"	}, { op_ins,	"ins"	}, { op_int,	"int"	},
	{ op_intr,	"intr"	}, { op_into,	"into"	}, { op_iret,	"iret"	},
	{ op_ja,	"ja"	}, { op_jnbe,	"jnbe"	}, { op_jbe,	"jbe"	}, { op_jae,	"jae"	},
	{ op_jna,	"jna"	}, { op_jnb,	"jnb"	}, { op_jb,	"jb"	}, { op_jnae,	"jnae"	},
	{ op_jc,	"jc"	}, { op_jcxz,	"jcxz"	}, { op_je,	"je"	}, { op_jz,	"jz"	},
	{ op_jg,	"jg"	}, { op_jnle,	"jnle"	}, { op_jge,	"jge"	}, { op_jnl,	"jnl"	},
	{ op_jl,	"jl"	}, { op_jnge,	"jnge"	}, { op_jle,	"jle"	}, { op_jng,	"jng"	},
	{ op_jmp,	"jmp"	}, { op_ljmp,	"ljmp"	}, { op_jnc,	"jnc"	}, { op_jne,	"jne"	},
	{ op_jnz,	"jnz"	}, { op_jno,	"jno"	}, { op_jnp,	"jnp"	}, { op_jpo,	"jpo"	},
	{ op_jns,	"jns"	}, { op_jo,	"jo"	}, { op_jp,	"jp"	}, { op_jpe,	"jpe"	},
	{ op_js,	"js"	},
	{ op_lahf,	"lahf"	}, { op_lds,	"lds"	}, { op_lea,	"lea"	}, { op_leave,	"leave"	},
	{ op_les,	"les"	}, { op_lods,	"lods"	},
	{ op_loop,	"loop"	}, { op_loope,	"loope"	}, { op_looppe,	"looppe"}, { op_looppz,	"looppz"},
	{ op_loopz,	"loopz"	}, { op_loopne,	"loopne"}, { op_loopna,	"loopna"}, { op_loopnz,	"loopnz"},
	{ op_mov,	"mov"	}, { op_movs,	"movs"	}, { op_movsb,	"movsb"	}, { op_movsw,	"movsw"	},
	{ op_mul,	"mul"	}, { op_neg,	"neg"	}, { op_nop,	"nop"	}, { op_not,	"not"	},
	{ op_or,	"or"	}, { op_out,	"out"	}, { op_outs,	"outs"	},
	{ op_pop,	"pop"	}, { op_popa,	"popa"	}, { op_popf,	"popf"	},
	{ op_push,	"push"	}, { op_pushf,	"pusha"	}, { op_pushf,	"pushf"	},
	{ op_rcl,	"rcl"	}, { op_rcr,	"rcr"	},
	{ op_ret,	"ret"	}, { op_lret,	"lret"	}, { op_rol,	"rol"	}, { op_ror,	"ror"	},
	{ op_sahf,	"sahf"	}, { op_sal,	"sal"	}, { op_shl,	"shl"	}, { op_sar,	"sar"	},
	{ op_sbb,	"sbb"	}, { op_scas,	"scas"	}, { op_shr,	"shr"	}, { op_stc,	"stc"	},
	{ op_std,	"std"	}, { op_sti,	"sti"	}, { op_stos,	"stos"	}, { op_sub,	"sub"	},
	{ op_test,	"test"	}, { op_wait,	"wait"	}, { op_xchg,	"xchg"	}, { op_xlat,	"xlat"	},
	{ op_xor,	"xor"	},
	/*
	 *	Prefixes
	 */
	{ pref_lock,	"lock"	}, { pref_rep,	"rep"	}, { pref_repe,	"repe"	},
	{ pref_repz,	"repz"	}, { pref_repne, "repne" }, { pref_repnz, "repnz" },
	/*
	 *	Registers
	 */
	{ reg_al,	"al"	}, { reg_ah,	"ah"	}, { reg_ax,	"ax"	},
	{ reg_bl,	"bl"	}, { reg_bh,	"bh"	}, { reg_bx,	"bx"	},
	{ reg_cl,	"cl"	}, { reg_ch,	"ch"	}, { reg_cx,	"cx"	},
	{ reg_dl,	"dl"	}, { reg_dh,	"dh"	}, { reg_dx,	"dx"	},
	{ reg_sp,	"sp"	}, { reg_bp,	"bp"	}, { reg_si,	"si"	}, { reg_di,	"di"	},
	{ reg_cs,	"cs"	}, { reg_ds,	"ds"	}, { reg_ss,	"ss"	}, { reg_es,	"es"	},
	/*
	 *	Op code modifiers.
	 */
	{ mod_byte,	"byte"	}, { mod_word,	"word"	}, { mod_ptr,	"ptr"	},
	{ mod_near,	"near"	}, { mod_far,	"far"	},
	/*
	 *	Assembler directives.
	 */
	{ asm_org,	"org"		}, { asm_align,	"align"	},
	{ asm_segment,	"segment"	}, { asm_group,	"group"	},
	{ asm_db,	"db"		}, { asm_dw,	"dw"	},
	{ asm_reserve,	"reserve"	}, { asm_equ,	"equ"	},
	/*
	 *	Directives related to the current file
	 */
	{ asm_include,	"include"	},
	{ asm_export,	"export"	},
	{ asm_import,	"import"	},
	{ asm_end,	"end"		},
	/*
	 *	End of array marked thus:
	 */
	{ nothing }
};

/*
 *	Define the table of symbols.
 */
static match all_symbols[] = {
	{ tok_semicolon,	";"	}, { tok_colon,		":"	},
	{ tok_comma,		","	}, { tok_period,	"."	},
	{ tok_oparen,		"("	}, { tok_cparen,	")"	},
	{ tok_obracket,		"["	}, { tok_cbracket,	"]"	},
	{ tok_plus,		"+"	}, { tok_minus,		"-"	},
	{ tok_mul,		"*"	}, { tok_div,		"/"	},
	{ tok_and,		"&"	}, { tok_or,		"|"	},
	{ tok_not,		"!"	}, { tok_xor,		"^"	},
	{ tok_shl,		"<<"	}, { tok_shr,		">>"	},
	{ nothing }
};

/*
 *	Define the table of symbols.
 */
static match all_synthetic[] = {
	{ tok_immediate,	"<immediate>"		},
	{ tok_label,		"<label>"		},
	{ tok_string,		"<string>"		},
	{ nothing }
};

/*
 *	Conversion of component ID back to printable
 *	text.
 */
const char *component_text( component comp ) {
	match	*look;

	for( look = all_keywords; look->id != nothing; look++ ) {
		if( look->id == comp ) return( look->text );
	}
	for( look = all_symbols; look->id != nothing; look++ ) {
		if( look->id == comp ) return( look->text );
	}
	for( look = all_synthetic; look->id != nothing; look++ ) {
		if( look->id == comp ) return( look->text );
	}
	return( "<Unknown>" );
}

/*
 *	Component identification routines..
 */
static int match_all( char *test, char *target ) {
	int	l;
	char	c;

	/*
	 *	Attempts to find a match for 'target' at the head of
	 *	the 'test' string.
	 *
	 *	Returns either:
	 *
	 *	o	Length of target, if (and only if) test matches
	 *		all the characters.
	 *
	 * 	o	Zero
	 */
	l = 0;
	c = EOS;
	if( BOOL( command_flags & ignore_keyword_case )) {
		while(( tolower( *test++ ) == tolower( c = *target++ ))) {
			if( c == EOS ) break;
			l++;
		}
	}
	else {
		while(( *test++ == ( c = *target++ ))) {
			if( c == EOS ) break;
			l++;
		}
	 }
	 return(( c == EOS )? l: 0 );
}

static int find_best( char *search, match *here, component *found ) {
	int		k, l;
	component	f;

	/*
	 *	Searches an array of values and finds the longest
	 *	match of the search string with these values.
	 *
	 *	If something was found returns its length (and fills in
	 *	the found value) or 0 if there was no match.
	 */
	l = 0;
	f = nothing;
	while( here->id != nothing ) {
		if(( k = match_all( search, here->text )) > l ) {
			l = k;
			f = here->id;
		}
		here++;
	}
	*found = f;
	return( l );
}

/*
 *	Keyword and symbol identification
 */
int find_best_keyword( char *search, component *found ) {
	return( find_best( search, all_keywords, found ));
}

int find_best_symbol( char *search, component *found ) {
	return( find_best( search, all_symbols, found ));
}


int match_identifier( char *search ) {
	int	l;
	char	c;

	/*
	 *	Returns the number of characters which correctly
	 *	form an identifier (or keyword or opcode etc) from
	 *	the head of the string supplied.
	 */
	l = 0;
	c = *search++;
	if( isalpha( c ) ||( c == USCORE )||( c == PERIOD )) {
		l++;
		c = *search++;
		while( isalnum( c )||( c == USCORE )) {
			l++;
			c = *search++;
		}
	}
	return( l );
}

int digit_value( char d ) {
	if(( d >= '0' )&&( d <= '9' )) return( (int)( d - '0' ));
	if(( d >= 'A' )&&( d <= 'F' )) return( (int)( d - ( 'A' - 10 )));
	if(( d >= 'a' )&&( d <= 'f' )) return( (int)( d - ( 'a' - 10 )));
	return( ERROR );
}

boolean isoctal( char o ) {
	return(( o >= '0' )&&( o <= '7' ));
}

boolean ishex( char h ) {
	return(	isdigit( h ) ||
		(( h >= 'a' )&&( h <= 'f' )) ||
		(( h >= 'A' )&&( h <= 'F' )));
}

int character_constant( char *string, char *value ) {
	char	c;
	int	v;

	/*
	 *	Reads a single character value into value
	 *	from string, returning the number of characters
	 *	it took to build the value.
	 *
	 *	Simple cases first:
	 */
	if(( c = *string++ ) == EOS ) return( 0 );
	if( c != ESCAPE ) {
		*value = c;
		return( 1 );
	}
	if(( c = *string++ ) == EOS ) return( ERROR );
	/*
	 *	So, something preceded by an escape character '\',
	 *	now stored in c.
	 */
	switch( c ) {
		case 'a': {
			*value = '\a';
			return( 2 );
		}
		case 'b': {
			*value = '\b';
			return( 2 );
		}
		case 'e': {
			*value = '\e';
			return( 2 );
		}
		case 'f': {
			*value = '\f';
			return( 2 );
		}
		case 'n': {
			*value = '\n';
			return( 2 );
		}
		case 'r': {
			*value = '\r';
			return( 2 );
		}
		case 't': {
			*value = '\t';
			return( 2 );
		}
		case 'v': {
			*value = '\v';
			return( 2 );
		}
		case 'x': {
			if( !ishex(( c = *string++ ))) return( ERROR );
			v = digit_value( c );
			if( !ishex(( c = *string++ ))) return( ERROR );
			v = ( v << 4 ) | digit_value( c );
			*value = (char)v;
			return( 4 );
		}
		default: {
			if( isoctal( c )) {
				/*
				 *	This *must* now be a three digit octal
				 *	number.
				 */
				v = digit_value( c );
				if( !isoctal(( c = *string++ ))) return( ERROR );
				v = ( v << 3 ) | digit_value( c );
				if( !isoctal(( c = *string++ ))) return( ERROR );
				v = ( v << 3 ) | digit_value( c );
				*value = (char)v;
				return( 4 );
			}
		}
		break;
	}
	/*
	 *	Escaping a meaningless character gives just that character.
	 */
	*value = c;
	return( 2 );
}

int string_constant( char quote, char *search, char *value, int max, int *fill, boolean *errors ) {
	int	used,
		filled;
	boolean	errd;

	/*
	 *	Quote at the beginning should be a given, if not then
	 *	this is not a string/character constant.
	 */
	if( *search++ != quote ) {
		*fill = 0;
		return( 0 );
	}
	/*
	 *	Now loop through the data.
	 */
	used = 1;
	filled = 0;
	errd = FALSE;
	while( *search != quote ) {
		int	l;
		char	c;

		switch(( l = character_constant( search, &c ))) {
			case ERROR: {
				log_error( "Malformed ASCII constant" );
				*fill = filled;
				*errors = TRUE;
				return( used );
			}
			case 0: {
				log_error( "Unterminated ASCII constant" );
				*fill = filled;
				*errors = TRUE;
				return( used );
			}
			default: {
				if( filled < max ) {
					value[ filled++ ] = c;
				}
				else {
					if( !errd ) {
						log_error( "ASCII constant too long" );
						errd = TRUE;
						*errors = TRUE;
					}
				}
				break;
			}
		}
		used += l;
		search += l;
	}
	used += 1;
	*fill = filled;
	return( used );
}

int match_constant( char *search, integer *value, boolean *errors ) {
	/*
	 *	Routine matches any explicit numerical
	 *	value (either a character or a number in
	 *	one of the supported bases) and returns
	 *	the length of the representation as the
	 *	result and the value via the parameter.
	 *
	 *	Formats:
	 *		$xxxx		Hexidecimal
	 *		@xxxx		Octal
	 *		%xxxx		Binary
	 *		xxxx		Decimal
	 *		xxxxH		Hexidecimal
	 *		xxxxO		Octal
	 *		xxxxB		Binary
	 *
	 *		C style numeric constants
	 *
	 *		0xhhhh		Hexidecimal
	 *		0ooooo		Octal
	 */
	char		number[ MAX_CONST_SIZE ],
			*ptr;
	int		used, len, base, starts, i, d;
	integer		sum;
	boolean		err;

	sum = 0;
	used = 0;
	len = 0;
	starts = 0;
	base = 10;
	ptr = search;
	switch( *ptr ) {
		case DOLLAR: {
			base = 16;
			used++;
			ptr++;
			break;
		}
		case AT: {
			base = 8;
			used++;
			ptr++;
			break;
		}
		case PERCENT: {
			base = 2;
			used++;
			ptr++;
			break;
		}
		default: {
			/*
			 *	Assume base 10, for the moment.
			 */
			break;
		}
	}
	/*
	 *	If a leading base symbol has not been used then
	 *	the first digit of the number constant must be
	 *	in the range '0' to '9'.
	 */
	if(( used == 0 )&& !isdigit( *ptr )) return( 0 );
	/*
	 *	We gather up all digits, regardless of the base
	 *	being used.  If we run out of buffer space we
	 *	will (for the moment) look the other way.
	 */
	err = FALSE;
	while( isalnum( *ptr )) {
		if( len < MAX_CONST_SIZE ) {
			number[ len++ ] = *ptr++;
		}
		else {
			if( !err ) {
				log_error_s( "Numeric constant too long", search );
				*errors = TRUE;
				err = TRUE;
			}
			ptr++;
		}
		used++;
	}
	/*
	 *	Check out C style constants but, once again, only
	 *	if the base is still 10.
	 */
	if(( base == 10 )&&( len > 2 )&&( number[ 0 ] == '0' )) {
		switch( number[ 1 ]) {
			case 'x':
			case 'X': {
				base = 16;
				starts = 2;
				break;
			}
			case 'b':
			case 'B': {
				base = 2;
				starts = 2;
				break;
			}
			case 'o':
			case 'O': {
				base = 8;
				starts = 2;
				break;
			}
			default: {
				base = 8;
				break;
			}
		}
	}
	/*
	 *	If we are still in base 10 we shall see if there
	 *	is a post-fix base assignment.
	 */
	if(( base == 10 )&&( len > 1 )) {
		switch( number[ len-1 ]) {
			case 'h':
			case 'H': {
				base = 16;
				len--;
				break;
			}
			case 'o':
			case 'O': {
				base = 8;
				len--;
				break;
			}
			case 'b':
			case 'B': {
				base = 2;
				len--;
				break;
			}
			default: {
				break;
			}
		}
	}
	/*
	 *	Finally, the last C style version is OCTAL because
	 *	the number started with a leading 0.
	 */
	if(( base == 10 )&&( number[ 0 ] == '0' )) base = 8;
	/*
	 *	Now calculate the value of the constant.
	 */
	err = FALSE;
	for( i = starts; i < len; i++ ) {
		sum *= base;
		d = digit_value( number[ i ]);
		if(( d < 0 )||( d >= base )) {
			log_error_c( "Invalid digit", number[ i ]);
			*errors = TRUE;
		}
		else {
			sum += d;
		}
		if( sum < 0 ) {
			if( !err ) {
				log_error( "Constant overflow" );
				err = TRUE;
			}
			sum = -sum;
			*errors = TRUE;
		}
	}
	/*
	 *	Wrap up and return.
	 */
	*value = sum;
	return( used );
}


/*
 *	EOF
 */
