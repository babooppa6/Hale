#include <QMessageBox>
#include <QCommandLineParser>

#include "application.h"

#include <oniguruma.h>
#include "grammar.h"

//
//
//

#include <QApplication>
#include <QFileInfo>
#include "util.h"


#define TEST 1
#if 0
#include "hale_test_document.cpp"
#include "hale_test_encoding.cpp"
#if TEST
int main(int argc, char *argv[])
{
    hale_unused(argc);
    hale_unused(argv);

    onig_init();

    hale::test_encoding();
    hale::test_document();
}
#else
int main(int argc, char *argv[])
{
    onig_init();

    Application a(argc, argv);

    // TODO: Move this to hale.lua
    QCommandLineParser cli_parser;
    cli_parser.addPositionalArgument("file", "File to open.");
    cli_parser.process(*a.application());
    const QStringList args = cli_parser.positionalArguments();
    if (args.size() >= 1) {
        // TODO: Check if the other instance is running. If it is, send the command line to the other instance.
        // TODO: Pass the arguments to the application to open the file.
    }

    int ret = a.exec(cli_parser);

    // onig_end();
    return ret;
}
#endif
#endif
