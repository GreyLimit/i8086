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
 *	Constants
 *	=========
 *
 *	Application specific configuration constants
 */

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

/*
 *	The programs overall version ID.
 */
#define PROGRAM_VERSION_NUMBER		"V0.1.0"

/*
 *	Define an arbitary size limit on a numberical
 *	values' representation (not its value).  This
 *	if sized to enable a 16 bit binary value to
 *	be expressed.
 */
#define MAX_CONST_SIZE	20

/*
 *	Define the size of the line buffer used to read the input from
 *	the source file.  This creates a number of source code limitations
 *	but realistically (for assembly language) anything over 80
 *	characters should be adequate.
 */
#define MAX_LINE_SIZE	128

/*
 *	Define another arbitary limit on the size of a single token.
 */
#define MAX_TOKEN_SIZE	64

/*
 *	Declare the maximum number of arguments a single line can
 *	contains.
 */
#define MAX_ARG_COUNT	16

/*
 *	Define the maximum number of file that can be nested.
 */
#define MAX_FILE_NESTING	8

/*
 *	Indicate how many flags could be applied at once.
 */
#define MAXIMUM_FLAGS		8

/*
 *	EVAL_STACK is the depth of the stacks used to 'keep state' while
 *	the evaluation algorithm converts from infix notation to reverse
 *	polish notation to perform the calculation.
 */
#define EVAL_STACK		8

/*
 *	Define the width of the space set asside for
 *	the hexadecimal machine dump.
 */
#define HEX_DUMP_COLS		15

#endif

/*
 *	EOF
 */
