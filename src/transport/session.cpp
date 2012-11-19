#include <boost/bind.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <QDesktopServices>
#include <QDirIterator>

#include "transport/session.h"
#include "torrentpersistentdata.h"

using namespace libtorrent;


SessionStatus operator + (const SessionStatus& s1, const SessionStatus& s2){
    SessionStatus s = s1;
/*
    s.has_incoming_connections = s1.has_incoming_connections || s2.has_incoming_connections;

    s.upload_rate += s2.upload_rate;
    s.download_rate += s2.download_rate;
    s.total_download += s2.total_download;
    s.total_upload += s2.total_upload;
    */

    s.payload_upload_rate += s2.payload_upload_rate;
    s.payload_download_rate += s2.payload_download_rate;
    /*
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
*/
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

Session::Session() : m_root(NULL, QFileInfo(), true)
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
    connect(&m_btSession, SIGNAL(fileError(Transfer, QString)),
            this, SIGNAL(fileError(Transfer, QString)));

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
    connect(&m_edSession, SIGNAL(finishedTransfer(Transfer)),
            this, SIGNAL(finishedTransfer(Transfer)));
    connect(&m_edSession, SIGNAL(registerNode(Transfer)),
            this, SLOT(on_registerNode(Transfer)));
    connect(&m_edSession, SIGNAL(deletedTransfer(QString)),
            this, SIGNAL(deletedTransfer(QString)));
    connect(&m_edSession, SIGNAL(transferParametersReady(const libed2k::add_transfer_params&, const libed2k::error_code&)),
            this, SLOT(on_transferParametersReady(libed2k::add_transfer_params,libed2k::error_code)));
    connect(&m_edSession, SIGNAL(transferAboutToBeRemoved(Transfer, bool)),
            this, SLOT(on_transferAboutToBeRemoved(Transfer, bool)));
    connect(&m_edSession, SIGNAL(fileError(Transfer, QString)),
            this, SIGNAL(fileError(Transfer, QString)));
    connect(&m_edSession, SIGNAL(savePathChanged(Transfer)), this, SIGNAL(savePathChanged(Transfer)));
    connect(&m_edSession, SIGNAL(fastResumeDataLoadCompleted()), this, SLOT(on_ED2KResumeDataLoaded()));

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

void Session::deferPlayMedia(Transfer t, int fileIndex)
{
    if (t.is_valid() && !playMedia(t, fileIndex))
    {
        qDebug() << "Defer playing file: " << t.filename_at(fileIndex);
        t.set_sequential_download(false);
        t.prioritize_extremity_pieces(true, fileIndex);
        m_pending_medias.insert(qMakePair(t.hash(), fileIndex));
    }
}

void Session::playLink(const QString& strLink)
{
    deferPlayMedia(addLink(strLink), 0);
}

bool Session::playMedia(Transfer t, int fileIndex)
{
    if (t.is_valid() && t.has_metadata() &&
        misc::isPreviewable(misc::file_extension(t.filename_at(fileIndex))))
    {
        TransferBitfield pieces = t.pieces();
        const std::vector<int> extremity_pieces = t.file_extremity_pieces_at(fileIndex);

        // check we have all boundary pieces for the file
        foreach (int p, extremity_pieces) if (!pieces[p]) return false;

        t.set_sequential_download(true);
        return QDesktopServices::openUrl(QUrl::fromLocalFile(t.absolute_files_path().at(fileIndex)));
    }

    return false;
}

void Session::playPendingMedia()
{
    for (std::set<QPair<QString, int> >::iterator i = m_pending_medias.begin(); i != m_pending_medias.end();)
    {
        Transfer t = getTransfer(i->first);
        if (!t.is_valid() || playMedia(t, i->second))
            m_pending_medias.erase(i++);
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
    qDebug() << "add ED2K/magnet link: " << strLink;

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
    QDir save_path(h.save_path());
    int num_files = h.num_files();
    std::set<QString> roots;

    for (int i = 0; i < num_files; ++i)
        roots.insert(h.filepath_at(i).split(QDir::separator()).first());

    foreach(const QString& str, roots)
    {
        share(save_path.filePath(str), true);
    }

    emit finishedTransfer(Transfer(h));
}

void Session::on_metadataReceived(const QTorrentHandle& h) { emit metadataReceived(Transfer(h)); }

void Session::on_torrentAboutToBeRemoved(const QTorrentHandle& h, bool del_files)
{
    qDebug() << "torrent about to be removed " << h.hash()
              << " files " << (del_files?"delete":"stay");

    if (del_files)
    {
        // remove emule transfers
        QDir save_path(h.save_path());
        int num_files = h.num_files();
        std::set<QString> roots;

        for (int i = 0; i < num_files; ++i)
            roots.insert(h.filepath_at(i).split(QDir::separator()).first());

        foreach(const QString& str, roots)
        {
            unshare(save_path.filePath(str), true);
        }

        /*
        foreach(const QString& str, roots)
        {
            FileNode* p = node(str);

            if (p != &m_root)
            {
                DirNode* parent = p->m_parent;
                parent->delete_node();
            }
        }
        */

    }

    emit transferAboutToBeRemoved(Transfer(h), del_files);
}

void Session::on_transferAboutToBeRemoved(const Transfer& t, bool del_files)
{
    qDebug() << "transfer about to be removed " << t.hash()
             << " files " << (del_files?"delete":"stay");

    QHash<QString, FileNode*>::iterator itr = m_files.find(t.hash());
    // erase node if exists
    if (itr != m_files.end())
    {
        FileNode* node = itr.value();
        Q_ASSERT(node);
        m_files.erase(itr);

        if (del_files)
        {
            DirNode* parent = node->m_parent;
            Q_ASSERT(parent);
            parent->delete_node(node);
        }
        else
        {
            node->on_transfer_deleted();
        }
    }

    emit transferAboutToBeRemoved(t, del_files);

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
    saveFileSystem();
    m_btSession.saveFastResumeData();
    m_edSession.saveFastResumeData();
}

void Session::on_ED2KResumeDataLoaded()
{
    loadFileSystem();
}

void Session::on_registerNode(Transfer t)
{
    qDebug() << "Session::on_registerNode";
    FileNode* n = NULL;

    if (!m_files.contains(t.hash()))
    {
        n = node(t.filepath_at(0));
        Q_ASSERT(n);
        n->on_transfer_finished(t.hash());
    }

}

void Session::on_transferParametersReady(const libed2k::add_transfer_params& atp, const libed2k::error_code& ec)
{
    FileNode* p = node(misc::toQStringU(atp.m_filepath));

    if (p != &m_root)
    {
        p->on_metadata_completed(atp, ec);
    }
}

void Session::removeDirectory(DirNode* dir)
{
    std::set<DirNode*>::iterator itr = m_dirs.find(dir);

    if (itr != m_dirs.end()) m_dirs.erase(itr);

    m_dirs.erase(std::find(m_dirs.begin(), m_dirs.end(), dir), m_dirs.end());
}

void Session::addDirectory(DirNode* dir)
{
    m_dirs.insert(dir);
}

void Session::setDirectLink(const QString& hash, DirNode* node)
{
    m_files.insert(hash, node);
}

void Session::registerNode(FileNode* node)
{
    m_files.insert(node->hash(), node);
}

FileNode* Session::node(const QString& filepath)
{
    qDebug() << "node: " << filepath;
    if (filepath.isEmpty() || filepath == tr("My Computer") ||
            filepath == tr("Computer") || filepath.startsWith(QLatin1Char(':')))
        return (&m_root);

#ifdef Q_OS_WIN32
    QString longPath = qt_GetLongPathName(filepath);
#else
    QString longPath = filepath;
#endif

    QFileInfo fi(longPath);

    QString absolutePath = QDir(longPath).absolutePath();

    // ### TODO can we use bool QAbstractFileEngine::caseSensitive() const?
    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);

    if ((pathElements.isEmpty())
#if (!defined(Q_OS_WIN) || defined(Q_OS_WINCE)) && !defined(Q_OS_SYMBIAN)
        && QDir::fromNativeSeparators(longPath) != QLatin1String("/")
#endif
        )
        return (&m_root);

#if (defined(Q_OS_WIN) && !defined(Q_OS_WINCE)) || defined(Q_OS_SYMBIAN)
    {
        if (!pathElements.at(0).contains(QLatin1String(":")))
        {
            // The reason we express it like this instead of with anonymous, temporary
            // variables, is to workaround a compiler crash with Q_CC_NOKIAX86.
            QString rootPath = QDir(longPath).rootPath();
            pathElements.prepend(rootPath);
        }

        if (pathElements.at(0).endsWith(QLatin1Char('/')))
            pathElements[0].chop(1);
    }
#else
    // add the "/" item, since it is a valid path element on Unix
    if (absolutePath[0] == QLatin1Char('/'))
        pathElements.prepend(QLatin1String("/"));
#endif

    DirNode *parent = &m_root;
    qDebug() << pathElements;
    QString last_filename = pathElements.back();
    pathElements.pop_back();

    for (int i = 0; i < pathElements.count(); ++i)
    {
        QString element = pathElements.at(i);
        DirNode* node;
#ifdef Q_OS_WIN
        // On Windows, "filename......." and "filename" are equivalent Task #133928
        while (element.endsWith(QLatin1Char('.')))
            element.chop(1);
#endif
        bool alreadyExists = parent->m_dir_children.contains(element);

        if (alreadyExists)
        {
            node = parent->m_dir_children.value(element);
        }
        else
        {

            // Someone might call ::index("file://cookie/monster/doesn't/like/veggies"),
            // a path that doesn't exists, I.E. don't blindly create directories.
            QFileInfo info(absolutePath);

            if (!info.exists())
            {
                qDebug() << "absolute path " << absolutePath << " is not exists";
                return (&m_root);
            }

            // generate node with fake info and next request real info
            node = new DirNode(parent, info);
            node->m_filename = element;
            qDebug() << "node request for: " << node->filepath();
            QFileInfo node_info(node->filepath());
            node->m_info = node_info;
            parent->add_node(node);
        }

        Q_ASSERT(node);
        parent = node;
    }

    if (parent->m_dir_children.contains(last_filename))
    {
        return parent->m_dir_children.value(last_filename);
    }

    if (parent->m_file_children.contains(last_filename))
    {
        return parent->m_file_children.value(last_filename);
    }


    FileNode* p = &m_root;
    QFileInfo info(absolutePath);

    if (info.exists())
    {
        if (info.isFile())
        {
            p = new FileNode(parent, info);
            parent->add_node(p);
        }
        else if (info.isDir())
        {
            p = new DirNode(parent, info);
            ((DirNode*)p)->populate();
            parent->add_node(p);
        }
    }

    return (p);
}

// simple compare operator for our pairs
bool operator<(const QVector<QString>& v1, const QVector<QString>& v2)
{
    return v1.size() < v2.size();
}

void Session::saveFileSystem()
{
    qDebug() << "saveFileSystem: " << m_dirs.size();
    Preferences pref;
    pref.beginGroup("SharedDirectories");
    pref.beginWriteArray("ShareDirs");

    int dir_indx = 0;
    for (std::set<DirNode*>::const_iterator itr = m_dirs.begin(); itr != m_dirs.end(); ++itr)
    {
        const DirNode* p = *itr;
        qDebug() << "save shared directory: " << p->filepath();

        pref.setArrayIndex(dir_indx);
        pref.setValue("Path", p->filepath());
        QStringList efiles = p->exclude_files();

        if (!efiles.isEmpty())
        {
            int file_indx = 0;
            pref.beginWriteArray("ExcludeFiles", efiles.size());

            foreach(const QString& efile, efiles)
            {
                pref.setArrayIndex(file_indx);
                pref.setValue("FileName", efile);
                ++file_indx;
            }

            pref.endArray();
        }

        ++dir_indx;
    }

    pref.endArray();
    pref.endGroup();
}

void Session::loadFileSystem()
{
    Preferences pref;
    typedef QPair<QString, QVector<QString> > SD;
    QVector<SD> vf;

    pref.beginGroup("SharedDirectories");
    int dcount = pref.beginReadArray("ShareDirs");
    vf.resize(dcount);

    for (int i = 0; i < dcount; ++i)
    {

        pref.setArrayIndex(i);
        vf[i].first = pref.value("Path").toString();

        int fcount = pref.beginReadArray("ExcludeFiles");
        vf[i].second.resize(fcount);

        for (int j = 0; j < fcount; ++ j)
        {
            pref.setArrayIndex(j);
            vf[i].second[j] = pref.value("FileName").toString();
        }

        pref.endArray();

    }

    pref.endArray();

    // sort dirs ASC to avoid update states on sharing
    std::sort(vf.begin(), vf.end());

    foreach(const SD& item, vf)
    {
        FileNode* dir_node = node(item.first);

        if (dir_node != &m_root)
        {
            qDebug() << "load shared directory: " << dir_node->filepath();
            dir_node->share(false);
            QDir filepath(item.first);

            for(int i = 0; i < item.second.size(); ++i)
            {
                FileNode* file_node = node(filepath.absoluteFilePath(item.second[i]));

                if (file_node != &m_root)
                {
                    file_node->unshare(false);
                }
            }
        }

    }
}

void Session::share(const QString& filepath, bool recursive)
{
    FileNode* p = node(filepath);
    if (p != &m_root) p->share(recursive);
}

void Session::unshare(const QString& filepath, bool recursive)
{
    FileNode* p = node(filepath);
    if (p != &m_root) p->unshare(recursive);
}
