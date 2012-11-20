#ifndef __SESSION_FILESYSTEM__
#define __SESSION_FILESYSTEM__

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

#include <libed2k/add_transfer_params.hpp>
#include <libed2k/error_code.hpp>

class DirNode;

class FileNode
{
public:

    FileNode(DirNode* parent, const QFileInfo& info);
    virtual ~FileNode();

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);
    virtual bool has_metadata() const { return m_atp != NULL; }
    virtual bool has_transfer() const { return !m_hash.isEmpty(); }

    // signal handlers
    virtual void on_transfer_finished(const QString& hash);
    virtual void on_transfer_deleted();
    virtual bool on_metadata_completed(const libed2k::add_transfer_params& atp, const libed2k::error_code& ec);

    virtual QString collection_name() const { return QString(""); }
    virtual QString filepath() const;
    virtual bool is_dir() const { return false; }
    virtual bool is_root() const { return false; }
    virtual int children() const { return 0; }  // for tests
    virtual bool is_active() const { return m_active; }
    virtual bool contains_active_children() const { return m_active; }
    virtual bool all_active_children() const { return m_active; }
    QString hash() const { return m_hash; }
    int level() const;
    QString indention() const;

    QString string() const;
    QString filename() const { return m_filename; }
    void create_transfer();
    virtual qint64 size_on_disk() const { return m_info.size(); }

    DirNode*    m_parent;
    bool        m_active;
    QFileInfo   m_info;
    libed2k::add_transfer_params* m_atp;
    libed2k::error_code  m_error;
    QIcon       m_icon;
    QString     m_displayType;
    QString     m_hash;
    QString     m_filename;
};

class DirNode : public FileNode
{
public:
    DirNode(DirNode* parent, const QFileInfo& info, bool root = false);
    virtual ~DirNode();

    virtual bool is_dir() const { return true; }
    virtual bool is_root() const { return m_root; }
    virtual int children() const { return m_file_children.count(); }
    virtual bool contains_active_children() const;
    virtual bool all_active_children() const;

    virtual void share(bool recursive);
    virtual void unshare(bool recursive);
    void deleteTransfer();

    // signal handlers
    virtual void on_transfer_deleted();
    virtual bool on_metadata_completed(const libed2k::add_transfer_params& atp, const libed2k::error_code& ec);

    QString collection_name() const;
    FileNode* child(const QString& filename);
    void add_node(FileNode* node);
    void delete_node(const FileNode* node);
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
    QList<FileNode*>            m_file_vector;
    QList<DirNode*>             m_dir_vector;
};


#endif //__SESSION_FILESYSTEM__
