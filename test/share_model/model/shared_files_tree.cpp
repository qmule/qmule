#include <QTimerEvent>
#include <QDebug>
#include "shared_files_tree.h"

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

shared_files_tree::shared_files_tree(QObject *parent) :
    QObject(parent)
{
}

void shared_files_tree::filesystem_changed(const QString &path, const FileInfoList& updates)
{
    QFileSystemNode* parentNode = node(path, false);

    int nMin = -1;
    int nMax = -1;

    for (int i = 0; i < updates.count(); ++i)
    {
        QString fileName = updates.at(i).first;
        Q_ASSERT(!fileName.isEmpty());

        QExtendedInformation info = m_fileinfo_gatherer.getInfo(updates.at(i).second);
        bool previouslyHere = parentNode->children.contains(fileName);

        if (!previouslyHere)
        {
            addNode(parentNode, fileName, info.fileInfo());
        }

        shared_files_tree::QFileSystemNode * node = parentNode->children.value(fileName);
        bool isCaseSensitive = parentNode->caseSensitive();

        if (isCaseSensitive)
        {
            if (node->fileName != fileName)
                continue;
        }
        else
        {
            if (QString::compare(node->fileName, fileName, Qt::CaseInsensitive) != 0)
                continue;
        }

        if (isCaseSensitive)
        {
            Q_ASSERT(node->fileName == fileName);
        }
        else
        {
            node->fileName = fileName;
        }

        if (info.size() == -1 && !info.isSymLink())
        {
            removeNode(parentNode, fileName);
            continue;
        }

        // update node
        if (*node != info )
        {
            if (nMin == -1)
            {
                nMin = node->m_order;
            }
            else if (nMin > node->m_order)
            {
                nMin = node->m_order;
            }

            if (nMax == -1)
            {
                nMax = node->m_order;
            }
            else if (nMax < node->m_order)
            {
                nMax = node->m_order;
            }

            node->populate(info);        
        }
    }

    if (nMin != -1 && nMax != -1)
    {
        // TODO emit data changed signal
    }
}


shared_files_tree::QFileSystemNode* shared_files_tree::node(const QString& path, bool fetch)
{
    qDebug() << "get node for: " << path;

    if (path.isEmpty() || path == myComputer() || path.startsWith(QLatin1Char(':')))
        return const_cast<QFileSystemNode*>(&m_root);

    // Construct the nodes up to the new root path if they need to be built
    QString absolutePath;
#ifdef Q_OS_WIN32
    QString longPath = qt_GetLongPathName(path);
#else
    QString longPath = path;
#endif

    if (longPath == m_rootDir.path())
        absolutePath = m_rootDir.absolutePath();
    else
        absolutePath = QDir(longPath).absolutePath();

    // ### TODO can we use bool QAbstractFileEngine::caseSensitive() const?
    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);

    if ((pathElements.isEmpty())
#if (!defined(Q_OS_WIN) || defined(Q_OS_WINCE)) && !defined(Q_OS_SYMBIAN)
        && QDir::fromNativeSeparators(longPath) != QLatin1String("/")
#endif
        )
        return const_cast<QFileSystemNode*>(&m_root);

    /*
    QModelIndex index = QModelIndex(); // start with "My Computer"
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    if (absolutePath.startsWith(QLatin1String("//"))) { // UNC path
        QString host = QLatin1String("\\\\") + pathElements.first();
        if (absolutePath == QDir::fromNativeSeparators(host))
            absolutePath.append(QLatin1Char('/'));
        if (longPath.endsWith(QLatin1Char('/')) && !absolutePath.endsWith(QLatin1Char('/')))
            absolutePath.append(QLatin1Char('/'));
        int r = 0;
        QFileSystemModelPrivate::QFileSystemNode *rootNode = const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&root);
        if (!root.children.contains(host.toLower())) {
            if (pathElements.count() == 1 && !absolutePath.endsWith(QLatin1Char('/')))
                return rootNode;
            QFileInfo info(host);
            if (!info.exists())
                return rootNode;
            QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate*>(this);
            p->addNode(rootNode, host,info);
            p->addVisibleFiles(rootNode, QStringList(host));
        }
        r = rootNode->visibleLocation(host);
        r = translateVisibleLocation(rootNode, r);
        index = q->index(r, 0, QModelIndex());
        pathElements.pop_front();
    } else
#endif
*/

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

    QFileSystemNode *parent = &m_root;
    qDebug() << pathElements;

    for (int i = 0; i < pathElements.count(); ++i)
    {
        QString element = pathElements.at(i);
#ifdef Q_OS_WIN
        // On Windows, "filename......." and "filename" are equivalent Task #133928
        while (element.endsWith(QLatin1Char('.')))
            element.chop(1);
#endif
        bool alreadyExisted = parent->children.contains(element);

        // we couldn't find the path element, we create a new node since we
        // _know_ that the path is valid
        if (alreadyExisted)
        {
            if ((parent->children.count() == 0)
                    || (parent->caseSensitive() && parent->children.value(element)->fileName != element)
                || (!parent->caseSensitive() && parent->children.value(element)->fileName.toLower() != element.toLower()))
                alreadyExisted = false;
        }

        QFileSystemNode *node;

        if (!alreadyExisted)
        {
            // Someone might call ::index("file://cookie/monster/doesn't/like/veggies"),
            // a path that doesn't exists, I.E. don't blindly create directories.
            QFileInfo info(absolutePath);
            if (!info.exists())
                return const_cast<QFileSystemNode*>(&m_root);

            node = addNode(parent, element, info);
#ifndef QT_NO_FILESYSTEMWATCHER
            node->populate(m_fileinfo_gatherer.getInfo(info));
#endif
        }
        else
        {
            node = parent->children.value(element);
        }

        Q_ASSERT(node);        

        // when current node hasn't information - get parent directory and fetch info
        if (!node->hasInformation() && fetch)
        {
            qDebug() << "fetch info {" << node->fileName << "}";
            QString dir = filePath(parent);
            Fetching f;
            f.dir = dir;
            f.file = element;
            f.node = node;
            toFetch.append(f);
            m_ftm.start(0, const_cast<shared_files_tree*>(this));
        }

        /*
        if (!node->isVisible)
        {
            // It has been filtered out
            if (alreadyExisted && node->hasInformation() && !fetch)
                return const_cast<QFileSystemModelPrivate::QFileSystemNode*>(&root);

            QFileSystemModelPrivate *p = const_cast<QFileSystemModelPrivate*>(this);
            p->addVisibleFiles(parent, QStringList(element));

            if (!p->bypassFilters.contains(node))
                p->bypassFilters[node] = 1;

            QString dir = q->filePath(this->index(parent));

            if (!node->hasInformation() && fetch)
            {
                Fetching f;
                f.dir = dir;
                f.file = element;
                f.node = node;
                p->toFetch.append(f);
                p->fetchingTimer.start(0, const_cast<QFileSystemModel*>(q));
            }
        }
        */
        parent = node;
    }

    return parent;
}

