#include "login_dlg.h"

login_dlg::login_dlg(QWidget *parent, QString login, QString password)
    : QDialog(parent), strLogin(login) , strPassword(password)
{
    setupUi(this);

    edtUsername->setText(strLogin);
    edtPasswd->setText(strPassword);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(onReject()));
}

login_dlg::~login_dlg()
{    
}

void login_dlg::onAccept()
{
    strLogin = edtUsername->text();
    strPassword = edtPasswd->text();
    accept();
}

void login_dlg::onReject()
{
    strLogin = "";
    strPassword = "";
    reject();
}

void login_dlg::setLogin(QString login)
{
    strLogin = login;
    edtUsername->insert(strLogin);
}