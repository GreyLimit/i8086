/*
 *	evaluation
 *	==========
 *
 *	Evaluation of expression captured in a set of tokens.
 */

#include "os.h"
#include "includes.h"

/*
 *	Set of routine which implement the basic mathematical operations
 *	required from the expression evaluation mechanism.
 */

/*
 *	INFIX Operators
 *	---------------
 */
static boolean eval_plus( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( b->scope )) {
		if( numeric_scope( a->scope )) {
			a->scope = get_scope(( a->value += b->value ));
			return( TRUE );
		}
		if( address_scope( a->scope )) {
			a->scope = get_scope(( a->value += b->value ));
			return( TRUE );
		}
	}
	log_error( "Cannot ADD incompatible values" );
	return( FALSE );
}
static boolean eval_minus( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value -= b->value ));
		return( TRUE );
	}
	if( address_scope( a->scope ) && address_scope( b->scope )) {
		if( b->segment != a->segment ) {
			log_error( "Address DIFFERENCE from different segments" );
			return( FALSE );
		}
		if( b->value > a->value ) {
			log_error( "Address DIFFERENCE results in invalid value" );
			return( FALSE );
		}
		a->scope = get_scope(( a->value -= b->value ));
		return( FALSE );
	}
	log_error( "Cannot SUBTRACT incompatible values" );
	return( FALSE );
}
static boolean eval_mul( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value *= b->value ));
		return( TRUE );
	}
	log_error( "Cannot MULTIPLY incompatible values" );
	return( FALSE );
}
static boolean eval_div( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value *= b->value ));
		return( TRUE );
	}
	log_error( "Cannot DIVIDE incompatible values" );
	return( FALSE );
}
static boolean eval_and( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value &= b->value ));
		return( TRUE );
	}
	log_error( "Cannot AND incompatible values" );
	return( FALSE );
}
static boolean eval_or( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value |= b->value ));
		return( TRUE );
	}
	log_error( "Cannot OR incompatible values" );
	return( FALSE );
}
static boolean eval_xor( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value ^= b->value ));
		return( TRUE );
	}
	log_error( "Cannot XOR incompatible values" );
	return( FALSE );
}
static boolean eval_shl( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value <<= b->value ));
		return( TRUE );
	}
	log_error( "Cannot LSHIFT incompatible values" );
	return( FALSE );
}
static boolean eval_shr( constant_value *a, constant_value *b ) {
	ASSERT( a != NIL( constant_value ));
	ASSERT( b != NIL( constant_value ));
	if( numeric_scope( a->scope ) && numeric_scope( b->scope )) {
		a->scope = get_scope(( a->value >>= b->value ));
		return( TRUE );
	}
	log_error( "Cannot RSHIFT incompatible values" );
	return( FALSE );
}

/*
 *	PREFIX Operators
 *	----------------
 */
static boolean plus_eval( constant_value *a ) {
	ASSERT( a != NIL( constant_value ));
	if( numeric_scope( a->scope )) return( TRUE );
	log_error( "Cannot POSITIVE non numeric value" );
	return( FALSE );
}
static boolean minus_eval( constant_value *a ) {
	ASSERT( a != NIL( constant_value ));
	if( numeric_scope( a->scope )) {
		a->scope = get_scope(( a->value = -a->value ));
		return( TRUE );
	}
	log_error( "Cannot NEGATE non numeric value" );
	return( FALSE );
}
static boolean not_eval( constant_value *a ) {
	ASSERT( a != NIL( constant_value ));
	if( numeric_scope( a->scope )) {
		a->scope = get_scope(( a->value = ~a->value ));
		return( TRUE );
	}
	log_error( "Cannot COMPLEMENT non numeric value" );
	return( FALSE );

}

