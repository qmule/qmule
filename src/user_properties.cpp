#include "user_properties.h"
#include "libed2k/util.hpp"
#include "qed2kpeerhandle.h"

user_properties::user_properties(QWidget *parent, const QString& name, const libed2k::net_identifier& np)
    : QDialog(parent)
{
    setupUi(this);

    labelName->setText(name);
    labelIP->setText(QString::fromStdString(libed2k::int2ipstr(np.m_nIP)));
    labelPort->setText(QString::number(np.m_nPort));

    QED2KPeerHandle peer = QED2KPeerHandle::getPeerHandle(np);
    labelHash->setText(QString::fromStdString(peer.getHash().toString()));

    connect(btnClose, SIGNAL(clicked()), this, SLOT(accept()));
}

user_properties::~user_properties()
{

}

void user_properties::onCloseButton()
{
    accept();
}
