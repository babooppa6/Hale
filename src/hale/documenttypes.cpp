#include "document.h"
#include "documentmodel.h"

#include "documenttypes.h"

void _DocumentEdit::commit()
{
    Q_ASSERT(m_undo || m_document);
    if (!m_undo && (m_document)) {
        m_document->commit(this);
        m_document = NULL;
        m_view = NULL;
    }
}

void _DocumentEdit::revert()
{
    Q_ASSERT(m_undo || m_document);
    if (!m_undo && (m_document)) {
        m_document->revert(this);
        m_document = NULL;
        m_view = NULL;
    }
}

QDebug operator<<(QDebug dbg, const DocumentPosition &p)
{
    dbg.nospace() << "[" << p.block << ", " << p.position << "]";

    return dbg.space();
}

QDebug operator<<(QDebug dbg, const DocumentRange &r)
{
    dbg.nospace() << "(" << r.first << ", " << r.second << ")";

    return dbg.space();
}

//
//
//

struct LineEndingImpl {
    int length;
    QString string;
};

LineEndingImpl impl_unknown = { 0, "" };
LineEndingImpl impl_cr = { 1, "\r" };
LineEndingImpl impl_lf = { 1, "\n" };
LineEndingImpl impl_crlf = { 2, "\r\n" };

LineEnding LineEnding::UNKNOWN(&impl_unknown);
LineEnding LineEnding::CR(&impl_cr);
LineEnding LineEnding::LF(&impl_lf);
LineEnding LineEnding::CRLF(&impl_crlf);

LineEnding::LineEnding() :
    m_impl(&impl_unknown)
{
}

LineEnding::LineEnding(LineEndingImpl *impl) :
    m_impl(impl)
{
}

LineEnding::LineEnding(const LineEnding &other) :
    m_impl(other.m_impl)
{
}

int LineEnding::length()
{
    return m_impl->length;
}
