#include <QTimerEvent>
#include <QDirIterator>
#include <QFileSystemModel>
#include <QUrl>
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
    m_atp(NULL)
{
}

FileNode::~FileNode()
{
}

void FileNode::share(bool recursive)
{
    Q_UNUSED(recursive);
    if (m_active) return;
    m_active = true;

    if (has_metadata())
    {
        m_session->addTransfer(*m_atp);
    }
    else
    {
        m_session->makeTransferParamsters(filepath());
    }

    // inform our container to change state
    m_parent->drop_transfer();
}

void FileNode::unshare(bool recursive)
{
    Q_UNUSED(recursive);
    if (!m_active) return;
    m_active = false;

    if (has_transfer())
    {
        m_session->deleteTransfer(m_hash, false);
    }
    else
    {
        m_session->cancelTransferParams(filepath());
    }

    m_parent->drop_transfer();
}

void FileNode::process_add_transfer(const QString& hash)
{
    m_hash = hash;

    if (!is_active())
    {
        m_session->deleteTransfer(hash, false);
    }
}

void FileNode::process_delete_transfer()
{
    m_hash.clear();

    if (is_active())
    {
        m_session->addTransfer(*m_atp);
    }
}

void FileNode::process_add_metadata(const add_transfer_params& atp, const error_code& ec)
{
    if (ec.m_ec == no_error)
    {
        if (!m_atp) m_atp = new add_transfer_params;
        *m_atp = atp;

        if (is_active())
        {
            m_session->addTransfer(atp);
        }
    }

    m_error = ec;
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

QString FileNode::string() const
{
    QString res;

    if (m_atp)
    {
        res = genColItem(filename(), m_atp->m_filesize, m_atp->m_hash);
    }

    return (res);
}

DirNode::DirNode(DirNode* parent, const QString& filename, Session* session) :    
    FileNode(parent, filename, session),    
    m_populated(false)
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

    foreach(DirNode* p, m_dir_children.values())
    {
        if (recursive)
        {
            p->share(recursive);
        }
        else
        {
            p->drop_transfer(); // all included collections must execute rename
        }
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
            drop_transfer();
        }
    }
}

void DirNode::process_delete_transfer()
{
    // do nothing
}

void DirNode::process_add_metadata(const add_transfer_params& atp, const error_code& ec)
{
    // do nothing
}

void DirNode::drop_transfer()
{
    if (has_transfer()) m_session->deleteTransfer(m_hash, true);
}

void DirNode::build_collection()
{
    // collection would hash, but hasn't transfer yet
    if (m_active && !has_transfer())
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
            if (!m_coll_hash.isEmpty())
            {
                m_session->deleteTransfer(m_hash, true);
            }

            QStringList lines;

            foreach(const FileNode* p, m_file_children.values())
            {
                if (p->is_active())
                {
                    lines << p->string();
                    Q_ASSERT(!line.isEmpty());
                }
            }

            int iteration = 0;
            // generate unique filename
            QDir cd(misc::collectionsLocation());
            QString collection_filepath;

            while(collection_filepath.isEmpty())
            {
                QString filename = collection_name() + QString("-") + QString::number(lines.count()) + (iteration?(QString("_") + QString::number(iteration)):QString()) +  QString(".emulecollection");
                QFileInfo fi(cd.filePath(filename));

                if (fi.exists())
                {
                    ++iteration;
                }
                else
                {
                    collection_filepath = fi.absoluteFilePath();
                }
            }

            QFile data(collection_filepath);

            if (data.open(QFile::WriteOnly | QFile::Truncate))
            {
                 QTextStream out(&data);

                 foreach(const QString& line, lines)
                 {
                     out << line << "\n";
                 }

                 data.close();

                 // hash file and add node
                 m_session->addTransfer(file2atp(cd.absolutePath()));
                 m_session->setNode(m_hash, this);  // must assign new transfer hash to out collection, not to original path
            }
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

    return (name_list.join(QString("-")));
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

void Session::setNode(const QString& hash, FileNode* node)
{
    m_files.insert(hash, node);
}


void Session::on_transfer_added(Transfer t)
{        
    FileNode* p = NULL;

    if (m_files.contains(t.m_hash))
    {
        p = m_files.value(t.m_hash);
    }
    else
    {
        p = node(t.m_filepath);
    }

    Q_ASSERT(p);
    Q_ASSERT(p != &m_root);

    m_files.insert(t.m_hash, p);        // store for use on delete
    p->process_add_transfer(t.m_hash);
}

void Session::on_transfer_deleted(QString hash)
{        
    QHash<QString, FileNode*>::iterator itr = m_files.find(hash);
    Q_ASSERT(itr != m_files.end());
    FileNode* p = itr.value();
    Q_ASSERT(p);
    m_files.erase(itr);
    p->process_delete_transfer();
}

void Session::on_parameters_ready(const add_transfer_params& atp, const error_code& ec)
{
    FileNode* p = node(atp.m_filepath);
    Q_ASSERT(p != &m_root);
    p->process_add_metadata(atp, ec);
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

