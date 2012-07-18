#include <iostream>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <shlobj.h>
#include <windows.h>
#include <PowrProf.h>
#include <libed2k/is_crypto.hpp>
#include "windows.h"
#include "Shlobj.h"

QString getUserIDString()
{
  QString uid = "0";
#ifdef Q_WS_WIN
  char buffer[256+1] = {0};
  DWORD buffer_len = 256 + 1;
  if (GetUserNameA(buffer, &buffer_len))
  {
        uid = QString::fromLocal8Bit(buffer,  buffer_len);
  }

  qDebug() << "err " << GetLastError() <<  " len " << buffer_len;
#else
  uid = QString::number(getuid());
#endif
  return uid;
}

//QString emuleConfig(const QString& filename)
//{
//    return QDir::home().filePath(QString("config") + QDir::separator() + filename);
//}

QString ShellGetFolderPath(int iCSIDL)
{
    QString str;
    TCHAR szPath[MAX_PATH];

    if ( SHGetFolderPath(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, szPath) == S_OK )
        str = QString::fromWCharArray(szPath);

    return str;
}

QString emuleConfig(const QString& filename)
{
    QString res;
    static QList<QDir> dl = QList<QDir>()
            << QDir(ShellGetFolderPath(CSIDL_LOCAL_APPDATA)).filePath("eMule IS Mod\\config")
            << QDir(ShellGetFolderPath(CSIDL_APPDATA)).filePath("eMule IS Mod\\config")
            << QDir(ShellGetFolderPath(CSIDL_PERSONAL)).filePath("eMule IS Mod\\config");

    QList<QDir>::iterator itr = std::find_if(dl.begin(), dl.end(), std::mem_fun_ref(static_cast<bool (QDir::*)() const>(&QDir::exists)));

    if (itr != dl.end())
    {
        qDebug() << itr->path();
        res = (*itr).filePath(filename);
    }

    qDebug() << "emule config " << res;
    return res;
}

QString emuleKeyFile()
{
    QString filename = emuleConfig(getUserIDString() + QString(".rnd"));

    qDebug() << "f::  " << filename;
    if (QFile::exists(filename))
    {
        return (filename);
    }

    return (QString());
}

QStringList getFileLines(const QString& filename)
{
    QStringList slist;
    QFile textFile(filename);

    if (!textFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return slist;
    }


    QTextStream textStream(&textFile);
    textStream.setCodec("UTF-8");
    textStream.setAutoDetectUnicode(true);

    while (true)
    {
        QString line = textStream.readLine();
        if (line.isNull()) break;
        slist.append(line);
    }

    return slist;
}

QStringList emuleSharedFiles()
{
    return getFileLines(emuleConfig("sharedfiles.dat"));
}

QStringList emuleSharedDirs()
{
    return getFileLines(emuleConfig("shareddir.dat"));
}

typedef QMap<QString, QList<QString> > shared_entry;

shared_entry getSharedFiles()
{
    shared_entry se;
    QStringList minus_f =
            emuleSharedFiles().filter(QRegExp("^-")).replaceInStrings(QRegExp("^-"), "");
    QDir dir;

    foreach(dir, emuleSharedDirs())
    {
        shared_entry::iterator itr = se.insert(dir.path(), QList<QString>());
        QFileInfo fi;

        foreach(fi, minus_f)
        {
            qDebug() << "dir: " << dir.path() << " : " << fi.dir().path();
            if (fi.exists() && (dir == fi.dir()))
            {
                itr.value().append(fi.fileName());
            }
        }
    }

    return se;
}

void saveSharedDirs(const shared_entry& se)
{
    QSettings settings("d:\\1.ini", QSettings::IniFormat);

    int index = 0;

    settings.beginWriteArray("Preferences/eDonkey/SharedDirectories");
    settings.remove("Preferences/eDonkey/SharedDirectories");
    for (shared_entry::iterator itr = se.begin(); itr != se.end(); ++itr)
    {
        settings.setArrayIndex(index);
        settings.setValue("SD", itr.key());
        ++index;
    }

    settings.endArray();

    // generate directory excludes
    for (shared_entry::iterator itr = se.begin(); itr != se.end(); ++itr)
    {
        if (!itr.value().empty())
        {
            QString key = itr.key();
            key.remove(QRegExp("[:\\/]"));

            if (!key.isEmpty())
            {
                index = 0;
                settings.remove(QString("Preferences/eDonkey/SharedDirectoryKeys/" + key));
                settings.beginWriteArray(QString("Preferences/eDonkey/SharedDirectoryKeys/" + key));

                for (int n = 0; n < itr.value().size(); ++n)
                {
                    settings.setArrayIndex(n);
                    settings.setValue("ExcludeFile", itr.value().at(n));
                }

                settings.endArray();
            }
        }
    }
}

