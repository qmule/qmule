#include <QDebug>
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
/*
    QString base = QDir::currentPath() + QDir::separator() + "tmp";
    Session sf;

    FileNode* p1 = sf.node(base);
    QVERIFY(p1);
    FileNode* p2 = sf.node(base);
    QVERIFY(p2);
    QVERIFY(p1->is_dir());
    QVERIFY(p2->is_dir());

    qDebug() << p1;
    qDebug() << p2;

    QVERIFY(!p1->filename().isEmpty());

    QVERIFY(sf.node(base + QDir::separator() + "dir0/dir1/level_file0"));
    FileNode* p = sf.node(base);
    QVERIFY(p);
    QVERIFY(p->collection_name().isEmpty());    // is not shared - collection name empty
    qDebug() << "fpath: " << p->filepath();
    QCOMPARE(QDir(base), QDir(p->filepath()));
    p1->share(false);
    p2->share(true);
    */
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
