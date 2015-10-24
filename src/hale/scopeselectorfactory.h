#pragma once
#ifndef __INO_SCOPE_SELECTOR_FACTORY_H__
#define __INO_SCOPE_SELECTOR_FACTORY_H__

#include "ino/scope/ScopeSelector.h"

namespace ino
{
	class StringScanner;

	//------------------------------------------------------------------------
	class ScopeSelectorFactory
	//------------------------------------------------------------------------
	{
	public:

		/// Parses and returns a selector
		static ScopeSelector * create( const tstring & string );
		/// Parses and returns a selector
		static ScopeSelector * create( const TCHAR * string, xint length );

	private:
		ScopeSelectorFactory() {};
		~ScopeSelectorFactory() {};

		/// Parses the string
		static bool parse( const TCHAR * string, xint length, std::vector< U32 > & out );

		struct ParseContext
		{
			const TCHAR * begin;
			const TCHAR * end;
			const TCHAR * p;
			xint level;

			std::vector< U32 > * out;
			std::stack< U32 > stack;

			size_t write( U32 v )
			{
				out->push_back( v );
				return out->size() - 1;
			}

			size_t writeOperator( U32 o )
			{
				if( o == ScopeSelector::__T_OrH )
				{
					return write( ScopeSelector::T_Or );
				}
				return write( o );
			}

			void writeAt( size_t pos, U32 v )
			{
				(*out)[ pos ] = v;
			}

			xint position()
			{
				return p - begin;
			}
		};

		/// Writes operator
		static void write( U32 o1, ParseContext & context );

		typedef ScopeSelector::TokenType (*ParseFunc)( ParseContext & context );

		static ScopeSelector::TokenType pBegin( ParseContext & context );
		static ScopeSelector::TokenType pAfterBinaryOperator( ParseContext & context );
		static ScopeSelector::TokenType pAfterOperand( ParseContext & context );
		static ScopeSelector::TokenType pDescendant( ParseContext & context );
		static ScopeSelector::TokenType pDescendantElement( ParseContext & context, U32 & o_specificity, bool & stopped_with_operator );
	};
}

#endif // __INO_SCOPE_SELECTOR_FACTORY_H__