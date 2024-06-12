/*
 *	Definitions
 *	===========
 *
 *	Syntactic simplification definitions.
 */

#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

/*
 *	Define some basic types and values which make the
 *	coding of the assembler clearer.
 */
typedef uint8_t		boolean;
typedef uint8_t		byte;
typedef uint16_t	word;
typedef uint32_t	dword;
typedef int16_t		offset;
typedef uint16_t	address;

/*
 *	To facilitate the assembler handling values with
 *	undeclared scope (type/size/range) we need a integer
 *	type which while being signed can also hold all unsigned
 *	values outlined above.
 */
typedef int32_t		integer;


#define TRUE		(0==0)
#define FALSE		(1==0)
#define ERROR		(-1)

#define USCORE		'_'
#define PERIOD		'.'
#define HASH		'#'
#define DOLLAR		'$'
#define AT		'@'
#define PERCENT		'%'
#define QUOTE		'\''
#define QUOTES		'"'
#define ESCAPE		'\\'
#define NL		'\n'
#define SPACE		' '
#define EOS		'\0'

#define MIN_UBYTE	(0)
#define MAX_UBYTE	(255)

#define MIN_SBYTE	(-128)
#define MAX_SBYTE	(127)

#define MIN_UWORD	(0)
#define MAX_UWORD	(65535)

#define MIN_SWORD	(-32768)
#define MAX_SWORD	(32767)


/*
 *	Wrap up pointers to functions to clarify
 *	the actual syntax of doing so.
 */
#define FUNC(a)		(*(a))

/*
 *	Try and catch miss-understandings about pointers
 */
#define NIL(t)		((t *)(NULL))

/*
 *	Ensure that "bit wise" calculations being used as
 *	boolean True/False values are correctly converted
 *	into a boolean value.
 */
#define BOOL(e)		((e)!=0)

/*
 *	Some binary short hand to clarify some aspects of
 *	encoding data/tables.
 */
#define	B00		0
#define B01		1
#define B10		2
#define B11		3

#define	B000		0
#define B001		1
#define B010		2
#define B011		3
#define B100		4
#define B101		5
#define B110		6
#define B111		7


#endif

/*
 *	EOF
 */
