/*
 *	process
 *	=======
 *
 *	This is where we take a file and attempt to assemble
 *	its content.
 */

#include "os.h"
#include "includes.h"

/*
 *	Process the token list
 *
 *	Some thought is needed here.  We need to identify that
 *	all lines follow basic syntax:
 *
 *	[{label}[:]?]?	[{opcode}|{directive}]	{arg}[,{arg}]*
 *
 *	Once the optional label has been identified, then the
 *	format of the arguments splits thus:
 *
 *	Directives has simple, comma separated, labels or values
 *	providing context for the directive.
 *
 *	Opcodes will have limited arguments (0, 1 or 2), but these
 *	need to fall into either an offset, a register or an effective
 *	address format.
 */
static boolean process_tokens( token_record *list ) {
	id_record	*label;
	opcode_prefix	prefs;
	modifier	mods;
	component	op_dir;
	token_record	*arg[ MAX_ARG_COUNT ];
	int		args,
			len[ MAX_ARG_COUNT ],
			c;

	/*
	 *	This routine assumes that list is always a valid
	 *	pointer and that the end_of_line token is used
	 *	to note the end of the valid input (rather than
	 *	a NIL pointer).
	 */
	ASSERT( list != NIL( token_record ));

	/*
	 *	Pick out the line label?
	 */
	if( list->id == tok_label ) {
		/*
		 *	save pointer to label record.
		 */
		label = list->var.label;
		list = list->next;

		ASSERT( list != NIL( token_record ));

		/*
		 *	Drop optional colon after label.
		 */
		if( list->id == tok_colon ) {
			list = list->next;

			ASSERT( list != NIL( token_record ));
		}
	}
	else {
		label = NIL( id_record );
	}
	/*
	 *	An empty line is not invalid, its just empty, only need
	 *	to remember to save any label.
	 */
	if(( op_dir = list->id ) == end_of_line ) {
		if( label ) return( set_label_here( label, this_segment ));
		return( TRUE );
	}
	/*
	 *	If we do not have a directive then (assuming that it is
	 *	an operator coming) we need to hunt for any prefix words
	 *	before the opcode.
	 */
	prefs = no_prefix;
	mods = no_modifier;
	if( !is_directive( op_dir )) {
		while( is_prefix( op_dir )) {
			opcode_prefix p = map_prefix( op_dir );

			ASSERT( p != no_prefix );

			if( BOOL( prefs & p )) {
				log_error( "Duplicate opcode prefix" );
				return( FALSE );
			}
			prefs |= p;
			list = list->next;
			op_dir = list->id;
		}
		while( is_modifier( op_dir )) {
			modifier m = map_modifier( op_dir );

			if( BOOL( mods & m )) {
				log_error( "Duplicate opcode modifiers" );
				return( FALSE );
			}
			mods |= m;
			list = list->next;
			op_dir = list->id;
		}
		if( !is_opcode( op_dir )) {
			log_error( "Unrecognised line syntax" );
			return( FALSE );
		}
	}
	list = list->next;

	ASSERT( list != NIL( token_record ));

	/*
	 *	Break up the comma separated arguments.
	 */
	args = 0;
	arg[ args ] = list;
	while( TRUE ) {
		c = 0;
		while(( list->id != tok_comma )&&( list->id != end_of_line )) {
			c++;
			list = list->next;

			ASSERT( list != NIL( token_record ));
		}
		if(( len[ args ] = c )) args++;

		if( list->id == end_of_line ) break;

		ASSERT( list->id == tok_comma );

		list = list->next;

		ASSERT( list != NIL( token_record ));

		if( args == MAX_ARG_COUNT ) {
			log_error( "Maximum argument count exceeded" );
			return( FALSE );
		}
		arg[ args ] = list;

		ASSERT( list != NIL( token_record ));
	}
	/*
	 *	Now handle the broken down line.
	 */
	if( is_opcode( op_dir )) {
		boolean	r;
		if( label ) {
			r = set_label_here( label, this_segment );
		}
		else {
			r = TRUE;
		}
		return( process_opcode( prefs, mods, op_dir, args, arg, len ) && r );
	}
	return( process_directive( label, op_dir, args, arg, len ));
}

/*
 *	Handle the source code on a "per line" basis
 */
