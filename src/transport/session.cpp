
#include <libtorrent/torrent_handle.hpp>
#include "transport/session.h"

using namespace libtorrent;

const qreal Session::MAX_RATIO = 9999.;

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

Session::Session()
{
    // libtorrent signals
    connect(&m_btSession, SIGNAL(addedTorrent(QTorrentHandle)),
            this, SLOT(on_addedTorrent(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(deletedTorrent(QString)),
            this, SIGNAL(deletedTransfer(QString)));
    connect(&m_btSession, SIGNAL(pausedTorrent(QTorrentHandle)),
            this, SLOT(on_pausedTorrent(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(resumedTorrent(QTorrentHandle)),
            this, SLOT(on_resumedTorrent(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(finishedTorrent(QTorrentHandle)),
            this, SLOT(on_finishedTorrent(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(metadataReceived(QTorrentHandle)),
            this, SLOT(on_metadataReceived(QTorrentHandle)));
    connect(&m_btSession, SIGNAL(fullDiskError(QTorrentHandle, QString)),
            this, SLOT(on_fullDiskError(QTorrentHandle, QString)));
    connect(&m_btSession, SIGNAL(torrentAboutToBeRemoved(QTorrentHandle)),
            this, SLOT(on_torrentAboutToBeRemoved(QTorrentHandle)));
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

    // libed2k signals
    connect(&m_edSession, SIGNAL(addedTransfer(Transfer)), this, SIGNAL(addedTransfer(Transfer)));
    connect(&m_edSession, SIGNAL(pausedTransfer(Transfer)), this, SIGNAL(pausedTransfer(Transfer)));
    connect(&m_edSession, SIGNAL(resumedTransfer(Transfer)), this, SIGNAL(resumedTransfer(Transfer)));
}

QBtSession* Session::get_torrent_session()
{
	return &m_btSession;
}

QED2KSession* Session::get_ed2k_session() {
    return &m_edSession;
}

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

std::vector<SessionBase*> Session::delegates() const {
    std::vector<SessionBase*> sessions;
    sessions.push_back(const_cast<QBtSession*>(&m_btSession));
    return sessions;
}

Transfer Session::getTransfer(const QString& hash) const {
    return delegate(hash)->getTransfer(hash);
}

std::vector<Transfer> Session::getTransfers() const {
    std::vector<Transfer> transfers;
    std::vector<SessionBase*> sessions = delegates();
    for(std::vector<SessionBase*>::iterator si = sessions.begin();
        si != sessions.end(); ++si)
    {
        std::vector<Transfer> sessionTransfers = (*si)->getTransfers();
        for(std::vector<Transfer>::iterator ti = sessionTransfers.begin();
            ti != sessionTransfers.end(); ++ti)
            transfers.push_back(*ti);
    }

    return transfers;
}

qlonglong Session::getETA(const QString& hash) const {
    return delegate(hash)->getETA(hash); }

qreal Session::getRealRatio(const QString& hash) const {
    return delegate(hash)->getRealRatio(hash);
}
qreal Session::getGlobalMaxRatio() const { return m_btSession.getGlobalMaxRatio(); }
qreal Session::getMaxRatioPerTransfer(const QString& hash, bool* use_global) const {
    return delegate(hash)->getMaxRatioPerTransfer(hash, use_global);
}
bool Session::isFilePreviewPossible(const QString& hash) const {
    return delegate(hash)->isFilePreviewPossible(hash);
}
void Session::changeLabelInSavePath(
    const Transfer& t, const QString& old_label, const QString& new_label) {
    return delegate(t)->changeLabelInSavePath(t, old_label, new_label);
}
QStringList Session::getConsoleMessages() const { return m_btSession.getConsoleMessages(); }
QStringList Session::getPeerBanMessages() const { return m_btSession.getPeerBanMessages(); }
SessionStatus Session::getSessionStatus() const { return m_btSession.getSessionStatus(); }

QTorrentHandle Session::addTorrent(const QString& path, bool fromScanDir/* = false*/,
                                   QString from_url /*= QString()*/, bool resumed/* = false*/) {
    return m_btSession.addTorrent(path, fromScanDir, from_url, resumed);
}
QTorrentHandle Session::addMagnetUri(const QString& url, bool resumed/*=false*/) {
    return m_btSession.addMagnetUri(url, resumed);
}
QED2KHandle Session::addTransfer(const libed2k::add_transfer_params& params) {
    return QED2KHandle(m_edSession.delegate()->add_transfer(params));
}
void Session::downloadFromUrl(const QString& url) {
    m_btSession.downloadFromUrl(url);
}
void Session::processDownloadedFile(const QString& url, const QString& path) {
    m_btSession.processDownloadedFile(url, path);
}
void Session::pauseTransfer(const QString& hash) {
    delegate(hash)->pauseTransfer(hash);
}
void Session::resumeTransfer(const QString& hash) {
    delegate(hash)->resumeTransfer(hash);
}
void Session::deleteTransfer(const QString& hash, bool delete_files) {
    delegate(hash)->deleteTransfer(hash, delete_files);
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
void Session::addConsoleMessage(const QString& msg, QColor color) {
    m_btSession.addConsoleMessage(msg, color);
}
void Session::banIP(QString ip) {
	m_btSession.banIP(ip);
	m_edSession.banIP(ip);
}
QHash<QString, TrackerInfos> Session::getTrackersInfo(const QString &hash) const {
	return delegate(hash)->getTrackersInfo(hash);
}

void Session::setDownloadRateLimit(long rate)
{
	m_btSession.setDownloadRateLimit(rate);
	m_edSession.setDownloadRateLimit(rate);
}

void Session::setUploadRateLimit(long rate)
{
	m_btSession.setUploadRateLimit(rate);
	m_edSession.setUploadRateLimit(rate);
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
void Session::startUpTransfers() {
	m_btSession.startUpTransfers();
	m_edSession.startUpTransfers();
}

void Session::configureSession() {
	m_btSession.configureSession();
	m_edSession.configureSession();
}

void Session::enableIPFilter(const QString &filter_path, bool force/*=false*/) {
	m_btSession.enableIPFilter(filter_path, force);
	m_edSession.enableIPFilter(filter_path, force);
}

void Session::on_addedTorrent(const QTorrentHandle& h) { emit addedTransfer(Transfer(h)); }
void Session::on_pausedTorrent(const QTorrentHandle& h) { emit pausedTransfer(Transfer(h)); }
void Session::on_resumedTorrent(const QTorrentHandle& h) { emit resumedTransfer(Transfer(h)); }
void Session::on_finishedTorrent(const QTorrentHandle& h) { emit finishedTransfer(Transfer(h)); }
void Session::on_metadataReceived(const QTorrentHandle& h) { emit metadataReceived(Transfer(h)); }
void Session::on_fullDiskError(const QTorrentHandle& h, QString msg) {
    emit fullDiskError(Transfer(h), msg);
}
void Session::on_torrentAboutToBeRemoved(const QTorrentHandle& h) {
    emit transferAboutToBeRemoved(Transfer(h));
}
void Session::on_torrentFinishedChecking(const QTorrentHandle& h) {
    emit transferFinishedChecking(Transfer(h));
}
void Session::on_trackerAuthenticationRequired(const QTorrentHandle& h) {
    emit trackerAuthenticationRequired(Transfer(h));
}
void Session::on_savePathChanged(const QTorrentHandle& h) { emit savePathChanged(Transfer(h)); }
