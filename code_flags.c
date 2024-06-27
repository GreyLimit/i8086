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
	{ flag_priv,	'P'	},
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


