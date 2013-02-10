#ifndef SILENT_UPDATER_H
#define SILENT_UPDATER_H

#include <QObject>
#include <QTimer>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

/**
  * this class provide silent program update directly in program catalog
  * 1. check last program version
  * 2. download new version
  * 3. install new version
  * Update will execute only one time - when we have no errors on download process
 */
class silent_updater : public QObject
{
    Q_OBJECT
public:
    enum { update_timeout = 3600000 };
    explicit silent_updater(int major, int minor, int update, int build, QObject *parent = 0);
    ~silent_updater();
    void start();
private:
    bool                    m_download_aborted;
    bool                    m_started;
    int                     m_major;
    int                     m_minor;
    int                     m_update;
    int                     m_build;
    QNetworkReply*          m_reply;        //!< download reply
    QNetworkReply*          m_update_reply; //!< update reply
    QScopedPointer<QTimer>  m_check_tm;
    QScopedPointer<QNetworkAccessManager> m_nm;
    QScopedPointer<QFile>                 m_file;
    QFile::FileError        m_filesystem_error;
    
signals:
    /**
      * @params int - major, minor, update, build for new version
    */
    void new_version_ready(int, int, int, int);
    void current_version_is_obsolete(int, int, int, int);
private slots:
    void on_check_updates();
    void on_update_check_finished();
    void on_data_finished();
    void on_data_ready();

};

#endif // SILENT_UPDATER_H
