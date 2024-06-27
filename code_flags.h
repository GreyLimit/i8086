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

#ifndef _CODE_FLAGS_H_
#define _CODE_FLAGS_H_

/*
 *	Define mnemonic flags allowing instructions to be categorised by various
 *	criteria.
 */
typedef enum {
	flag_none	= 00000,	/* No flags specified, invalid instruction */
	flag_086	= 00001,	/* First available on the 8086/88 CPU */
	flag_186	= 00002,	/* First available on the 80186/188 CPU */
	flag_286	= 00004,	/* First available on the 80286 CPU */
	flag_386	= 00010,	/* 80386 - not yet used */
	flag_486	= 00020,	/* 80486 - not yet used */
	flag_abs	= 00100,	/* Instruction is absolute, position specific */
	flag_seg	= 00200,	/* Instruction modifies a segment register */
	flag_priv	= 00400,	/* Instructions requires priviledged status */

	/*
	 *	Composite flags to simplify encoding.  The flags indicate
	 *	which of the CPUs an instruction was introduced on and so
	 *	which CPUs it can be found on.
	 */
	cpu_8086	= flag_086 | flag_186 | flag_286,
	cpu_80186	= flag_186 | flag_286,
	cpu_80286	= flag_286,

	inst_abs	= flag_abs,
	inst_seg	= flag_seg
} mnemonic_flags;

/*
 *	Define the variable holding the current set of assembly
 *	language control settings.
 */
extern mnemonic_flags assembler_parameters;


#ifdef VERIFICATION
/*
 *	Routine used to expand flags into textual data.
 */
extern void expand_mnemonic_flags( mnemonic_flags flags, char *buffer, int max );

#endif

#endif

/*
 *	EOF
 */

