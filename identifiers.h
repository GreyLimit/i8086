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
 *	identifiers
 *	===========
 *
 *	System for classification and storage of labels with values.
 */

#ifndef _IDENTIFIERS_H_
#define _IDENTIFIERS_H_

/*
 *	All identifiers will be classified with a
 *	"class" giving its perceived roll.
 */
typedef enum {
	class_unknown	= 0,	/* An identifier which has not be classified. */
	class_label,		/* A location label defined by the programmer. */
	class_const,		/* A numerical value defined by the programmer. */
	class_group,		/* References to a group */
	class_segment		/* and a segment. */
} id_class;

/*
 *	Define a structure used to hold a numerical value
 *	of some sort.
 */
typedef struct {
	integer		value;
	value_scope	scope;
	segment_record	*segment;
} constant_value;

/*
 *	The data structure used to track all identifiers that are
 *	created in the assembly language file.
 */
typedef struct _id_record {
	char			*id;
	id_class		type;
	union {
		constant_value		value;
		segment_record		*segment;
		segment_group		*group;
	} var;
	struct _id_record	*next;
} id_record;

/*
 *	Reset identifier system to "top of input" for the start
 *	of a new pass.
 */
extern void restart_identifiers( void );

/*
 *	Save/Find a label record.
 */
extern id_record *find_label( char *label, boolean definition );

/*
 *	To support verbose-ness and debugging this routine can be called
 *	to produce a dump of the label held by the assembler at this point
 *	in time.
 */
extern void dump_labels( void );

/*
 *	Dump the content of a constant value record, used
 *	as part of the verbose/debug output.
 */
extern void dump_value( constant_value *v );


#endif

/*
 *	EOF
 */