/*
 *	C standard precedence levels:
 *
 *	Precedence	Operator	Description				Associativity
 *	----------	--------	-----------				-------------
 *	1		()		Parentheses (function call)		Left-to-Right
 *			[]		Array Subscript (Square Brackets)
 *			.		Dot Operator
 *			->		Structure Pointer Operator
 *			++ --		Postfix increment, decrement
 *
 *	2		++ --		Prefix increment, decrement		Right-to-Left
 *			+ –		Unary plus, minus
 *			! ~		Logical NOT,  Bitwise complement
 *			()		Cast Operator
 *			*		Dereference Operator
 *			&		Address of Operator
 *			sizeof		Determine size in bytes
 *
 *	3		* / %		Multiplication, division, modulus	Left-to-Right
 *
 *	4		+ -		Addition, subtraction			Left-to-Right
 *
 *	5		<< >>		Bitwise shift left, Bitwise shift right	Left-to-Right
 *
 *	6		< <=		Relational less than (or equal)		Left-to-Right
 *			> >=		Relational greater than, (or equal)
 *
 *	7		== !=		Relational is equal to, is not equal to	Left-to-Right
 *
 *	8		&		Bitwise AND				Left-to-Right
 *	9		^		Bitwise exclusive OR			Left-to-Right
 *
 *	10		|		Bitwise inclusive OR			Left-to-Right
 *
 *	11				Logical AND				Left-to-Right
 *
 *	12		||		Logical OR				Left-to-Right
 *
 *	13		?:		Ternary conditional			Right-to-Left
 *
 *	14		=		Assignment				Right-to-Left
 *			+= -=		Addition, subtraction assignment
 *			*= /=		Multiplication, division assignment
 *			%= &=		Modulus, bitwise AND assignment
 *			^= |=		Bitwise exclusive, inclusive OR assignment
 *			<<= >>=		Bitwise shift left, right assignment
 *
 *	15		,		comma (expression separator)		Left-to-Right
 *
 *	Table courtesy of 'https://www.geeksforgeeks.org/operator-precedence-and-associativity-in-c/'
 *
 *	Obviously this assembler will not, and cannot, implement many of the
 *	operators listed above, but those operators which are implemented will
 *	maintain their relative precedence and associativity.
 */
/*
 *	Define a single data structure that captures features and functions of all
 *	expression operators.
 *
 *	The operator table is listed in precedence, though this is not a requirement.
 *	Note that the precedence numbering is the reverse of the C explanation
 *	included above.  Here higher number inndicate higher precedence.
 */
typedef struct {
	component	symbol;
	byte		level;
	boolean		infix,
			nesting;
	boolean		FUNC( eval_prefix )( constant_value *a );
	boolean		FUNC( eval_infix )( constant_value *a, constant_value *b );
} expr_operator;
static expr_operator operator_list[] = {
	{ tok_oparen,		9,	FALSE,	TRUE,	NIL( void ),	NIL( void )	},
	{ tok_plus,		8,	FALSE,	FALSE,	plus_eval,	NIL( void )	},
	{ tok_minus,		8,	FALSE,	FALSE,	minus_eval,	NIL( void )	},
	{ tok_not,		8,	FALSE,	FALSE,	not_eval,	NIL( void )	},
	{ tok_mul,		7,	TRUE,	FALSE,	NIL( void ),	eval_mul	},
	{ tok_div,		7,	TRUE,	FALSE,	NIL( void ),	eval_div	},
	{ tok_plus,		6,	TRUE,	FALSE,	NIL( void ),	eval_plus	},
	{ tok_minus,		6,	TRUE,	FALSE,	NIL( void ),	eval_minus	},
	{ tok_shl,		5,	TRUE,	FALSE,	NIL( void ),	eval_shl	},
	{ tok_shr,		5,	TRUE,	FALSE,	NIL( void ),	eval_shr	},
	{ tok_and,		4,	TRUE,	FALSE,	NIL( void ),	eval_and	},
	{ tok_xor,		3,	TRUE,	FALSE,	NIL( void ),	eval_xor	},
	{ tok_or,		2,	TRUE,	FALSE,	NIL( void ),	eval_or		},
	{ tok_cparen,		1,	TRUE,	TRUE,	NIL( void ),	NIL( void )	},
	{ end_of_line,		0,	FALSE,	FALSE,	NIL( void ),	NIL( void )	}
};
static expr_operator *find_operator( boolean infix, component op ) {
	expr_operator	*p;

	for( p = operator_list; p->symbol != end_of_line; p++ ) if(( p->infix == infix )&&( p->symbol == op )) return( p );
	return( NIL( expr_operator ));
}

