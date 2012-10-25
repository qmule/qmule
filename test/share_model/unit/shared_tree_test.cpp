#include "shared_tree_test.h"
#include "model/shared_files_tree.h"

shared_tree_test::shared_tree_test(QObject *parent) :
    QObject(parent)
{
}

void shared_tree_test::test1()
{
    shared_files_tree sft;
    shared_files_tree::QFileSystemNode* node = sft.node("/home/apavlov", true);
    QVERIFY(node != sft.rootNode());
    QTest::qSleep(2000);
}
