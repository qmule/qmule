#include <fstream>
#include <iostream>
#include "qed2ksession.h"
#include <libed2k/bencode.hpp>
#include <libed2k/file.hpp>
#include <libed2k/md4_hash.hpp>
#include <libed2k/search.hpp>
#include <libed2k/error_code.hpp>
#include "libed2k/transfer_handle.hpp"
#include "libed2k/alert_types.hpp"


#include <QNetworkInterface>
#include <QMessageBox>
#include <QDir>
#include <QDirIterator>

#include "preferences.h"

using namespace libed2k;

/* Converts a QString hash into a  libed2k md4_hash */
static libed2k::md4_hash QStringToMD4(const QString& s)
{
    Q_ASSERT(s.length() == libed2k::md4_hash::hash_size*2);
    return libed2k::md4_hash::fromString(s.toStdString());
}

QString md4toQString(const libed2k::md4_hash& hash)
{
    return QString::fromAscii(hash.toString().c_str(), hash.toString().size());
}

QED2KSearchResultEntry::QED2KSearchResultEntry() :
        m_nFilesize(0),
        m_nSources(0),
        m_nCompleteSources(0),
        m_nMediaBitrate(0),
        m_nMediaLength(0)
{
}

QED2KSearchResultEntry::QED2KSearchResultEntry(const Preferences& pref)
{
    m_nFilesize = pref.value("Filesize", 0).toULongLong();
    m_nSources  = pref.value("Sources", 0).toULongLong();
    m_nCompleteSources  = pref.value("CompleteSources", 0).toULongLong();
    m_nMediaBitrate     = pref.value("MediaBitrate", 0).toULongLong();
    m_nMediaLength      = pref.value("MediaLength", 0).toULongLong();
    m_hFile             = pref.value("File", QString()).toString();
    m_strFilename       = pref.value("Filename", QString()).toString();
    m_strMediaCodec     = pref.value("MediaCodec", QString()).toString();
    m_network_point.m_nIP = pref.value("IP", 0).toUInt();
    m_network_point.m_nPort = pref.value("Port", 0).toUInt();
}

void QED2KSearchResultEntry::save(Preferences& pref) const
{
    pref.setValue("Filesize",       m_nFilesize);
    pref.setValue("Sources",        m_nSources);
    pref.setValue("CompleteSources",m_nCompleteSources);
    pref.setValue("MediaBitrate",   m_nMediaBitrate);
    pref.setValue("MediaLength",    m_nMediaLength);
    pref.setValue("File",           m_hFile);
    pref.setValue("Filename",       m_strFilename);
    pref.setValue("MediaCodec",     m_strMediaCodec);
    pref.setValue("IP",             m_network_point.m_nIP);
    pref.setValue("Port",           m_network_point.m_nPort);
}

