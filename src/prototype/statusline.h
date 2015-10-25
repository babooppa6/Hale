#ifndef DOCUMENTSTATUSLINE_H
#define DOCUMENTSTATUSLINE_H

#include <QWidget>
#include <QHBoxLayout>
#include <QIcon>

#include "configurationobserver.h"
#include "option.h"

class Theme;
class DocumentModel;
class StatusLineSegment;
class StatusLineController;

class StatusLine : public QWidget, public ConfigurationObserver
{
    Q_OBJECT
public:
    explicit StatusLine(QWidget *parent = 0);

    CommonOption options;
    StatusLineController *controller;

    void scope(ScopePath *scope);
    void configure(QJsonObject &object);

    void clear();
    void addSegment(StatusLineSegment *segment, int stretch = 0);
//    QSize sizeHint() const;
//    QSize minimumSizeHint() const;

protected:
    void paintEvent(QPaintEvent *);

signals:

public slots:

private:
    DocumentModel *m_dv;
    QHBoxLayout *m_layout;
    typedef QList<StatusLineSegment *> Segments;
    Segments m_segments;
};

class StatusLineController : public QObject
{
    Q_OBJECT
public:
    StatusLineController(QObject *parent);

    virtual void scope(ScopePath *scope, StatusLine *line);
    virtual void install(StatusLine *line);
    virtual void uninstall();

signals:

public slots:
};

class StatusLineSegment : public QWidget, public ConfigurationObserver
{
    Q_OBJECT
public:
    StatusLineSegment(StatusLine *line, const QString &name);

    CommonOption options;

    enum struct SeparatorStyle {
        OverlapArrowLeft,
        OverlapArrowRight,
        Pipe,
        Space
    };

    void configure(QJsonObject &object);

    void setForeground(QColor foreground);
    void setBackground(QColor background);

    int addSeparator(SeparatorStyle style);
    int addIcon(const QIcon &icon);
    int addText(const QString &text);

    void setText(int i, const QString &text);

    QSize sizeHint() const;

protected:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);

signals:
    void updated();

private:
    StatusLine *m_line;

    struct Element
    {
        Element() :
            width_opt(0),
            width_min(0),
            width(0),
            padding(0)
        {}

        enum Type {
            Separator,
            Icon,
            Text
        } type;

        int separator;
        QIcon icon;
        QString text;
        QFont font;
        QColor foreground;
        QColor background;

        qreal width_opt;
        qreal width_min;
        qreal width;
        qreal height;
        qreal padding;
    };

    QColor m_builder_foreground;
    QColor m_builder_background;

    typedef QList<Element> Elements;
    Elements m_elements;

    qreal m_elements_height;
    qreal m_elements_width_min;
    qreal m_elements_width_opt;
    int m_dynamic_elements;
    void calculateElementsWidth();
    void layout();
};


#endif // DOCUMENTSTATUSLINE_H
