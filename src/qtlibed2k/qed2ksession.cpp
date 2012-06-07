#include <libed2k/file.hpp>
#include <libed2k/md4_hash.hpp>
#include <libed2k/search.hpp>
#include <libed2k/error_code.hpp>
#include "qed2ksession.h"
#include "libed2k/alert_types.hpp"

#include <QMessageBox>

using namespace libed2k;

/* Converts a QString hash into a  libed2k md4_hash */
static libed2k::md4_hash QStringToMD4(const QString& s)
{
    Q_ASSERT(s.length() == libed2k::md4_hash::hash_size*2);
    return libed2k::md4_hash(s.toStdString());
}

static QString md4toQString(const libed2k::md4_hash& hash)
{
	return QString::fromAscii(hash.toString().c_str(), hash.toString().size());
}

QED2KSearchResultEntry::QED2KSearchResultEntry()
{

}

QED2KSearchResultEntry::QED2KSearchResultEntry(const libed2k::shared_file_entry& sf)
{
    m_hFile = md4toQString(sf.m_hFile);
    m_network_point = sf.m_network_point;

	try
	{
		for (size_t n = 0; n < sf.m_list.count(); n++)
		{
			boost::shared_ptr<libed2k::base_tag> ptag = sf.m_list[n];

			switch(ptag->getNameId())
			{

			case libed2k::FT_FILENAME:
				m_strFilename = QString::fromUtf8(ptag->asString().c_str(), ptag->asString().size());
				break;
			case libed2k::FT_FILESIZE:
				m_nFilesize = ptag->asInt();
				break;
			case libed2k::FT_SOURCES:
				m_nSources = ptag->asInt();
				break;
			case libed2k::FT_COMPLETE_SOURCES:
				m_nCompleteSources = ptag->asInt();
				break;
			case libed2k::FT_MEDIA_BITRATE:
				m_nMediaBitrate = ptag->asInt();
				break;
			case libed2k::FT_MEDIA_CODEC:
				m_strMediaCodec = QString::fromUtf8(ptag->asString().c_str(), ptag->asString().size());
				break;
			case libed2k::FT_MEDIA_LENGTH:
				m_nMediaLength = ptag->asInt();
				break;
			default:
			    break;
			}
		}

		if (m_nMediaLength == 0)
		{
            if (boost::shared_ptr<libed2k::base_tag> p = sf.m_list.getTagByName(libed2k::FT_ED2K_MEDIA_LENGTH))
            {
                m_nMediaLength = p->asInt();
            }
		}

		if (m_nMediaBitrate == 0)
		{
		    if (boost::shared_ptr<libed2k::base_tag> p = sf.m_list.getTagByName(libed2k::FT_ED2K_MEDIA_BITRATE))
            {
                m_nMediaLength = p->asInt();
            }
		}

		// for users
		// m_nMediaLength  - low part of real size
		// m_nMediaBitrate - high part of real size
	}
	catch(libed2k::libed2k_exception& e)
	{
		qDebug("%s", e.what());
	}
}

bool QED2KSearchResultEntry::isCorrect() const
{
	return (m_hFile.size() == libed2k::MD4_HASH_SIZE*2 && !m_strFilename.isEmpty());
}

QED2KPeerOptions::QED2KPeerOptions(const libed2k::misc_options& mo, const libed2k::misc_options2& mo2)
{
    m_nAICHVersion      = static_cast<quint8>(mo.m_nAICHVersion);
    m_bUnicodeSupport   = mo.m_nUnicodeSupport;
    m_nUDPVer           = static_cast<quint8>(mo.m_nUDPVer);
    m_nDataCompVer      = static_cast<quint8>(mo.m_nDataCompVer);
    m_nSupportSecIdent  = static_cast<quint8>(mo.m_nSupportSecIdent);
    m_nSourceExchange1Ver = static_cast<quint8>(mo.m_nSourceExchange1Ver);
    m_nExtendedRequestsVer= static_cast<quint8>(mo.m_nExtendedRequestsVer);
    m_nAcceptCommentVer = static_cast<quint8>(mo.m_nAcceptCommentVer);
    m_bNoViewSharedFiles= mo.m_nNoViewSharedFiles;
    m_bMultiPacket      = mo.m_nMultiPacket;
    m_bSupportsPreview  = mo.m_nSupportsPreview;

    m_bSupportCaptcha   = mo2.support_captcha();
    m_bSourceExt2       = mo2.support_source_ext2();
    m_bExtMultipacket   = mo2.support_ext_multipacket();
    m_bLargeFiles       = mo2.support_large_files();
}

QED2KSession::QED2KSession()
{
	m_alerts_timer.reset(new QTimer(this));
	m_settings.server_hostname = "che-d-113.rocketsoftware.com";
	m_session.reset(new libed2k::session(m_finger, "0.0.0.0", m_settings));
    m_session->set_alert_mask(alert::all_categories);

	connect(m_alerts_timer.data(), SIGNAL(timeout()), SLOT(readAlerts()));
    m_alerts_timer->start(500);
}

