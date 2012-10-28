#include <QTimerEvent>
#include <QDirIterator>
#include <QFileSystemModel>
#include "share_files.h"

#ifdef Q_OS_WIN32
static QString qt_GetLongPathName(const QString &strShortPath)
{
    if (strShortPath.isEmpty()
        || strShortPath == QLatin1String(".") || strShortPath == QLatin1String(".."))
        return strShortPath;
    if (strShortPath.length() == 2 && strShortPath.endsWith(QLatin1Char(':')))
        return strShortPath.toUpper();
    const QString absPath = QDir(strShortPath).absolutePath();
    if (absPath.startsWith(QLatin1String("//"))
        || absPath.startsWith(QLatin1String("\\\\"))) // unc
        return QDir::fromNativeSeparators(absPath);
    if (absPath.startsWith(QLatin1Char('/')))
        return QString();
    const QString inputString = QLatin1String("\\\\?\\") + QDir::toNativeSeparators(absPath);
    QVarLengthArray<TCHAR, MAX_PATH> buffer(MAX_PATH);
    DWORD result = ::GetLongPathName((wchar_t*)inputString.utf16(),
                                     buffer.data(),
                                     buffer.size());
    if (result > DWORD(buffer.size())) {
        buffer.resize(result);
        result = ::GetLongPathName((wchar_t*)inputString.utf16(),
                                   buffer.data(),
                                   buffer.size());
    }
    if (result > 4) {
        QString longPath = QString::fromWCharArray(buffer.data() + 4); // ignoring prefix
        longPath[0] = longPath.at(0).toUpper(); // capital drive letters
        return QDir::fromNativeSeparators(longPath);
    } else {
        return QDir::fromNativeSeparators(strShortPath);
    }
}
#endif

QString NodeCommand2String(FileNode::NodeCommand ns)
{
    static QString strs[] = { QString("N"), QString("S"), QString("U") };
    return strs[ns];
}

FileNode::FileNode(DirNode* parent, const QString& filename) :
    m_parent(parent),
    m_command(FileNode::nc_none),
    m_filename(filename),
    m_atp(NULL)
{
}

FileNode::~FileNode()
{
}

void FileNode::share(bool recursive)
{
    Q_UNUSED(recursive);

    // avoid hash twice!
    if (m_command != nc_share)
    {
        m_command = nc_share;

        if (m_atp)
        {
            // TODO add transfer
        }
        else
        {
            // TODO - send request to parameters maker
        }
    }
}

void FileNode::unshare(bool recursive)
{
    Q_UNUSED(recursive);

    if (m_command != nc_unshare)
    {
        if (has_associated_transfer())
        {
            // TODO - remove transfer
        }
        else
        {
            // TODO - send cancel request to parameters maker
        }

        m_command = nc_unshare;

        // inform collection
        m_parent->check_items();
    }
}

void FileNode::associate_transfer(const QString& hash)
{
    if (m_command == nc_unshare)
    {
        // collision - user canceled share after request was sended and before answer
        // TODO - remove transfer
        return;
    }

    m_hash = hash;
    // inform directory we have changes
    m_parent->check_items();
}

QString FileNode::filepath() const
{
    QStringList path;
    const DirNode* parent = m_parent;
    path << m_filename;

    while(parent && !parent->is_root())
    {
        path.prepend(parent->filename());
        parent = parent->m_parent;
    }

    QString fullPath = QDir::fromNativeSeparators(path.join(QDir::separator()));

#if !defined(Q_OS_WIN) || defined(Q_OS_WINCE)
    if ((fullPath.length() > 2) && fullPath[0] == QLatin1Char('/') && fullPath[1] == QLatin1Char('/'))
        fullPath = fullPath.mid(1);
#endif

#if defined(Q_OS_WIN) || defined(Q_OS_SYMBIAN)
    if (fullPath.length() == 2 && fullPath.endsWith(QLatin1Char(':')))
        fullPath.append(QLatin1Char('/'));
#endif

    return fullPath;
}

void FileNode::set_transfer_params(const add_transfer_params& atp)
{
    if (!m_atp) m_atp = new add_transfer_params;
    *m_atp = atp;

    if (m_command == nc_share)
    {
        // TODO add transfer
    }

    // when status is not shared - we have situation when user cancelled hash, but signal was already submitted
}

