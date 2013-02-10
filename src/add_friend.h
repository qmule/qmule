#ifndef ADD_FRIEND_H
#define ADD_FRIEND_H

#include <QDialog>
#include "ui_add_friend.h"

class add_friend : public QDialog, public Ui::add_friend
{
    Q_OBJECT

public:
    add_friend(QWidget *parent = 0);
    ~add_friend();

    QString getName() { return editName->text(); }
    QString getIP()   { return editIP->text(); }
    int getPort() { return spinPort->value(); }

private slots:
    void onAccept();
    void onReject();
};

#endif // ADD_FRIEND_H
