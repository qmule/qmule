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
    if (editIP->text().isEmpty() ||
            editName->text().isEmpty())
    {
        QMessageBox::warning(NULL, "qMule", tr("Empty IP or user name"), QMessageBox::Ok);
        return;
    }

    QRegExpValidator validatorIP(QRegExp("(\\d{1,3}.\\d{1,3}.\\d{1,3}.\\d{1,3})"), this);

    int pos;
    QString strIP = editIP->text();

    if (validatorIP.validate(strIP, pos) != QValidator::Acceptable)
    {
        QMessageBox::warning(NULL, "qMule", tr("Incorrect IP"), QMessageBox::Ok);
        return;
    }

    accept();
}

void add_friend::onReject()
{
    reject();
}
