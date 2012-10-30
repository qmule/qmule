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

// fake add transfer params
struct add_transfer_params
{
    QString m_filepath;
    QString m_hash;
};

/**
  * SharedFiles schema:
  * RootNode --> DirNode --> DirNode --> DirNode ...
  *                      |     DirNode --> DirNode ...
  *                      |       ...
  *                       -> FileNode
  *                            FileNode
  *                              ...
  *
  * Scenarious:
  * 1. share file:   if file is not shared it calls params maker for params, on params signal generate transfer,
  *                  associate it and informs parent by check_items call
  * 2. unshare file: when file shared it removes transfer or call cancel params to params maker, set state to unshared
  *                  and informs parent.
  *
  * 3. share dir:    doesn't check self state, for each file execute share and next execute check_items
  *                  if call recursive it will delegates to child directories. In any case for child directories
  *                  will be execute update_names method for refresh collection names down
  * 4. unshare dir:  check self state, firstly drop self transfer/params_maker and status to avoid multiple calls check_items,
  *                  next unshare each child file
  *                  For recursive call delegate unshare to child dirs. For not recursive call executes update_names down
  *                  to refresh collection names.
  *
 */

class Session;
class DirNode;

class FileNode
{
public:

    FileNode(DirNode* parent, const QString& filename, Session* session);
    virtual ~FileNode();

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);
    virtual bool has_metadata() const;
    virtual bool has_transfer() const;

    virtual void set_metadata(const add_transfer_params& atp, int error);

    virtual QString collection_name() const { return QString(""); }
    virtual QString filepath() const;
    virtual bool is_dir() const { return false; }
    virtual bool is_root() const { return false; }
    bool is_active() const { return m_active; }

    QString filename() const { return m_filename; }    
    DirNode*    m_parent;    
protected:
    bool        m_active;
    QString     m_filename;
    add_transfer_params* m_atp;
    int             m_error;
    Session*    m_session;
    QString     m_hash;
    friend class Session;
};

class DirNode : public FileNode
{
public:
    DirNode(DirNode* parent, const QString& filename, Session* session);
    virtual ~DirNode();

    virtual bool is_dir() const { return true; }

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);    

    QString collection_name() const;
    FileNode* child(const QString& filename);
    void add_node(FileNode* node);    

    /**
      * pupulate directory with items no_share status
     */
    void populate();

    bool is_populated() const { return m_populated; }

    /**
      * this method must be called first time in share method for first check
      * and next every time when we have some changes on items
      * after all pending children were completed we finish sharing directory
     */
    void build_collection();
private:       
    bool                        m_populated;
    FileNode*                   m_collection;       //!< collection node
    QHash<QString, FileNode*>   m_file_children;
    QHash<QString, DirNode*>    m_dir_children;
    bool                        m_rehash;           //!< flag will set when object gets update_items before hash completed
    friend class Session;
    friend QDebug operator<<(QDebug dbg, const FileNode* node);
};

/**
  * base node of tree, can't be used for any operations exclude add child nodes
 */
class RootNode : public DirNode
{
public:
    RootNode() : DirNode(NULL, "", NULL) {}
    virtual bool is_root() const { return true; }
    virtual bool is_dir() const { Q_ASSERT(false);  return true; }
    virtual void share(bool recursive) { Q_UNUSED(recursive); Q_ASSERT(false); }
    virtual void unshare(bool recursive) { Q_UNUSED(recursive); Q_ASSERT(false); }
};

const int no_error = 0;
const int error_cancel = 1;

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
    void addTransfer(const add_transfer_params& atp);
    void makeTransferParamsters(const QString& filepath);
    void cancelTransferParams(const QString& filepath);

    void share(const QString& filepath, bool recursive);
    void unshare(const QString& filepath, bool recursive);
private:
    RootNode    m_root;
    QHash<QString, FileNode*>   m_files;
    TransferParamsMaker         m_maker;

public slots:
    void on_transfer_added(Transfer);
    void on_transfer_deleted(QString hash);
    void on_parameters_ready(add_transfer_params atp, int error);
    friend class FileNode;
    friend class DirNode;
};

QDebug operator<<(QDebug dbg, const FileNode* node);

#endif // SHARE_FILE_H
