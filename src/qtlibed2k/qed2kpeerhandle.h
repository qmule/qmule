#ifndef QED2KPEERHANDLE_H
#define QED2KPEERHANDLE_H

#include <libed2k/session.hpp>

class QED2KPeerHandle
{
public:
    QED2KPeerHandle(const libed2k::peer_connection_handle& pch);
    ~QED2KPeerHandle();

    void sendMessageToPeer(const QString& strMessage);
    bool isAllowedSharedFilesView();
    void requestDirs();

    static QED2KPeerHandle getPeerHandle(const libed2k::net_identifier& np);

private:
    libed2k::peer_connection_handle m_delegate;
};

#endif // QED2KPEERHANDLE_H
