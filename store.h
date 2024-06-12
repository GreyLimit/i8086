/*
 *	Store
 *	=====
 *
 *	Mechanism for caching blobs of memory to (try to)
 *	optimise overall memory utilisation.
 */

#ifndef _STORE_H_
#define _STORE_H_


extern byte *save_block( byte *block, int len );

extern char *save_string( char *string );

#endif

/*
 *	EOF
 */
