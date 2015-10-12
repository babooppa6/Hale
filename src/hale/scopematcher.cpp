#include "ino/base/defaults.h"
#include "ino/scope/ScopeMatcher.h"

using namespace ino;

/************************************************************************/
/* ScopeMatcher                                                         */
/************************************************************************/
//------------------------------------------------------------------------
ScopeMatcher::ScopeMatcher( const ScopePath & path )
//------------------------------------------------------------------------
:	m_path( path )
{
	m_path.compile();
}

//------------------------------------------------------------------------
ScopeMatcher::~ScopeMatcher()
//------------------------------------------------------------------------
{
}

#define POP_MATCHES m1 = &m_stack.top(); m_stack.pop(); m2 = &m_stack.top(); m_stack.pop()

//------------------------------------------------------------------------
bool ScopeMatcher::match( const ScopeSelector * selector, Result & o_result, xint flags /* = 0 */ )
//------------------------------------------------------------------------
{
	if( selector == NULL )
	{
		o_result.rank = Result::EMPTY_RANK;
		o_result.descendant = NULL;
		return (flags & MF_Top) ? false : true;
	}
	return match( *selector, o_result, flags );
}

//------------------------------------------------------------------------
bool ScopeMatcher::match( const ScopeSelector & selector, Result & o_result, xint flags /* = 0 */ )
//------------------------------------------------------------------------
{
	if( selector.getCount() == 0 )
	{
		o_result.rank = Result::EMPTY_RANK;
		o_result.descendant = NULL;
		return (flags & MF_Top) ? false : true;
	}

	const U32 * tokens = selector.getTokens();

	Result * m1;
	Result * m2;
	Result::Rank rank;

	const ScopeSelector::Descendant * d;

	U32 i = 0;

	while( i < (selector.getSize()-1) )
	{
		switch( tokens[i] )
		{
		case ScopeSelector::T_Or:
			POP_MATCHES;

			if( m1->descendant || m2->descendant )
			{
				if( m1->descendant && m2->descendant )
				{
					// ToDo: Compare specificities?
					m_stack.push( m1->rank > m2->rank ? (*m1) : (*m2) );
				}
				else if( !m1->descendant && m2->descendant )
				{
					m_stack.push( *m2 );
				}
				else
				{
					m_stack.push( *m1 );
				}
			}
			else
			{
				/// Push NULL
				m_stack.push( Result() );
			}
			break;

		case ScopeSelector::T_And:
			POP_MATCHES;

			if( m1->descendant && m2->descendant )
			{
				// ToDo: Compare specificities?
				m_stack.push( m1->rank > m2->rank ? (*m1) : (*m2) );
			}
			else
			{
				/// Push NULL
				m_stack.push( Result() );
			}
			break;

		case ScopeSelector::T_AndNot:
			POP_MATCHES;

			// m1 is the second (as is on the top of the stack)
			if( !m1->descendant && m2->descendant )
			{
				m_stack.push( *m2 );
			}
			else
			{
				/// Push NULL
				m_stack.push( Result( 0, NULL ) );
			}
			break;

		case ScopeSelector::T_Descendant:
			/// Get the descendant from the tokens
			d = reinterpret_cast< const ScopeSelector::Descendant * >( tokens + i );
			/// Match descendant, and get the rank
			if( matchDescendant( *d, rank, NULL, flags ) )
			{
				// Store the match result to the stack
				m_stack.push( Result( rank, d ) );
			}
			else
			{
				/// Push NULL
				m_stack.push( Result( 0, NULL ) );
			}

			// Advance

			// Skip descendant header
			i += (sizeof(ScopeSelector::Descendant) >> 2);
			// Skip descendant elements
			i += (sizeof(ScopeSelector::DescendantElement) >> 2) * (d->count-1);

			// Jump to loop start
			continue;

		default:
			INO_ASSERT( 0 && "Invalid token." );
			return false;
		}

		i++;
	}

	o_result = m_stack.top();

	while( !m_stack.empty() )
		m_stack.pop();

	return o_result.descendant != NULL;
}

/*

	Matching descendants
	--------------------

	Path: text.html meta.tag.any.html string.quoted.double
		

		In memory, path is ordered from last to first

		x i crc      name

		0 quantifier = 0
		  1 9ebeb2a9 string
		  2 e613353f string.quoted
		  3 b7037ee0 string.quoted.double

		1 quantifier = 3 (0+3)
		  1 d7f21435 meta
		  2 7f86aef7 meta.tag
		  3 9f2f24a3 meta.tag.any
		  4 5cf10ba6 meta.tag.any.html

		2 quantifier = 10 (0+3 + 3+4)
		  1 3b8ba7c7 text
		  2 dbb54faf text.html


	Descendant: string.quoted
		CRC		= e613353f
		count	= 2

	1) We start with the first element of the path: string.quoted.double:

		0 quantifier = 0
  		  1 9ebeb2a9 string
		  2 e613353f string.quoted
		  3 b7037ee0 string.quoted.double

	2) And last element in the descendant 
	3) Descendant element has count of names 2, that means, we have to compare it to path element's name at index 1 (zero-based; count - 1):
		
		if( descendant_element->crc == path_element->crcs[ descendant_element->count - 1 ] )
		{
			// MATCHED!
		}
*/

//------------------------------------------------------------------------
bool ScopeMatcher::matchDescendant( const ScopeSelector::Descendant & descendant, Result::Rank & o_rank, MatchResultElements * o_elements /* = NULL */, xint flags /* = 0 */ )
//------------------------------------------------------------------------
{
	if( descendant.count > m_path.size() )
	{
		// If descendant has more elements than the path, we return NULL immediately
		return false;
	}

	/// Descendant's element
	const ScopeSelector::DescendantElement * element_descendant = (&descendant.first_element) + (descendant.count - 1);
	/// Path's element
	ScopePath::iterator element_path = m_path.begin();

	xint path_i = m_path.size() - 1;
	xint descendant_i = descendant.count - 1;

	o_rank = 0;

	while( path_i >= 0 )
	{
		if
		(
			// If matched path-element has count greater or equal to match-element
			( (*element_path)->count >= element_descendant->count )
			&&
			// And crcs match
			( m_path.getElementCrc( *element_path, element_descendant->count - 1 ) == element_descendant->crc )
		)
		{
			// Add quantifier and count of elements
			o_rank += (*element_path)->quantifier + element_descendant->count;

			// Add result to elements
			if( o_elements )
			{
				o_elements->push_back( *element_path );
			}

			// Move to previous element in the descendant
			element_descendant--;
			descendant_i--;
			
			// If we have no more elements in descendant
			if( descendant_i == -1 )
			{
				// Return the match
				return true;
			}
		}
		else if( (flags & MF_Top) && path_i == (m_path.size() - 1) )
		{
			// Top match requested, but not matched
			break;
		}

		element_path++;
		path_i--;

		// If there is less elements in path as in the descendant
		if( path_i < descendant_i )
		{
			// Cannot match
			break;
		}
	}

	return false;
}