DirNode::DirNode(DirNode* parent, const QString& filename) : FileNode(parent, filename), m_populated(false)
{
}

DirNode::~DirNode()
{
    foreach(FileNode* p, m_file_children.values()) { delete p; }
    foreach(DirNode* p, m_dir_children.values()) { delete p; }
}

void DirNode::share(bool recursive)
{
    // execute without check current state
    // we can re-share files were unshared after directory was shared

    populate();

    m_command = nc_share;

    foreach(FileNode* p, m_file_children.values())
    {
        p->share(recursive);
    }

    // possibly all files were completed already
    check_items();

    foreach(DirNode* p, m_dir_children.values())
    {
        if (recursive)
        {
            p->share(recursive);
        }

        p->update_names();
    }
}

void DirNode::unshare(bool recursive)
{
    if (m_command != nc_unshare)
    {
        // to avoid cycle - unshare dir self firstly
        m_command = nc_unshare;

        if (has_associated_transfer())
        {
            // TODO - remove transfer
        }

        foreach(FileNode* p, m_file_children.values())
        {
            p->unshare(recursive);
        }

        foreach(DirNode* p, m_dir_children.values())
        {
            if (recursive)
            {
                p->unshare(recursive);
            }
            else
            {
                // we must update names on all children collections
                p->update_names();
            }
        }
    }
}

void DirNode::associate_transfer(const QString& hash)
{
    if (m_command != nc_unshare)
    {
        m_hash = hash;
        // we must not inform parent because it is directory now
    }
}

void DirNode::update_names()
{
    if (has_associated_transfer())
    {
        // TODO - re-calculate collection name
    }
}

void DirNode::check_items()
{
    // collection would hash, but hasn't transfer yet
    if (last_command() == nc_share)
    {
        // we have transfer on old data - remove it
        if (has_associated_transfer())
        {
            // TODO - cancel transfer
        }
        else
        {
            // TODO - cancel hashing
        }

        bool pending = false;
        // check children
        foreach(const FileNode* p, m_file_children.values())
        {
            if (p->in_progress())
            {
                pending = true;
                break;
            }
        }

        if (!pending)
        {
            // TODO - prepare collection and hash it
        }
    }
}

QString DirNode::collection_name() const
{
    QStringList name_list;
    const DirNode* parent = this;

    while(parent && !parent->is_root())
    {
        if (parent->last_command() == nc_share)
        {
            name_list.prepend(parent->filename());
        }

        parent = parent->m_parent;
    }

    QString res;

    if (!name_list.isEmpty())
    {
        res = name_list.join(QString("-")) + QString::number(m_file_children.count());
    }

    return res;
}

FileNode* DirNode::child(const QString& filename)
{
    return m_file_children.value(filename);
}

void DirNode::add_node(FileNode* node)
{
    if (node->is_dir())
    {
        m_dir_children.insert(node->filename(), static_cast<DirNode*>(node));
    }
    else
    {
        m_file_children.insert(node->filename(), node);
    }
}

void DirNode::populate()
{
    if (m_populated) return;

    QString path = filepath();
    qDebug() << "populate " << path;

    QString itPath = QDir::fromNativeSeparators(path);
    QDirIterator dirIt(itPath, QDir::NoDotAndDotDot| QDir::AllEntries | QDir::System | QDir::Hidden);
    QStringList dirs;
    QStringList files;

    while(dirIt.hasNext())
    {
        dirIt.next();
        QFileInfo fileInfo = dirIt.fileInfo();

        if (fileInfo.isDir() && !m_dir_children.contains(fileInfo.fileName()))
        {
            dirs << fileInfo.fileName();
        }
        else if (fileInfo.isFile() && !m_file_children.contains(fileInfo.fileName()))
        {
            files << fileInfo.fileName();
        }
    }

    foreach(const QString& str, dirs)
    {
        m_dir_children.insert(str, new DirNode(this, str));
    }

    foreach(const QString& str, files)
    {
        m_file_children.insert(str, new FileNode(this, str));
    }

    m_populated = true;
}

