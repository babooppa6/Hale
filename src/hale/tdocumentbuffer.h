#ifndef TDOCUMENTBUFFER_H
#define TDOCUMENTBUFFER_H

template<class Ch, class Off, class Len>
class TDocumentBuffer
{
public:

    /// Constructor
    TDocumentBuffer(Len capacity = 1024, Len prealloc = 1024) :
        m_text(0),
        m_gap_start(0),
        m_gap_end(0),
        m_length(0),
        m_prealloc(prealloc),
        m_capacity(capacity)
    {
        m_text =  new Ch[m_capacity];
    }

    /// Destructor
    ~TDocumentBuffer()
    {
        delete [] m_text;
    }

    /// Length of a string (without gap)
    inline Len length() const
    { return m_length; }

    inline Len capacity() const
    { return m_capacity; }

    /// Returns copy of the text
    void getText(Off start, Len length, Ch* buf, bool add_null_char = true) const
    {
        // Ch * result = new Ch[ length + 1];

        // if region is after the gap
        if (start >= m_gap_start)
        {
            // _memcpy_s(buf, textLength, m_text + (start + m_gapEnd - m_gap_start), textLength);
            bufferCopy(
                buf, length, 0,
                m_text, m_capacity, start + /* size of gap */ (m_gap_end - m_gap_start),
                length
                );
        }
        // if region is before gap
        else if ((start + length) <= m_gap_start)
        {
            // _memcpy_s(buf, length, m_text + start, length);
            bufferCopy(
                buf, length, 0,
                m_text, m_capacity, start,
                length
                );
        }
        else
        // if region overlaps the gap
        {
            bufferCopy(
                buf, length, 0,
                m_text, m_capacity, start,
                m_gap_start - start
                );

            bufferCopy(
                buf, length, (m_gap_start - start),
                m_text, m_capacity, m_gap_end,
                start + length - m_gap_start
                );
        }

        if (add_null_char)
        {
            buf[ length ] = L'\0';
        }
    }

    /// Text reference retrieval
    // void getSegment(int start, int length, StringSegment * segment);

    /// Inserts a string to a buffer
    Off insert(Off start, Len length, const Ch *string)
    {
        moveGapStart(start);
        if ((m_gap_end - m_gap_start) < length)
        {
            ensureCapacity(m_length + length + m_prealloc);
            moveGapEnd(start + length + m_prealloc);
        }

        // str.geChs(0, len, text, start);
        bufferCopy(
            m_text, m_capacity, start,
            string, length, 0,
            length
            );

        m_gap_start += length;
        m_length += length;

    #ifdef DEBUG_GAPPED_STRING
        debugGap();
    #endif
        return start + length;
    }

    /// Removes a region from a buffer
    void remove(Off start, Len length)
    {
        moveGapStart(start);
        m_gap_end += length;
        m_length -= length;

    #ifdef DEBUG_GAPPED_STRING
        debugGap();
    #endif
    }

    /// Appends a string
    void append(Len length, const Ch * string)
    {
        insert(m_length, length, string);
    }

    /// Returns pointer to the continuous text. Moves the gap if needed.
    const Ch * getPtr(Off offset)
    {
        // if (offset <= m_gap_start)
        // {
            moveGapStart(m_length);
        // }
        return m_text + offset;
    }


    /// Returns character at position
    Ch & charAt(Off offset)
    {
        // ASSERT(offset < m_length);
        // ASSERT(offset >= 0);
        offset = qBound<Off>(0, offset, m_length - 1);

        if (offset < m_gap_start)
        {
            return m_text[ offset ];
        }

        return m_text[ offset + (m_gap_end - m_gap_start) ];
    }

    void validateChunk(Len i, Len e)
    {
        i -= m_text;
        e -= m_text;

        Q_ASSERT(i >= 0);
        Q_ASSERT(i <= (m_gap_end-m_gap_start) + m_length);
        Q_ASSERT((i < m_gap_start) || (i >= m_gap_end));

        Q_ASSERT(e >= 0);
        Q_ASSERT(e <= (m_gap_end-m_gap_start + m_length));
        Q_ASSERT((e <= m_gap_start) || (e >= m_gap_end));

//        int i = it - m_text;
//        int e = it_end - m_text;

//        Q_ASSERT(i <= e);

//        Q_ASSERT(i >= 0);
//        Q_ASSERT(i <= (m_gap_end-m_gap_start) + m_length);
//        Q_ASSERT((i < m_gap_start) || (i >= m_gap_end));

//        Q_ASSERT(e >= 0);
//        Q_ASSERT(e <= (m_gap_end-m_gap_start + m_length));
//        Q_ASSERT((e <= m_gap_start) || (e >= m_gap_end));
    }