// static
QED2KSearchResultEntry QED2KSearchResultEntry::fromSharedFileEntry(const libed2k::shared_file_entry& sf)
{
    QED2KSearchResultEntry sre;

    sre.m_hFile = md4toQString(sf.m_hFile);
    sre.m_network_point = sf.m_network_point;

    try
    {
        for (size_t n = 0; n < sf.m_list.count(); n++)
        {
            boost::shared_ptr<libed2k::base_tag> ptag = sf.m_list[n];

            switch(ptag->getNameId())
            {

            case libed2k::FT_FILENAME:
                sre.m_strFilename = QString::fromUtf8(ptag->asString().c_str(), ptag->asString().size());
                break;
            case libed2k::FT_FILESIZE:
                sre.m_nFilesize += ptag->asInt();
                break;
            case libed2k::FT_FILESIZE_HI:
            	sre.m_nFilesize += (ptag->asInt() << 32);
            	break;
            case libed2k::FT_SOURCES:
                sre.m_nSources = ptag->asInt();
                break;
            case libed2k::FT_COMPLETE_SOURCES:
                sre.m_nCompleteSources = ptag->asInt();
                break;
            case libed2k::FT_MEDIA_BITRATE:
                sre.m_nMediaBitrate = ptag->asInt();
                break;
            case libed2k::FT_MEDIA_CODEC:
                sre.m_strMediaCodec = QString::fromUtf8(ptag->asString().c_str(), ptag->asString().size());
                break;
            case libed2k::FT_MEDIA_LENGTH:
                sre.m_nMediaLength = ptag->asInt();
                break;
            default:
                break;
            }
        }

        if (sre.m_nMediaLength == 0)
        {
            if (boost::shared_ptr<libed2k::base_tag> p = sf.m_list.getTagByName(libed2k::FT_ED2K_MEDIA_LENGTH))
            {
                if (p->getType() == libed2k::TAGTYPE_STRING)
                {
                    // TODO - possible need store string length
                }
                else
                {
                    sre.m_nMediaLength = p->asInt();
                }
            }
        }

        if (sre.m_nMediaBitrate == 0)
        {
            if (boost::shared_ptr<libed2k::base_tag> p = sf.m_list.getTagByName(libed2k::FT_ED2K_MEDIA_BITRATE))
            {
                sre.m_nMediaLength = p->asInt();
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

    return (sre);
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

bool writeResumeData(const libed2k::save_resume_data_alert* p)
{
    qDebug() << Q_FUNC_INFO;
    try
    {
        QED2KHandle h(p->m_handle);
        qDebug() << "save fast resume data for " << h.hash();

        if (h.is_valid() && p->resume_data)
        {
            QDir libed2kBackup(misc::ED2KBackupLocation());
            // Remove old fastresume file if it exists
            std::vector<char> out;
            libed2k::bencode(back_inserter(out), *p->resume_data);
            const QString filepath = libed2kBackup.absoluteFilePath(h.hash() +".fastresume");
            libed2k::transfer_resume_data trd(p->m_handle.hash(), p->m_handle.save_path(), p->m_handle.name(), p->m_handle.size(), out);

            std::ofstream fs(filepath.toLocal8Bit(), std::ios_base::out | std::ios_base::binary);

            if (fs)
            {
                libed2k::archive::ed2k_oarchive oa(fs);
                oa << trd;
                return true;
            }
        }
    }
    catch(const libed2k::libed2k_exception& e)
    {
        qDebug() << "error on write resume data " << misc::toQStringU(e.what());
    }

    return false;
}

namespace aux
{

QED2KSession::QED2KSession()
{    
}

void QED2KSession::start()
{
    qDebug() <<  Q_FUNC_INFO;
    Preferences pref;
    libed2k::session_settings settings;
    libed2k::fingerprint finger;

    // set zero to port for stop automatically listening
    settings.listen_port = pref.listenPort();
    settings.server_reconnect_timeout = 20;
    settings.server_keep_alive_timeout = -1;
    settings.server_timeout = 8; // attempt connect to ed2k server in 8 seconds
    settings.m_collections_directory = misc::ED2KCollectionLocation().toUtf8().constData();
    settings.m_known_file = misc::emuleConfig("known.met").toUtf8().constData(); // always set known because user can close before all hashes will process
    settings.client_name  = pref.nick().toUtf8().constData();
    settings.mod_name = misc::productName().toUtf8().constData();
    settings.m_announce_timeout = 10; // announcing every 10 seconds
    const QString iface_name = misc::ifaceFromHumanName(pref.getNetworkInterfaceMule());

    qDebug() << "known " << misc::toQStringU(settings.m_known_file);
#ifdef AMD1
    settings.server_hostname = "che-s-amd1";
#else
    settings.server_hostname = "emule.is74.ru";
#endif    
    if (iface_name.isEmpty())
    {
        m_session.reset(new libed2k::session(finger, NULL, settings));
    }
    else
    {
        m_session.reset(new libed2k::session(finger, iface_name.toAscii().constData(), settings));
    }

    m_session->set_alert_mask(alert::all_categories);
    m_session->set_alert_queue_size_limit(100000);
    // start listening on special interface and port and start server connection
    configureSession();
}

void QED2KSession::stop()
{
    m_session->pause();
    saveFastResumeData();
}

bool QED2KSession::started() const { return !m_session.isNull(); }

QED2KSession::~QED2KSession()
{
    if (started())
    {
        stop();
    }
}


Transfer QED2KSession::getTransfer(const QString &hash) const
{
    return QED2KHandle(m_session->find_transfer(libed2k::md4_hash::fromString(hash.toStdString())));
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

qreal QED2KSession::getMaxRatioPerTransfer(const QString& hash, bool* use_global) const { return 0; }
SessionStatus QED2KSession::getSessionStatus() const { return m_session->status(); }
void QED2KSession::changeLabelInSavePath(
    const Transfer& t, const QString& old_label,const QString& new_label) {}
void QED2KSession::deleteTransfer(const QString& hash, bool delete_files)
{
    const Transfer t = getTransfer(hash);
    if (!t.is_valid()) return;
    emit transferAboutToBeRemoved(t, delete_files);

    m_session->remove_transfer(
        t.ed2kHandle().delegate(),
        delete_files ? libed2k::session::delete_files : libed2k::session::none);

    if (QFile::remove(QDir(misc::ED2KBackupLocation()).absoluteFilePath(hash + ".fastresume")))
    {
        qDebug() << "Also deleted temp fast resume data: " << hash;
    }
    else
    {
        qDebug() << "fast resume wasn't removed for " << hash;
    }

    emit deletedTransfer(hash);
}
void QED2KSession::recheckTransfer(const QString& hash) {}
void QED2KSession::setDownloadLimit(const QString& hash, long limit) {}
void QED2KSession::setUploadLimit(const QString& hash, long limit) {}
void QED2KSession::setMaxRatioPerTransfer(const QString& hash, qreal ratio){}
void QED2KSession::removeRatioPerTransfer(const QString& hash) {}
void QED2KSession::banIP(QString ip) {}
QHash<QString, TrackerInfos> QED2KSession::getTrackersInfo(const QString &hash) const{ 
    return QHash<QString, TrackerInfos>();
}
void QED2KSession::setDownloadRateLimit(long rate) {
    session_settings settings = m_session->settings();
    settings.download_rate_limit = rate;
    m_session->set_settings(settings);
}
void QED2KSession::setUploadRateLimit(long rate) {
    session_settings settings = m_session->settings();
    settings.upload_rate_limit = rate;
    m_session->set_settings(settings);
}
void QED2KSession::startUpTransfers()
{
    loadFastResumeData();
}

void QED2KSession::configureSession()
{
    qDebug() << Q_FUNC_INFO;
    Preferences pref;
    const unsigned short old_listenPort = m_session->settings().listen_port;
    const unsigned short new_listenPort = pref.listenPort();
    const int down_limit = pref.getED2KDownloadLimit();
    const int up_limit = pref.getED2KUploadLimit();

    // set common settings before for announce correct nick on server
    libed2k::session_settings s = m_session->settings();
    s.client_name = pref.nick().toUtf8().constData();
    s.m_show_shared_catalogs = pref.isShowSharedDirectories();
    s.m_show_shared_files = pref.isShowSharedFiles();
    s.download_rate_limit = down_limit <= 0 ? -1 : down_limit*1024;
    s.upload_rate_limit = up_limit <= 0 ? -1 : up_limit*1024;
    m_session->set_settings(s);

    if (new_listenPort != old_listenPort)
    {
        qDebug() << "stop listen on " << old_listenPort << " and start on " << new_listenPort;
        const QString iface_name = misc::ifaceFromHumanName(pref.getNetworkInterfaceMule());

        if (iface_name.isEmpty())
        {
            m_session->listen_on(new_listenPort);
        }
        else
        {
            // Attempt to listen on provided interface
            const QNetworkInterface network_iface = QNetworkInterface::interfaceFromName(iface_name);

            if (!network_iface.isValid())
            {
                qDebug("Invalid network interface: %s", qPrintable(iface_name));
                addConsoleMessage(tr("The network interface defined is invalid: %1").arg(iface_name), "red");
                addConsoleMessage(tr("Trying any other network interface available instead."));
                m_session->listen_on(new_listenPort);
            }
            else
            {
                QString ip;
                qDebug("This network interface has %d IP addresses", network_iface.addressEntries().size());

                foreach (const QNetworkAddressEntry &entry, network_iface.addressEntries())
                {
                    qDebug("Trying to listen on IP %s (%s)", qPrintable(entry.ip().toString()), qPrintable(iface_name));

                    if (m_session->listen_on(new_listenPort, entry.ip().toString().toAscii().constData()))
                    {
                        ip = entry.ip().toString();
                        break;
                    }
                }

                if (m_session->is_listening())
                {
                    addConsoleMessage(tr("Listening on IP address %1 on network interface %2...").arg(ip).arg(iface_name));
                }
                else
                {
                    qDebug("Failed to listen on any of the IP addresses");
                    addConsoleMessage(tr("Failed to listen on network interface %1").arg(iface_name), "red");
                }
            }
        }
    }
    else
    {
        qDebug() << "don't execute re-listen";
    }

}

void QED2KSession::enableIPFilter(const QString &filter_path, bool force /*=false*/){}

QPair<Transfer,ErrorCode> QED2KSession::addLink(QString strLink, bool resumed /* = false */)
{
    qDebug("Load ED2K link: %s", strLink.toUtf8().constData());

    libed2k::emule_collection_entry ece =
        libed2k::emule_collection::fromLink(strLink.toUtf8().constData());
    QED2KHandle h;
    ErrorCode ec;

    if (ece.defined())
    {
        qDebug("Link is correct, add transfer");
        QString filepath = QDir(Preferences().getSavePath()).filePath(
            QString::fromUtf8(ece.m_filename.c_str(), ece.m_filename.size()));
        libed2k::add_transfer_params atp;
        atp.file_hash = ece.m_filehash;
        atp.file_path = filepath.toUtf8().constData();
        atp.file_size = ece.m_filesize;
        atp.duplicate_is_error = true;

        try
        {
            h = addTransfer(atp);
        }
        catch(libed2k::libed2k_exception e)
        {
            ec = e.error();
        }
    }
    else
        ec = "Incorrect link";

    return qMakePair(Transfer(h), ec);
}

void QED2KSession::addTransferFromFile(const QString& filename)
{
    if (QFile::exists(filename))
    {
        libed2k::emule_collection ecoll = libed2k::emule_collection::fromFile(
            filename.toLocal8Bit().constData());

        foreach(const libed2k::emule_collection_entry& ece, ecoll.m_files)
        {
            QString filepath = QDir(Preferences().getSavePath()).filePath(
                QString::fromUtf8(ece.m_filename.c_str(), ece.m_filename.size()));
            qDebug() << "add transfer " << filepath;
            libed2k::add_transfer_params atp;
            atp.file_hash = ece.m_filehash;
            atp.file_path = filepath.toUtf8().constData();
            atp.file_size = ece.m_filesize;
            addTransfer(atp);
        }
    }
}

QED2KHandle QED2KSession::addTransfer(const libed2k::add_transfer_params& atp)
{
    qDebug() << "add transfer for " << QString::fromUtf8(atp.file_path.c_str());

    {
        // do not create file on windows with last point because of Qt truncate it point!
        bool touch = true;
#ifdef Q_WS_WIN
        touch = (!atp.file_path.empty() && (atp.file_path.at(atp.file_path.size() - 1) != '.'));
#endif
        QFile f(misc::toQStringU(atp.file_path));
        if (!f.exists() && touch)
        {                  
            f.open(QIODevice::WriteOnly);
        }
    }

    return QED2KHandle(delegate()->add_transfer(atp));
}

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

void QED2KSession::cancelSearch()
{
    m_session->post_cancel_search();
}

libed2k::peer_connection_handle QED2KSession::getPeer(const libed2k::net_identifier& np)
{
    libed2k::peer_connection_handle pch = m_session->find_peer_connection(np);

    if (pch.empty())
    {
        pch = m_session->add_peer_connection(np);
    }
    return pch;
}

libed2k::peer_connection_handle QED2KSession::findPeer(const libed2k::net_identifier& np)
{
    libed2k::peer_connection_handle pch = m_session->find_peer_connection(np);

    return pch;
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
            emit serverConnectionInitialized(p->m_nClientId, p->m_nTCPFlags, p->m_nAuxPort);
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
            emit serverConnectionClosed(QString::fromLocal8Bit(p->m_error.message().c_str()));
        }
        else if (libed2k::shared_files_alert* p = dynamic_cast<libed2k::shared_files_alert*>(a.get()))
        {
            std::vector<QED2KSearchResultEntry> vRes;
            vRes.resize(p->m_files.m_collection.size());
            bool bMoreResult = p->m_more;

            for (size_t n = 0; n < p->m_files.m_collection.size(); ++n)
            {
                QED2KSearchResultEntry sre = QED2KSearchResultEntry::fromSharedFileEntry(p->m_files.m_collection[n]);

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
            }
            else if (libed2k::ismod_shared_directory_files_alert* p2 =
                     dynamic_cast<libed2k::ismod_shared_directory_files_alert*>(p))
            {
                emit peerIsModSharedFiles(p2->m_np, md4toQString(p2->m_hash), md4toQString(p2->m_dir_hash), vRes);
            }
            else
            {
                emit searchResult(p->m_np, md4toQString(p->m_hash), vRes, bMoreResult);
            }
        }
        else if (libed2k::mule_listen_failed_alert* p =
                 dynamic_cast<libed2k::mule_listen_failed_alert*>(a.get()))
        {
            Q_UNUSED(p)
            // TODO - process signal - it means we have different client on same port
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
            if (!p->m_captcha.empty()) pm.loadFromData((const uchar*)&p->m_captcha[0], p->m_captcha.size()); // avoid windows rtl error
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
            if (!m_fast_resume_transfers.empty())
            {
                remove_by_state();
                if (m_fast_resume_transfers.empty())
                {
                    emit fastResumeDataLoadCompleted();
                }
            }
        }
        else if (libed2k::paused_transfer_alert* p =
                 dynamic_cast<libed2k::paused_transfer_alert*>(a.get()))
        {
            emit pausedTransfer(Transfer(QED2KHandle(p->m_handle)));
        }
        else if (libed2k::resumed_transfer_alert* p =
                 dynamic_cast<libed2k::resumed_transfer_alert*>(a.get()))
        {
            emit resumedTransfer(Transfer(QED2KHandle(p->m_handle)));
        }
        else if (libed2k::deleted_transfer_alert* p =
                 dynamic_cast<libed2k::deleted_transfer_alert*>(a.get()))
        {            
            QString hash = QString::fromStdString(p->m_hash.toString());
            qDebug() << "delete transfer alert" << hash;
            emit deletedTransfer(QString::fromStdString(p->m_hash.toString()));
        }
        else if (libed2k::finished_transfer_alert* p =
                 dynamic_cast<libed2k::finished_transfer_alert*>(a.get()))
        {
            Transfer t(QED2KHandle(p->m_handle));

            if (p->m_had_picker)
                emit finishedTransfer(t);

            if (t.is_seed())
                emit registerNode(t);

            if (!m_fast_resume_transfers.empty())
            {
                m_fast_resume_transfers.remove(t.hash());
                remove_by_state();

                if (m_fast_resume_transfers.empty())
                {
                    emit fastResumeDataLoadCompleted();
                }                
            }

            Preferences pref;
            if (pref.isAutoRunEnabled() && p->m_had_picker)
                autoRunExternalProgram(t);
        }
        else if (libed2k::save_resume_data_alert* p = dynamic_cast<libed2k::save_resume_data_alert*>(a.get()))
        {
            writeResumeData(p);
        }
        else if (libed2k::transfer_params_alert* p = dynamic_cast<libed2k::transfer_params_alert*>(a.get()))
        {
            emit transferParametersReady(p->m_atp, p->m_ec);
        }
        else if (libed2k::file_renamed_alert* p = dynamic_cast<libed2k::file_renamed_alert*>(a.get()))
        {
            emit savePathChanged(Transfer(QED2KHandle(p->m_handle)));
        }
        else if (libed2k::storage_moved_alert* p = dynamic_cast<libed2k::storage_moved_alert*>(a.get()))
        {
            emit savePathChanged(Transfer(QED2KHandle(p->m_handle)));
        }
        else if (libed2k::file_error_alert* p = dynamic_cast<libed2k::file_error_alert*>(a.get()))
        {
            QED2KHandle h(p->m_handle);

            if (h.is_valid())
            {
                emit fileError(Transfer(h),
                               QString::fromLocal8Bit(p->error.message().c_str(), p->error.message().size()));
                h.pause();
            }
        }

        a = m_session->pop_alert();
    }
}

// Called periodically
void QED2KSession::saveTempFastResumeData()
{
    std::vector<libed2k::transfer_handle> transfers =  m_session->get_transfers();

    for (std::vector<libed2k::transfer_handle>::iterator th_itr = transfers.begin(); th_itr != transfers.end(); ++th_itr)
    {
        QED2KHandle h = QED2KHandle(*th_itr);

        try
        {
            if (!h.is_valid() || !h.has_metadata()) continue;

            if (h.state() == qt_checking_files ||
                  h.state() == qt_queued_for_checking) continue;

            qDebug("Saving fastresume data for %s", qPrintable(h.name()));
            h.save_resume_data();
        }
        catch(std::exception&)
        {}
    }
}

void QED2KSession::saveFastResumeData()
{
    qDebug("Saving fast resume data...");
    int num_resume_data = 0;
    // Pause session
    delegate()->pause();
    std::vector<transfer_handle> transfers =  delegate()->get_transfers();

    for (std::vector<transfer_handle>::iterator th_itr = transfers.begin(); th_itr != transfers.end(); th_itr++)
    {
        QED2KHandle h = QED2KHandle(*th_itr);
        if (!h.is_valid() || !h.has_metadata())
        {
            qDebug() << "transfer invalid or hasn't metadata";
            continue;
        }

        try
        {

            if (h.state() == qt_checking_files || h.state() == qt_queued_for_checking)
            {
                qDebug() << "transfer " << h.hash() << " in checking files or queued for checking";
                continue;
            }

            h.save_resume_data();
            ++num_resume_data;
        }
        catch(libed2k::libed2k_exception& e)
        {
            qDebug() << "exception on request saving " << misc::toQStringU(e.what());
        }
    }

    while (num_resume_data > 0)
    {
        libed2k::alert const* a = delegate()->wait_for_alert(libed2k::seconds(30));

        if (a == 0)
        {
            qDebug("On save fast resume data we got empty alert - alert wasn't generated");
            break;
        }

        if (libed2k::save_resume_data_failed_alert const* rda = dynamic_cast<libed2k::save_resume_data_failed_alert const*>(a))
        {
            qDebug() << "save resume data failed alert " << misc::toQStringU(rda->message().c_str());
            --num_resume_data;

            try
            {
                // Remove torrent from session
                if (rda->m_handle.is_valid())
                    delegate()->remove_transfer(rda->m_handle);
            }
            catch(const libed2k::libed2k_exception& e)
            {
                qDebug() << "exception on remove transfer after save " << misc::toQStringU(e.what());
            }
        }
        else if (libed2k::save_resume_data_alert const* rd = dynamic_cast<libed2k::save_resume_data_alert const*>(a))
        {
            --num_resume_data;
            writeResumeData(rd);

            try
            {
                delegate()->remove_transfer(rd->m_handle);
            }
            catch(const libed2k::libed2k_exception& e)
            {
                qDebug() << "exception on remove transfer after save " << misc::toQStringU(e.what());
            }
        }

        delegate()->pop_alert();
    }
}

void QED2KSession::loadFastResumeData()
{
    qDebug("load fast resume data");

    // avoid load collections from previous fail
    QDir bkp_dir(misc::ED2KCollectionLocation());
    if (bkp_dir.exists())
    {
        foreach(QString filename, bkp_dir.entryList(QDir::Files))
        {
            qDebug() << "remove fail file: " << filename;
            QFile::remove(bkp_dir.absoluteFilePath(filename));
        }
    }

    // we need files 32 length(MD4_HASH_SIZE*2) name and extension fastresume
    QStringList filter;
    filter << "????????????????????????????????.fastresume";

    QDir fastresume_dir(misc::ED2KBackupLocation());
    const QStringList files = fastresume_dir.entryList(filter, QDir::Files, QDir::Unsorted);

    foreach (const QString &file, files)
    {
        qDebug("Trying to load fastresume data: %s", qPrintable(file));
        const QString file_abspath = fastresume_dir.absoluteFilePath(file);
        // extract hash from name
        libed2k::md4_hash hash = libed2k::md4_hash::fromString(file.toStdString().substr(0, libed2k::MD4_HASH_SIZE*2));

        if (hash.defined())
        {
            try
            {
                std::ifstream fs(file_abspath.toLocal8Bit(), std::ios_base::in | std::ios_base::binary);

                if (fs)
                {
                    libed2k::transfer_resume_data trd;
                    libed2k::archive::ed2k_iarchive ia(fs);
                    ia >> trd;
                    // compare hashes
                    if (trd.m_hash == hash)
                    {
                        // add transfer
                        libed2k::add_transfer_params params;
                        params.seed_mode = false;
                        params.file_path = trd.m_filepath.m_collection;
                        params.file_size = trd.m_filesize;
                        params.file_hash = trd.m_hash;

                        if (trd.m_fast_resume_data.count() > 0)
                        {
                            params.resume_data = const_cast<std::vector<char>* >(&trd.m_fast_resume_data.getTagByNameId(libed2k::FT_FAST_RESUME_DATA)->asBlob());
                        }

                        QFileInfo qfi(QString::fromUtf8(trd.m_filepath.m_collection.c_str()));
                        // add transfer only when file still exists
                        if (qfi.exists() && qfi.isFile())
                        {                            
                            QED2KHandle h(delegate()->add_transfer(params));
                            m_fast_resume_transfers.insert(h.hash(), h);
                        }
                        else
                        {
                            qDebug() << "file not exists: " << qfi.fileName();
                        }
                    }
                }
            }
            catch(const libed2k::libed2k_exception&)
            {}
        }

        QFile::remove(file_abspath);
    }

    if (m_fast_resume_transfers.empty())
    {
        // no fast resume data found - session ready for share
        emit fastResumeDataLoadCompleted();
    }
}

void QED2KSession::startServerConnection()
{
    delegate()->server_conn_start();
}

void QED2KSession::stopServerConnection()
{
    delegate()->server_conn_stop();
}

bool QED2KSession::isServerConnected() const
{
    return (delegate()->server_conn_online());
}

void QED2KSession::makeTransferParametersAsync(const QString& filepath)
{
    delegate()->make_transfer_parameters(filepath.toUtf8().constData());
}

void QED2KSession::cancelTransferParameters(const QString& filepath)
{
    delegate()->cancel_transfer_parameters(filepath.toUtf8().constData());
}

void QED2KSession::remove_by_state()
{
    // begin analize states
    QHash<QString, Transfer>::iterator i = m_fast_resume_transfers.begin();

    while (i != m_fast_resume_transfers.end())
    {
        if (!i.value().is_valid() ||
                (i.value().state() == qt_downloading))
        {
            qDebug() << "remove state " << i.value().hash() << " is valid " << i.value().is_valid();
            i = m_fast_resume_transfers.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

std::pair<libed2k::add_transfer_params, libed2k::error_code> QED2KSession::makeTransferParameters(const QString& filepath) const
{
    bool cancel = false;
    return libed2k::file2atp()(filepath.toUtf8().constData(), cancel);
}

}
