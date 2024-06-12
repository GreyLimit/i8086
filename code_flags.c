/*
 *	code_flags
 *	==========
 *
 *	Flags required to capture machine code characteristics.
 */

#include "os.h"
#include "includes.h"

/*
 *	Define the variable holding the current set of assembly
 *	language control settings.
 */
mnemonic_flags assembler_parameters = flag_none;


#ifdef VERIFICATION

/*
 *	Define a routine to convert a mnemonic_flags value into a
 *	displayable textual representation.
 */
static struct {
	mnemonic_flags		flag;
	char			symbol;
} mnemonic_flags_table[] = {
	{ flag_086,	'0'	},
	{ flag_186,	'1'	},
	{ flag_286,	'2'	},
	{ flag_386,	'3'	},
	{ flag_486,	'4'	},
	{ flag_abs,	'A'	},
	{ flag_seg,	'S'	},
	{ flag_none,	EOS	}
};

void expand_mnemonic_flags( mnemonic_flags flags, char *buffer, int max ) {
	int	i, f;
	
	ASSERT( max > 0 );
	
	f = 0;
	max--;	/* reserve 1 byte for EOS */
	for( i = 0; ( f < max )&&( mnemonic_flags_table[ i ].flag != flag_none ); i++ ) {
		if( BOOL( mnemonic_flags_table[ i ].flag & flags )) buffer[ f++ ] = mnemonic_flags_table[ i ].symbol;
	}
	buffer[ f ] = EOS;
}


#endif

/*
 *	EOF
 */


