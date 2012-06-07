#include <QMessageBox>

#include "add_friend.h"

add_friend::add_friend(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(onReject()));
}

add_friend::~add_friend()
{

}

void add_friend::onAccept()
{
    if (!editIP->text().length() || !editPort->text().length())
    {
        bool bok;
        editPort->text().toInt(&bok);
        QRegExpValidator validatorIP(QRegExp("[0-9]{1,3}(.[0-9]{1,3}){3,3}"), this);
        int pos;
        QString strIP = editIP->text();

        if (!bok || validatorIP.validate(strIP, pos) != QValidator::Acceptable)
        {
            QMessageBox::warning(NULL, "eMule", tr("Incorrect IP and port!"), QMessageBox::Ok);
            return;
        }
    }
    else
    {
        accept();
    }
}

void add_friend::onReject()
{
    reject();
}
