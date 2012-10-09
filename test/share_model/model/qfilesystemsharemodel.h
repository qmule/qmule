#ifndef QFILESYSTEMSHAREMODEL_H
#define QFILESYSTEMSHAREMODEL_H

#include <QAbstractItemModel>
#include <QString>
#include <QList>
#include <QHash>
#include <QDateTime>
#include <QFile>
#include <QIcon>
#include <QBasicTimer>

#include "qfileinfogatherer.h"

class QFileSystemShareModel : public QAbstractItemModel
{
public:
    class QFileSystemNode
    {
    public:
        QFileSystemNode(const QString &filename = QString(), QFileSystemNode *p = 0)
            : fileName(filename), populatedChildren(false), isVisible(false), dirtyChildrenIndex(-1), parent(p), info(0) {}
        ~QFileSystemNode()
        {
            QHash<QString, QFileSystemNode*>::const_iterator i = children.constBegin();
            qDeleteAll(children);
            while (i != children.constEnd())
            {
                delete i.value();
                ++i;
            }

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

        // children shouldn't normally be accessed directly, use node()
        inline int visibleLocation(QString childName)
        {
            return visibleChildren.indexOf(childName);
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

        bool populatedChildren;
        bool isVisible;
        QHash<QString,QFileSystemNode *> children;
        QList<QString> visibleChildren;
        int dirtyChildrenIndex;
        QFileSystemNode *parent;
        QExtendedInformation *info;
    };

    QFileSystemShareModel();

private:

    QFileSystemNode root;
    QBasicTimer fetchingTimer;
    struct Fetching
    {
        QString dir;
        QString file;
        const QFileSystemNode *node;
    };
};

#endif // QFILESYSTEMSHAREMODEL_H
