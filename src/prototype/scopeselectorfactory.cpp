#include "ino/base/defaults.h"

#include "ino/scope/ScopeSelectorFactory.h"
#include "ino/scope/ScopeSelector.h"
#include "ino/text/search/StringScanner.h"

#include "ino/base/Crc.h"

using namespace ino;

/************************************************************************/
/* ScopeSelectorFactory                                                 */
/************************************************************************/
//------------------------------------------------------------------------
ScopeSelector * ScopeSelectorFactory::create( const tstring & string )
//------------------------------------------------------------------------
{
	return create( string.c_str(), string.length() );
}


//------------------------------------------------------------------------
ScopeSelector * ScopeSelectorFactory::create( const TCHAR * string, xint length )
//------------------------------------------------------------------------
{
	ScopeSelector * s = new ScopeSelector();

	try
	{
		parse( string, length, s->m_v );
	}
	catch( SyntaxException& e )
	{
		delete s;
		throw e;
	}

	return s;
}

#define SKIP_SPACE { while( (context.p < context.end) && (*context.p) <= ' ' ) context.p++; }

//------------------------------------------------------------------------
bool ScopeSelectorFactory::parse( const TCHAR * string, xint length, std::vector< U32 > & out )
//------------------------------------------------------------------------
{
	ParseContext context;
	context.begin = string;
	context.end = string + length;
	context.p = context.begin;
	context.level = 0;
	context.out = &out;

	U32 token_count = 0;
	size_t ix_token_count = context.write( 0 );

	ParseFunc f = &ScopeSelectorFactory::pBegin;

	SKIP_SPACE;

	ScopeSelector::TokenType token = ScopeSelector::__T_Unknown;
	while( true )
	{
		token = f( context );

		if( token == ScopeSelector::__T_Eos )
		{
			break;
		}

		switch( token )
		{
		case ScopeSelector::T_Descendant:
			token_count++;
			f = &ScopeSelectorFactory::pAfterOperand;
			break;

		case ScopeSelector::T_Or:
		case ScopeSelector::__T_OrH:
		case ScopeSelector::T_And:
		case ScopeSelector::T_AndNot:
			token_count++;
			write( token, context );
			f = &ScopeSelectorFactory::pAfterBinaryOperator;
			break;

		case ScopeSelector::__T_GroupBegin:
			context.stack.push( ScopeSelector::__T_GroupBegin );
			context.level++;
			f = &ScopeSelectorFactory::pBegin;
			break;

		case ScopeSelector::__T_GroupEnd:
			// Until the token at the top of the stack is a left parenthesis,...
			while( context.stack.top() != ScopeSelector::__T_GroupBegin )
			{
				// ...pop operators off the stack onto the output queue.
				context.writeOperator( context.stack.top() );
				context.stack.pop();
				if( context.stack.empty() )
				{
					// Unbalanced grouping
					throw SyntaxException( (tformat(_T("Unbalanced parenthesis at %d.")) % context.position()).str().c_str() );
				}
			}

			if( context.stack.empty() )
			{
				// Unbalanced grouping
				throw SyntaxException( (tformat(_T("Unbalanced parenthesis at %d.")) % context.position()).str().c_str() );
			}

			// Pop the left parenthesis from the stack, but not onto the output queue.
			context.stack.pop();
			context.level--;
			f = &ScopeSelectorFactory::pAfterOperand;
			break;

		case ScopeSelector::__T_Unknown:
			throw SyntaxException( (tformat(_T("Unexpected token at %d.")) % context.position()).str().c_str() );
		}

		SKIP_SPACE;
	}

	while( !context.stack.empty() )
	{
		U32 top = context.stack.top();
		if( top == ScopeSelector::__T_GroupBegin )
		{
			// Unbalanced grouping
			throw SyntaxException( (tformat(_T("Unbalanced parenthesis at %d.")) % context.position()).str().c_str() );
		}
		context.writeOperator( top );
		context.stack.pop();
	}

	context.writeAt( ix_token_count, token_count );

	return true;
}

//------------------------------------------------------------------------
void ScopeSelectorFactory::write( U32 o1, ParseContext & context )
//------------------------------------------------------------------------
{
	U32 o2;

	// While there is an operator token, o2, at the top of the stack
	while( !context.stack.empty() )
	{
		o2 = context.stack.top();
		// Either o1 is left-associative and its precedence is less than or equal to that of o2,
		// or o1 is right-associative and its precedence is less than that of o2.
		// We do not have equal precedence, so don't have to handle associatives
		if( o1 < o2 )
		{
			// pop o2 off the stack, onto the output queue
			context.stack.pop();
			context.writeOperator( o2 );
		}
		else
		{
			break;
		}
	}

	// Push o1 onto the stack
	context.stack.push( o1 );
}

