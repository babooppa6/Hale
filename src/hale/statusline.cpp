#include <QPainter>

#include "configuration.h"

#include "theme.h"
#include "document.h"
#include "documentmodel.h"
#include "statusline.h"

StatusLine::StatusLine(QWidget *parent) :
    QWidget(parent),
    m_dv(NULL),
    m_layout(NULL),
    controller(NULL)
{
    qObject = this;
    m_layout = new QHBoxLayout(this);
    // m_layout->setContentsMargins(0, 4, 0, 4);
    m_layout->setMargin(0);
    m_layout->setSpacing(0);
    setLayout(m_layout);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

void StatusLine::scope(ScopePath *scope)
{
    if (controller) {
        controller->scope(scope, this);
    }
}

void StatusLine::configure(QJsonObject &object)
{
    set(&options, object);
}

//QSize StatusLine::sizeHint() const
//{
//    return QSize(0, 18 + 8);
//}

//QSize StatusLine::minimumSizeHint() const
//{
//    return sizeHint();
//}

void StatusLine::clear()
{
    for (auto segment : m_segments) {
        Configuration::instance->forget(segment);
        m_layout->removeWidget(segment);
        delete segment;
    }
    m_segments.clear();
}

void StatusLine::addSegment(StatusLineSegment *segment, int stretch)
{
    m_layout->addWidget(segment, stretch);
    m_segments.append(segment);
    Configuration::instance->observe(segment);
}

void StatusLine::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.fillRect(rect(), Configuration::theme()->backgroundColor());
    QWidget::paintEvent(e);
}

//
//
//


StatusLineController::StatusLineController(QObject *parent) :
    QObject(parent)
{

}

void StatusLineController::scope(ScopePath *scope, StatusLine *line)
{

}

void StatusLineController::install(StatusLine *)
{

}

void StatusLineController::uninstall()
{

}


//
//
//

StatusLineSegment::StatusLineSegment(StatusLine *line, const QString &name) :
    QWidget(line),
    m_line(line),
    m_elements_width_min(0),
    m_elements_width_opt(0),
    m_dynamic_elements(0)
{
    qObject = this;
    setObjectName(name);
    options.foreground = Qt::white;
    options.background = Qt::black;
}

void StatusLineSegment::configure(QJsonObject &object)
{
    set(&options, object);
    calculateElementsWidth();
    update();
}

void StatusLineSegment::setForeground(QColor foreground)
{
    m_builder_foreground = foreground;
}

void StatusLineSegment::setBackground(QColor background)
{
    m_builder_background = background;
}

int StatusLineSegment::addSeparator(SeparatorStyle style)
{
    Element element;
    element.type = Element::Separator;
    element.separator = (int)style;
    element.width_min = 0;
    element.width_opt = 4;
    element.height = 0;
    element.padding = 4;
    element.foreground = options.foreground;
    element.background = options.background;
    m_elements.append(element);
    calculateElementsWidth();
    return m_elements.size() - 1;
}

int StatusLineSegment::addIcon(const QIcon &icon)
{
    Element element;
    element.type = Element::Icon;
    element.icon = icon;
    element.width_min = 0;
    element.width_opt = 16;
    element.height = 16;
    element.padding = 4;
    element.foreground = options.foreground;
    element.background = options.background;
    m_elements.append(element);
    calculateElementsWidth();
    return m_elements.size() - 1;
}

int StatusLineSegment::addText(const QString &text)
{
    Element element;
    element.type = Element::Text;
    element.font = font();
    element.foreground = options.foreground;
    element.background = options.background;
    m_elements.append(element);

    setText(m_elements.size() - 1, text);

    return m_elements.size() - 1;
}

void StatusLineSegment::setText(int i, const QString &text)
{
    Element &element = m_elements[i];
    element.text = text;
    calculateElementsWidth();
    m_line->update();
}

void StatusLineSegment::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    layout();
}

void StatusLineSegment::paintEvent(QPaintEvent *)
{
    // qDebug() << __FUNCTION__ << geometry();

    QPainter painter(this);
    int content_height = height() - (options.padding.top() + options.padding.bottom());
    qreal height_pad, x = 0;
    QRect rect;

    rect = QRect(0, 0, width(), height());
    painter.fillRect(rect, options.background);
    rect.adjust(0,
                options.padding.top(),
                0,
                -options.padding.bottom());

    x = options.padding.left();

    for (Element &element : m_elements)
    {
        height_pad = (content_height - element.height);
        rect = QRect(x + element.padding,
                         options.padding.top() + (height_pad / 2),
                         element.width,
                         height() - height_pad);

        switch (element.type)
        {
        case Element::Separator: {
            }
            break;
        case Element::Icon: {
            }
            break;
        case Element::Text: {
            painter.setPen(options.foreground);
            painter.setFont(options.font);
            painter.drawText(rect, element.text);
            }
            break;
        }
        x += element.width + element.padding * 2;
    }
}

void StatusLineSegment::calculateElementsWidth()
{
    m_elements_width_min = options.padding.left() + options.padding.right();
    m_elements_width_opt = m_elements_width_min;
    m_elements_height = 0;
    m_dynamic_elements = 0;
    for (Element &element : m_elements)
    {
        switch (element.type)
        {
        case Element::Text: {
            QFontMetrics fm(options.font);
            element.width_min = 0;
            element.width_opt = fm.width(element.text);
            // element.padding = fm.averageCharWidth();
            element.height = fm.lineSpacing();

            m_dynamic_elements++;
        } break;
        }
        m_elements_width_min += element.width_min;
        m_elements_width_opt += element.width_opt + (element.padding * 2);
        m_elements_height = qMax(m_elements_height, element.height + (options.padding.top() + options.padding.bottom()));
    }

    setMinimumWidth(m_elements_width_min);
    setMinimumHeight(m_elements_height);
    if (m_elements_width_min == m_elements_width_opt) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    } else {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    }
    updateGeometry();
}

QSize StatusLineSegment::sizeHint() const
{
    return QSize(m_elements_width_opt, m_elements_height);
}

void StatusLineSegment::layout()
{
    qreal w = width();
    if (w > m_elements_width_opt) {
        // We have more space than we wanted.
        // We will distribute the difference between the items.
        qreal delta = w - m_elements_width_opt;
        qreal delta_per_item = delta / (qreal)m_dynamic_elements;
        for (Element &element : m_elements) {
            if (element.width_min != element.width_opt) {
                element.width = element.width_opt + delta_per_item;
            } else {
                element.width = element.width_opt;
            }
            element.width += options.padding.left() + options.padding.right();
            element.width += element.padding * 2;
        }
    } else {
        // We have less space than is optimal.
        qreal delta_per_item;
        if (w > m_elements_width_min) {
            qreal delta = w - m_elements_width_min;
            delta_per_item = delta / (qreal)m_dynamic_elements;
        } else {
            delta_per_item = 0;
        }
        for (Element &element : m_elements) {
            if (element.width_min != element.width_opt) {
                element.width = element.width_min + delta_per_item;
            } else {
                element.width = element.width_min;
            }
            element.width += options.padding.left() + options.padding.right();
            element.width += element.padding * 2;
        }
    }
}
