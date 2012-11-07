#include <QTimerEvent>
#include <QDirIterator>
#include <QFileSystemModel>
#include <QTextStream>
#include <QCryptographicHash>
#ifdef Q_WS_WIN32
#include <QVarLengthArray>
#include <windows.h>
#endif
#include "share_files.h"
#include "../../src/qinisettings.h"

#define CNAME "unit"
#define PNAME "test"

add_transfer_params file2atp(const QString& filepath)
{        
    return add_transfer_params(filepath, QString(QCryptographicHash::hash(filepath.toLocal8Bit(), QCryptographicHash::Md5).toHex()));
}

class Preferences : public QIniSettings
{
  Q_DISABLE_COPY(Preferences)

public:
  Preferences() : QIniSettings(CNAME,  PNAME)
  {
      qDebug() << "Preferences constructor: " << CNAME << ":" << PNAME;
  }
};

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

QString translateDriveName(const QFileInfo &drive)
{
    QString driveName = drive.absoluteFilePath();
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    if (driveName.startsWith(QLatin1Char('/'))) // UNC host
        return drive.fileName();
#endif
#if (defined(Q_OS_WIN) && !defined(Q_OS_WINCE)) || defined(Q_OS_SYMBIAN)
    if (driveName.endsWith(QLatin1Char('/')))
        driveName.chop(1);
#endif
    return driveName;
}

FileNode::FileNode(DirNode* parent, const QString& filename, Session* session) :    
    m_parent(parent),
    m_active(false),
    m_filename(filename),
    m_atp(NULL),
    m_session(session)
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
        try
        {
            m_session->addTransfer(*m_atp);
        }
        catch(...)
        {
            // catch dublicate errors
        }
    }
    else
    {
        m_session->makeTransferParamsters(filepath());
    }

    m_parent->drop_transfer_by_file();
}

void FileNode::unshare(bool recursive)
{
    Q_UNUSED(recursive);
    if (!m_active) return;
    qDebug() << indention() << "unshare file: " << filename();
    m_active = false;

    if (has_transfer())
    {
        m_session->deleteTransfer(m_hash, false);
    }
    else
    {
        m_session->cancelTransferParams(filepath());
    }

    m_parent->drop_transfer_by_file();
}