void shared_files_tree::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_ftm.timerId())
    {
        qDebug() << "timer event, targets: " << toFetch.count();
        m_ftm.stop();
    #ifndef QT_NO_FILESYSTEMWATCHER
        for (int i = 0; i < toFetch.count(); ++i)
        {
            const QFileSystemNode *node = toFetch.at(i).node;

            if (!node->hasInformation())
            {
                m_fileinfo_gatherer.fetchExtendedInformation(toFetch.at(i).dir,
                                                 QStringList(toFetch.at(i).file));
            }
            else
            {
                qDebug() << "yah!, you saved a little gerbil soul";
            }
        }
    #endif
        toFetch.clear();
    }
}

shared_files_tree::QFileSystemNode* shared_files_tree::node(QFileSystemNode* parent, int index)
{
    QFileSystemNode* node = rootNode();
    foreach(QFileSystemNode* p, parent->children)
    {
        if (p->m_order == index)
        {
            node = p;
            break;
        }
    }

    return (node);
}

void shared_files_tree::removeNode(QFileSystemNode *parentNode, const QString& name)
{
    QFileSystemNode* pNode = parentNode->children.take(name);
    Q_ASSERT(pNode);
    unsigned int order = pNode->m_order;

    foreach(QFileSystemNode* pn, parentNode->children)
    {
        pn->updateOrder(order);
    }

    delete pNode;
}

shared_files_tree::QFileSystemNode* shared_files_tree::addNode(QFileSystemNode *parentNode, const QString &fileName, const QFileInfo &info)
{
    qDebug() << parentNode->fileName << " add node {" << fileName << "}";
    // In the common case, itemLocation == count() so check there first
    QFileSystemNode *node = new QFileSystemNode(parentNode->children.size(), fileName, parentNode);
#ifndef QT_NO_FILESYSTEMWATCHER
    node->populate(info);
#endif

#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    //The parentNode is "" so we are listing the drives
    if (parentNode->fileName.isEmpty())
    {
        wchar_t name[MAX_PATH + 1];
        //GetVolumeInformation requires to add trailing backslash
        const QString nodeName = fileName + QLatin1String("\\");
        BOOL success = ::GetVolumeInformation((wchar_t *)(nodeName.utf16()),
                name, MAX_PATH + 1, NULL, 0, NULL, NULL, 0);
        if (success && name[0])
            node->volumeName = QString::fromWCharArray(name);
    }
#endif

    parentNode->children.insert(fileName, node);
    return node;
}

QString shared_files_tree::filePath(QFileSystemNode *node) const
{
    QStringList path;
    QFileSystemNode* parent = node;

    while(parent)
    {
        path.prepend(parent->fileName);
        parent = parent->parent;
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
