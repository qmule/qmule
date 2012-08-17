#ifndef USER_PROPERTIES_H
#define USER_PROPERTIES_H

#include <QDialog>
#include "ui_user_properties.h"
#include "qtlibed2k/qed2ksession.h"

class user_properties : public QDialog, public Ui::user_properties
{
    Q_OBJECT

public:
    user_properties(QWidget *parent, const QString& name, const libed2k::net_identifier& np);
    ~user_properties();

private slots:
    void onCloseButton();
};

#endif // USER_PROPERTIES_H
