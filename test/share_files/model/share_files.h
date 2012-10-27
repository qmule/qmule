#ifndef SHARE_FILES_H
#define SHARE_FILES_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QDateTime>
#include <QFile>
#include <QIcon>
#include <QFileIconProvider>
#include <QDir>
#include <QBasicTimer>
#include <QDebug>
#include "qfileinfogatherer.h"

// fake transfer
struct Transfer
{
    QString m_hash;
    QString m_filepath;
};

class SharedFiles : public QObject
{
    Q_OBJECT
public:
    struct FileNode
    {
        /**
          * by default node is none
          * when node unshared it means all incoming signals like add transfer or params complete will ignore
          * transfer also will deleted
         */
        enum NodeStatus
        {
            ns_none,
            ns_shared,
            ns_unshared
        };

        FileNode*   m_parent;
        QString     m_filename;
        bool        m_dir;
        NodeStatus  m_status;
        bool        m_populated;
        QString     m_hash;
        QHash<QString, FileNode*> m_file_children;
        QHash<QString, FileNode*> m_dir_children;

        FileNode(FileNode* parent, const QString& filename, bool dir);
        ~FileNode();

        void share(bool recursive);
        void unshare(bool recursive);

        QString collection_name() const;
        QString filepath() const;
        FileNode* file(const QString& filename);
        void update_names();
        bool transfer_associated() const { return !m_hash.isEmpty(); }
        void add_node(FileNode* node);

        /**
          * node children scheduled to share but are not completed yet
         */
        bool children_in_progress() const;
        void populate();

        bool operator==(const FileNode& n) const
        {
            return (//m_filename == n.m_filename
                    //&& m_dir == n.m_dir
                    //&&
                    m_status == n.m_status
                    && m_populated == n.m_populated
                    //&& m_hash == n.m_hash
                    //&& m_file_children == n.m_file_children
                    //&& m_dir_children == n.m_dir_children
                    );
        }
    };

    SharedFiles();
    FileNode* node(const QString& filepath);
    bool associate_transfer(const Transfer& transfer);
    const FileNode* root() const { return &m_root; }
private:
    FileNode    m_root;
    QHash<QString, FileNode*>   m_files;
};

QDebug operator<<(QDebug dbg, const SharedFiles::FileNode* node);

#endif // SHARE_FILE_H
