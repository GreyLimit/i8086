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
 *	In the instruction encodings and register tables the registers
 *	are indexed by their CPU internal index number.  These are fixed by
 *	the CPU and rely on contextual knowledge to know if you are specifying
 *	a byte, word or segment register.
 */
 
/* BYTE Registers */
#define BYTE_REGISTERS		8
#define REG_AL			0
#define REG_CL			1
#define REG_DL			2
#define REG_BL			3
#define REG_AH			4
#define REG_CH			5
#define REG_DH			6
#define REG_BH			7

/* WORD Registers */
#define WORD_REGISTERS		8
#define REG_AX			0
#define REG_CX			1
#define REG_DX			2
#define REG_BX			3
#define REG_SP			4
#define REG_BP			5
#define REG_SI			6
#define REG_DI			7

/* Segment Registers */
#define SEGMENT_REGISTERS	4
#define REG_CS			0
#define REG_DS			1
#define REG_SS			2
#define REG_ES			3
/*
 *	Special case handles outside the normal segment registers
 *	internal numbers.
 */
#define UNKNOWN_SEG		(SEGMENT_REGISTERS)
#define UNREQUIRED_SEG		(SEGMENT_REGISTERS+1)

/*
 *	Number of registers which can be used to directly reference
 *	into memory without displacements or base registers.  This is
 *	an 'odd' number due to the way EAs are encoded.
 */
#define POINTER_REGISTERS	3

/*
 *	Number of base and index registers used in EAs.
 */
#define BASE_REGISTERS		2
#define INDEX_REGISTERS		2

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
