#ifndef OPTION_H
#define OPTION_H

#include <QFont>
#include <QColor>
#include <QMargins>

#include <QJsonObject>


struct CommonOption
{
    CommonOption() : visible(true) {}
    bool visible;
    QFont font;
    QColor foreground;
    QColor background;
    QMarginsF padding;

    // TODO: This are properties for "selected" state
    QColor selection_foreground;
    QColor selection_background;
};

static void set(bool *flag, QJsonValue value)
{
    if (value.type() != QJsonValue::Bool) {
        return;
    }
    *flag = value.toBool(*flag);
}

static void set(QString *string, QJsonValue value)
{
    if (value.type() != QJsonValue::String) {
        return;
    }
    *string = value.toString(*string);
}

static void set(QFont *font, QJsonValueRef value)
{
    if (value.type() != QJsonValue::Object) {
        return;
    }
    QJsonObject o(value.toObject());
    font->setFamily(o["family"].toString(font->family()));
    font->setPointSizeF(o["size"].toDouble(font->pointSizeF()));
}

static void set(QColor *color, QJsonValue value)
{
    if (value.type() != QJsonValue::String) {
        return;
    }
    color->setNamedColor(value.toString(color->name()));
}

static void set(QMarginsF *margins, QJsonValue value)
{
    if (value.type() != QJsonValue::Object) {
        return;
    }
    QJsonObject o(value.toObject());
    margins->setLeft(o["left"].toDouble(margins->left()));
    margins->setTop(o["top"].toDouble(margins->top()));
    margins->setRight(o["right"].toDouble(margins->right()));
    margins->setBottom(o["bottom"].toDouble(margins->bottom()));
}

static void set(CommonOption *option, QJsonValue value)
{
    if (value.type() != QJsonValue::Object) {
        return;
    }
    QJsonObject o(value.toObject());
    set(&option->visible, o["visible"]);
    set(&option->font, o["font"]);
    set(&option->padding, o["padding"]);
    set(&option->foreground, o["foreground"]);
    set(&option->background, o["background"]);
    set(&option->selection_foreground, o["selection_foreground"]);
    set(&option->selection_background, o["selection_background"]);
}

static void inherit(CommonOption *child, CommonOption *parent)
{
    child->background = parent->background;
    child->foreground = parent->foreground;
    child->font = parent->font;
//    if (value.type() != QJsonValue::Object) {
//        return;
//    }
    // QJsonObject o(value.toObject());
//    *child = *parent;
//    set(&option->visible, o["visible"]);
//    set(&option->font, o["font"]);
//    set(&option->padding, o["padding"]);
//    set(&option->foreground, o["foreground"]);
//    set(&option->background, o["background"]);
}

#include <QWidget>

static void set(QWidget *widget, CommonOption *option)
{
    widget->setFont(option->font);
    auto palette = widget->palette();
    palette.setColor(QPalette::Window, option->background);
    palette.setColor(QPalette::WindowText, option->foreground);
    palette.setColor(QPalette::Base, option->background);
    palette.setColor(QPalette::AlternateBase, option->background);
    palette.setColor(QPalette::Text, option->foreground);
    palette.setColor(QPalette::BrightText, option->foreground);
    palette.setColor(QPalette::Highlight, option->selection_background);
    palette.setColor(QPalette::HighlightedText, option->selection_foreground);
    widget->setPalette(palette);
}

#endif // OPTION_H
