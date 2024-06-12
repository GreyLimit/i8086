/*
 *	evaluation
 *	==========
 *
 *	Evaluation of expression captured in a set of tokens.
 */

#ifndef _EVALUATION_H_
#define _EVALUATION_H_

/*
 *	Expression evaluation routine.
 *
 *	In Paramters:
 *
 *		token_record *expr	Linked list of of tokens forming
 *					the expression to be evaluated.
 *
 * 		int len			Number of tokens (maximum) forming
 *					the expression.
 *
 *		boolean negate		True if the expression is preceded
 *					by a negative sign it will not see.
 *
 * 	Out Parameters:
 *
 *		int *consumed		Returns the actual number of tokens
 *					used to form the expression result.
 *
 *		constant_value *v	The evaluated result of the expression.
 *
 *	Returns
 *
 * 		TRUE			Expression successfully calculated.
 *
 *		FALSE			Errors detected in expression.  Consumed
 *					indicates how many tokens were used
 *					before an error was detected.
 */
extern boolean evaluate( token_record *expr, int len, int *consumed, constant_value *v, boolean negate );


#endif


/*
 *	EOF
 */
