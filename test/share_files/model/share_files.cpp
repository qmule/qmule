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

FileNode::FileNode(DirNode* parent, const QString& filename, Session* session) :    
    m_parent(parent),
    m_active(false),
    m_filename(filename),
    m_atp(NULL),
    m_error(0)
{
}

FileNode::~FileNode()
{
}

void FileNode::share(bool recursive)
{
    Q_UNUSED(recursive);
    m_active = true;

    if (has_metadata())
    {
        m_session->addTransfer(*m_atp);
    }
    else
    {
        m_session->makeTransferParamsters(filepath());
    }
}

void FileNode::unshare(bool recursive)
{
    Q_UNUSED(recursive);
    m_active = false;

    if (has_transfer())
    {
        m_session->deleteTransfer(m_hash, false);
    }
    else
    {
        m_session->cancelTransferParams(filepath());
    }
}

bool FileNode::has_metadata() const
{
    return (m_atp != NULL);
}

bool FileNode::has_transfer() const
{
    return (!m_hash.isEmpty());
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

void FileNode::set_metadata(const add_transfer_params& atp, int error)
{
    if (!m_atp) m_atp = new add_transfer_params;
    *m_atp = atp;    
    m_error = error;
}

DirNode::DirNode(DirNode* parent, const QString& filename, Session* session) :    
    FileNode(parent, filename, session),    
    m_populated(false),
    m_collection(NULL)
{
}

DirNode::~DirNode()
{
    foreach(FileNode* p, m_file_children.values()) { delete p; }
    foreach(DirNode* p, m_dir_children.values()) { delete p; }
}

void DirNode::share(bool recursive)
{
    m_active = true;
    // execute without check current state
    // we can re-share files were unshared after directory was shared
    populate();

    foreach(FileNode* p, m_file_children.values())
    {
        p->share(recursive);
    }

    // possibly all files were completed already
    build_collection();

    foreach(DirNode* p, m_dir_children.values())
    {
        if (recursive)
        {
            p->share(recursive);
        }

        p->build_collection();
    }
}

void DirNode::unshare(bool recursive)
{
    m_active = false;

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
            p->build_collection();
        }
    }
}

void DirNode::build_collection()
{
    // collection would hash, but hasn't transfer yet
    if (m_active)
    {
        bool pending = false;

        // check children        
        foreach(const FileNode* p, m_file_children.values())
        {
            // item in pending state
            if (p->is_active() && !p->has_transfer())
            {
                pending = true;
                break;
            }
        }

        if (!pending)
        {
            // when we have collection - unshare it
            if (m_collection)
            {
                m_collection->unshare(false);
            }

            // TODO
            //1. create file
            //2. fill file
            //3. get node
            //4. share node
        }
    }
}

QString DirNode::collection_name() const
{
    QStringList name_list;
    const DirNode* parent = this;

    while(parent && !parent->is_root())
    {
        if (parent->is_active())
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
        m_dir_children.insert(str, new DirNode(this, str, m_session));
    }

    foreach(const QString& str, files)
    {
        m_file_children.insert(str, new FileNode(this, str, m_session));
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

            node = new DirNode(parent, element, this);
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

void Session::deleteTransfer(const QString& hash, bool delete_files)
{

}

void Session::addTransfer(const add_transfer_params& atp)
{

}

void Session::makeTransferParamsters(const QString& filepath)
{

}

void Session::cancelTransferParams(const QString& filepath)
{

}

void Session::share(const QString& filepath, bool recursive)
{
    FileNode* p = node(filepath);
    if (p != &m_root) p->share(recursive);
}

void Session::unshare(const QString& filepath, bool recursive)
{
    FileNode* p = node(filepath);
    if (p != &m_root) p->unshare(recursive);
}

void Session::on_transfer_added(Transfer t)
{    
    FileNode* p = node(t.m_filepath);

    if (p != &m_root) // is root - fail?
    {
        m_files.insert(t.m_hash, p);        // store for use on delete
        p->m_hash = t.m_hash;

        if (!p->is_active())
        {
            deleteTransfer(t.m_hash, false);
        }
        else
        {
            DirNode* parent = p->m_parent;
            if (parent != &m_root) parent->build_collection();
        }
    }
}

void Session::on_transfer_deleted(QString hash)
{    
    if (m_files.contains(hash))
    {
        FileNode* p = m_files.value(hash);
        p->set_transfer("");

        if (p->is_active())
        {
            addTransfer(*(p->m_atp));
        }
        else
        {
            DirNode* parent = p->m_parent;
            if (parent != &m_root) parent->build_collection();
        }
    }
}

void Session::on_parameters_ready(add_transfer_params atp, int error)
{
    FileNode* p = node(atp.m_filepath);

    if (p != &m_root)
    {
        switch(error)
        {
            case no_error:
                p->set_metadata(atp, error);
                if (p->is_active()) addTransfer(atp);
                break;
            case error_cancel:
                break;
            default:
                break;
        }
    }
}

QDebug operator<<(QDebug dbg, const FileNode* node)
{
    dbg.nospace() << "{" << (node->is_dir()?"D:":"F:") << node->filename();

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

