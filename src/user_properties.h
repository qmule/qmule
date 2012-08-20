#ifndef USER_PROPERTIES_H
#define USER_PROPERTIES_H

#include <QDialog>
#include "ui_user_properties.h"
#include "qtlibed2k/qed2ksession.h"

class user_properties : public QDialog, public Ui::user_properties
{
    Q_OBJECT

    libed2k::net_identifier netPoint;
public:
    user_properties(QWidget *parent, const QString& name, const libed2k::net_identifier& np);
    ~user_properties();
    void setPeerOptions();

private slots:
    void onCloseButton();
    void peerConnected(const libed2k::net_identifier& np, const QString&, bool bActive);
};

#endif // USER_PROPERTIES_H
