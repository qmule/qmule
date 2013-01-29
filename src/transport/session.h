
#ifndef __SESSION_H__
#define __SESSION_H__

#include <QScopedPointer>

#include "delay.h"
#include "transport/transfer.h"
#include "qtlibtorrent/qbtsession.h"
#include "qtlibed2k/qed2ksession.h"
#include "torrentspeedmonitor.h"
#include "session_filesystem.h"


/**
 * Generic data transfer session
 */
class Session: public SessionBase
{
    Q_OBJECT
    Q_DISABLE_COPY(Session)

public:
    static Session* instance();
    static void drop();

    virtual ~Session();
    QBtSession* get_torrent_session();
    QED2KSession* get_ed2k_session();

    void start();
    void stop();
    bool started() const;

    Transfer getTransfer(const QString& hash) const;
    std::vector<Transfer> getTransfers() const;
    qlonglong getETA(const QString& hash) const;
    qreal getGlobalMaxRatio() const;
    qreal getMaxRatioPerTransfer(const QString& hash, bool* use_global) const;
    QStringList getConsoleMessages() const;
    QStringList getPeerBanMessages() const;
    SessionStatus getSessionStatus() const;
    void changeLabelInSavePath(const Transfer& t, const QString& old_label, const QString& new_label);
    QTorrentHandle addTorrent(const QString& path, bool fromScanDir = false,
                              QString from_url = QString(), bool resumed = false);    
    QED2KHandle addTransfer(const libed2k::add_transfer_params& params);
    void downloadFromUrl(const QString& url);
    void processDownloadedFile(const QString& url, const QString& path);
    void deleteTransfer(const QString& hash, bool delete_files);
    void recheckTransfer(const QString& hash);
    void setDownloadLimit(const QString& hash, long limit);
    void setUploadLimit(const QString& hash, long limit);
    void setMaxRatioPerTransfer(const QString& hash, qreal ratio);
    void removeRatioPerTransfer(const QString& hash);
    void useAlternativeSpeedsLimit(bool alternative);
    void banIP(QString ip);
    QHash<QString, TrackerInfos> getTrackersInfo(const QString &hash) const;
    void setDownloadRateLimit(long rate);
    void setUploadRateLimit(long rate);
    bool hasActiveTransfers() const;

    bool useTemporaryFolder() const;
    bool isDHTEnabled() const;
    bool isPexEnabled() const;
    bool isLSDEnabled() const;
    bool isQueueingEnabled() const;
    bool isListening() const;

    void deferPlayMedia(Transfer t, int fileIndex);
    bool playMedia(Transfer t, int fileIndex);

    void saveFileSystem();
    void loadFileSystem();
    void dropDirectoryTransfers();
    void share(const QString& filepath, bool recursive);
    void unshare(const QString& filepath, bool recursive);
    DirNode* root() { return &m_root; }
    std::set<DirNode*>& directories() { return m_dirs; }
    QHash<QString, FileNode*>& files() { return m_files; }

public slots:
    void playPendingMedia();
	void startUpTransfers();
	void configureSession();
	void enableIPFilter(const QString &filter_path, bool force=false);
    void playLink(const QString& strLink);

    /** select appropriate session and run command on it */
    QPair<Transfer,ErrorCode> addLink(QString strLink, bool resumed = false);
    void addTransferFromFile(const QString& filename);

signals:
    void metadataReceived(Transfer t);
    void transferFinishedChecking(Transfer t);
    void trackerAuthenticationRequired(Transfer t);
    void newDownloadedTransfer(QString path, QString url);
    void downloadFromUrlFailure(QString url, QString reason);
    void alternativeSpeedsModeChanged(bool alternative);
    void recursiveDownloadPossible(QTorrentHandle t);    
    void newBanMessage(QString msg);
    // filesystem signals
    void changeNode(const FileNode* node);
    void beginRemoveNode(const FileNode* node);
    void endRemoveNode();
    void beginInsertNode(const FileNode* node);
    void endInsertNode();

    void removeSharedDirectory(const DirNode*);
    void insertSharedDirectory(const DirNode*);

    void removeSharedFile(FileNode*);
    void insertSharedFile(FileNode*);

    void beginLoadSharedFileSystem();
    void endLoadSharedFileSystem();
private slots:
    void on_addedTorrent(const QTorrentHandle& h);
    void on_pausedTorrent(const QTorrentHandle& h);
    void on_finishedTorrent(const QTorrentHandle& h);
    void on_metadataReceived(const QTorrentHandle& h);
    void on_torrentAboutToBeRemoved(const QTorrentHandle& h, bool del_files);
    void on_transferAboutToBeRemoved(const Transfer& t, bool del_files);
    void on_torrentFinishedChecking(const QTorrentHandle& h);
    void on_trackerAuthenticationRequired(const QTorrentHandle& h);
    void on_savePathChanged(const QTorrentHandle& h);
    void saveTempFastResumeData();
    void readAlerts();
    void saveFastResumeData();

    void on_registerNode(Transfer);
    void on_transferParametersReady(const libed2k::add_transfer_params&, const libed2k::error_code&);
    void on_ED2KResumeDataLoaded();

private:
    Session();
    SessionBase* delegate(const QString& hash) const;
    SessionBase* delegate(const Transfer& t) const;

    template<typename Functor>
    void for_each(const Functor& f)
    {
        std::for_each(m_sessions.begin(), m_sessions.end(), f);
    }

    void addDirectory(DirNode* dir);
    void removeDirectory(DirNode* dir);
    void setDirectLink(const QString& hash, DirNode* node);
    void registerNode(FileNode*);
    FileNode* node(const QString& filepath);

    // emitters
    void signal_beginRemoveNode(const FileNode* node) { emit beginRemoveNode(node);}
    void signal_endRemoveNode() { emit endRemoveNode();}
    void signal_beginInsertNode(const FileNode* node) { emit beginInsertNode(node);}
    void signal_endInsertNode() { emit endInsertNode();}
    void signal_changeNode(const FileNode* node) { emit changeNode(node);}
    void prepare_collections();

    static Session* m_instance;

    QBtSession m_btSession;
    QED2KSession m_edSession;
    std::vector<SessionBase*> m_sessions;

    QScopedPointer<TorrentSpeedMonitor> m_speedMonitor;
    QScopedPointer<QTimer>  m_periodic_resume;
    QScopedPointer<QTimer>  m_alerts_reading;

    std::set<QPair<QString, int> > m_pending_medias;

    DirNode m_root;
    Delay                       m_delay;
    QHash<QString, FileNode*>   m_files;    // all registered files in ed2k filesystem
    std::set<DirNode*>          m_dirs;     // shared directories
    QString                     m_incoming; // incoming filepath

    friend class DirNode;
    friend class FileNode;
};

#endif
