/*
 *	i8086
 *	=====
 *
 *	Another simple (and simple minded) assembler by me
 *	(Jeff Penfold) for the 16 bit Intel x86 processors.
 *
 *	Specifically:	8086	8088
 *			80186	80188
 *			80286
 *
 *	The objective is to create an assembler that is as
 *	small as possible that can be compiled into an
 *	operational "tiny memory model" 64 KByte footprint.
 *
 *	We shall see.
 *
 *	Note also that data structures defined are primarily
 *	for simplicity of implementation first, minimal footprint
 *	next and speed last.
 *
 *	The marker 'TODO' is used to tag code which is either
 *	incomplete or contains a known error.
 *
 *	April 2024
 * 
 *		Additional Note:  This assembler is neither simple
 *		or simple minded; the structure of the machine code
 *		follows some regularity, but with sufficient exceptions
 *		that a simple minded approach is quickly swamped
 *		becoming complex code.  Therefore a more complex
 *		approach has been adopted which should enable the
 *		nature of the machine code to be more readily captured.
 * 
 *	May 2024
 * 
 * 		To assist with encoding the instructions (through
 *		provision of verification mechanism) the '--dump-
 *		opcodes' option is provided as part of the 'VERIFICATION'
 *		enabled option source code.  This can be used with
 *		(or without) the '--verbose' option to dump the
 *		internal instruction table.
 *
 * 	June 2024
 *
 * 		Source code broken into multiple modules, many, many
 *		modules.  As a consequence this main source code file
 *		contains only that code required to handle program
 *		arguments and a final call to the routine that assembles
 *		a program file.
 */

#include "os.h"
#include "includes.h"



/*
 *	Flags and bit field equivalent.
 */
static struct {
	char		*flag,
			*explain;
	command_flag	bit;
	mnemonic_flags	params;
} possible_flag[] = {
	{ "--ignore-keyword-case",	"Make keywords case insensitive",	ignore_keyword_case,	flag_none	},
	{ "--ignore-label-case",	"Make labels case insensitive",		ignore_label_case,	flag_none	},
	{ "--com",			"Output a '.COM' executable",		generate_dot_com,	flag_none	},
	{ "--exe",			"Output a '.EXE' executable",		generate_dot_exe,	flag_none	},
	{ "--obj",			"Output a '.OBJ' linkable file",	generate_dot_obj,	flag_none	},
	{ "--hex",			"Output binary files in ASCII",		generate_hex,		flag_none	},
	{ "--ascii",			"Output binary files in ASCII",		generate_hex,		flag_none	},
	{ "--listing",			"Produce detailed listing",		generate_listing,	flag_none	},
	{ "--8086",			"Only permit 8086 code",		intel_8086,		flag_086	},
	{ "--8088",			"Only permit 8088 code",		intel_8086,		flag_086	},
	{ "--80186",			"Only permit 80186 and earlier code",	intel_80186,		flag_186	},
	{ "--80188",			"Only permit 80188 and earlier code",	intel_80186,		flag_186	},
	{ "--80286",			"Only permit 80286 and earlier code",	intel_80286,		flag_286	},
	{ "--access-segments",		"Permit assignment to segments",	allow_segment_access,	flag_seg	},
	{ "--position-dependent",	"Permit fixed/absolute position code",	allow_position_dependent,flag_abs	},
	{ "--help",			"Show this help",			show_help,		flag_none	},

#ifdef VERIFICATION
	{ "--dump-opcodes",		"Dump internal opcode table",		dump_opcodes,		flag_none	},
#endif

	{ "--verbose",			"Show extra details during assembly",	be_verbose,		flag_none	},
	{ "--very-verbose",		"Show even more detail",		be_verbose|more_verbose,flag_none	},
	{ NIL( char ) }
};

/*
 *	Pick out the flags
 */
static boolean process_flags( int *argc, char *argv[] ) {
	int	i, j, k;

	command_flags = no_command_flags;
	i = 1;
	while( i < *argc ) {
		for( j = 0; possible_flag[ j ].flag != NIL( char ); j++ ) {
			if( strcmp( argv[ i ], possible_flag[ j ].flag ) == 0 ) break;
		}
		if( possible_flag[ j ].flag != NIL( char )) {
			command_flags |= possible_flag[ j ].bit;
			assembler_parameters |= possible_flag[ j ].params;
			for( k = i; k < *argc; k++ ) argv[ k ] = argv[ k+1 ];
			*argc -= 1;
		}
		else {
			i++;
		}
	}
	if( BOOL( command_flags & show_help )) {
		printf( "Options:-\n" );
		for( i = 0; possible_flag[ i ].flag != NIL( char ); i++ ) printf( "\t%-24s%s\n", possible_flag[ i ].flag, possible_flag[ i ].explain );
		exit( 0 );
	}

#ifdef VERIFICATION
	if( BOOL( command_flags & dump_opcodes )) {

		ASSERT( this_pass == no_pass );
		
		this_pass = data_verification;
		if( BOOL( command_flags & be_verbose )) {
			dump_opcode_list( BOOL( command_flags & more_verbose ));
		}
		else {
			dump_opcode_table();
		}
		exit( 0 );
	}
#endif

	if( !BOOL( command_flags & cpu_selection_mask )) {
		log_error( "Target CPU not specified" );
		return( FALSE );
	}
	if( !BOOL( command_flags & output_selection_mask )) {
		log_error( "Output format not specified" );
		return( FALSE );
	}
	return( TRUE );
}

/*
 *	The main entry point for the "i8086" assembler.
 */
int main( int argc, char *argv[] ) {
	int	count;

	/*
	 *	Process out arguments
	 */
	if( !process_flags( &argc, argv )) {
		log_error( "Error detected in assembler options" );
		return( 1 );
	}
	/*
	 *	We should have just one argument left, the file name.
	 */
	if( argc != 2 ) {
		log_error( "Expecting one source file" );
		return( 1 );
	}
	/*
	 *	Set up output data
	 */
	switch( command_flags & output_selection_mask ) {
		case generate_dot_com: {
			initialise_output( &com_output_api, BOOL( command_flags & generate_hex ));
			break;
		}
		case generate_dot_exe: {
			log_error( ".EXE not implemented" );
			return( 1 );
		}
		case generate_dot_obj: {
			log_error( ".OBJ not implemented" );
			return( 1 );
		}
		case generate_listing: {
			initialise_output( &listing_output_api, BOOL( command_flags & generate_hex ));
			break;
		}
	}
	/*
	 *	Initialise the selected output mechanism
	 */
	if( !open_file( argv[ 1 ])) {
		log_error( "Unable to initialise output." );
		return( 1 );
	}
	/*
	 *	We run the file processing code until the reset_state
	 *	routine says we have done it enough.
	 */
	count = 0;
	while( reset_state()) {
		count++;
		if( BOOL( command_flags & be_verbose )) printf( "Start PASS %d.\n", count );
		if( !process_file( argv[ 1 ])) {
			log_error( "Assembly terminated" );
			(void)close_file();
			return( 1 );
		}
		if( BOOL( command_flags & be_verbose ) && ( this_pass == pass_value_confirmation )) dump_labels();
	}
	/*
	 *	If we get here we have succeeded.
	 */
	if( !close_file()) {
		log_error( "Unable to finalise output." );
		return( 1 );
	}
	return( 0 );
}

/*
 *	EOF
 */
