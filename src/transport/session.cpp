#include <boost/bind.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <QDesktopServices>
#include <QDirIterator>

#include "transport/session.h"
#include "torrentpersistentdata.h"

using namespace libtorrent;

SessionStatus operator + (const SessionStatus& s1, const SessionStatus& s2){
    SessionStatus s = s1;

    s.has_incoming_connections = s1.has_incoming_connections || s2.has_incoming_connections;

    s.upload_rate += s2.upload_rate;
    s.download_rate += s2.download_rate;
    s.total_download += s2.total_download;
    s.total_upload += s2.total_upload;

    s.payload_upload_rate += s2.payload_upload_rate;
    s.payload_download_rate += s2.payload_download_rate;
    s.total_payload_download += s2.total_payload_download;
    s.total_payload_upload += s2.total_payload_upload;

    s.ip_overhead_upload_rate += s2.ip_overhead_upload_rate;
    s.ip_overhead_download_rate += s2.ip_overhead_download_rate;
    s.total_ip_overhead_download += s2.total_ip_overhead_download;
    s.total_ip_overhead_upload += s2.total_ip_overhead_upload;

    s.dht_upload_rate += s2.dht_upload_rate;
    s.dht_download_rate += s2.dht_download_rate;
    s.total_dht_download += s2.total_dht_download;
    s.total_dht_upload += s2.total_dht_upload;

    s.tracker_upload_rate += s2.tracker_upload_rate;
    s.tracker_download_rate += s2.tracker_download_rate;
    s.total_tracker_download += s2.total_tracker_download;
    s.total_tracker_upload += s2.total_tracker_upload;

    s.total_redundant_bytes += s2.total_redundant_bytes;
    s.total_failed_bytes += s2.total_failed_bytes;

    s.num_peers += s2.num_peers;
    s.num_unchoked += s2.num_unchoked;
    s.allowed_upload_slots += s2.allowed_upload_slots;

    s.up_bandwidth_queue += s2.up_bandwidth_queue;
    s.down_bandwidth_queue += s2.down_bandwidth_queue;

    s.up_bandwidth_bytes_queue += s2.up_bandwidth_bytes_queue;
    s.down_bandwidth_bytes_queue += s2.down_bandwidth_bytes_queue;

    s.optimistic_unchoke_counter += s2.optimistic_unchoke_counter;
    s.unchoke_counter += s2.unchoke_counter;

    return s;
}

Session* Session::m_instance = NULL;

Session* Session::instance()
{
    if (!m_instance)
        m_instance = new Session();

    return m_instance;
}

void Session::drop()
{
    delete m_instance;
    m_instance = NULL;
}

Session::~Session()
{    
}