shared_entry loadSharedDirs()
{
    shared_entry se;
    QSettings settings("d:\\1.ini", QSettings::IniFormat);
    int size = settings.beginReadArray("Preferences/eDonkey/SharedDirectories");

    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        shared_entry::iterator itr = se.insert(settings.value("SD").toString(), QList<QString>());
    }

    settings.endArray();

    // restore exclude files for each directory
    for (shared_entry::iterator itr = se.begin(); itr != se.end(); ++itr)
    {
        QString key = itr.key();
        key.remove(QRegExp("[:\\/]"));

        size = settings.beginReadArray(QString("Preferences/eDonkey/SharedDirectoryKeys/" + key));

        for (int i = 0; i < size; ++i)
        {
            settings.setArrayIndex(i);
            itr.value().append(settings.value("ExcludeFile").toString());
        }

        settings.endArray();
    }

    return se;
}

QString emuleIncomingDir()
{
    QStringList sl = getFileLines(emuleConfig("preferences.ini")).filter(QRegExp("^IncomingDir"));

    if (sl.empty())
    {
        return QString();
    }

    QStringList sres = sl.at(0).split(QRegExp("="));

    if (sres.size() > 1)
    {
        return sres[1];
    }

    return QString();
}

int emulePort()
{
    QSettings qs(QDir::home().filePath(emuleConfig("preferences.ini")), QSettings::IniFormat);
    return qs.value("eMule/Port", 4668).toInt();
}

QString emuleNick()
{
    QSettings qs(QDir::home().filePath(emuleConfig("preferences.ini")), QSettings::IniFormat);
    return qs.value("eMule/Nick", QString("")).toString();
}

QString emuleAuthLogin()
{
    QSettings qs(QString("HKEY_CURRENT_USER\\Software\\eMule IS Mod"), QSettings::NativeFormat);
    return qs.value("AuthLogin", QString("")).toString();
}

QString emuleAuthPassword()
{
    QSettings qs(QString("HKEY_CURRENT_USER\\Software\\eMule IS Mod"), QSettings::NativeFormat);
    return qs.value("AuthPassword", QString("")).toString();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug("Home dir is: %s", qPrintable(QDir::homePath().toAscii()));

    qDebug() << QDir::home().filePath(QString("config") + QDir::separator() + QString("preferences.ini"));
    if (QDir::home().exists(QString("config") + QDir::separator() + QString("preferences.ini")))
    {
        qDebug() << "exists" ;
    }

    qDebug() << "nick: " << emuleNick()
             << " idir: " << emuleIncomingDir()
             << " port: " << emulePort()
             << " auth login: " << emuleAuthLogin()
             << " auth pwd: " << emuleAuthPassword()
             << "key " << emuleKeyFile();

    qDebug() << emuleSharedFiles();
    qDebug() << emuleSharedDirs();

    QString qPwd = QString::fromStdString(is_crypto::DecryptPasswd(emuleAuthPassword().toStdString(), emuleKeyFile().toStdString()));
    //QString qEPwd = QString::fromStdString(is_crypto::EncryptPasswd(qPwd.toStdString(), emuleKeyFile().toStdString()));

    qDebug() << "PAsswd " << qPwd << " enc " << qEPwd;
    qDebug() << " ==== shared files ======";
    qDebug() << getSharedFiles();

    qDebug() << "===== sf ================";
    qDebug() << emuleSharedFiles().filter(QRegExp("^[^-]"));
    saveSharedDirs(getSharedFiles());
    qDebug() << " ==== load shared dirs ==== ";
    qDebug() << loadSharedDirs();
    qDebug() << "CSIDL_LOCAL_APPDATA " << ShellGetFolderPath(CSIDL_LOCAL_APPDATA);
    qDebug() << "CSIDL_LOCAL_APPDATA " << ShellGetFolderPath(CSIDL_APPDATA);
    qDebug() << "CSIDL_LOCAL_APPDATA " << ShellGetFolderPath(CSIDL_PERSONAL);

    return a.exec();
}
