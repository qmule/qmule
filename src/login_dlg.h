#ifndef LOGIN_DLG_H
#define LOGIN_DLG_H

#include <QDialog>
#include "ui_login_dlg.h"

class login_dlg : public QDialog, public Ui::login_dlg
{
    Q_OBJECT

public:
    login_dlg(QWidget *parent = 0);
    ~login_dlg();
    QString getLogin() { return strLogin; }
    void setLogin(QString login);
    QString getPasswd() { return strPassword; }

private:
    QString strLogin;
    QString strPassword;

private slots:
    void onAccept();
    void onReject();
};

#endif // LOGIN_DLG_H