static boolean process_line( char *ptr, token_record **tokens ) {
	char		token[ MAX_TOKEN_SIZE+1 ];
	int		l, t;
	integer		value;
	component	tok;
	token_record	*head,
			**tail,
			*rec;
	boolean		errors,
			first;

	/*
	 *	We loop along the input line taking off tokens
	 *	one by one.
	 */
	errors = FALSE;
	head = NIL( token_record );
	tail = &head;
	first = TRUE;
	while( *ptr != EOS ) {
		/*
		 *	Drop any leading space characters
		 */
		while( isspace( *ptr )) ptr++;
		/*
		 *	Now we step through the generic classes of token
		 *	to find what we have to deal with.
		 */
		if(( l = string_constant( QUOTE, ptr, token, MAX_TOKEN_SIZE, &t, &errors ))) {
			/*
			 *	A character constant, error if not just a single character.
			 *
			 *	Remember l is the number of source characters used; t is
			 *	the actual number of data bytes place in the token area.
			 *
			 *	errors will be TRUE if errors were detected in the parsed
			 *	string value.
			 */
			if( errors ) return( FALSE );
			if( t != 1 ) {
				log_error( "Invalid character constant size" );
				return( FALSE );
			}
			/*
			 *	Create record.
			 */
			rec = NEW( token_record );
			rec->id = tok_immediate;
			rec->var.constant.value = token[ 0 ];
			rec->var.constant.scope = scope_ubyte;
			rec->var.constant.segment = NIL( segment_record );
			rec->next = NIL( token_record );
			*tail = rec;
			tail = &( rec->next );
			/*
			 *	Skip character token representation
			 */
			ptr += l;
			first = FALSE;
		}
		else if(( l = string_constant( QUOTES, ptr, token, MAX_TOKEN_SIZE, &t, &errors ))) {
			/*
			 *	A string constant.  The length of the string
			 *	(once parsed) is in t, and will be shorter than
			 *	the return value l.
			 */
			if( errors ) return( FALSE );
			rec = NEW( token_record );
			rec->id = tok_string;
			rec->var.block.len = t;
			rec->var.block.ptr = save_block( (byte *)token, t );
			rec->next = NIL( token_record );
			*tail = rec;
			tail = &( rec->next );
			/*
			 *	Skip string token representation
			 */
			ptr += l;
			first = FALSE;
		}
		else if(( l = match_constant( ptr, &value, &errors ))) {
			/*
			 *	A numeric constant
			 */
			rec = NEW( token_record );
			rec->id = tok_immediate;
			rec->var.constant.value = value;
			rec->var.constant.scope = get_scope( value );
			rec->var.constant.segment = NIL( segment_record );
			rec->next = NIL( token_record );
			*tail = rec;
			tail = &( rec->next );
			/*
			 *	Skip numeric token representation
			 */
			ptr += l;
			first = FALSE;
		}
		else if(( l = match_identifier( ptr ))) {
			/*
			 *	An identifier but possibly a keyword
			 */
			t = find_best_keyword( ptr, &tok );
			if( t == l ) {
				/*
				 *	Definitely a keyword of some sort.
				 */
				rec = NEW( token_record );
				rec->id = tok;
			}
			else {
				/*
				 *	Just a label of some sort.
				 */
				if( l > MAX_TOKEN_SIZE ) {
					strncpy( token, ptr, MAX_TOKEN_SIZE );
					token[ MAX_TOKEN_SIZE ] = EOS;
					log_error_s( "Identifier truncated to", token );
					errors = TRUE;
				}
				else {
					strncpy( token, ptr, l );
					token[ l ] = EOS;
				}
				rec = NEW( token_record );
				rec->id = tok_label;
				rec->var.label = find_label( token, first );
			}
			rec->next = NIL( token_record );
			*tail = rec;
			tail = &( rec->next );
			/*
			 *	Skip numeric token representation
			 */
			ptr += l;
			first = FALSE;
		}
		else if(( l = find_best_symbol( ptr, &tok ))) {
			/*
			 *	Definitely a symbol
			 */
			if( tok == tok_semicolon ) {
				/*
				 *	This is the start of a comment,
				 *	so we artifically truncate the
				 *	line here to force the logic to
				 *	unroll normally.
				 */
				*ptr = EOS;
			}
			else {
				/*
				 *	Save the symbol as a token
				 */
				rec = NEW( token_record );
				rec->id = tok;
				rec->next = NIL( token_record );
				*tail = rec;
				tail = &( rec->next );
				/*
				 *	Skip numeric token representation
				 */
				ptr += l;
				first = FALSE;
			}
		}
		else if( *ptr != EOS ) {
			/*
			 *	Unrecognised token in data stream.
			 */
			log_error_c( "Unrecognised symbol", *ptr++ );
			errors = TRUE;
			first = FALSE;
		}
	}
	/*
	 *	Explicitly mark end of line.
	 */
	rec = NEW( token_record );
	rec->id = end_of_line;
	rec->next = NIL( token_record );
	*tail = rec;
	/*
	 *	Return tokens and error condition (TRUE if line content
	 *	parsed all correct).
	 */
	*tokens = head;
	return( !errors );
}

/*
 *	Traverse an input stream and perform a single pass on the
 *	assembly language contained.
 */
boolean process_file( char *source ) {
	char		buffer[ MAX_LINE_SIZE+1 ];
	token_record	*tokens;
	boolean	ret;

	/*
	 *	Point the streaming input to the file.
	 */
	if( !include_file( source )) return( FALSE );
	/*
	 *	While we succesfully read a line of data from the input
	 *	source we process it.
	 */
	ret = TRUE;
	while( next_line( buffer, MAX_LINE_SIZE )) {
		if( process_line( buffer, &tokens )) {
			if( !process_tokens( tokens )) {
				log_error( "Interpretation error" );
				ret = FALSE;
			}
		}
		else {
			log_error( "Tokenisation error" );
			ret = FALSE;
		}
		delete_tokens( tokens );
	}
	return( ret );
}

/*
 *	EOF
 */
