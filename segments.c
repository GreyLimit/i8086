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
 *	EOF
 */