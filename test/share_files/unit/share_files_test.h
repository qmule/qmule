#ifndef SHARE_FILES_TEST_H
#define SHARE_FILES_TEST_H

#include <QtTest/QTest>
#include <QStringList>
#include <QDebug>

class share_files_test : public QObject
{
    Q_OBJECT
public:
    static const size_t m_deep;
    explicit share_files_test(QObject *parent = 0);
private:
    QStringList m_files;
private slots:
    void prepare_filesystem();
    void simple_nodes();
    void collections_names();
    void test_save();
    void test_load();
    void test_states();
    void finalize_filesystem();
};

#endif // SHARE_FILES_TEST_H