QED2KSession::~QED2KSession()
{
}

Transfer QED2KSession::getTransfer(const QString &hash) const
{
    return QED2KHandle(m_session->find_transfer(libed2k::md4_hash(hash.toStdString())));
}

std::vector<Transfer> QED2KSession::getTransfers() const
{
    std::vector<libed2k::transfer_handle> handles = m_session->get_transfers();
    std::vector<Transfer> transfers;

    for (std::vector<libed2k::transfer_handle>::iterator i = handles.begin();
         i != handles.end(); ++i)
        transfers.push_back(QED2KHandle(*i));

    return transfers;
}

qlonglong QED2KSession::getETA(const QString& hash) const { return 0; }
qreal QED2KSession::getRealRatio(const QString& hash) const { return 0; }
qreal QED2KSession::getMaxRatioPerTransfer(const QString& hash, bool* use_global) const { return 0; }
bool QED2KSession::isFilePreviewPossible(const QString& hash) const { return false; }
void QED2KSession::changeLabelInSavePath(
    const Transfer& t, const QString& old_label,const QString& new_label) {}
void QED2KSession::pauseTransfer(const QString& hash) {}
void QED2KSession::resumeTransfer(const QString& hash) {}
void QED2KSession::deleteTransfer(const QString& hash, bool delete_files) {}
void QED2KSession::recheckTransfer(const QString& hash) {}
void QED2KSession::setDownloadLimit(const QString& hash, long limit) {}
void QED2KSession::setUploadLimit(const QString& hash, long limit) {}
void QED2KSession::setMaxRatioPerTransfer(const QString& hash, qreal ratio){}
void QED2KSession::removeRatioPerTransfer(const QString& hash) {}
void QED2KSession::banIP(QString ip) {}
QHash<QString, TrackerInfos> QED2KSession::getTrackersInfo(const QString &hash) const{ 
    return QHash<QString, TrackerInfos>();
}
void QED2KSession::setDownloadRateLimit(long rate) {}
void QED2KSession::setUploadRateLimit(long rate) {}
bool QED2KSession::hasActiveTransfers() const { return false; }
void QED2KSession::startUpTransfers() {}
void QED2KSession::configureSession() {}
void QED2KSession::enableIPFilter(const QString &filter_path, bool force /*=false*/) {}
libed2k::session* QED2KSession::delegate() const { return m_session.data(); }

void QED2KSession::searchFiles(const QString& strQuery,
        quint64 nMinSize,
        quint64 nMaxSize,
        unsigned int nSources,
        unsigned int nCompleteSources,
        QString strFileType,
        QString strFileExt,
        QString strMediaCodec,
        quint32 nMediaLength,
        quint32 nMediaBitrate)
{
	try
	{
		libed2k::search_request sr = libed2k::generateSearchRequest(nMinSize, nMaxSize, nSources, nCompleteSources,
            strFileType.toUtf8().constData(),
            strFileExt.toUtf8().constData(),
            strMediaCodec.toUtf8().constData(),
            nMediaLength,
            nMediaBitrate,
            strQuery.toUtf8().constData());

		m_session->post_search_request(sr);
	}
	catch(libed2k::libed2k_exception& e)
	{
		QMessageBox msgBox;
        msgBox.setText(e.what());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
	}
}

void QED2KSession::searchRelatedFiles(QString strHash)
{
    libed2k::search_request sr = (generateSearchRequest(QStringToMD4(strHash)));
    m_session->post_search_request(sr);
}

void QED2KSession::searchMoreResults()
{
    m_session->post_search_more_result_request();
}

void QED2KSession::initializePeer(const libed2k::net_identifier& np)
{
    libed2k::peer_connection_handle pch = m_session->find_peer_connection(np);

    if (pch.empty())
    {
        pch = m_session->add_peer_connection(np);
    }
}

void QED2KSession::sendMessageToPeer(const libed2k::net_identifier& np, const QString& strMessage)
{
    libed2k::peer_connection_handle pch = m_session->find_peer_connection(np);

    if (pch.empty())
    {
        pch = m_session->add_peer_connection(np);
    }
    pch.send_message(strMessage.toUtf8().constData());
}