/*
 *	Expression evaluation code.  Return the number of tokens
 *	used by the evaluation and places the result in the
 *	constant value at v.  If there was an error then ERROR will
 *	be returned.  Returning 0 is a valid result (no expression
 *	found).
 *
 *	For the Web Page at "https://www.geeksforgeeks.org/
 *	convert-infix-expression-to-postfix-expression/" comes
 *	the following description of converting infix to postfix
 *	notation:
 *
 *	1	Scan the infix expression from left to right.
 *
 *	2	If the scanned character is an operand, put it in
 *		the postfix expression.
 *
 *	3	Otherwise, do the following:
 *
 *		o	If the precedence and associativity of the
 *			scanned operator are greater than the precedence
 *			and associativity of the operator in the stack
 *			[or the stack is empty or the stack contains a
 *			‘(‘ ], then push it in the stack. [‘^‘ operator
 *			is right associative and other operators like
 *			‘+‘,’–‘,’*‘ and ‘/‘ are left-associative].
 *
 *			o	Check especially for a condition when
 *				the operator at the top of the stack
 *				and the scanned operator both are ‘^‘.
 *				In this condition, the precedence of
 *				the scanned operator is higher due to
 *				its right associativity. So it will be
 *				pushed into the operator stack.
 *
 *			o	In all the other cases when the top of
 *				the operator stack is the same as the
 *				scanned operator, then pop the operator
 *				from the stack because of left associativity
 *				due to which the scanned operator has
 *				less precedence.
 *
 *		o	Else, Pop all the operators from the stack which
 *			are greater than or equal to in precedence than
 *			that of the scanned operator.
 *
 *			o	After doing that Push the scanned operator
 *				to the stack. (If you encounter parenthesis
 *				while popping then stop there and push the
 *				scanned operator in the stack.)
 *
 *	4	If the scanned character is a ‘(‘, push it to the stack.
 *
 *	5	If the scanned character is a ‘)’, pop the stack and
 *		output it until a ‘(‘ is encountered, and discard both
 *		the parenthesis.
 *
 *	6	Repeat steps 2-5 until the infix expression is scanned.
 *
 *	7	Once the scanning is over, Pop the stack and add the
 *		operators in the postfix expression until it is not empty.
 *
 *	8	Finally, print the postfix expression.
 */
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
boolean evaluate( token_record *expr, int len, int *consumed, constant_value *v, boolean negate ) {
	constant_value	value_stack[ EVAL_STACK ];
	expr_operator	*op_stack[ EVAL_STACK ];
	int		vtop, otop, used;
	boolean		atom;

	vtop = 0;
	otop = 0;
	used = 0;
	atom = TRUE;

	if( negate ) {
		/*
		 *	We have been told that the expression is preceded
		 *	by a minus symbol.  This only happens in opcode
		 *	arguments where negative displacements are valid.
		 *
		 *	To simplify handling this and ensuring that
		 *	operator precedence is followed we will push a
		 *	single unary MINUS op onto the OP stack which
		 *	will negate the first atomic element in the
		 *	expression, as desired.
		 */
		op_stack[ otop++ ] = find_operator( FALSE, tok_minus );
	}
	while( used < len ) {

		ASSERT( expr != NIL( token_record ));

		if( atom ) {
			/*
			 *	Here we deal with atomic items in an
			 *	expression:  Labels, constants, prefix
			 *	operators and parenthesis sub expressions.
			 */
			switch( expr->id ) {
				case tok_mul: {
					constant_value	*p;

					/*
					 *	An ASTERIX, and an operand/atom, is read as
					 *	the current segment/offset combination.  This
					 *	value is simply placed onto the value stack.
					 */
					if( this_segment == NIL( segment_record )) {
						log_error( "Segment not set for expression" );
						goto bail_out;
					}
					if( vtop >= EVAL_STACK ) {
						log_error( "Expression value stack overflow" );
						goto bail_out;
					}
					p = &( value_stack[ vtop++ ]);
					p->value = this_segment->posn;
					p->scope = scope_address;
					p->segment = this_segment;
					atom = FALSE;
					break;
				}
				case tok_label: {
					ASSERT( expr->var.label != NIL( id_record ));

					/*
					 *	A LABEL is an OPERAND.  It is always
					 *	placed onto the value stack awaiting
					 *	an associated operator.
					 */
					if(( expr->var.label->type != class_unknown )&&( expr->var.label->type != class_const )&&( expr->var.label->type != class_label )) {
						log_error( "Invalid label in expression" );
						goto bail_out;
					}
					if( vtop >= EVAL_STACK ) {
						log_error( "Expression value stack overflow" );
						goto bail_out;
					}
					if( expr->var.label->type == class_unknown ) {
						value_stack[ vtop ].value = 0;
						value_stack[ vtop ].scope = scope_number;
						value_stack[ vtop ].segment = NIL( segment_record );
						vtop++;
					}
					else {
						value_stack[ vtop++ ] = expr->var.label->var.value;
					}
					atom = FALSE;
					break;
				}
				case tok_immediate: {
					/*
					 *	An IMMEDIATE is an OPERAND.  It is always
					 *	placed onto the value stack awaiting
					 *	an associated operator.
					 */
					if( vtop >= EVAL_STACK ) {
						log_error( "Expression value stack overflow" );
						goto bail_out;
					}
					value_stack[ vtop++ ] = expr->var.constant;
					atom = FALSE;
					break;
				}
				default: {
					expr_operator	*op;

					/*
					 *	If not a LABEL or IMMEDIATE then it might
					 *	be a prefix operator or a sub-expression.
					 *
					 *	Whatever is found here it is always stacked
					 *	on the operator stack pending the next
					 *	operand.
					 */
					if(!( op = find_operator( FALSE, expr->id ))) {
						if( used ) {
							log_error( "Atom not found in expression" );
							goto bail_out;
						}
						return( 0 );
					}
					if( otop >= EVAL_STACK ) {
						log_error( "Expression operator stack overflow" );
						goto bail_out;
					}
					op_stack[ otop++ ] = op;
					break;
				}
			}
		}
		else {
			/*
			 *	Here we are "between" atoms and so we
			 *	only have to deal with infix operators
			 *	and the end of a parenthesis sub-expression.
			 */
			expr_operator	*op;

			/*
			 *	We are looking for an infix operator of some sort.
			 *	If we do not find anything we recognise then this
			 *	could simply be the end of the expression (before
			 *	the end of the tokens), so we break out.
			 */
			if(!( op = find_operator( TRUE, expr->id ))) break;
			/*
			 *	If the operator located is the same precedence, or lower,
			 *	than the top of the stack then we drive the evaluation of
			 *	the stored (stacked) state until we can stack the operator.
			 *
			 *	Note that the above action is correct only for left-to-right
			 *	evaulation.  Right-to-left requires that the stack is evaluated
			 *	only while the stack precendence is lower.  There is (currently)
			 *	no need, requirement, for right associative operators.
			 *
			 *	Note that tok_cparen, ')', has the lowest priority where as
			 *	tok_oparen, '(', has the highest priority.
			 */
			while( otop ) {
				expr_operator	*t;

				t = op_stack[ otop-1 ];

				/*
				 *	If the stacked operator has a LOWER
				 *	priority than the operator in hand
				 *	then our work here is done!
				 */
				if( t->level < op->level ) break;
				/*
				 *	If we have uncovered a nesting operator
				 *	then we are also done.
				 */
				if( t->nesting ) {

					ASSERT( t->symbol == tok_oparen );

					break;
				}

				/*
				 *	Evaluate infix or prefix as appropriate.
				 */
				if( t->infix ) {

					ASSERT( vtop >= 2 );
					ASSERT( t->eval_infix != NIL( void ));

					vtop--;
					if(!( FUNC( t->eval_infix )( &( value_stack[ vtop-1 ]), &( value_stack[ vtop ])))) {
						log_error( "Evaluation error in expression" );
						goto bail_out;
					}
				}
				else {
					ASSERT( vtop >= 1 );
					ASSERT( t->eval_prefix != NIL( void ));

					if(!( FUNC( t->eval_prefix )( &( value_stack[ vtop-1 ])))) {
						log_error( "Evaluation error in expression" );
						goto bail_out;
					}
				}

				otop--;
			}
			/*
			 *	Before we stack the operator we have in hand
			 *	we need to handle the whole sub-expression
			 *	feature.
			 */
			if( op->nesting ) {
				ASSERT( op->symbol == tok_cparen );

				/*
				 *	The low priority of the tok_cparen token
				 *	should have left the corresponding tok_oparen
				 *	on the top of operator stack.
				 */
				if( otop && op_stack[ otop-1 ]->nesting ) {

					ASSERT( op_stack[ otop-1 ]->symbol == tok_oparen );

					otop--;
				}
				else {
					log_error( "Missing '(' in expression" );
					goto bail_out;
				}
			}
			else {
				/*
				 *	So we stack the operator we have been keeping in hand.
				 */
				if( otop >= EVAL_STACK ) {
					log_error( "Expression operator stack overflow" );
					goto bail_out;
				}
				op_stack[ otop++ ] = op;
				atom = TRUE;
			}
		}
		/*
		 *	We move onto the next token
		 */
		expr = expr->next;
		used++;
	}
	/*
	 *	Getting here means we may have successfully consumed all the
	 *	tokens in the expression, or, reached a point where the
	 *	expression has naturally came to an end.  Either way we have
	 *	to unwind the content of the stacks to realise the final
	 *	value of the expression.
	 */
	while( otop ) {
		expr_operator	*t;

		/*
		 *	Extract pending operator
		 */
		t = op_stack[ --otop ];

		/*
		 *	If we have uncovered a nesting operator then
		 *	there has been an error in the expression.
		 */
		if( t->nesting ) {
			log_error( "Missing ')' in expression" );
			goto bail_out;
		}
		if( t->infix ) {

			ASSERT( vtop >= 2 );
			ASSERT( t->eval_infix != NIL( void ));

			vtop--;
			if(!( FUNC( t->eval_infix )( &( value_stack[ vtop-1 ]), &( value_stack[ vtop ])))) {
				log_error( "Evaluation error with infix operator" );
				goto bail_out;
			}
		}
		else {
			ASSERT( vtop >= 1 );
			ASSERT( t->eval_prefix != NIL( void ));

			if(!( FUNC( t->eval_prefix )( &( value_stack[ vtop-1 ])))) {
				log_error( "Evaluation error with prefix operator" );
				goto bail_out;
			}
		}
	}
	/*
	 *	There should be ONLY 1 value left on the value stack - anything
	 *	else in an error!  Copy this value to the result space at 'v'.
	 */

	ASSERT( vtop == 1 );

	*v = value_stack[ 0 ];
	/*
	 *	Return how many tokens we used!
	 */
	*consumed = used;
	return( TRUE );

bail_out:
	/*
	 *	Things have gone wrong!
	 */
	*consumed = used;
	return( FALSE );
}


/*
 *	EOF
 */
