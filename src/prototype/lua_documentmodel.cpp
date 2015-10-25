#include <lua.hpp>
#include "luautil.h"

#include "application.h"
#include "panel.h"
#include "document.h"
#include "documentmodel.h"

#include "lua_documentmodel.h"

//
//
//

static int
luadoc_new(lua_State *L)
{
    auto document = Application::instance()->createDocumentModel();
    if (document == NULL) {
        luaL_error(L, "doc initialization failed");
    }

    return tlua::push<DocumentModel*>(L, document);
}

static int
luadoc_load(lua_State *L)
{
    QString path(luautil_checkstring(L, 1));

    auto document = Application::instance()->loadDocumentModel(path);
    if (document == NULL) {
        luaL_error(L, LERROR_F_NOT_FOUND, path.toUtf8().constData());
    }

    return tlua::push<DocumentModel*>(L, document);
}

static int
luadoc_save(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);

    if (lua_gettop(L) == 1) {
        return tlua::pushv<bool>(L, Application::instance()->documentManager()->saveDocument(document->document()));
    }

    auto path(tlua::read<QString>(L, 2));
    return tlua::pushv<bool>(L, Application::instance()->documentManager()->saveDocument(document->document(), path));
}


static int
luadoc_path(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    return tlua::push<QString>(L, document->document()->path());
}

static int
luadoc_hasPath(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    return tlua::pushv<bool>(L, !document->document()->path().isEmpty());
}

//
//
//

static int
luadoc_edit(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit == NULL) {
        document->lua_edit = document->edit();
    }
    return 0;
}

static int
luadoc_commit(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit != NULL) {
        document->lua_edit.reset();
    }
    return 0;
}

static int
luadoc_revert(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit != NULL) {
        document->lua_edit->revert();
        document->lua_edit.reset();
    }
    return 0;
}

//
// Undo
//

static int
luadoc_undo(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    document->undo();
    return 0;
}

static int
luadoc_redo(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    document->redo();
    return 0;
}

//
// Clipboard
//

static int
luadoc_copy(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    document->copy();
    return 0;
}

static int
luadoc_cut(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit) {
        document->cut(document->lua_edit);
    } else {
        luaL_error(L, "Edit not permitted.");
    }
    return 0;
}

static int
luadoc_paste(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit) {
        document->paste(document->lua_edit);
    } else {
        luaL_error(L, "Edit not permitted.");
    }
    return 0;
}

//
// Cursor
//

static int
luadoc_addCursor(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    int b1 = luaL_checkinteger(L, 2);
    int o1 = luaL_checkinteger(L, 3);
    int b2, o2;
    if (lua_gettop(L) >= 5) {
        b2 = luaL_checkinteger(L, 4);
        o2 = luaL_checkinteger(L, 5);
    } else {
        b2 = b1;
        o2 = o1;
    }
    document->addCursor(DocumentRange(DocumentPosition(b1, o1), DocumentPosition(b2, o2)));
    return 0;
}

static int
luadoc_moveCursor(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    int command = luaL_checkinteger(L, 2);
    // TODO: Flags should be optional.
    int flags = luaL_checkinteger(L, 3);

    document->moveCursor((DocumentModel::MoveCommand)command, (DocumentModel::MoveFlags)flags);

    return 0;
}

static int
luadoc_cursor(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    lua_pushinteger(L, document->cursor()->position().block + 1);
    lua_pushinteger(L, document->cursor()->position().position + 1);
    lua_pushinteger(L, document->cursor()->anchor().block + 1);
    lua_pushinteger(L, document->cursor()->anchor().position + 1);

    return 4;
}

static int
luadoc_cursorOffset(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    int p = document->document()->blockPositionToOffset(document->cursor()->position()) + 1;
    int a = document->document()->blockPositionToOffset(document->cursor()->anchor()) + 1;

    lua_pushinteger(L, p);
    lua_pushinteger(L, a);

    return 2;
}

//
// Text
//

static int
luadoc_insertText(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit) {
        QString text(luautil_checkstring(L, 2));
        document->insertText(document->lua_edit, text);
    } else {
        luaL_error(L, "Edit not permitted.");
    }
    return 0;
}

static int
luadoc_insertIndentation(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (!document->lua_edit) {
        luaL_error(L, "Edit not permitted.");
        return 0;
    }

    document->insertIndentation(document->lua_edit);

    return 0;
}

static int
luadoc_insertNewLine(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (!document->lua_edit) {
        luaL_error(L, "Edit not permitted.");
        return 0;
    }

    document->insertNewLine(document->lua_edit);

    return 0;
}

static int
luadoc_removeText(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit) {
        int command = luaL_checkinteger(L, 2);
        document->removeText(document->lua_edit, (DocumentModel::RemoveCommand)command);
    } else {
        luaL_error(L, "Edit not permitted.");
    }
    return 0;
}

static int
luadoc_clear(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    if (document->lua_edit) {
        document->clear(document->lua_edit);
    } else {
        luaL_error(L, "Edit not permitted.");
    }
    return 0;
}

static int
luadoc_text(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    QByteArray text(document->text().toUtf8());
    return tlua::push<QByteArray>(L, text);
}

static int
luadoc_getBlockText(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    int block = luaL_checkinteger(L, 2);
    QByteArray text(document->blockText(block).toUtf8());
    lua_pushstring(L, text.constData());
    return 1;
}

static int
luadoc_beginCompletion(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    int begin = tlua::read<int>(L, 2) - 1;
    int end = tlua::read<int>(L, 3) - 1;

    document->beginCompletionTest(begin, end);

    return 0;
}

static int
luadoc_endCompletion(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    document->endCompletion();
    return 0;
}

