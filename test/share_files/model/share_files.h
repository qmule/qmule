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
    Transfer(const QString& hash, const QString& filepath) : m_hash(hash), m_filepath(filepath){}
    QString m_hash;
    QString m_filepath;

};

/**
  * fake maker
 */
class TransferParamsMaker : public QObject
{
    Q_OBJECT
public:
    TransferParamsMaker()
    {
        m_hash = 123;
    }

    void make_params(const QString& filepath)
    {
        emit parameters_complete(Transfer(QString::number(++m_hash), filepath));
    }
private:
    int m_hash;
signals:
    void parameters_complete(Transfer t);
};

/**
  * SharedFiles schema:
  * RootNode --> DirNode --> DirNode --> DirNode ...
  *                      |     DirNode --> DirNode ...
  *                      |       ...
  *                       -> FileNode
  *                            FileNode
  *                              ...
 */

class Session;
class DirNode;

class FileNode
{
public:
    /**
      * by default node is hasn't share status
      * when node unshared it means all incoming signals like add transfer or params complete will ignore
      * transfer also will deleted
     */
    enum NodeStatus
    {
        ns_none,
        ns_shared,
        ns_unshared
    };

    FileNode(DirNode* parent, const QString& filename);
    virtual ~FileNode();

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);
    virtual void associate_transfer(const QString& hash);
    virtual QString collection_name() const { return QString(""); }
    virtual QString filepath() const;
    virtual bool is_dir() const { return false; }
    virtual bool is_root() const { return false; }
    QString filename() const { return m_filename; }
    NodeStatus status() const { return m_status; }
    bool is_transfer_associated() const { return !m_hash.isEmpty(); }
    QString hash() const { return m_hash; }
    void set_hash(const QString& hash) { m_hash = hash; }
protected:
    DirNode*    m_parent;
    NodeStatus  m_status;
    QString     m_filename;
    QString     m_hash;
};

class DirNode : public FileNode
{
public:
    DirNode(DirNode* parent, const QString& filename);
    virtual ~DirNode();

    virtual bool is_dir() const { return true; }

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);
    virtual void associate_transfer(const QString& hash);

    /**
      * special signal for update names on already shared nodes
      * always recursive
     */
    void update_names();
    QString collection_name() const;
    FileNode* child(const QString& filename);
    void add_node(FileNode* node);
    bool is_populated() const { return m_populated; }

    /**
      * pupulate directory with items no_share status
     */
    void populate();

    /**
      * this method must be called first time in share method for first check
      * and next every time when we have some changes on item
      * after all pending children finish sharing directory can finalize processing
     */
    void check_items();
private:
    bool                        m_populated;    //!< directory was refreshed
    QString                     m_collection;   //!< linked collection file
    QHash<QString, FileNode*>   m_file_children;
    QHash<QString, DirNode*>    m_dir_children;
    friend class Session;
    friend QDebug operator<<(QDebug dbg, const FileNode* node);
};

/**
  * base node of tree, can't be used for any operations exclude add child nodes
 */
class RootNode : public DirNode
{
public:
    RootNode() : DirNode(NULL, "") {}
    virtual bool is_root() const { return true; }
    virtual bool is_dir() const { Q_ASSERT(false);  return true; }
    virtual void share(bool recursive) { Q_UNUSED(recursive); Q_ASSERT(false); }
    virtual void unshare(bool recursive) { Q_UNUSED(recursive); Q_ASSERT(false); }
};

/**
  *
 */
class Session : public QObject
{
    Q_OBJECT
public:
    FileNode* node(const QString& filepath);
    bool associate_transfer(const Transfer& transfer);
    const FileNode* root() const { return &m_root; }
    void deleteTransfer(const QString& hash, bool delete_files);
    void addTransfer(Transfer t);
private:
    RootNode    m_root;
    QHash<QString, FileNode*>   m_files;
    TransferParamsMaker         m_maker;
public slots:
    void on_transfer_added(Transfer);
    void on_transfer_removed(QString hash);
    void on_made_parameters();
};

QDebug operator<<(QDebug dbg, const FileNode* node);

#endif // SHARE_FILE_H
