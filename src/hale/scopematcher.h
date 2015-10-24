#pragma once
#ifndef __INO_SCOPE_MATCHER_H__
#define __INO_SCOPE_MATCHER_H__

#include "ino/scope/ScopeSelector.h"
#include "ino/scope/ScopePath.h"

namespace ino
{
	//------------------------------------------------------------------------
	class ScopeMatcher
	//------------------------------------------------------------------------
	{
	public:		
		/// Describes match
		struct Result
		{
			/// Rank type
			typedef U64 Rank;

			/// Empty rank matched (this value will be set if the selector was empty)
			static const U64 EMPTY_RANK = 1;

			Result()
			:	rank( 0 ), descendant( NULL )
			{}

			Result( Rank _rank, const ScopeSelector::Descendant * _descendant )
			:	rank( _rank ), descendant( _descendant )
			{}

			Result( const Result & other )
			{
				memcpy( this, const_cast< Result* >( &other ), sizeof(Result) );
			}

			/// Rank (how good the descendant matched the path)
			Rank rank;
			/// Pointer to the descendant
			const ScopeSelector::Descendant * descendant;
		};

		/// Matching flags
		enum Flags
		{
			MF_None = 0,

			/// Returns true only if a top element of the path has been matched
			MF_Top	= 0x01
		};

		/// Elements type optionally returned from matchDescendant
		typedef std::vector< const ScopePath::Element * > MatchResultElements;

	public:
		/// Constructor
		ScopeMatcher( const ScopePath & path );
		~ScopeMatcher();

		/// Matches selector against the path. Returns empty result if selector is empty.
		bool match( const ScopeSelector & selector, Result & o_result, xint flags = 0 );
		/// Matches selector against the path. Returns empty result if selector is empty or NULL.
		bool match( const ScopeSelector * selector, Result & o_result, xint flags = 0 );
		/// Matches descendant against the path
		bool matchDescendant( const ScopeSelector::Descendant & descendant, Result::Rank & o_rank, MatchResultElements * o_elements = NULL, xint flags = 0 );

	private:
		typedef std::stack< Result > MatchStack;

		/// Path against which we're matching
		const ScopePath & m_path;
		/// Intermediate stack of results
		MatchStack m_stack;
	};
}

#endif __IT_SCOPE_MATCHER_H__