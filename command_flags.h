/*
 *	command_flags
 *	=============
 *
 *	Define the set of flags that modify the function
 *	of the command.
 */

#ifndef _COMMAND_FLAGS_H_
#define _COMMAND_FLAGS_H_

/*
 *	Define the set of flags which control some of the operational
 *	characteristics of the assembler.
 */
typedef enum {
	no_command_flags		= 0000000,	/* No command line flags enabled. */
	/*
	 *	Assembler operating modifications
	 */
	ignore_keyword_case		= 0000001,	/* Keywords are case insensitive. */
	ignore_label_case		= 0000002,	/* Labels are case insensitive. */
	/*
	 *	Select assembler output
	 */
	generate_dot_com		= 0000004,	/* Create a simple '.COM' executable. */
	generate_dot_exe		= 0000010,	/* Create a complex '.EXE. executable. */
	generate_dot_obj		= 0000020,	/* Create a '.OBJ' intermediate file. */
	generate_hex			= 0000040,	/* Output above files in HEXadecimal. */
	generate_listing		= 0000100,	/* Create an assembly listing file. */
	/*
	 *	Select target processor
	 *
	 *	8086/88 and 80186/88 Processors for 'pure' 16 bit CPUs.  While
	 *	the apparent memory capacity is 1 MBytes (20 bit address), the CPU
	 *	is incapable of accessing any memory without the inclusion of a
	 *	segment register effectively providing the top 16 bits of the address.
	 */
	intel_8086			= 0000200,	/* Basic 8086/88 CPU instructions */
	intel_80186			= 0000400,	/* Enhanced 80186/88 CPU instructions */
	/*
	 *	80286 is a 'bridgehead' between the 20 bit addressing of the earlier
	 *	CPUs and a full 32 bit CPU to follow.  This CPU is 'modal' and can operate
	 *	in either "Real Mode" pretending to be an 8086 CPU, or can operate in
	 *	"Protected Mode" where full 8086 compatibility is sacrificed to
	 *	enable a functional Memory Management System.  In Protected
	 *	Mode the full 24 bit address bus of the '286 can be accessed by a
	 *	program.
	 */
	intel_80286			= 0001000,	/* Extended 80286 CPU instructions */
	/*
	 *	Select restrictions on accepted code.
	 */
	allow_segment_access		= 0002000,	/* Allow access to segment registers */
	allow_position_dependent	= 0004000,	/* Allow position dependent code */
	/*
	 *	Misc options
	 */
	show_help			= 0010000,	/* Display help text */
	be_verbose			= 0020000,	/* Make more noise while working */
	more_verbose			= 0040000,	/* Display more details about internal ops. */
	
#ifdef VERIFICATION
	dump_opcodes			= 0100000,	/* Display the content of the opcode encoding table */
#endif

	/*
	 *	Define some group classifications.
	 */
	output_selection_mask		= ( generate_dot_com | generate_dot_exe | generate_dot_obj | generate_listing ),
	cpu_selection_mask		= ( intel_8086 | intel_80186 | intel_80286 )
	
} command_flag;


/*
 *	Define the global holding the flags current in force.
 */
extern command_flag command_flags;

/*
 *	Return TRUE if the output format matches the configuration of
 *	the source code being assembled.
 */
extern boolean output_format_valid( void );


#endif

/*
 *	EOF
 */
