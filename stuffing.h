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
