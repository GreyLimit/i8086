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
 *	segments
 * 	========
 *
 *	Segment definition and management code.
 */

#ifndef _SEGMENTS_H_
#define _SEGMENTS_H_

/*
 *	The underlying architecture of the CPU rests almost entirely
 *	on the use of Segment Registers which hold the upper parts of
 *	complete physical memory address (these are called "paragraph"
 *	numbers).
 *
 * 	"In principle" any assembly instruction that references memory
 *	needs to also specify which segment register should be combined
 *	with the 16-bit address componnent the program is currently
 *	working with.
 *
 *	The Intel "ASM86 Language Reference Manual (Nov 1983) suggests
 *	a cascade of mechanisms that will, in most cases, allow the
 *	programmer and assembler to agree how to implicitly set these
 *	ahead of time.
 *
 *	This mechanism itself requires the assembler and programmer to
 *	adopt the concept of the "current" segment, and the specific
 *	segment register which will used to reference this area in
 *	memory.
 */

/*
 *	The assembler will support the creation and population of
 *	multiple, distinct segments.  Each segment must be assigned
 *	a segment register that the assembler will *assume* is being
 *	used when labels within that segment are referenced.
 *
 * 	Individual segments will be paragraph aligned (16 byte boundaries).
 *
 *	Segments can be grouped together (facilitating their
 *	organisation within memory) into named groups.
 *
 *	Grouped segments will have their segment registers aligned with
 *	the start of the first segment in the group (and so unifying
 *	the offsets used across the group).  Their content will also be
 *	organised sequentially in memory in line with their position
 *	within the group definition.
 */

struct _segment_group;

typedef enum {
	segment_undefined_access	= 000000,		/* Undefined */
	segment_program_code		= 000001,		/* Executable machine code */
	segment_absolute_code		= 000002,		/* Code is position dependent */
	segment_relative_code		= 000004,		/* Code is relocatable */
	segment_priviledged_code	= 000010,		/* Code can contain priviledged instrucitons */
	segment_16_bit			= 000020,		/* Segment target 16-bit code */
	segment_32_bit			= 000040,		/* Segment target 32-bit code */
	segment_8086			= 000100,		/* Segment targets 8086/88 CPU */
	segment_80186			= 000200,		/* Segment targets 80186/88 CPU */
	segment_80286			= 000400,		/* Segment targets 80286 CPU */
	segment_80386			= 001000,		/* Segment targets 80386 CPU */
	segment_static_data		= 002000,		/* Statically defined data */
	segment_variable_data		= 004000,		/* Uninitialised data */
	segment_read_only		= 010000,		/* Is read only */
	segment_read_write		= 020000,		/* Is read/writable */
	segment_no_access		= 040000		/* Cannot be used */
} segment_access;

typedef struct _segment_record {
	char			*name;
	byte			seg_reg;
	segment_access		access;
	boolean			fixed;
	integer			start,
				posn,
				size;
	struct _segment_group	*group;
	struct _segment_record	**link,
				*next;
} segment_record;

typedef struct _segment_group {
	char			*name;
	integer			page;
	segment_record		*segments;
	struct _segment_group	*next;
} segment_group;

/*
 *	Here are the lists of segment and groups.
 */
extern segment_record *loose_segments;
extern segment_record **tail_loose_segments;

extern segment_group *all_groups;
extern segment_group **tail_all_groups;

/*
 *	Define the routine which rationalises all the segments.
 *
 *	Those 'ungrouped' segments are simply re-wound to their start
 *	position.  Those that have been grouped are organised such that
 *	their memory footprints align sequentially with all referenced
 *	segment registers point to the same paragraph page.
 */
extern boolean reset_segments( void );

/*
 *	Convert a segment_access into a human readable buffer
 */
extern int convert_segment_access_to_text( segment_access flags, char *buffer, int max );

/*
 *	Provide a "segment flags" parsing routine.  To avoid loading
 *	down the assembler with even more keywords the flags to a segment
 *	will be parsed from the content of a string value.  This will
 *	give the finer control of the flags format, meanings and names
 *	to this module.
 *
 *	Returns true on success; flags is used to return the result.
 *	Returns false on failure; error is used to point to the source of the problem.
 */
extern boolean parse_segment_access_flags( char *src, int len, segment_access *flags, char **error );

#endif

/*
 *	EOF
 */
