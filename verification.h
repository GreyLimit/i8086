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
