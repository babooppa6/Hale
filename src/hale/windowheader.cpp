#include "precompiled.h"

#include "windowheader.h"

WindowHeader::WindowHeader(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

QSize WindowHeader::sizeHint() const
{
    return QSize(0, 18);
}

