#include <QDebug>
#include <QScopedPointer>
#include <fstream>
#include "share_files_test.h"
#include "share_files.h"

const size_t share_files_test::m_deep = 5;

share_files_test::share_files_test(QObject *parent) :
    QObject(parent)
{
}

void share_files_test::prepare_filesystem()
{
    QString base = QDir::currentPath() + QDir::separator() + "tmp";
    QDir dir;

    QVERIFY(dir.mkdir(base));
    m_files << base;

    for (size_t level = 0; level < m_deep; ++level)
    {
        base += QDir::separator() + QString("dir") + QString::number(level);
        QVERIFY(dir.mkdir(base));
        m_files << base;

        for (size_t branch = 0; branch <= level; ++branch)
        {
            QString branch_path = base + QDir::separator() + QString("branch") + QString::number(branch);
            QVERIFY(dir.mkdir(branch_path));
            m_files << branch_path;

            for (size_t file = 0; file <= branch; ++file)
            {
                QString filepath = branch_path + QDir::separator() + QString("file") + QString::number(file);
                m_files << filepath;
                std::ofstream of;
                of.open(filepath.toLocal8Bit());
                of << "some test data\n";
            }
        }

        for (size_t file = 0; file <= level; ++file)
        {
            QString filepath = base + QDir::separator() + QString("level_file") + QString::number(file);
            m_files << filepath;
            std::ofstream of;
            of.open(filepath.toLocal8Bit());
            of << "some test data for level file\n";
        }
    }

}

void share_files_test::simple_nodes()
{
    QString base = QDir::currentPath() + QDir::separator() + "tmp";
    QScopedPointer<Session> sf(new Session);

    FileNode* p1 = sf->node(base);
    QVERIFY(p1);
    FileNode* p2 = sf->node(base);
    QVERIFY(p2);
    QVERIFY(p1->is_dir());
    QVERIFY(p2->is_dir());

    qDebug() << p1;
    qDebug() << p2;

    QVERIFY(!p1->filename().isEmpty());

    QVERIFY(sf->node(base + QDir::separator() + "dir0/dir1/level_file0"));
    FileNode* p = sf->node(base);
    QVERIFY(p);
    QVERIFY(p->collection_name().isEmpty());    // is not shared - collection name empty
    qDebug() << "fpath: " << p->filepath();
    QCOMPARE(QDir(base), QDir(p->filepath()));
    p1->share(false);
    p2->share(true);
}

void share_files_test::collections_names()
{
    QString base = QDir::currentPath() + QDir::separator() + "tmp";
    Session sf;
    FileNode* p3 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/dir3");
    QVERIFY(p3);
    QCOMPARE(p3->children(), 4);
    QCOMPARE(p3->collection_name(), QString(""));
    p3->share(false);
    QCOMPARE(p3->children(), 4);
    QCOMPARE(p3->collection_name(), QString("dir3"));
    p3->unshare(false);
    QCOMPARE(p3->collection_name(), QString(""));
    FileNode* p2 = sf.node(base + QDir::separator() + "dir0/dir1/dir2");
    QVERIFY(p2);
    p2->share(false);
    p3->share(false);
    QCOMPARE(p2->collection_name(), QString("dir2"));
    QCOMPARE(p3->collection_name(), QString("dir2-dir3"));
    FileNode* p1 = sf.node(base + QDir::separator() + "dir0/dir1");
    QVERIFY(p1);
    p1->share(false);
    QCOMPARE(p2->collection_name(), QString("dir1-dir2"));
    QCOMPARE(p3->collection_name(), QString("dir1-dir2-dir3"));
    QCOMPARE(p2->children(), 3);

    qDebug() << "unshare recursive dir1";
    p1->unshare(true);
    qDebug() << "unshare recursive dir1 was completed";
    QVERIFY(p1->collection_name().isEmpty());
    QVERIFY(p2->collection_name().isEmpty());
    qDebug() << "p3 name " << p3->collection_name();
    QVERIFY(p3->collection_name().isEmpty());
}

void share_files_test::test_save()
{
    QString base = QDir::currentPath() + QDir::separator() + "tmp";
    Session sf;
    FileNode* p2 = sf.node(base + QDir::separator() + "dir0/dir1/dir2");
    QVERIFY(p2);
    p2->share(true);
    FileNode* b20 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/branch0");
    QVERIFY(b20);
    FileNode* b22 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/branch2");
    QVERIFY(b22);
    QVERIFY(b20->is_active());
    QVERIFY(b22->is_active());

    b20->unshare(false);
    b22->unshare(false);

    FileNode* b32 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/dir3/branch3");
    QVERIFY(b32);
    QVERIFY(b32->is_active());
    b32->unshare(false);

    FileNode* f22 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/level_file0");
    FileNode* f23 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/level_file1");
    QVERIFY(f22);
    QVERIFY(f23);
    f22->unshare(false);
    f23->unshare(false);

    sf.produce_collections();
    sf.save();

}

void share_files_test::test_load()
{
    QString base = QDir::currentPath() + QDir::separator() + "tmp";
    Session sf;
    sf.load();
    // check restore states
    FileNode* p2 = sf.node(base + QDir::separator() + "dir0/dir1/dir2");
    QVERIFY(p2);
    QVERIFY(p2->is_active());

    FileNode* f22 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/level_file0");
    FileNode* f23 = sf.node(base + QDir::separator() + "dir0/dir1/dir2/level_file1");

    QVERIFY(f22);
    QVERIFY(!f22->is_active());

    QVERIFY(f23);
    QVERIFY(!f23->is_active());
}

void share_files_test::test_states()
{
    QString base = QDir::currentPath() + QDir::separator() + "tmp";
    Session sf;
    FileNode* p2 = sf.node(base + QDir::separator() + "dir0/dir1/dir2");
    QVERIFY(p2);
    p2->share(true);
    QVERIFY(p2->is_active());

    for (int i = 0; i <= 2; ++i)
    {
        QString num = QString::number(i);
        FileNode* p = sf.node(base + QDir::separator() + "dir0/dir1/dir2/level_file" + num);
        QVERIFY(p);
        QVERIFY(p->is_active());
        p->unshare(false);
    }

    QVERIFY(!p2->is_active());
}

void share_files_test::finalize_filesystem()
{
    while(!m_files.empty())
    {
        int indx = m_files.count() - 1;
        QFileInfo fi(m_files.at(indx));

        if (fi.isFile())
        {
            QFile::remove(m_files.at(indx));
        }
        else
        {
            QDir dir;
            dir.rmdir(m_files.at(indx));
        }

        m_files.removeAt(indx);
    }
}
