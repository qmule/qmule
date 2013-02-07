#include <iostream>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QDirIterator>

#ifdef Q_WS_WIN
#include <PowrProf.h>
#include "windows.h"
#include "Shlobj.h"
#endif

#include "../../src/wgetter.h"

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

#ifndef Q_WS_WIN
const int CSIDL_LOCAL_APPDATA = 1;
const int CSIDL_APPDATA = 2;
const int CSIDL_PERSONAL = 3;
#endif

QString ShellGetFolderPath(int iCSIDL)
{
    QString str;
#ifdef Q_WS_WIN
    TCHAR szPath[MAX_PATH];

    if ( SHGetFolderPath(NULL, iCSIDL, NULL, SHGFP_TYPE_CURRENT, szPath) == S_OK )
        str = QString::fromWCharArray(szPath);
#endif
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
        res = (*itr).filePath(filename);
    }

    qDebug() << "emuleConfig result:  " << res;
    return res;
}

QString emuleKeyFile()
{
    QString filename = emuleConfig(getUserIDString() + QString(".rnd"));

    if (!QFile::exists(filename))
    {
        filename.clear();
    }

    return (filename);
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
    for (shared_entry::const_iterator itr = se.begin(); itr != se.end(); ++itr)
    {
        settings.setArrayIndex(index);
        settings.setValue("SD", itr.key());
        ++index;
    }

    settings.endArray();

    // generate directory excludes
    for (shared_entry::const_iterator itr = se.begin(); itr != se.end(); ++itr)
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

QStringList getSubDirs(const QString& strBase)
{
    QDir d(strBase);
    QStringList dlist;
    dlist << d.path();

    QDirIterator it(strBase, QDirIterator::Subdirectories);

    while(it.hasNext())
    {
        QDir d = QFileInfo(it.next()).dir();
        dlist << d.path();
    }

    dlist.removeDuplicates();

    return dlist;
}

int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);
    QStringList al = a.arguments();
    al.removeFirst();

    if (!al.filter(QRegExp("^-+help$")).isEmpty())
    {
        qDebug() << " this is help message";
    }

    qDebug() << al.filter(QRegExp("^[^--]"));

    qDebug() << "nick: " << emuleNick()
             << " idir: " << emuleIncomingDir()
             << " port: " << emulePort()
             << " auth login: " << emuleAuthLogin()
             << " auth pwd: " << emuleAuthPassword()
             << "key " << emuleKeyFile()
                << " key file " << emuleKeyFile();

    qDebug() << emuleSharedFiles();
    qDebug() << emuleSharedDirs();

    qDebug() << " ==== shared files ======";
    qDebug() << getSharedFiles();

    qDebug() << "===== sf ================";
    qDebug() << emuleSharedFiles().filter(QRegExp("^[^-]"));
    //saveSharedDirs(getSharedFiles());
    //qDebug() << " ==== load shared dirs ==== ";
    //qDebug() << loadSharedDirs();
    qDebug() << "CSIDL_LOCAL_APPDATA " << ShellGetFolderPath(CSIDL_LOCAL_APPDATA);
    qDebug() << "CSIDL_APPDATA " << ShellGetFolderPath(CSIDL_APPDATA);
    qDebug() << "CSIDL_PERSONAL " << ShellGetFolderPath(CSIDL_PERSONAL);
    QString result("FFGHsdAlex<?xml version=\"1.0\"?><DATA><AuthResult>0</AuthResult><Message type=\"1\"><![CDATA[]]></Message><filter><![CDATA[]]></filter><server>emule.is74.ru</server></DATA>< / br>");
    int pos = result.indexOf("<?xml", 0, Qt::CaseInsensitive);

    if (pos != -1)
    {
        result.remove(0, pos);
    }

    pos = result.lastIndexOf("</DATA>", -1, Qt::CaseInsensitive);

    if (pos != -1)
    {
        result.remove(pos + 7, result.length() - pos - 7);
    }

    qDebug() << result;

    QString strHTML("<html><body></body></html>");
    QRegExp re1("<body>[\n\t\s]*</body>", Qt::CaseInsensitive);

    if (strHTML.contains(re1))
    {
        qDebug() << "contains ";
    }

    QString strHTML2("<html><body>XXXdfg</body></html>");

    if (strHTML2.contains(re1))
    {
        qDebug() << "contains ";
    }
    else
    {
        qDebug() << " isn't contain";
    }

    qDebug() << getSubDirs("/home/apavlov/work/newmule");

    wgetter w("http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz", "/root/xxx.dat.gz");
    return a.exec();
}
