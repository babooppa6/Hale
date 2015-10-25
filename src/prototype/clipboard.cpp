#include "application.h"
#include "clipboard.h"

Clipboard *Clipboard::instance = NULL;

Clipboard::Clipboard(QObject *parent) : QObject(parent)
{
    Q_ASSERT(instance == NULL);
    instance = this;
}

QClipboard *Clipboard::clipboard()
{
    return Application::instance()->application()->clipboard();
}

void Clipboard::copy(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }

    // TODO: Put to ring.
    clipboard()->setText(text, QClipboard::Clipboard);
}

QString Clipboard::paste()
{
    return clipboard()->text(QClipboard::Clipboard);
}
