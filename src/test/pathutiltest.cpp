#include "pathutil.cpp"
#include "pathutiltest.h"

PathUtilTest::PathUtilTest(QObject *parent) : QObject(parent)
{}


void PathUtilTest::findFirstNonExisting()
{
//    QVERIFY(PathUtil::findFirstNonExisting("C:/NonExisting") == 3);
//    QVERIFY(PathUtil::findFirstNonExisting("C:/NonExisting/..") == 17);
//    QVERIFY(PathUtil::findFirstNonExisting("C:/Program Files/../NonExisting") == 20);
}

static int isRoot(const QString &path)
{
//    if (path == "C:/") {
//        return 2;
//    }

    if (path.toLower() == "c:/users/cohen/") {
        return 2;
    }

    QDir dir(path);
    if (dir.exists(".git") || dir.exists(".svn")) {
        return 2;
    }

    return 1;
}

void PathUtilTest::parseRequest()
{
    PathUtil::RequestInfo request;

    QVERIFY(PathUtil::parseRequest("C:/Windows", "c:/users/hello/yalla", -1, &isRoot, &request) == true);
    QVERIFY(request.root == "C:/users/");
    QVERIFY(request.request == "hello/yalla");
    QVERIFY(request.prefix == "");
    QVERIFY(request.root_type == 1);
    QVERIFY(request.begin = 9);
    QVERIFY(request.end = 20);

    QVERIFY(PathUtil::parseRequest("C:/Windows", "c:/users/hello/../yalla", -1, &isRoot, &request) == true);
    QVERIFY(request.root == "C:/users/");
    QVERIFY(request.request == "yalla");
    QVERIFY(request.prefix == "");
    QVERIFY(request.root_type == 1);
    QVERIFY(request.begin = 18);
    QVERIFY(request.end = 23);

    QVERIFY(PathUtil::parseRequest("C:/Windows", "c:/users/cohen/documents/yalla", -1, &isRoot, &request) == true);
    QVERIFY(request.root == "C:/users/cohen/");
    QVERIFY(request.request == "yalla");
    QVERIFY(request.prefix == "documents/");
    QVERIFY(request.root_type == 2);
    QVERIFY(request.begin = 25);
    QVERIFY(request.end = 30);

    QVERIFY(PathUtil::parseRequest("C:/Windows", "c:/users/hello", -1, &isRoot, &request) == true);
    QVERIFY(request.root == "C:/users/");
    QVERIFY(request.request == "hello");
    QVERIFY(request.prefix == "");
    QVERIFY(request.root_type == 1);
}

