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
