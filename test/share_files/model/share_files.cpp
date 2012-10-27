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

QString NodeStatus2String(SharedFiles::FileNode::NodeStatus ns)
{
    static QString strs[] = { QString("N"), QString("S"), QString("U") };
    return strs[ns];
}

SharedFiles::FileNode::FileNode(FileNode* parent, const QString& filename, bool dir) :
    m_parent(parent),
    m_filename(filename),
    m_dir(dir),
    m_status(FileNode::ns_none),
    m_populated(false)
{
}

SharedFiles::FileNode::~FileNode()
{
    foreach(FileNode* p, m_file_children.values()) { delete p; }
    foreach(FileNode* p, m_dir_children.values()) { delete p; }
}

void SharedFiles::FileNode::share(bool recursive)
{
    m_status = ns_shared;
    populate();

    foreach(FileNode* p, m_file_children.values())
    {
        p->share(recursive);
    }

    if (recursive)
    {
        foreach(FileNode* p, m_dir_children.values())
        {
            p->share(recursive);
        }
    }
}

void SharedFiles::FileNode::unshare(bool recursive)
{
    m_status = ns_unshared;

    foreach(FileNode* p, m_file_children.values())
    {
        p->share(recursive);
    }

    if (recursive)
    {
        foreach(FileNode* p, m_dir_children.values())
        {
            p->share(recursive);
        }
    }
}

QString SharedFiles::FileNode::collection_name() const
{
    QStringList name_list;
    const FileNode* parent = this;

    while(parent)
    {
        if (parent->m_status == ns_shared)
        {
            name_list.prepend(parent->m_filename);
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


QString SharedFiles::FileNode::filepath() const
{
    QStringList path;
    const FileNode* parent = this;

    while(parent)
    {
        path.prepend(parent->m_filename);
        parent = parent->m_parent;
    }

    QString fullPath = QDir::fromNativeSeparators(path.join(QDir::separator()));
    qDebug() << path;
    qDebug() << "orig: " << fullPath;

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

SharedFiles::FileNode* SharedFiles::FileNode::file(const QString& filename)
{
    return m_file_children.value(filename);
}

void SharedFiles::FileNode::update_names()
{

    foreach(FileNode* p, m_dir_children.values())
    {
        p->update_names();
    }
}

void SharedFiles::FileNode::add_node(FileNode* node)
{
    if (node->m_dir)
    {
        m_dir_children.insert(node->m_filename, node);
    }
    else
    {
        m_file_children.insert(node->m_filename, node);
    }
}

bool SharedFiles::FileNode::children_in_progress() const
{
    bool res = false;

    if (m_status == ns_shared)
    {
        // check children only directory wait params on self
        foreach(const FileNode* p, m_file_children.values())
        {
            if (p->ns_shared && !p->m_dir)
            {
                res = true;
                break;
            }
        }
    }

    return res;
}

void SharedFiles::FileNode::populate()
{
    if (m_populated || !m_dir) return;
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
        m_dir_children.insert(str, new FileNode(this, str, true));
    }

    foreach(const QString& str, files)
    {
        m_file_children.insert(str, new FileNode(this, str, false));
    }

    m_populated = true;
}

SharedFiles::SharedFiles(): m_root(NULL, "", true)
{}

SharedFiles::FileNode* SharedFiles::node(const QString& filepath)
{
    if (filepath.isEmpty() || filepath == tr("My Computer") ||
            filepath == tr("Computer") || filepath.startsWith(QLatin1Char(':')))
        return const_cast<FileNode*>(&m_root);

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
        return const_cast<FileNode*>(&m_root);

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

    FileNode *parent = &m_root;
    qDebug() << pathElements;

    for (int i = 0; i < pathElements.count(); ++i)
    {
        QString element = pathElements.at(i);
        FileNode* node;
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
                return const_cast<FileNode*>(&m_root);
            }

            node = new FileNode(parent, element, true);
            parent->add_node(node);
        }

        Q_ASSERT(node);
        parent = node;
    }

    parent->populate();

    if (fi.isFile())
    {
        qDebug() << "load filename";
        parent = parent->file(fi.fileName());
        if (!parent) parent = &m_root;
    }

    return parent;
}

bool SharedFiles::associate_transfer(const Transfer& transfer)
{
    FileNode* p = node(transfer.m_filepath);

    if (p != &m_root)
    {
        p->m_hash = transfer.m_hash;
    }

    return !p->m_hash.isEmpty();
}


QDebug operator<<(QDebug dbg, const SharedFiles::FileNode* node)
{
    dbg.nospace() << "{" << (node->m_dir?"D:":"F:") << node->m_filename
                  << "(" << NodeStatus2String(node->m_status) << ")"
                  << "ppl:" << (node->m_populated?"Y":"N");

    foreach(const SharedFiles::FileNode* p, node->m_dir_children)
    {
        dbg.nospace() << p;
    }

    foreach(const SharedFiles::FileNode* p, node->m_file_children)
    {
        dbg.nospace() << p;
    }

    dbg.nospace() << "}";

    return dbg.space();
}

