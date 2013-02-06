#ifndef __WGETTER__
#define __WGETTER__

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class wgetter : public QObject
{
    Q_OBJECT
private:
    QString m_dst;
    QFile::FileError       m_filesystem_error;
    QScopedPointer<QFile>  m_file;
    QNetworkReply*         m_get_reply;
    QNetworkAccessManager  m_nm;
public:
    wgetter(const QString& url, const QString& dst);
private slots:
    void on_data_finished();
    void on_data_ready();
};

#endif