    template<typename Param>
    using ProcessFunction = bool (*)(Ch *it, Ch *end, Param param);

    template<typename Param>
    void chunk(Off from, Off to, ProcessFunction<Param> f, Param param)
    {
        Q_ASSERT(from >= 0);
        Q_ASSERT(from <= to);
        Q_ASSERT(to <= m_length);

        int gap_size = m_gap_end - m_gap_start;
        if (to < m_gap_start) {
            f(m_text + from, m_text + to, param);
        } else if (from >= m_gap_start) {
            f(m_text + gap_size + from, m_text + gap_size + to, param);
        } else {
            if (f(m_text + from, m_text + m_gap_start, param) == false) {
                f(m_text + m_gap_end, m_text + gap_size + to, param);
            }
        }
    }

//    static void calculateColumnInChunk(Ch* it, Ch *it_end, Len tab_size, Len *column)
//    {
//        validateChunk(it, it_end);

//        while (it != it_end) {
//            if (*it == '\t') {
//                *column += tab_size - (*column % tab_size);
//            } else {
//                (*column)++;
//            }
//            it++;
//        }
//    }

    static void calculateIndentationInChunk(Ch *it, Ch *it_end, Len tab_size, Len *indentation)
    {
        validateChunk(it, it_end);

        while (it != it_end) {
            switch (*it) {
            case '\t':
                *indentation += tab_size - (*indentation % tab_size);
                break;
            case ' ':
                *indentation += 1;
                break;
            default:
                return;
            }
            it++;
        }
    }

    int calculateIndentation(Off from, Off to, Len tab_size)
    {
        Q_ASSERT(from >= 0);
        Q_ASSERT(from <= to);
        Q_ASSERT(to < m_length);

        int column = 0;
        int gap_size = m_gap_end - m_gap_start;
        if (to < m_gap_start) {
            calculateColumnInChunk(m_text + from, m_text + to, tab_size, &column);
        } else if (from >= m_gap_start) {
            calculateColumnInChunk(m_text + gap_size + from, m_text + gap_size + to, tab_size, &column);
        } else {
            calculateColumnInChunk(m_text + from, m_text + m_gap_start, tab_size, &column);
            calculateColumnInChunk(m_text + m_gap_end, m_text + gap_size + to, tab_size, &column);
        }
        return column;
    }

protected:
    /// Text buffer
    Ch * m_text;

    /// Offset to #m_text where the gap starts
    Off m_gap_start;

    /// Offset to #m_text where the gap ends
    Off m_gap_end;

    /// Length of a string without gap
    Len m_length;

    /// Amount of Chs allocated in #m_text (length of #m_text)
    Len m_capacity;

    Len m_prealloc;

    /// Moves a m_gap_start to #newStart
    void moveGapStart(Off newStart)
    {
        Off newEnd = m_gap_end + (newStart - m_gap_start);

        if (newStart == m_gap_start)
        {
            // nothing to do
        }
        else if (newStart > m_gap_start)
        {
            // INO_TRACE(_T("GS: Maintenance bufferCopy() (moveGapStart:1)"));
            bufferCopy(
                m_text, m_capacity, m_gap_start,
                m_text, m_capacity, m_gap_end,
                newStart - m_gap_start
                );
        }
        else if (newStart < m_gap_start)
        {
            // INO_TRACE(_T("GS: Maintenance bufferCopy() (moveGapStart:2)"));
            bufferCopy(
                m_text, m_capacity, newEnd,
                m_text, m_capacity, newStart,
                m_gap_start - newStart
                );
        }

        m_gap_start = newStart;
        m_gap_end = newEnd;
    }

    /// Moves a m_gapEnd to newEnd
    void moveGapEnd(Off newEnd)
    {
        // _memmove_s(m_text + newEnd, m_length - m_gap_start, m_text + m_gapEnd, m_length - m_gap_start);
        // INO_TRACE(_T("GS: Maintenance bufferCopy() (moveGapEnd) %d"), m_length - m_gap_start);
        bufferCopy(
            m_text, m_capacity, newEnd,
            m_text, m_capacity, m_gap_end,
            m_length - m_gap_start
            );

        m_gap_end = newEnd;
    }

