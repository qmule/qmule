#ifndef SHARED_FILES_TREE_H
#define SHARED_FILES_TREE_H

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
#include "qfileinfogatherer.h"

class shared_files_tree : public QObject
{
    Q_OBJECT
public:
    class QFileSystemNode
    {
    public:
        QFileSystemNode(int order = -1, const QString &filename = QString(), QFileSystemNode *p = 0)
            : fileName(filename), m_order(order), populatedChildren(false), parent(p), info(0) {}

        ~QFileSystemNode()
        {            
            qDeleteAll(children);
            delete info;
            info = 0;
            parent = 0;
        }

        QString fileName;
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
        QString volumeName;
#endif

        inline qint64 size() const { if (info && !info->isDir()) return info->size(); return 0; }
        inline QString type() const { if (info) return info->displayType; return QLatin1String(""); }
        inline QDateTime lastModified() const { if (info) return info->lastModified(); return QDateTime(); }
        inline QFile::Permissions permissions() const { if (info) return info->permissions(); return 0; }
        inline bool isReadable() const { return ((permissions() & QFile::ReadUser) != 0); }
        inline bool isWritable() const { return ((permissions() & QFile::WriteUser) != 0); }
        inline bool isExecutable() const { return ((permissions() & QFile::ExeUser) != 0); }
        inline bool isDir() const
        {
            if (info)
                return info->isDir();
            if (children.count() > 0)
                return true;
            return false;
        }

        inline bool isFile() const { if (info) return info->isFile(); return true; }
        inline bool isSystem() const { if (info) return info->isSystem(); return true; }
        inline bool isHidden() const { if (info) return info->isHidden(); return false; }
        inline bool isSymLink() const { if (info) return info->isSymLink(); return false; }
        inline bool caseSensitive() const { if (info) return info->isCaseSensitive(); return false; }
        inline QIcon icon() const { if (info) return info->icon; return QIcon(); }

        inline bool operator <(const QFileSystemNode &node) const
        {
            if (caseSensitive() || node.caseSensitive())
                return fileName < node.fileName;
            return QString::compare(fileName, node.fileName, Qt::CaseInsensitive) < 0;
        }

        inline bool operator >(const QString &name) const
        {
            if (caseSensitive())
                return fileName > name;
            return QString::compare(fileName, name, Qt::CaseInsensitive) > 0;
        }

        inline bool operator <(const QString &name) const
        {
            if (caseSensitive())
                return fileName < name;
            return QString::compare(fileName, name, Qt::CaseInsensitive) < 0;
        }

        inline bool operator !=(const QExtendedInformation &fileInfo) const
        {
            return !operator==(fileInfo);
        }

        bool operator ==(const QString &name) const
        {
            if (caseSensitive())
                return fileName == name;
            return QString::compare(fileName, name, Qt::CaseInsensitive) == 0;
        }

        bool operator ==(const QExtendedInformation &fileInfo) const
        {
            return info && (*info == fileInfo);
        }

        inline bool hasInformation() const { return info != 0; }

        void populate(const QExtendedInformation &fileInfo)
        {
            if (!info)
                info = new QExtendedInformation(fileInfo.fileInfo());
            (*info) = fileInfo;
        }

        void updateIcon(QFileIconProvider *iconProvider, const QString &path)
        {
            if (info)
                info->icon = iconProvider->icon(QFileInfo(path));
            QHash<QString, QFileSystemNode *>::const_iterator iterator;
            for(iterator = children.constBegin() ; iterator != children.constEnd() ; ++iterator) {
                //On windows the root (My computer) has no path so we don't want to add a / for nothing (e.g. /C:/)
                if (!path.isEmpty()) {
                    if (path.endsWith(QLatin1Char('/')))
                        iterator.value()->updateIcon(iconProvider, path + iterator.value()->fileName);
                    else
                        iterator.value()->updateIcon(iconProvider, path + QLatin1Char('/') + iterator.value()->fileName);
                } else
                    iterator.value()->updateIcon(iconProvider, iterator.value()->fileName);
            }
        }

        void retranslateStrings(QFileIconProvider *iconProvider, const QString &path)
        {
            if (info)
                info->displayType = iconProvider->type(QFileInfo(path));
            QHash<QString, QFileSystemNode *>::const_iterator iterator;
            for(iterator = children.constBegin() ; iterator != children.constEnd() ; ++iterator) {
                //On windows the root (My computer) has no path so we don't want to add a / for nothing (e.g. /C:/)
                if (!path.isEmpty()) {
                    if (path.endsWith(QLatin1Char('/')))
                        iterator.value()->retranslateStrings(iconProvider, path + iterator.value()->fileName);
                    else
                        iterator.value()->retranslateStrings(iconProvider, path + QLatin1Char('/') + iterator.value()->fileName);
                } else
                    iterator.value()->retranslateStrings(iconProvider, iterator.value()->fileName);
            }
        }

        /**
          * compare current node order and deleted node order
          * when current node order great this node moved one position down
         */
        void updateOrder(int order)
        {
            if (m_order >= order)
            {
                --m_order;
            }

            Q_ASSERT(m_order > -1);
        }

        int     m_order;
        bool    populatedChildren;
        QHash<QString, QFileSystemNode *> children;        
        QFileSystemNode *parent;
        QExtendedInformation *info;
    };
public:
    explicit shared_files_tree(QObject *parent = 0);

    /**
      * generate tree nodes from root last node in path
     */
    QFileSystemNode* node(const QString& path, bool fetch);

    /**
      * return root node
     */
    QFileSystemNode* rootNode() { return &m_root; }

    /**
      * search in children by index
     */
    QFileSystemNode* node(QFileSystemNode* parent, int index);

    /**
      * remove child and update indexes
     */
    void removeNode(QFileSystemNode *parentNode, const QString& name);

    QFileSystemNode* addNode(QFileSystemNode *parentNode, const QString &fileName, const QFileInfo &info);
    QString filePath(QFileSystemNode* node) const;

    inline static QString myComputer()
    {
        // ### TODO We should query the system to find out what the string should be
        // XP == "My Computer",
        // Vista == "Computer",
        // OS X == "Computer" (sometime user generated) "Benjamin's PowerBook G4"
#ifdef Q_OS_WIN
        return tr("My Computer");
#else
        return tr("Computer");
#endif
    }

private:
    QFileSystemNode m_root;
    QDir            m_rootDir;
    QFileInfoGatherer m_fileinfo_gatherer;

    QBasicTimer fetchingTimer;

    struct Fetching
    {
        QString dir;
        QString file;
        const QFileSystemNode *node;
    };

    QList<Fetching> toFetch;

    QBasicTimer m_ftm;
    void timerEvent(QTimerEvent *);
signals:
    
public slots:
private slots:
    void filesystem_changed(const QString& path, const FileInfoList& updates);
    
};

#endif // SHARED_FILES_TREE_H
