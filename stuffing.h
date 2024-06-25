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
 *	stuffing
 *	========
 *
 *	Macros for handling bits stuffing and extraction (used as part
 *	of the opcode encoding table).
 */
 
#ifndef _STUFFING_H_
#define _STUFFING_H_

/*
 *	Syntactic sugar for bit fiddling.
 */
#define BIT(b)		(1<<(b))
#define BITS(b)		(BIT(b)-1)
#define VALUE(v,b,l)	(((v)&BITS(b))<<(l))
#define EXTRACT(w,b,l)	(((w)>>(l))&BITS(b))


#endif

/*
 *	EOF
 */