    /// Expands buffer to specified capacity
    void ensureCapacity(Off capacity)
    {
        if (capacity >= m_capacity)
        {
            // INO_TRACE(_T("GS: Realloc %.2f to %.2f\n"), (float)m_capacity / 1024, (float)capacity / 1024);
            Ch * textN = new Ch[capacity * 2];

            // _memcpy_s(textN, capacity * 2, m_text, m_length + (m_gapEnd - m_gap_start));

            // INO_TRACE(_T("GS: Maintenance bufferCopy() (ensureCapacity)"));
            bufferCopy(
                textN, capacity * 2, 0,
                m_text, m_length + (m_gap_end - m_gap_start), 0,
                m_length + (m_gap_end - m_gap_start)
                );

            delete [] m_text;

            m_text = textN;
            m_capacity = capacity * 2;
        }
    }


    /// Returns length with gap
    inline Len getOccupiedLength()
    { return m_length + (m_gap_end - m_gap_start); }

    /// Sets entire #m_text buffer, resets gap information
    void setText(Ch * text, Len length)
    {
        m_gap_start = 0;
        m_gap_end = 0;
        m_capacity = length;
        m_length = length;

        if (m_text != NULL)
            delete [] m_text;

        m_text = text;
    }

    /// Makes a memcpy or memmove
    /**
        This method is reimplementation of Java's <A HREF="http://java.sun.com/j2se/1.4.2/docs/api/java/lang/System.html#arraycopy(java.lang.Object,%20int,%20java.lang.Object,%20int,%20int)">System::arraycopy</A>
        @param dest pointer to destination wchar buffer
        @param destLength length of destination buffer
        @param destOffset offset to copy data from
        @param source pointer to source wchar buffer
        @param sourceLength length of source buffer
        @param sourceOffset
        @return TRUE if destination buffer was modified, FALSE if otherwise
    */
    static bool bufferCopy(
        Ch * dest, Len destLength, Off destOffset,
        const Ch * source, Len sourceLength, Off sourceOffset,
        Len length
        )
    {

        if ((dest == 0) || (source == 0))
        {
            //INO_TRACE(_T("bufferCopy: dest or source is NULL!\n"));
            return false;
        }

        if ((length <= 0) ||
            (destOffset >= destLength) ||
            (destOffset < 0) ||
            (sourceOffset >= sourceLength) ||
            (sourceOffset < 0) ||

            (
                (destOffset + length) > destLength) ||
                ((sourceOffset + length) > sourceLength)
            )
        {
            //INO_TRACE(_T("bufferCopy: boundary check failed, do(%d), dl(%d), so(%d), sl(%d), l(%d)\n"), destOffset, destLength, sourceOffset, sourceLength, length);
            return false;
        }


        Ch * destStart = dest + destOffset;
        const Ch * sourceStart = source + sourceOffset;

        /*
        // check whether destination is overlapped by source
        if (
            ((sourceStart >= destStart) && (sourceStart <= (destStart + length))) ||
            ((destStart >= sourceStart) && (destStart <= (sourceStart + length)))
            )
        {
            //move
            wmemmove(destStart, sourceStart, length);
        }
        else
        {
            //copy
            wmemcpy(destStart, sourceStart, length);
        }
        */

        memmove(destStart, sourceStart, length*(sizeof(Ch)));

        return true;
    }

    void debugGap()
    {
        Ch * textBeforeGap = new Ch[m_gap_start + 1];
        wmemcpy(textBeforeGap, m_text, m_gap_start);
        textBeforeGap[ m_gap_start ] = L'\0';

        Ch * textAfterGap = new Ch[ getOccupiedLength() - m_gap_end + 1 ];
        wmemcpy(textAfterGap, m_text + m_gap_end, getOccupiedLength() - m_gap_end);
        textAfterGap[ getOccupiedLength() - m_gap_end ] = L'\0';

        // INO_TRACE(_T("\tBEFORE:(%d:%s)\n\tGAP:(%d)\n\tAFTER:(%d:%s)\n"), m_gap_start, textBeforeGap, m_gapEnd - m_gap_start, getOccupiedLength() - m_gapEnd, textAfterGap);

        delete [] textAfterGap;
        delete [] textBeforeGap;
    }
};

////------------------------------------------------------------------------
///// A random-access STL compatible iterator for gapped string containers
///// Used in Document (instead of a GappedString) as it implements locking policy.
//template< class ContainerType >
//class TDocumentBufferIterator
////------------------------------------------------------------------------
//:	public std::iterator< std::random_access_iterator_tag, TCHAR >
//{
//public:
//    /// Iterator type
//    typedef typename TDocumentBufferIterator< ContainerType > myt;
//    /// Container type
//    typedef typename ContainerType container;

//public:
//    //------------------------------------------------------------------------
//    TDocumentBufferIterator()
//    //------------------------------------------------------------------------
//    :	m_container(NULL),
//        m_pos(0)
//    {}

