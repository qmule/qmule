#include <QPushButton>

#include "login_dlg.h"

login_dlg::login_dlg(QWidget *parent, QString login, QString password)
    : QDialog(parent), strLogin(login) , strPassword(password)
{
    setupUi(this);

    connect(edtUsername, SIGNAL(textChanged(const QString &)), this, SLOT(checkEmptyFields(const QString &)));
    connect(edtPasswd, SIGNAL(textChanged(const QString &)), this, SLOT(checkEmptyFields(const QString &)));

    edtUsername->setText(strLogin);
    edtPasswd->setText(strPassword);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(onReject()));

    checkEmptyFields("");
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

void login_dlg::checkEmptyFields(const QString & text)
{
    if (!edtUsername->text().length() || !edtPasswd->text().length())
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}