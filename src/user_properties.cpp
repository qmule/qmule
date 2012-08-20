#include "user_properties.h"
#include "libed2k/util.hpp"
#include "qed2kpeerhandle.h"
#include "transport/session.h"

user_properties::user_properties(QWidget *parent, const QString& name, const libed2k::net_identifier& np)
    : QDialog(parent)
{
    setupUi(this);

    netPoint = np;
    labelName->setText(name);
    labelIP->setText(QString::fromStdString(libed2k::int2ipstr(np.m_nIP)));
    labelPort->setText(QString::number(np.m_nPort));

    setPeerOptions();

    connect(btnClose, SIGNAL(clicked()), this, SLOT(accept()));
    connect(Session::instance()->get_ed2k_session(),
		SIGNAL(peerConnected(const libed2k::net_identifier&, const QString&, bool)),
        this, SLOT(peerConnected(const libed2k::net_identifier&, const QString&, bool)));
}

user_properties::~user_properties()
{

}

void user_properties::onCloseButton()
{
    accept();
}

void user_properties::peerConnected(const libed2k::net_identifier& np, const QString& hash, bool bActive)
{
    if (netPoint == np)
    {
        setPeerOptions();
    }
}

void user_properties::setPeerOptions()
{
    QED2KPeerHandle peer = QED2KPeerHandle::getPeerHandle(netPoint);

    libed2k::peer_connection_options options = peer.getConnectionOptions();
    if (options.m_strModVersion.length())
        labelProgram->setText(QString::fromStdString(options.m_strModVersion));

    libed2k::md4_hash hash = peer.getHash();
    libed2k::md4_hash empty_hash;
    if (hash != empty_hash)
        labelHash->setText(QString::fromStdString(hash.toString()));
}