void FileNode::process_add_transfer(const QString& hash)
{
    m_hash = hash;

    if (!is_active())
    {
        try
        {
            m_session->deleteTransfer(hash, false);
        }
        catch(...)
        {
            // catch dublicate errors
        }
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
        m_atp->dublicate_is_error = true;

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

int FileNode::level() const
{
    int level = 0;
    const DirNode* parent = m_parent;

    while(parent && !parent->is_root())
    {
        ++level;
        parent = parent->m_parent;
    }

    return level;
}

QString FileNode::indention() const
{
    QString indent;
    indent.fill(' ', level()*2);
    return indent;
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
    if (!m_active)
    {
        m_active = true;
        m_session->addDir(this);

        // execute without check current state
        // we can re-share files were unshared after directory was shared
        populate();

        foreach(FileNode* p, m_file_children.values())
        {
            p->share(recursive);
        }

        // update state on all children
        foreach(DirNode* p, m_dir_children.values())
        {
            p->update_state();
        }
    }

    // share all what are not shared
    foreach(DirNode* p, m_dir_children.values())
    {        
        if (recursive)
        {
            p->share(recursive);
        }
    }
}

void DirNode::unshare(bool recursive)
{
    qDebug() << indention() << "unshare dir: " << filename();

    if (m_active)
    {
        m_active = false;
        m_session->removeDir(this);

        if (has_transfer())
        {
            m_session->deleteTransfer(m_hash, true);
        }

        foreach(FileNode* p, m_file_children.values())
        {
            p->unshare(recursive);
        }

        // on non-recursive we update state because current node state was changed
        if (!recursive)
        {
            foreach(DirNode* p, m_dir_children.values())
            {
                p->update_state();
            }
        }
    }

    if (recursive)
    {
        foreach(DirNode* p, m_dir_children.values())
        {
            p->unshare(recursive);
        }
    }
}

bool DirNode::is_active() const
{
    bool active = false;

    if (m_active)
    {
        foreach(const FileNode* node, m_file_children)
        {
            active = node->is_active();
            if (active) break;
        }
    }

    return active;
}

void DirNode::process_delete_transfer()
{
    m_hash.clear();
}

void DirNode::process_add_metadata(const add_transfer_params& atp, const error_code& ec)
{
    // do nothing
}

void DirNode::update_state()
{    
    if (m_active && (has_transfer()))
    {
        m_session->deleteTransfer(m_hash, true);
    }

    foreach(DirNode* node, m_dir_children)
    {
        node->update_state();
    }
}

void DirNode::drop_transfer_by_file()
{
    if (m_active && has_transfer())
    {
        m_session->deleteTransfer(m_hash, true);
    }
}

void DirNode::build_collection()
{
    // collection would hash, but hasn't transfer yet
    qDebug() << "build collection " << filename();
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
        qDebug() << "collection " << filename() << " ready";
        QStringList lines;

        foreach(const FileNode* p, m_file_children.values())
        {
            if (p->is_active())
            {
                QString line = p->string();
                Q_ASSERT(!line.isEmpty());
                lines << line;
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

        qDebug() << "collection filepath " << collection_filepath;

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
             // TODO - replace testing code
             add_transfer_params atp = file2atp(collection_filepath);   //!< get apt
             m_session->setNode(atp.m_hash, this);                      //!< set node
             m_session->addTransfer(atp);                               //!< add transfer
        }
    }
}

QString DirNode::collection_name() const
{
    QString res;

    // check active by user wish, do not check files
    if (m_active)
    {
        QStringList name_list;
        const DirNode* parent = this;

        while(parent && !parent->is_root())
        {
            if (parent->m_active)
            {
                name_list.prepend(parent->filename());
            }

            parent = parent->m_parent;
        }

        res = name_list.join(QString("-"));
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
        m_dir_vector.push_back(static_cast<DirNode*>(node));
    }
    else
    {
        m_file_children.insert(node->filename(), node);
        m_file_vector.push_back(node);
    }
}

QStringList DirNode::exclude_files() const
{
    QStringList res;

    if (is_active())
    {
        foreach(const FileNode* p, m_file_children)
        {
            if (!p->is_active())
            {
                res << p->filename();
            }
        }
    }

    return res;
}

void DirNode::populate()
{
    if (m_populated) return;

    QString path = filepath();
    qDebug() << "populate " << path;
    QStringList dirs;
    QStringList files;

    if (path.isEmpty())
    {
        foreach(const QFileInfo& fi, QDir::drives())
        {
            dirs << translateDriveName(fi);
        }
    }
    else
    {
        QString itPath = QDir::fromNativeSeparators(path);
        QDirIterator dirIt(itPath, QDir::NoDotAndDotDot| QDir::AllEntries | QDir::System | QDir::Hidden);

        while(dirIt.hasNext())
        {
            dirIt.next();
            QFileInfo fileInfo = dirIt.fileInfo();

            if (fileInfo.isDir() && !m_dir_children.contains(fileInfo.fileName()))
            {
                dirs << fileInfo.fileName();
                continue;
            }

            if (fileInfo.isFile() && !m_file_children.contains(fileInfo.fileName()))
            {
                files << fileInfo.fileName();
                continue;
            }

            qDebug() << "member " << fileInfo.fileName() << " exists";
        }
    }

    foreach(const QString& str, dirs)
    {
        qDebug() << "add member: " << str;
        add_node(new DirNode(this, str, m_session));
    }

    foreach(const QString& str, files)
    {
        qDebug() << "add member: " << str;
        add_node(new FileNode(this, str, m_session));
    }

    m_populated = true;
}

Session::Session() : m_root(NULL, "", NULL)
{

}

FileNode* Session::node(const QString& filepath)
{
    qDebug() << "node: " << filepath;
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
    QString last_filename = pathElements.back();
    pathElements.pop_back();

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

    if (parent->m_dir_children.contains(last_filename))
    {
        return parent->m_dir_children.value(last_filename);
    }

    if (parent->m_file_children.contains(last_filename))
    {
        return parent->m_file_children.value(last_filename);
    }


    FileNode* p = &m_root;
    QFileInfo info(absolutePath);

    if (info.exists())
    {

        if (info.isFile())
        {
            p = new FileNode(parent, last_filename, this);
            parent->add_node(p);
        }
        else if (info.isDir())
        {
            p = new DirNode(parent, last_filename, this);
            ((DirNode*)p)->populate();
            parent->add_node(p);
        }

    }

    return (p);
}

void Session::removeDir(DirNode* dir)
{
    std::set<DirNode*>::iterator itr = m_dirs.find(dir);

    if (itr != m_dirs.end()) m_dirs.erase(itr);

    m_dirs.erase(std::find(m_dirs.begin(), m_dirs.end(), dir), m_dirs.end());
}

void Session::addDir(DirNode* dir)
{
    m_dirs.insert(dir);
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

void Session::produce_collections()
{
    for (std::set<DirNode*>::iterator itr = m_dirs.begin(); itr != m_dirs.end(); ++itr)
    {
        DirNode* p = *itr;

        if (p->is_active() && !p->has_transfer())
        {
            p->build_collection();
        }
    }
}

void Session::save() const
{
    Preferences pref;
    pref.beginGroup("SharedDirectories");
    pref.beginWriteArray("ShareDirs");

    int dir_indx = 0;
    for (std::set<DirNode*>::const_iterator itr = m_dirs.begin(); itr != m_dirs.end(); ++itr)
    {
        const DirNode* p = *itr;
        qDebug() << "save: " << p->filepath();

        pref.setArrayIndex(dir_indx);
        pref.setValue("Path", p->filepath());
        QStringList efiles = p->exclude_files();

        if (!efiles.isEmpty())
        {
            int file_indx = 0;
            pref.beginWriteArray("ExcludeFiles", efiles.size());

            foreach(const QString& efile, efiles)
            {
                pref.setArrayIndex(file_indx);
                pref.setValue("FileName", efile);
                ++file_indx;
            }

            pref.endArray();
        }

        ++dir_indx;
    }

    pref.endArray();
    pref.endGroup();
}

bool operator<(const QVector<QString>& v1, const QVector<QString>& v2)
{
    return v1.size() < v2.size();
}

void Session::load()
{
    Preferences pref;
    typedef QPair<QString, QVector<QString> > SD;
    QVector<SD> vf;

    pref.beginGroup("SharedDirectories");
    int dcount = pref.beginReadArray("ShareDirs");
    vf.resize(dcount);

    for (int i = 0; i < dcount; ++i)
    {

        pref.setArrayIndex(i);
        vf[i].first = pref.value("Path").toString();

        int fcount = pref.beginReadArray("ExcludeFiles");
        vf[i].second.resize(fcount);

        for (int j = 0; j < fcount; ++ j)
        {
            pref.setArrayIndex(j);
            vf[i].second[j] = pref.value("FileName").toString();
        }

        pref.endArray();

    }

    pref.endArray();

    // sort dirs ASC to avoid update states on sharing
    std::sort(vf.begin(), vf.end());

    foreach(const SD& item, vf)
    {
        FileNode* dir_node = node(item.first);

        if (dir_node != &m_root)
        {
            dir_node->share(false);
            QDir filepath(item.first);

            for(int i = 0; i < item.second.size(); ++i)
            {
                FileNode* file_node = node(filepath.absoluteFilePath(item.second[i]));

                if (file_node != &m_root)
                {
                    file_node->unshare(false);
                }
            }
        }

    }
}

void Session::finalize_collections()
{
    foreach(DirNode* node, m_dirs)
    {
        if (node->has_transfer())
        {
            deleteTransfer(node->hash(), true);
            node->process_delete_transfer();
        }
    }
}


void Session::on_transfer_added(Transfer t)
{        
    qDebug() << "on transfer added: " << t.m_filepath << " hash " << t.m_hash;
    FileNode* p = NULL;

    if (m_files.contains(t.m_hash))
    {
        p = m_files.value(t.m_hash);
    }
    else
    {        
        p = node(t.m_filepath);
        m_files.insert(t.m_hash, p);        // store for use on delete
    }

    Q_ASSERT(p);
    Q_ASSERT(p != &m_root);

    p->process_add_transfer(t.m_hash);
    m_ct.setInterval(10000);
}

void Session::on_transfer_deleted(QString hash)
{        
    QHash<QString, FileNode*>::iterator itr = m_files.find(hash);
    Q_ASSERT(itr != m_files.end());
    FileNode* p = itr.value();
    Q_ASSERT(p);
    m_files.erase(itr);
    p->process_delete_transfer();
    m_ct.setInterval(10000);
}

void Session::on_parameters_ready(const add_transfer_params& atp, const error_code& ec)
{
    FileNode* p = node(atp.m_filepath);
    Q_ASSERT(p != &m_root);
    p->process_add_metadata(atp, ec);
    m_ct.setInterval(10000);
}

void Session::deleteTransfer(const QString& hash, bool delete_files)
{
    bool exists = m_transfers.contains(hash);
    Q_ASSERT(exists);
    QString filepath = m_transfers.value(hash);
    Q_ASSERT(m_transfers.remove(hash));

    if (delete_files) // only on dir nodes in test
    {
        qDebug() << "deleteTransfer: " << filepath;
        QFile f(filepath);
        Q_ASSERT(f.remove());
    }

    on_transfer_deleted(hash);
}

void Session::addTransfer(const add_transfer_params& atp)
{
    Transfer t(atp.m_hash, atp.m_filepath);
    m_transfers.insert(atp.m_hash, atp.m_filepath); // test code
    on_transfer_added(t);   // immediately signal on add
}

void Session::makeTransferParamsters(const QString& filepath)
{
    on_parameters_ready(file2atp(filepath), error_code());
}

void Session::cancelTransferParams(const QString& filepath)
{

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

