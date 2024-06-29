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

#include "os.h"
#include "includes.h"

/*
 *	Here are the lists of segment and groups.
 */
segment_record *loose_segments = NIL( segment_record );
segment_record **tail_loose_segments = &loose_segments;

segment_group *all_groups = NIL( segment_group );
segment_group **tail_all_groups = &all_groups;

/*
 *	Define the routine which rationalises all the segments.
 *
 *	Those 'ungrouped' segments are simply re-wound to their start
 *	position.  Those that have been grouped are organised such that
 *	their memory footprints align sequentially with all referenced
 *	segment registers point to the same paragraph page.
 */
boolean reset_segments( void ) {
	segment_record	*seg;
	segment_group	*grp;

	for( seg = loose_segments; seg; seg = seg->next ) {
		seg->size = seg->posn - seg->start;
		seg->posn = seg->start;
	}
	for( grp = all_groups; grp; grp = grp->next ) {
		integer		where;

		where = 0;
		for( seg = grp->segments; seg; seg = seg->next ) {
			seg->size = seg->posn - seg->start;
			if( seg->fixed ) {
				seg->posn = where = seg->start;
			}
			else {
				seg->posn = seg->start = where;
			}
			where += seg->size;
		}
	}
	return( TRUE );
}

/*
 *	Routines which are related to the definition and display of the
 *	set of flags which can be associated with a defined segment.
 */

/*
 *	The full set of possible segment flags and their relationships
 */
typedef struct {
	segment_access	flag;
	char		*name;
	segment_access	implies,
			excludes;
} flag_name;
static flag_name flag_names[] = {
	{ segment_program_code,		"code",		segment_undefined_access,	segment_static_data|segment_variable_data						},
	{ segment_absolute_code,	"absolute",	segment_program_code,		segment_static_data|segment_variable_data|segment_relative_code				},
	{ segment_relative_code,	"relative",	segment_program_code,		segment_static_data|segment_variable_data|segment_absolute_code				},
	{ segment_priviledged_code,	"priv",		segment_program_code,		segment_static_data|segment_variable_data						},
	{ segment_16_bit,		"16-bit",	segment_program_code,		segment_static_data|segment_variable_data|segment_32_bit				},
	{ segment_32_bit,		"32-bit",	segment_program_code,		segment_static_data|segment_variable_data|segment_16_bit				},
	{ segment_8086,			"8086",		segment_program_code,		segment_static_data|segment_variable_data|segment_80186|segment_80286|segment_80386	},
	{ segment_80186,		"80186",	segment_program_code,		segment_static_data|segment_variable_data|segment_8086|segment_80286|segment_80386	},
	{ segment_80286,		"80286",	segment_program_code,		segment_static_data|segment_variable_data|segment_8086|segment_80186|segment_80386	},
	{ segment_80386,		"80386",	segment_program_code,		segment_static_data|segment_variable_data|segment_8086|segment_80186|segment_80286	},
	{ segment_static_data,		"data",		segment_undefined_access,	segment_program_code|segment_variable_data						},
	{ segment_variable_data,	"bss",		segment_undefined_access,	segment_program_code|segment_static_data						},
	{ segment_read_only,		"read-only",	segment_undefined_access,	segment_read_write									},
	{ segment_read_write,		"read-write",	segment_undefined_access,	segment_read_only									},
	{ segment_no_access,		"no-access",	segment_undefined_access,	segment_program_code|segment_static_data|segment_variable_data				},
	{ segment_undefined_access,	NIL( char ),	segment_undefined_access,	segment_undefined_access								}
};


int convert_segment_access_to_text( segment_access flags, char *buffer, int max ) {
	flag_name	*fp;
	int		left,
			len;

	left = max;
	for( fp = flag_names; fp->flag != segment_undefined_access; fp++ ) {
		if( BOOL( flags & fp->flag )) {
			if(( len = strlen( fp->name )) <= left ) {
				memcpy( buffer, fp->name, len );
				left -= len;
				buffer += len;
			}
		}
	}
	return( max - left );
}

/*
 *	flag separator search routine
 */
static char *next_separator( char *src, int len ) {
	while( len-- ) {
		if(( *src == COMMA )||( *src == BAR )) return( src );
		src++;
	}
	return( NIL( char ));
}

/*
 *	add (potentially) a flag to the segment access
 */
static boolean add_segment_flag( char *src, segment_access *result ) {
	flag_name	*fp;

	ASSERT( src != NIL( char ));
	ASSERT( result != NIL( segment_access ));

	for( fp = flag_names; fp->flag != segment_undefined_access; fp++ ) {
		if( match_all( src, fp->name )) {
			if( BOOL( *result & fp->excludes )) break;
			*result |= ( fp->flag | fp->implies );
			return( TRUE );
		}
	}
	/*
	 *	Getting here indicates an error.
	 */
	return( FALSE );
}

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
boolean parse_segment_access_flags( char *src, int len, segment_access *flags, char **error ) {
	segment_access	result;
	char		*tail,
			sep;

	ASSERT( src != NIL( char ));
	ASSERT( error != NIL( char * ));
	ASSERT( flags != NIL( segment_access ));

	result = segment_undefined_access;
	while(( tail = next_separator( src, len )) != NIL( char )) {
		sep = *tail;
		*tail = EOS;
		if( !add_segment_flag( src, &result )) {
			*error = src;
			return( FALSE );
		}
		len -= strlen( src ) + 1;
		*tail++ = sep;
		src = tail;
	}
	if( len ) {
		if( !add_segment_flag( src, &result )) {
			*error = src;
			return( FALSE );
		}
	}
	*flags = result;
	return( TRUE );
}


/*
 *	EOF
 */