static int
luadoc_nextCompletion(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    document->nextCompletion();
    return 0;
}

static int
luadoc_previousCompletion(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    document->previousCompletion();
    return 0;
}

static int
luadoc_confirmCompletion(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    document->confirmCompletion();
    return 0;
}


//
// Model
//

static int
luadoc_panel(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    auto panel = Application::instance()->findPanelForModel(document);
    return tlua::push<Panel*>(L, panel);
}

static int
luadoc_activate(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    auto panel = Application::instance()->findPanelForModel(document);
    Q_ASSERT(panel);
    panel->setModel(document);
    return 0;
}


static int
luadoc_close(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    Application::instance()->closeModel(document);
    return 0;
}

static int
luadoc_setIndentationMode(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    int mode = tlua::read<int>(L, 2);
    if (mode != (int)IndentationMode::Spaces && mode != (int)IndentationMode::Tabs) {
        luaL_error(L, "invalid indentation mode");
    }
    document->document()->setIndentationMode((IndentationMode)mode);
    return 0;
}

static int
luadoc_indentationMode(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    return tlua::pushv<int>(L, (int)document->document()->indentation_mode);
}

static int
luadoc_setIndentationSize(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    int size = tlua::read<int>(L, 2);
    document->document()->setIndentationSize(size);
    return 0;
}

static int
luadoc_indentationSize(lua_State *L)
{
    auto document = tlua::read<DocumentModel*>(L, 1);
    return tlua::push<int>(L, document->document()->indentation_size);
}


int
lua_documentmodel(lua_State *L)
{
    static const struct luaL_Reg
    f[] = {
    {"new", 			luadoc_new},
    {"load", 			luadoc_load},

    {"save", luadoc_save},
    {"path", luadoc_path},
    {"hasPath", luadoc_hasPath},

    {"copy", luadoc_copy},
    {"cut", luadoc_cut},
    {"paste", luadoc_paste},

    {"edit", luadoc_edit},
    {"commit", luadoc_commit},
    {"revert", luadoc_revert},
    {"undo", luadoc_undo},
    {"redo", luadoc_redo},

    {"addCursor", luadoc_addCursor},
    {"moveCursor", luadoc_moveCursor},
    {"cursor", luadoc_cursor},
    {"cursorOffset", luadoc_cursorOffset},
    {"removeText", luadoc_removeText},
    {"insertText", luadoc_insertText},
    {"insertIndentation", luadoc_insertIndentation},
    {"insertNewLine", luadoc_insertNewLine},
    {"clear", luadoc_clear},
    {"getBlockText", luadoc_getBlockText},

    {"beginCompletion", luadoc_beginCompletion},
    {"endCompletion", luadoc_endCompletion},
    {"nextCompletion", luadoc_nextCompletion},
    {"previousCompletion", luadoc_previousCompletion},
    {"confirmCompletion", luadoc_confirmCompletion},

    {"indentationMode", luadoc_indentationMode},
    {"setIndentationMode", luadoc_setIndentationMode},
    {"indentationSize", luadoc_indentationSize},
    {"setIndentationSize", luadoc_setIndentationSize},

    // Model
    {"panel", luadoc_panel},
    {"activate", luadoc_activate},
    {"close", luadoc_close},
    {NULL, NULL}
    };

    tlua::table metatable(tlua::table::createMetatable(L, tlua::value<DocumentModel*>::className()));
    metatable.set("__index", metatable);
    metatable.set(f);

    metatable.setv<int>("MOVE_CHARACTER_NEXT",      (int)DocumentModel::Move_CharacterNext);
    metatable.setv<int>("MOVE_CHARACTER_PREVIOUS",  (int)DocumentModel::Move_CharacterPrevious);
    metatable.setv<int>("MOVE_WORD_NEXT",           (int)DocumentModel::Move_WordNext);
    metatable.setv<int>("MOVE_WORD_PREVIOUS",       (int)DocumentModel::Move_WordPrevious);
    metatable.setv<int>("MOVE_LINE_NEXT",           (int)DocumentModel::Move_LineNext);
    metatable.setv<int>("MOVE_LINE_PREVIOUS",       (int)DocumentModel::Move_LinePrevious);
    metatable.setv<int>("MOVE_LINE_BEGIN",          (int)DocumentModel::Move_LineBegin);
    metatable.setv<int>("MOVE_LINE_END",            (int)DocumentModel::Move_LineEnd);
    metatable.setv<int>("MOVE_BLOCK_BEGIN",         (int)DocumentModel::Move_BlockBegin);
    metatable.setv<int>("MOVE_BLOCK_END",           (int)DocumentModel::Move_BlockEnd);
    metatable.setv<int>("MOVE_PAGE_NEXT",           (int)DocumentModel::Move_PageNext);
    metatable.setv<int>("MOVE_PAGE_PREVIOUS",       (int)DocumentModel::Move_PagePrevious);

    metatable.setv<int>("REMOVE_CHARACTER_BACKWARD",    (int)DocumentModel::Remove_CharacterBackward);
    metatable.setv<int>("REMOVE_CHARACTER_FORWARD",     (int)DocumentModel::Remove_CharacterForward);

    metatable.setv<int>("MOVE_FLAG_SELECT",             (int)DocumentModel::MoveFlag_Select);
    metatable.setv<int>("MOVE_FLAG_VERTICAL_ANCHOR",    (int)DocumentModel::MoveFlag_VerticalAnchor);

    metatable.setv<int>("INDENTATION_MODE_SPACES",      (int)IndentationMode::Spaces);
    metatable.setv<int>("INDENTATION_MODE_TABS",        (int)IndentationMode::Tabs);

    return metatable.push(L);
}
