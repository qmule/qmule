#ifndef INFODLG_H
#define INFODLG_H

#include <QDialog>
#include <QTimer>
#include <QScopedPointer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "ui_infodlg.h"

class is_info_dlg : public QDialog, public Ui::info_dlg
{
    Q_OBJECT

public:
    is_info_dlg(QWidget *parent);
    void start();
    ~is_info_dlg();
private:
    bool    m_first_call;
    QScopedPointer<QTimer> m_alert;
    QScopedPointer<QNetworkAccessManager> m_query;  //!< get info from server
    QScopedPointer<QNetworkAccessManager> m_answer; //!< inform server - user had read message
private slots:
    void onAccepted();
    void onTimeout();
    void replyFinished(QNetworkReply* pReply);
};


#endif // INFODLG_H
