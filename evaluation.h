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