FileNode* Session::node(const QString& filepath)
{
    if (filepath.isEmpty() || filepath == tr("My Computer") ||
            filepath == tr("Computer") || filepath.startsWith(QLatin1Char(':')))

        return (&m_root);

#ifdef Q_OS_WIN32
    QString longPath = qt_GetLongPathName(filepath);
#else
    QString longPath = filepath;
#endif

    QFileInfo fi(longPath);

    QString absolutePath = QDir(longPath).absolutePath();

    // ### TODO can we use bool QAbstractFileEngine::caseSensitive() const?
    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);

    if ((pathElements.isEmpty())
#if (!defined(Q_OS_WIN) || defined(Q_OS_WINCE)) && !defined(Q_OS_SYMBIAN)
        && QDir::fromNativeSeparators(longPath) != QLatin1String("/")
#endif
        )
        return (&m_root);

#if (defined(Q_OS_WIN) && !defined(Q_OS_WINCE)) || defined(Q_OS_SYMBIAN)
    {
        if (!pathElements.at(0).contains(QLatin1String(":")))
        {
            // The reason we express it like this instead of with anonymous, temporary
            // variables, is to workaround a compiler crash with Q_CC_NOKIAX86.
            QString rootPath = QDir(longPath).rootPath();
            pathElements.prepend(rootPath);
        }

        if (pathElements.at(0).endsWith(QLatin1Char('/')))
            pathElements[0].chop(1);
    }
#else
    // add the "/" item, since it is a valid path element on Unix
    if (absolutePath[0] == QLatin1Char('/'))
        pathElements.prepend(QLatin1String("/"));
#endif

    DirNode *parent = &m_root;
    qDebug() << pathElements;

    for (int i = 0; i < pathElements.count(); ++i)
    {
        QString element = pathElements.at(i);
        DirNode* node;
#ifdef Q_OS_WIN
        // On Windows, "filename......." and "filename" are equivalent Task #133928
        while (element.endsWith(QLatin1Char('.')))
            element.chop(1);
#endif
        bool alreadyExists = parent->m_dir_children.contains(element);

        if (alreadyExists)
        {
            node = parent->m_dir_children.value(element);
        }
        else
        {
            // Someone might call ::index("file://cookie/monster/doesn't/like/veggies"),
            // a path that doesn't exists, I.E. don't blindly create directories.
            QFileInfo info(absolutePath);

            if (!info.exists())
            {
                qDebug() << "absolute path " << absolutePath << " is not exists";
                return (&m_root);
            }

            node = new DirNode(parent, element);
            parent->add_node(node);
        }

        Q_ASSERT(node);
        parent = node;
    }

    parent->populate();

    FileNode* f = parent;

    if (fi.isFile())
    {
        qDebug() << "load filename";
        f = parent->child(fi.fileName());
        if (!f) f = &m_root;
    }

    return f;
}

bool Session::associate_transfer(const Transfer& transfer)
{
    FileNode* p = node(transfer.m_filepath);

    if (p != &m_root)
    {
        if (p->last_command() == FileNode::nc_unshare)
        {
            // transfer must be erased!
        }

        p->set_hash(transfer.m_hash);
    }

    return !p->hash().isEmpty();
}

void Session::deleteTransfer(const QString& hash, bool delete_files)
{

}

void Session::addTransfer(Transfer t)
{

}

void Session::on_transfer_added(Transfer)
{    
}

void Session::on_transfer_removed(QString hash)
{

}

void Session::on_made_parameters()
{

}

QDebug operator<<(QDebug dbg, const FileNode* node)
{
    dbg.nospace() << "{" << (node->is_dir()?"D:":"F:") << node->filename()
                  << "(" << NodeCommand2String(node->last_command()) << ")";

    if (const DirNode* dnode = dynamic_cast<const DirNode*>(node))
    {
        dbg.nospace() << "ppl:" << (dnode->is_populated()?"Y":"N");

        foreach(const DirNode* p, dnode->m_dir_children)
        {
            dbg.nospace() << p;
        }

        foreach(const FileNode* p, dnode->m_file_children)
        {
            dbg.nospace() << p;
        }
    }

    dbg.nospace() << "}";

    return dbg.space();
}