Session::Session()
{
    // prepare sessions container
    m_sessions.push_back(&m_btSession);
    m_sessions.push_back(&m_edSession);

    // libtorrent signals
    connect(&m_btSession, SIGNAL(addedTorrent(QTorrentHandle)),
            this, SLOT(on_addedTorrent(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(deletedTorrent(QString)),
            this, SIGNAL(deletedTransfer(QString)));
    connect(&m_btSession, SIGNAL(pausedTorrent(QTorrentHandle)),
            this, SLOT(on_pausedTorrent(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(finishedTorrent(QTorrentHandle)),
            this, SLOT(on_finishedTorrent(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(metadataReceived(QTorrentHandle)),
            this, SLOT(on_metadataReceived(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(fullDiskError(QTorrentHandle, QString)),
            this, SLOT(on_fullDiskError(QTorrentHandle, QString)));
    connect(&m_btSession, SIGNAL(torrentAboutToBeRemoved(QTorrentHandle, bool)),
            this, SLOT(on_torrentAboutToBeRemoved(QTorrentHandle, bool)));
    connect(&m_btSession, SIGNAL(torrentFinishedChecking(QTorrentHandle)),
            this, SLOT(on_torrentFinishedChecking(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(trackerAuthenticationRequired(QTorrentHandle)),
            this, SLOT(on_trackerAuthenticationRequired(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(newDownloadedTorrent(QString, QString)),
            this, SIGNAL(newDownloadedTransfer(QString, QString)));
    connect(&m_btSession, SIGNAL(downloadFromUrlFailure(QString, QString)),
            this, SIGNAL(downloadFromUrlFailure(QString, QString)));
    connect(&m_btSession, SIGNAL(alternativeSpeedsModeChanged(bool)),
            this, SIGNAL(alternativeSpeedsModeChanged(bool)));
    connect(&m_btSession, SIGNAL(recursiveTorrentDownloadPossible(QTorrentHandle)),
            this, SIGNAL(recursiveDownloadPossible(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(savePathChanged(QTorrentHandle)),
            this, SLOT(on_savePathChanged(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(newConsoleMessage(QString)),
            this, SIGNAL(newConsoleMessage(QString)));
    connect(&m_btSession, SIGNAL(newBanMessage(QString)),
            this, SIGNAL(newBanMessage(QString)));

    // periodic save temp fast resume data
    m_alerts_reading.reset(new QTimer(this));
    m_periodic_resume.reset(new QTimer(this));
    connect(m_alerts_reading.data(), SIGNAL(timeout()), SLOT(readAlerts()));
    connect(m_periodic_resume.data(), SIGNAL(timeout()), SLOT(saveTempFastResumeData()));

    m_alerts_reading->start(1000);
    m_periodic_resume->start(170000);   // 3 min

    // libed2k signals
    connect(&m_edSession, SIGNAL(addedTransfer(Transfer)), this, SIGNAL(addedTransfer(Transfer)));
    connect(&m_edSession, SIGNAL(pausedTransfer(Transfer)), this, SIGNAL(pausedTransfer(Transfer)));
    connect(&m_edSession, SIGNAL(resumedTransfer(Transfer)), this, SIGNAL(resumedTransfer(Transfer)));
    connect(&m_edSession, SIGNAL(finishedTransfer(Transfer)), this, SIGNAL(finishedTransfer(Transfer)));
    connect(&m_edSession, SIGNAL(deletedTransfer(QString)), this, SIGNAL(deletedTransfer(QString)));
    connect(&m_edSession, SIGNAL(transferAboutToBeRemoved(Transfer)),
            this, SIGNAL(transferAboutToBeRemoved(Transfer)));

    m_speedMonitor.reset(new TorrentSpeedMonitor(this));
    m_speedMonitor->start();
}

QBtSession* Session::get_torrent_session() { return &m_btSession; }
QED2KSession* Session::get_ed2k_session() { return &m_edSession; }

SessionBase* Session::delegate(const QString& hash) const {
    QByteArray raw = hash.toAscii();

    if (raw.size() == 40) // SHA-1
        return const_cast<QBtSession*>(&m_btSession);
    else if (raw.size() == 32) // MD4
        return const_cast<QED2KSession*>(&m_edSession);

    Q_ASSERT(false);
    return NULL;
}

SessionBase* Session::delegate(const Transfer& t) const { return delegate(t.hash()); }

void Session::start()
{
    if (!started())
    {
        for_each(std::mem_fun(&SessionBase::start));
    }
}

void Session::stop()
{
    if (started())
    {
        for_each(std::mem_fun(&SessionBase::stop));
    }
}

bool Session::started() const
{
    return (*m_sessions.begin())->started();
}

Transfer Session::getTransfer(const QString& hash) const {
    return delegate(hash)->getTransfer(hash);
}

std::vector<Transfer> Session::getTransfers() const
{
    std::vector<Transfer> transfers;
    for(std::vector<SessionBase*>::const_iterator si = m_sessions.begin();
        si != m_sessions.end(); ++si)
    {
        std::vector<Transfer> sessionTransfers = (*si)->getTransfers();
        for(std::vector<Transfer>::iterator ti = sessionTransfers.begin();
            ti != sessionTransfers.end(); ++ti)
            transfers.push_back(*ti);
    }

    return transfers;
}

qlonglong Session::getETA(const QString& hash) const {
    return m_speedMonitor->getETA(hash);
}

qreal Session::getGlobalMaxRatio() const { return m_btSession.getGlobalMaxRatio(); }
qreal Session::getMaxRatioPerTransfer(const QString& hash, bool* use_global) const {
    return delegate(hash)->getMaxRatioPerTransfer(hash, use_global);
}
void Session::changeLabelInSavePath(
    const Transfer& t, const QString& old_label, const QString& new_label) {
    return delegate(t)->changeLabelInSavePath(t, old_label, new_label);
}
QStringList Session::getConsoleMessages() const { return m_btSession.getConsoleMessages(); }
QStringList Session::getPeerBanMessages() const { return m_btSession.getPeerBanMessages(); }
SessionStatus Session::getSessionStatus() const {
    return m_edSession.getSessionStatus() + m_btSession.getSessionStatus();
}
QTorrentHandle Session::addTorrent(const QString& path, bool fromScanDir/* = false*/,
                                   QString from_url /*= QString()*/, bool resumed/* = false*/) {
    return m_btSession.addTorrent(path, fromScanDir, from_url, resumed);
}
QED2KHandle Session::addTransfer(const libed2k::add_transfer_params& params) {
    return m_edSession.addTransfer(params);
}
void Session::downloadFromUrl(const QString& url) {
    m_btSession.downloadFromUrl(url);
}
void Session::processDownloadedFile(const QString& url, const QString& path) {
    m_btSession.processDownloadedFile(url, path);
}
void Session::deleteTransfer(const QString& hash, bool delete_files) {
    delegate(hash)->deleteTransfer(hash, delete_files);
    TorrentPersistentData::deletePersistentData(hash);
}
void Session::recheckTransfer(const QString& hash) {
    delegate(hash)->recheckTransfer(hash);
}
void Session::setDownloadLimit(const QString& hash, long limit) {
    delegate(hash)->setDownloadLimit(hash, limit); }

void Session::setUploadLimit(const QString& hash, long limit) {
    delegate(hash)->setUploadLimit(hash, limit); }

void Session::setMaxRatioPerTransfer(const QString& hash, qreal ratio) {
    delegate(hash)->setMaxRatioPerTransfer(hash, ratio); }

void Session::removeRatioPerTransfer(const QString& hash) {
    delegate(hash)->removeRatioPerTransfer(hash); }

void Session::useAlternativeSpeedsLimit(bool alternative) {
    m_btSession.useAlternativeSpeedsLimit(alternative);
}
void Session::banIP(QString ip)
{
    for_each(std::bind2nd(std::mem_fun(&SessionBase::banIP), ip));
}
QHash<QString, TrackerInfos> Session::getTrackersInfo(const QString &hash) const {
	return delegate(hash)->getTrackersInfo(hash);
}

void Session::setDownloadRateLimit(long rate)
{
    for_each(std::bind2nd(std::mem_fun(&SessionBase::setDownloadRateLimit), rate));
}

void Session::setUploadRateLimit(long rate)
{
    for_each(std::bind2nd(std::mem_fun(&SessionBase::setUploadRateLimit), rate));
}

bool Session::hasActiveTransfers() const
{
	return (m_btSession.hasActiveTransfers() || m_edSession.hasActiveTransfers());
}

bool Session::useTemporaryFolder() const { return m_btSession.useTemporaryFolder(); }
bool Session::isDHTEnabled() const { return m_btSession.isDHTEnabled(); }
bool Session::isPexEnabled() const { return m_btSession.isPexEnabled(); }
bool Session::isLSDEnabled() const { return m_btSession.isLSDEnabled(); }
bool Session::isQueueingEnabled() const { return m_btSession.isQueueingEnabled(); }
bool Session::isListening() const { return m_btSession.getSession()->is_listening(); }

void Session::deferPlayMedia(Transfer t)
{
    if (t.is_valid())
    {
        t.prioritize_first_last_piece(true);
        m_pending_medias.push_back(t.hash());
    }
}

void Session::playLink(const QString& strLink)
{
    deferPlayMedia(addLink(strLink));
}

bool Session::playMedia(Transfer t)
{
    if (t.is_valid() && t.has_metadata() &&
        t.num_files() == 1 && misc::isPreviewable(misc::file_extension(t.filename_at(0))) &&
        (t.first_last_piece_first() || t.is_seed()))
    {
        TransferBitfield pieces = t.pieces();
        int last_piece = pieces.size() - 1;
        int penult_piece = std::max(last_piece - 1, 0);
        if (pieces[0] && pieces[last_piece] && pieces[penult_piece])
        {
            t.set_sequential_download(true);
            return (t.progress() >= 0.05 && QDesktopServices::openUrl(QUrl::fromLocalFile(t.filepath_at(0))));
        }
    }

    return false;
}

void Session::playPendingMedia()
{
    for (std::vector<QString>::iterator i = m_pending_medias.begin(); i != m_pending_medias.end();)
    {
        Transfer t = getTransfer(*i);
        if (!t.is_valid() || playMedia(t))
            i = m_pending_medias.erase(i);
        else
            ++i;
    }
}

void Session::startUpTransfers()
{
    for_each(std::mem_fun(&SessionBase::startUpTransfers));
}

void Session::configureSession()
{
    for_each(std::mem_fun(&SessionBase::configureSession));
}

void Session::enableIPFilter(const QString &filter_path, bool force/*=false*/)
{
    for_each(boost::bind(&SessionBase::enableIPFilter, _1, filter_path, force));
}

Transfer Session::addLink(QString strLink, bool resumed)
{
   if (strLink.startsWith("ed2k://"))
   {
       return m_edSession.addLink(strLink, resumed);
   }

   return m_btSession.addLink(strLink, resumed);
}

void Session::addTransferFromFile(const QString& filename)
{
    if (filename.endsWith(".emulecollection"))
    {
        m_edSession.addTransferFromFile(filename);
    }
    else
    {
        m_btSession.addTransferFromFile(filename);
    }
}

void Session::on_addedTorrent(const QTorrentHandle& h) { emit addedTransfer(Transfer(h)); }
void Session::on_pausedTorrent(const QTorrentHandle& h) { emit pausedTransfer(Transfer(h)); }
void Session::on_finishedTorrent(const QTorrentHandle& h)
{
    emit finishedTransfer(Transfer(h));
    shareByED2K(h, false);
}
void Session::on_metadataReceived(const QTorrentHandle& h) { emit metadataReceived(Transfer(h)); }
void Session::on_fullDiskError(const QTorrentHandle& h, QString msg) {
    emit fullDiskError(Transfer(h), msg);
}
void Session::on_torrentAboutToBeRemoved(const QTorrentHandle& h, bool del_files)
{
    if (del_files) shareByED2K(h, true);
    emit transferAboutToBeRemoved(Transfer(h));
}
void Session::on_torrentFinishedChecking(const QTorrentHandle& h) {
    emit transferFinishedChecking(Transfer(h));
}
void Session::on_trackerAuthenticationRequired(const QTorrentHandle& h) {
    emit trackerAuthenticationRequired(Transfer(h));
}
void Session::on_savePathChanged(const QTorrentHandle& h) { emit savePathChanged(Transfer(h)); }

void Session::saveTempFastResumeData()
{
    for_each(std::mem_fun(&SessionBase::saveTempFastResumeData));
}

void Session::readAlerts()
{
    for_each(std::mem_fun(&SessionBase::readAlerts));
}

void Session::saveFastResumeData()
{
    m_periodic_resume->stop();
    m_alerts_reading->stop();
    m_btSession.saveFastResumeData();
    m_edSession.saveFastResumeData();
}

void Session::shareByED2K(const QTorrentHandle& h, bool unshare)
{
    m_edSession.shareByED2K(h, unshare);
}
