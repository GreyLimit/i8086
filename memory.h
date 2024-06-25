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
 *	Memory
 *	======
 *
 *	Definitions providing a common memory allocation
 *	wrapping.
 */

#ifndef _MEMORY_H_
#define _MEMORY_H_

/*
 *	Wrap up the memory allocation
 */
#define NEW(t)			((t *)malloc(sizeof(t)))
#define NEW_ARRAY(t,n)		((t *)malloc(sizeof(t)*(n)))

#define FREE(p)			free(p)

#define STACK(t)		((t *)alloca(sizeof(t)))
#define STACK_ARRAY(t,n)	((t *)alloca(sizeof(t)*(n)))


#endif

/*
 *	EOF
 */
