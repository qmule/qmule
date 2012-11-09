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
#include <QTimer>
#include <QDebug>
#include <QIcon>
#include <set>
#include "qfileinfogatherer.h"

const int no_error = 0;
const int error_cancel = 1;

struct error_code
{
    error_code() : m_ec(no_error){}
    int m_ec;
};

// fake transfer
struct Transfer
{
    Transfer(const QString& hash, const QString& filepath) : m_hash(hash), m_filepath(filepath){}
    QString m_hash;
    QString m_filepath;

};

// fake add transfer params
struct add_transfer_params
{
    add_transfer_params(){}
    add_transfer_params(const QString& filepath, const QString& hash) :
        m_filepath(filepath),
        m_filesize(100),
        m_hash(hash),
        dublicate_is_error(false) {}
    QString m_filepath;
    quint64 m_filesize;
    QString m_hash;
    bool    dublicate_is_error;
};

inline QString genColItem(const QString& filename, quint64 filesize, const QString& hash)
{
    return filename + QString("|") + QString::number(filesize) + QString("|") + hash;
}

add_transfer_params file2atp(const QString& filepath);


class misc
{
public:
    static QString collectionsLocation()
    {
        const QString location = QDir::currentPath() + QDir::separator() + "collections";
        QDir locationDir(location);
        if (!locationDir.exists()) locationDir.mkpath(locationDir.absolutePath());
        return location;
    }
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
  * 1. share file:   if file is not shared set state to active it calls params maker for params, on params signal generate transfer and associate it, inform dir
  * 2. unshare file: if file shared it removes transfer or call cancel params to params maker, set state to inactive, inform dir
  *
  * 3. share dir:    doesn't check self state, for each file execute share
  *                  if call recursive it will delegates to child directories. In any case for child directories
  *                  will be execute update_names method for refresh collection names down
  * 4. unshare dir:  check self state, firstly drop self transfer/params_maker and status to avoid multiple calls check_items,
  *                  next unshare each child file
  *                  For recursive call delegate unshare to child dirs. For not recursive call executes update_names down
  *                  to refresh collection names.
  *
 */

class DirNode;
class Session;

class FileNode
{
public:

    FileNode(DirNode* parent, const QFileInfo& info, Session* session);
    virtual ~FileNode();

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);
    virtual bool has_metadata() const { return m_atp != NULL; }
    virtual bool has_transfer() const { return !m_hash.isEmpty(); }

    // signal handlers
    virtual void process_add_transfer(const QString& hash);
    virtual void process_delete_transfer();
    virtual void process_add_metadata(const add_transfer_params& atp, const error_code& ec);

    virtual QString collection_name() const { return QString(""); }
    virtual QString filepath() const;
    virtual bool is_dir() const { return false; }
    virtual bool is_root() const { return false; }
    virtual int children() const { return 0; }  // for tests
    virtual bool is_active() const { return m_active; }
    virtual bool contains_active_children() const { return m_active; }
    QString hash() const { return m_hash; }
    int level() const;
    QString indention() const;

    QString string() const;
    QString filename() const { return m_filename; }
    virtual qint64 size_on_disk() const { return m_info.size(); }

    DirNode*    m_parent;    
    bool        m_active;
    add_transfer_params* m_atp;
    QFileInfo   m_info;
    QIcon       m_icon;
    QString     m_displayType;
    Session*    m_session;
    error_code  m_error;    
    QString     m_hash;
    QString     m_filename;
};

class DirNode : public FileNode
{
public:
    DirNode(DirNode* parent, const QFileInfo& info, Session* session, bool root = false);
    virtual ~DirNode();

    virtual bool is_dir() const { return true; }
    virtual bool is_root() const { return m_root; }
    virtual int children() const { return m_file_children.count(); }
    virtual bool is_active() const;
    virtual bool contains_active_children() const;

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);

    // signal handlers
    virtual void process_delete_transfer();
    virtual void process_add_metadata(const add_transfer_params& atp, const error_code& ec);

    QString collection_name() const;
    FileNode* child(const QString& filename);
    void add_node(FileNode* node);    
    QStringList exclude_files() const;
    virtual qint64 size_on_disk() const { return 0; }

    /**
      * pupulate directory with items no_share status
     */
    void populate();

    bool is_populated() const { return m_populated; }

    /**
      * call when collection share/unshare to inform children
     */
    void update_state();

    /**
      * will call by file when file change state to unshare
      * if directory was active - checks it status
     */
    void drop_transfer_by_file();

    /**
      * prepare collection file and add transfer based on it
     */
    void build_collection();    

    bool                        m_populated;
    bool                        m_root;
    QHash<QString, FileNode*>   m_file_children;
    QHash<QString, DirNode*>    m_dir_children;
    QVector<FileNode*>          m_file_vector;
    QVector<DirNode*>           m_dir_vector;
    bool                        m_rehash;           //!< flag will set when object gets update_items before hash completed
    friend class Session;
    friend QDebug operator<<(QDebug dbg, const FileNode* node);
};

/**
  *
 */
class Session : public QObject
{
    Q_OBJECT
public:
    Session();
    FileNode* node(const QString& filepath);
    bool associate_transfer(const Transfer& transfer);
    const FileNode* root() const { return &m_root; }
    void deleteTransfer(const QString& hash, bool delete_files);
    void addTransfer(const add_transfer_params& atp);
    void makeTransferParamsters(const QString& filepath);
    void cancelTransferParams(const QString& filepath);

    void removeDir(DirNode* dir);
    void addDir(DirNode* dir);
    void share(const QString& filepath, bool recursive);
    void unshare(const QString& filepath, bool recursive);
    void setNode(const QString& hash, FileNode* node);
    void produce_collections();
    void save() const;
    void load();
    void finalize_collections();
    DirNode                     m_root;

private:
    QHash<QString, FileNode*>   m_files;
    std::set<DirNode*>          m_dirs;
    QHash<QString, QString>     m_transfers; // only for testing
    QTimer                      m_ct;

    void on_transfer_added(Transfer);
    void on_transfer_deleted(QString hash);
    void on_parameters_ready(const add_transfer_params& atp, const error_code& ec);
};

QDebug operator<<(QDebug dbg, const FileNode* node);

#endif // SHARE_FILE_H
