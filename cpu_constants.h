/*
 *	cpu_constants
 *	=============
 *
 *	Constant values which are specific to the CPU architecture.
 */

#ifndef _CPU_CONSTANTS_H_
#define _CPU_CONSTANTS_H_

/*
 *	Define here all sorts of quantitive values that are
 *	directly dependent on the CPU(s) being catered for
 *	and the impact this has upon the implementation of the
 *	assembler.
 */
 
/*
 * 	Number of byte and word registers
 */
#define BYTE_REGISTERS		8
#define WORD_REGISTERS		8

/*
 *	Provide simple value names for the segment register numbers as
 *	well as values representing situations where either a segment
 *	register has not been identified yet (UNKNOWN_SEG) or where a
 *	segment register is not required/pertinent (UNREQUIRED_SEG).
 */
#define SEGMENT_REGISTERS	4

#define CODE_SEG_REG		0
#define DATA_SEG_REG		1
#define STACK_SEG_REG		2
#define EXTRA_SEG_REG		3

/*
 *	Special case handles outside the normal segment registers
 *	internal numbers.
 */
#define UNKNOWN_SEG		(SEGMENT_REGISTERS)
#define UNREQUIRED_SEG		(SEGMENT_REGISTERS+1)

/*
 *	Define some limts on the nature of a single assembler
 *	or machine code instruction.
 *
 *	MAX_CODE_BYTES		The maximum number of bytes which can
 *				form an single machine code instruction
 *				(excluding the prefix bytes).
 *
 *	MAX_PREFIX_BYTES	The maximum number of bytes which can
 *				precede a machine code instruction as
 *				a prefix sequence.
 *
 *	MAX_REGISTERS		The maximum number of registers which
 *				can be part of a single instruction
 *				argument.
 */
#define MAX_CODE_BYTES		6
#define MAX_PREFIX_BYTES	4
#define MAX_REGISTERS		2


/*
 *	Define the maximum number of arguments to an opcode and
 *	the maximum number of encoding words possible.
 */
#define MAX_OPCODE_ARGS		2
#define MAX_OPCODE_ENCODING	8




#endif

/*
 *	EOF
 */
