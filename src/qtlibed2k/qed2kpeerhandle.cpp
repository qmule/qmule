#include <QString>

#include "qed2kpeerhandle.h"
#include "qed2ksession.h"
#include "libed2k/error_code.hpp"
#include "transport/session.h"

QED2KPeerHandle::QED2KPeerHandle(const libed2k::peer_connection_handle& pch) : m_delegate(pch)
{
}

QED2KPeerHandle::~QED2KPeerHandle()
{
}

QED2KPeerHandle QED2KPeerHandle::getPeerHandle(const libed2k::net_identifier& np)
{
    return Session::instance()->get_ed2k_session()->getPeer(np);
}

void QED2KPeerHandle::sendMessageToPeer(const QString& strMessage)
{
    try
    {
        m_delegate.send_message(strMessage.toUtf8().constData());
    }
    catch(libed2k::libed2k_exception& e)
    {
        try
        {
            libed2k::net_identifier np = m_delegate.get_network_point();
            m_delegate = Session::instance()->get_ed2k_session()->getPeer(np);
            m_delegate.send_message(strMessage.toUtf8().constData());
        }
        catch(...)
        {
        }
    }
}

bool QED2KPeerHandle::isAllowedSharedFilesView()
{
    return (m_delegate.get_misc_options().m_nNoViewSharedFiles == 0);
}

void QED2KPeerHandle::requestDirs()
{
    m_delegate.get_shared_directories();
}

QString QED2KPeerHandle::getUserName()
{
    return QString::fromStdString(m_delegate.get_options().m_strName);
}