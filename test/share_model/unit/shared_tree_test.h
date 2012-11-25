#ifndef SHARED_TREE_TEST_H
#define SHARED_TREE_TEST_H

#include <QtTest/QTest>

class shared_tree_test : public QObject
{
    Q_OBJECT
public:
    explicit shared_tree_test(QObject *parent = 0);    
private slots:
    void test1();
};

#endif // SHARED_TREE_TEST_H
