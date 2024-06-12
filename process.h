/*
 *	process
 *	=======
 *
 *	This is where we take a file and attempt to assemble
 *	its content.
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

/*
 *	Traverse an input stream and perform a single pass on the
 *	assembly language contained.
 */
extern boolean process_file( char *source );

#endif

/*
 *	EOF
 */