#define TEST_BEGIN TCHAR c = *context.p
#define TEST_EOS if( context.p >= context.end ) { return (context.level == 0) ? ScopeSelector::__T_Eos : ScopeSelector::__T_Unknown; }
#define TEST_EOS_ERROR if( context.p >= context.end ) { return ScopeSelector::__T_Unknown; }
#define TEST_NOT if( c == '-' ) { context.p++; return ScopeSelector::T_AndNot; }
#define TEST_AND if( c == '&' ) { context.p++; return ScopeSelector::T_And; }
#define TEST_ORH if( c == ',' ) { context.p++; return ScopeSelector::__T_OrH; }
#define TEST_OR  if( c == '|' ) { context.p++; return ScopeSelector::T_Or; }
#define TEST_GB  if( c == '(' ) { context.p++; return ScopeSelector::__T_GroupBegin; }
#define TEST_GE  if( c == ')' && context.level > 0 ) { context.p++; return ScopeSelector::__T_GroupEnd; }
#define TEST_DESCENDANT { ScopeSelector::TokenType t = pDescendant( context ); if( t ) { return t; } }

//------------------------------------------------------------------------
ScopeSelector::TokenType ScopeSelectorFactory::pBegin( ParseContext & context )
//------------------------------------------------------------------------
{
	TEST_BEGIN;
	TEST_EOS;

	TEST_GB;
	TEST_GE;
	TEST_DESCENDANT;

	return ScopeSelector::__T_Unknown;
}

//------------------------------------------------------------------------
ScopeSelector::TokenType ScopeSelectorFactory::pAfterBinaryOperator( ParseContext & context )
//------------------------------------------------------------------------
{
	TEST_BEGIN;
	TEST_EOS_ERROR;

	TEST_GB;
	TEST_DESCENDANT;

	return ScopeSelector::__T_Unknown;
}

//------------------------------------------------------------------------
ScopeSelector::TokenType ScopeSelectorFactory::pAfterOperand( ParseContext & context )
//------------------------------------------------------------------------
{
	TEST_BEGIN;
	TEST_EOS;
	TEST_AND;
	TEST_OR;
	TEST_ORH;
	TEST_NOT;
	TEST_GE;

	return ScopeSelector::__T_Unknown;
}

#define IS_OPERATOR(c) (c == '&' || c == '|' || c == ',' || c == '-' || c == '(' || c == ')')

//------------------------------------------------------------------------
ScopeSelector::TokenType ScopeSelectorFactory::pDescendant( ParseContext & context )
//------------------------------------------------------------------------
{
	context.write( ScopeSelector::T_Descendant );

	size_t ix_specificity = context.write( 0 );
	size_t ix_element_count = context.write( 0 );
	// size_t ix_element_end = context.write( 0 );

	U32 specificity = 0;
	U32 element_count = 0;
	U32 element_end = context.out->size();
	
	bool stopped_with_operator = false;
	while( context.p < context.end && pDescendantElement( context, specificity, stopped_with_operator ) )
	{
		element_count++;

		if( stopped_with_operator )
			break;

		if( context.p >= context.end )
		{
			break;
		}
	}

	if( element_count > 0 )
	{
		context.writeAt( ix_specificity, specificity );
		context.writeAt( ix_element_count, element_count );
		// context.writeAt( ix_element_end, context.out->size() - element_end );
		return ScopeSelector::T_Descendant;
	}

	return ScopeSelector::__T_Unknown;
}

//------------------------------------------------------------------------
ScopeSelector::TokenType ScopeSelectorFactory::pDescendantElement( ParseContext & context, U32 & o_specificity, bool & stopped_with_operator )
//------------------------------------------------------------------------
{
	TCHAR c;

	U32 element_name_count = 0;
	const TCHAR * element_begin = context.p;
	const TCHAR * element_end = context.p;
	const TCHAR * element_name_begin = element_begin;

	// ToDo: Dot should not be at the end of the element name `text.html.`

	bool is_stop = false;
	bool is_end = false;
	while( context.p <= context.end )
	{
		if( context.p == context.end )
		{
			c = 0;
			is_stop = true;
			is_end = true;
		}
		else
		{
			c = *context.p;
			if( c != '.' )
			{
				stopped_with_operator = IS_OPERATOR(c);
				is_stop = c <= ' ' || stopped_with_operator;
			}
			else is_stop = false;
			is_end = false;
		}

		if( c == '.' || is_stop )
		{
			// Element match cannot start, end with a dot or have double dot (empty element)
			if( (( c == '.' ) || is_end) && element_name_begin == context.p )
				return ScopeSelector::__T_Unknown;

			if( context.p > element_begin )
			{
				// Write it to the output
				// context.write( CRC::String32Len( element_begin, context.p - element_begin ) );
				// Increase number of names
				element_name_count++;
			}

			if( c == '.' )
			{
				// Advance after the dot
				context.p++;
				element_name_begin = context.p;
				continue;
			}

			element_end = context.p;

			if( !is_end && c <= ' ' )
			{
				SKIP_SPACE;
			}

			break;
		}
		else
		{
			context.p++;
		}
	}

	if( element_name_count )
	{
		context.write( CRC::string32Len( element_begin, element_end - element_begin ) );
		o_specificity += element_name_count;
		context.write( element_name_count );
		return ScopeSelector::__T_DescendantElement;
	}

	return ScopeSelector::__T_Unknown;
}