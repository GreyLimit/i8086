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
	segment_undefined_access	= 000,		/* Undefined */
	segment_program_code		= 001,		/* Executable machine code */
	segment_program_data		= 002,		/* Program defined data */
	segment_variable_data		= 004,		/* Uninitialised data */
	segment_read_only		= 010,		/* Is read only */
	segment_read_write		= 020,		/* Is read/writable */
	segment_no_access		= 040		/* Cannot be used */
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

#endif

/*
 *	EOF
 */
