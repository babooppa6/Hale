#pragma once
#ifndef __INO_SCOPE_SELECTOR_H__
#define __INO_SCOPE_SELECTOR_H__

namespace ino
{
	class ScopePath;

	//------------------------------------------------------------------------
	class ScopeSelector
	//------------------------------------------------------------------------
	{
	public:

	public:
		ScopeSelector( const ScopeSelector & other );
		virtual ~ScopeSelector();

		/// Returns size of internal buffer
		size_t getSize() const;
		/// Returns size of tokens
		size_t getTokensSize() const;
		/// Returns count of tokens
		U32 getCount() const;
		/// Returns ptr to tokens
		const U32 * getTokens() const;

		/// Assignment operator
		ScopeSelector & operator=( const ScopeSelector & other );

	private:
		friend class ScopeSelectorFactory;
		friend class ScopeMatcher;
		friend class TestScopeSelector;

		struct DescendantElement
		{
			/// CRC of the whole element
			U32 crc;
			/// Count of names in element
			U32 count;
		};

		struct Descendant
		{
			/// T_Descendant
			U32 type;
			/// Specificity
			U32 specificity;
			/// Count of elements following
			U32 count;

			/// First element
			DescendantElement first_element;
		};

		enum TokenType
		{
			/// Operators are ordered by precedence.

			//------------------------------------------------------------------------
			/// Tokens used by ScopeSelectorFactory parser only

			__T_Unknown				= 0x00,	// Must be zero
			__T_Eos					= 0x01,
			__T_DescendantElement	= 0x02,
			__T_GroupBegin			= 0x03,
			__T_GroupEnd			= 0x04,

			//------------------------------------------------------------------------
			/// Shared tokens (used by factory and selector)

			/// ORH operator (not used in m_v)
			__T_OrH			= 0x11,

			/// AND operator
			T_And			= 0x12,
			/// OR operator
			T_Or			= 0x13,
			/// AND_NOT operator
			T_AndNot		= 0x14,

			/// Descendant
			/// Count Match Match Match ... 
			T_Descendant	= 0x100,
		};

		ScopeSelector();

		/// Ptr to storage
		std::vector< U32 > m_v;
	};
}

#endif // __IT_SCOPE_SELECTOR_H__