#include "precompiled.h"

#include "util.h"
#include "scopepath.h"
#include "document.h"

#include "grammar.h"

GrammarReader::GrammarReader()
{
}

QSharedPointer<Grammar> GrammarReader::loadFromJson(const QString &path)
{
    m_path = QFileInfo(path).absoluteFilePath();

    QString s;
    if (!Util::loadStringFromFile(&s, m_path)) {
        // TODO(cohen) File not found.
        return QSharedPointer<Grammar>();
    }

    QSharedPointer<Grammar> grammar(new Grammar);
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8());
    QJsonObject root = json.object();
    grammar->title = root.value("name").toString();
    grammar->path = m_path;

    for (auto v : root.value("fileTypes").toArray()) {
        grammar->extensions.append(v.toString());
    }

    grammar->rule = new GrammarRule;
    grammar->rule->name = root.value("scopeName").toString();
    readPatterns(grammar->rule, root.value("patterns").toArray());

    return grammar;
}

bool GrammarReader::readPatterns(GrammarRule *rule, const QJsonArray &patterns)
{
    // rule->children.reserve(patterns.size());
    foreach (const QJsonValue &value, patterns)
    {
        const QJsonObject object = value.toObject();
        GrammarRule child_rule;
        if (!readPattern(&child_rule, object)) {
            continue;
        }
        rule->children.push_back(child_rule);
        // rules.emplace(child_rule);
    }
    return true;
}

bool GrammarReader::readPattern(GrammarRule *rule, const QJsonObject &object)
{
    QString error;
    rule->name = object.value("name").toString();
    if (object.contains("begin"))
    {
        rule->is_block = true;
        rule->begin.reset(new OnigRegExp(object.value("begin").toString(), &error));
        if (!rule->begin->isValid()) {
            qWarning().noquote() << __FUNCTION__ << "Invalid `begin` regular expression:" << error;
            qWarning().noquote() << m_path;
            qWarning().noquote() << QJsonDocument(object).toJson();
        }
        rule->end.reset(new OnigRegExp(object.value("end").toString(), &error));
        if (!rule->end->isValid()) {
            qWarning().noquote() << __FUNCTION__ << "Invalid `end` regular expression:" << error;
            qWarning().noquote() << m_path;
            qWarning().noquote() << QJsonDocument(object).toJson();
        }
        if (object.contains("patterns")) {
            readPatterns(rule, object.value("patterns").toArray());
        }
    }
    else if (object.contains("match"))
    {
        rule->is_block = false;
        rule->begin.reset(new OnigRegExp(object.value("match").toString(), &error));
        rule->end.reset();
        if (!rule->begin->isValid()) {
            qWarning().noquote() << __FUNCTION__ << "Invalid `match` regular expression: " << error;
            qWarning().noquote() << m_path;
            qWarning().noquote() << QJsonDocument(object).toJson();
        }
    }
    else
    {
        qWarning().noquote() << __FUNCTION__ << "Unsupported pattern definition.";
        qWarning().noquote() << m_path;
        qWarning().noquote() << QJsonDocument(object).toJson();
        return false;
    }
    return true;
}

void GrammarReader::error(const QString &message)
{
    qWarning() << message;
}


//
//
//


GrammarManager *GrammarManager::instance = NULL;

GrammarManager::GrammarManager(QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(instance == NULL);
    instance = this;

    m_grammar_default.reset(new Grammar);
    m_grammar_default->title = "Plain text";
    m_grammar_default->rule = new GrammarRule;
    m_grammar_default->rule->name = "text.plain";
}

void GrammarManager::observeDocument(Document *document)
{
    Q_ASSERT(!m_documents.contains(document));

    m_documents.append(document);
    connect(document, &Document::pathChanged, this, &GrammarManager::documentPathChanged);
    connect(document, &Document::grammarNameChanged, this, &GrammarManager::documentGrammarNameChanged);
    connect(document, &Document::destroyed, this, &GrammarManager::documentDestroyed);

    document->setGrammar(findGrammarForDocument(document));
}

void GrammarManager::documentPathChanged(const QString &path)
{
    auto document = qobject_cast<Document*>(sender());
    document->setGrammar(findGrammarForDocument(document));
    // TODO: If document has "GrammarName" set, we won't do this?
    // TODO: Find a grammar and set it to the document.
}

void GrammarManager::documentGrammarNameChanged(const QString &name)
{
    auto document = qobject_cast<Document*>(sender());
    document->setGrammar(findGrammar(name));
}

void GrammarManager::documentDestroyed(QObject *object)
{
    // Document is now destroyed, so we only have object.
    auto document = reinterpret_cast<Document*>(object);
    Q_ASSERT(document);
    m_documents.removeOne(document);
}

QSharedPointer<Grammar> GrammarManager::findGrammar(const QString &name)
{
    QSharedPointer<Grammar> best_grammar;
    int score = 0, s;
    for (auto grammar : m_grammars) {
        s = ScopePath::matchName(grammar->rule->name, name);
        if (s > score) {
            score = s;
            best_grammar = grammar;
        }
    }
    return best_grammar;
}

QSharedPointer<Grammar> GrammarManager::findGrammarForDocument(const Document *document)
{
    // TODO: Get the first line and match it against the grammar.
    QFileInfo info(document->path());

    for (auto grammar : m_grammars) {
        if (grammar->extensions.contains(info.suffix())) {
            return grammar;
        }
    }
    return m_grammar_default;
}

void GrammarManager::load(const QString &path)
{
    m_grammars.clear();

    QDirIterator it(path + "/grammars");
    while (it.hasNext())
    {
        it.next();
        QFileInfo info = it.fileInfo();
        if (info.isFile() && info.suffix() == "json") {
            loadGrammar(info.absoluteFilePath());
        }
    }
}

QSharedPointer<Grammar> GrammarManager::loadGrammar(const QString &path)
{
    GrammarReader reader;
    auto grammar = QSharedPointer<Grammar>(reader.loadFromJson(path));
    m_grammars.append(grammar);
    return grammar;
}

void GrammarManager::reloadGrammar(const QString &path)
{
    for (auto grammar : m_grammars) {
        if (grammar->path == path) {
            reloadGrammar(grammar);
            break;
        }
    }
}

void GrammarManager::reloadGrammar(QSharedPointer<Grammar> grammar)
{
    auto replacement = loadGrammar(grammar->path);
    for (auto document : m_documents) {
        if (document->grammar() == grammar) {
            document->setGrammar(replacement);
        }
    }
    m_grammars.removeOne(grammar);
}