//    //------------------------------------------------------------------------
//    TDocumentBufferIterator(container * string, xint pos)
//    //------------------------------------------------------------------------
//    :	m_container(string),
//        m_pos(pos)
//    {

//    }

//    //------------------------------------------------------------------------
//    TDocumentBufferIterator(const myt & other)
//    //------------------------------------------------------------------------
//    :	m_container(other.m_container),
//        m_pos(other.m_pos)
//    {

//    }

//    //------------------------------------------------------------------------
//    /// Equal
//    bool operator == (const myt & other) const
//    //------------------------------------------------------------------------
//    {
//        // Test for compatible containers
//        INO_ASSERT_DEBUG(m_container != NULL || m_container != other.m_container);
//        return (m_container == other.m_container && m_pos == other.m_pos);
//    }

//    //------------------------------------------------------------------------
//    bool operator != (const myt & other) const
//    //------------------------------------------------------------------------
//    {
//        return (!(*this == other));
//    }

//    //------------------------------------------------------------------------
//    bool operator >(const myt & other) const
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(other.m_container == m_container);
//        return m_pos > other.m_pos;
//    }

//    //------------------------------------------------------------------------
//    bool operator >=(const myt & other) const
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(other.m_container == m_container);
//        return m_pos >= other.m_pos;
//    }

//    //------------------------------------------------------------------------
//    bool operator <(const myt & other) const
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(other.m_container == m_container);
//        return m_pos < other.m_pos;
//    }

//    //------------------------------------------------------------------------
//    bool operator <=(const myt & other) const
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(other.m_container == m_container);
//        return m_pos <= other.m_pos;
//    }

//    //------------------------------------------------------------------------
//    /// Returns a TCHAR at current positions
//    reference operator *() const
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(m_pos >= 0 && m_pos < m_container->getLength());
//        return m_container->charAt(m_pos);
//    }

//    //------------------------------------------------------------------------
//    /// Pre-Increments by one
//    myt & operator++()
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(m_pos < m_container->getLength());
//        m_pos++;
//        return *this;
//    }

//    //------------------------------------------------------------------------
//    /// Post-Increment by one
//    myt operator++(int)
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(m_pos < m_container->getLength());
//        myt it = *this;
//        m_pos++;
//        return *this;
//    }

//    //------------------------------------------------------------------------
//    /// Pre-Decrements by one
//    myt & operator--()
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(m_pos > 0);
//        m_pos--;
//        return *this;
//    }

//    //------------------------------------------------------------------------
//    /// Post-Decrements by one
//    myt operator--(int)
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(m_pos > 0);
//        myt it = *this;
//        m_pos--;
//        return *this;
//    }

//    //------------------------------------------------------------------------
//    /// Increments by n
//    myt & operator+=(difference_type n)
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG((m_pos + n) >= 0 && (m_pos + n) <= m_container->getLength());
//        m_pos = MINMAX< xint >(m_pos + n, 0, m_container->getLength() + 1);
//        return *this;
//    }

//    //------------------------------------------------------------------------
//    /// Increments by n
//    myt & operator-=(difference_type n)
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG((m_pos - n) >= 0 && (m_pos - n) <= m_container->getLength());
//        m_pos = MINMAX< xint >(m_pos - n, 0, m_container->getLength() + 1);
//        return *this;
//    }

//    //------------------------------------------------------------------------
//    /// Increments by n
//    myt operator+(difference_type n) const
//    //------------------------------------------------------------------------
//    {
//        myt it = *this;
//        return (it += n);
//    }

//    //------------------------------------------------------------------------
//    /// Increments by n
//    difference_type operator+(const myt & other)
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(m_container == other.m_container);
//        return m_pos + other.m_pos;
//    }

//    //------------------------------------------------------------------------
//    /// Decrements by n
//    myt operator-(difference_type n) const
//    //------------------------------------------------------------------------
//    {
//        myt it = *this;
//        return (it -= n);
//    }

//    //------------------------------------------------------------------------
//    /// Increments by n
//    difference_type operator-(const myt & other)
//    //------------------------------------------------------------------------
//    {
//        INO_ASSERT_DEBUG(m_container == other.m_container);
//        return m_pos - other.m_pos;
//    }

//    //------------------------------------------------------------------------
//    value_type operator[](difference_type n) const
//    //------------------------------------------------------------------------
//    {
//        return m_container->charAt(m_pos + n);
//    }

//private:
//    /// String we're iterating through
//    container * m_container;
//    /// Current position of the iterator
//    xint m_pos;
//};


#endif // TGAPPEDSTRING_H