void QED2KSession::readAlerts()
{
    std::auto_ptr<libed2k::alert> a = m_session->pop_alert();

    while (a.get())
    {
        if (libed2k::server_name_resolved_alert* p =
            dynamic_cast<libed2k::server_name_resolved_alert*>(a.get()))
        {
            emit serverNameResolved(QString::fromUtf8(p->m_strServer.c_str(), p->m_strServer.size()));
        }
        if (libed2k::server_connection_initialized_alert* p =
            dynamic_cast<libed2k::server_connection_initialized_alert*>(a.get()))
        {
            emit serverConnectionInitialized(p->m_nClientId);
        }
        else if (libed2k::server_status_alert* p = dynamic_cast<libed2k::server_status_alert*>(a.get()))
        {
            emit serverStatus(p->m_nFilesCount, p->m_nUsersCount);
        }
        else if (libed2k::server_identity_alert* p = dynamic_cast<libed2k::server_identity_alert*>(a.get()))
        {
            emit serverIdentity(QString::fromUtf8(p->m_strName.c_str(), p->m_strName.size()),
                                QString::fromUtf8(p->m_strDescr.c_str(), p->m_strDescr.size()));
        }
        else if (libed2k::server_message_alert* p = dynamic_cast<libed2k::server_message_alert*>(a.get()))
        {
            emit serverMessage(QString::fromUtf8(p->m_strMessage.c_str(), p->m_strMessage.size()));
        }
        else if (libed2k::server_connection_closed* p =
                 dynamic_cast<libed2k::server_connection_closed*>(a.get()))
        {
        	qDebug("server connection closed");
        	emit serverConnectionClosed(QString::fromStdString(p->m_error.message()));
        }
        else if (libed2k::shared_files_alert* p = dynamic_cast<libed2k::shared_files_alert*>(a.get()))
        {
        	std::vector<QED2KSearchResultEntry> vRes;
        	vRes.resize(p->m_files.m_collection.size());
        	bool bMoreResult = p->m_more;

        	for (size_t n = 0; n < p->m_files.m_collection.size(); ++n)
        	{
        		QED2KSearchResultEntry sre(p->m_files.m_collection[n]);

        		if (sre.isCorrect())
        		{
        			vRes[n] = sre;
        		}
        	}

        	// emit special signal for derived class
        	if (libed2k::shared_directory_files_alert* p2 =
                dynamic_cast<libed2k::shared_directory_files_alert*>(p))
        	{
        	    emit peerSharedDirectoryFiles(
                    p2->m_np, md4toQString(p2->m_hash),
                    QString::fromUtf8(p2->m_strDirectory.c_str(), p2->m_strDirectory.size()), vRes);
        	    continue;
        	}

        	emit searchResult(p->m_np, md4toQString(p->m_hash), vRes, bMoreResult);
        }
        else if (libed2k::mule_listen_failed_alert* p =
                 dynamic_cast<libed2k::mule_listen_failed_alert*>(a.get()))
        {

        }
        else if (libed2k::peer_connected_alert* p = dynamic_cast<libed2k::peer_connected_alert*>(a.get()))
        {
            emit peerConnected(p->m_np, md4toQString(p->m_hash), p->m_active);
        }
        else if (libed2k::peer_disconnected_alert* p = dynamic_cast<libed2k::peer_disconnected_alert*>(a.get()))
        {
            emit peerDisconnected(p->m_np, md4toQString(p->m_hash), p->m_ec);
        }
        else if (libed2k::peer_message_alert* p = dynamic_cast<libed2k::peer_message_alert*>(a.get()))
        {
            emit peerMessage(p->m_np, md4toQString(p->m_hash),
                             QString::fromUtf8(p->m_strMessage.c_str(), p->m_strMessage.size()));
        }
        else if (libed2k::peer_captcha_request_alert* p =
                 dynamic_cast<libed2k::peer_captcha_request_alert*>(a.get()))
        {
            QPixmap pm;
            pm.loadFromData((const uchar*)&p->m_captcha[0], p->m_captcha.size());
            emit peerCaptchaRequest(p->m_np, md4toQString(p->m_hash), pm);
        }
        else if (libed2k::peer_captcha_result_alert* p =
                 dynamic_cast<libed2k::peer_captcha_result_alert*>(a.get()))
        {
            emit peerCaptchaResult(p->m_np, md4toQString(p->m_hash), p->m_nResult);
        }
        else if (libed2k::shared_files_access_denied* p =
                 dynamic_cast<libed2k::shared_files_access_denied*>(a.get()))
        {
            emit peerSharedFilesAccessDenied(p->m_np, md4toQString(p->m_hash));
        }
        else if (libed2k::shared_directories_alert* p = dynamic_cast<libed2k::shared_directories_alert*>(a.get()))
        {
            QStringList qstrl;

            for (size_t n = 0; n < p->m_dirs.size(); ++n)
            {
                qstrl.append(QString::fromUtf8(p->m_dirs[n].c_str(), p->m_dirs[n].size()));
            }

            emit peerSharedDirectories(p->m_np, md4toQString(p->m_hash), qstrl);
        }
        else if (libed2k::added_transfer_alert* p =
                 dynamic_cast<libed2k::added_transfer_alert*>(a.get()))
        {
            emit addedTransfer(Transfer(QED2KHandle(p->m_handle)));
        }


        a = m_session->pop_alert();
    }
}
