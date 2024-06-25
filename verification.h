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
 *	Verification
 *	============
 *
 *	Definitions to enable/disable code segments for
 *	process debugging and algorithm confirmation.
 */

#ifndef _VERIFICATION_H_
#define _VERIFICATION_H_

/*
 *	Define DEBUG here (or on the GCC compiler
 *	command line) to include all sorts of code and
 *	algorithm checking.  This will make the final
 *	program larger, slower and more verbose.
 */
/* #define DEBUG */

/*
 *	Define VERIFICATION here (or on the GCC compiler
 *	command line) to include additionaal command line
 *	options allowing examiniation of instruciton table.
 */
/* #define VERIFICATION */

/*
 *	Some basic code assurance elements
 */
#ifdef DEBUG

#define ASSERT(v)	do{if(!(v)){log_error_i("Assert failed",__LINE__);abort();}}while(0)
#define ABORT(m)	do{log_error_s("Assembler Abort",(m));abort();}while(1)
#define DPRINT(a)	printf a
#define DCODE(c)	c

#else

#define ASSERT(v)
#define ABORT(m)	abort()
#define DPRINT(a)
#define DCODE(c)

#endif

#ifdef VERIFICATION

#define VPRINT(a)	printf a
#define VCODE(c)	c

#else

#define VPRINT(a)
#define VCODE(c)

#endif

#endif

/*
 *	EOF
 */
