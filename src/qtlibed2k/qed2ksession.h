#ifndef __QED2KSESSION__
#define __QED2KSESSION__

#ifdef DISABLE_GUI
#include <QCoreApplication>
#else
#include <QApplication>
#include <QPalette>
#endif

#include <QPixmap>
#include <QPointer>
#include <QTimer>
#include <QHash>
#include <QStringList>

#include <transport/session_base.h>
#include <libed2k/session.hpp>
#include <libed2k/session_settings.hpp>
#include "qed2khandle.h"
#include "trackerinfos.h"

struct QED2KSearchResultEntry
{
    quint64                 m_nFilesize;
    quint64                 m_nSources;
    quint64                 m_nCompleteSources;
    quint64                 m_nMediaBitrate;
    quint64                 m_nMediaLength;
	QString	                m_hFile;
	QString                 m_strFilename;
	QString	                m_strMediaCodec;
    libed2k::net_identifier m_network_point; 
	bool isCorrect() const;
	QED2KSearchResultEntry();
	static QED2KSearchResultEntry fromSharedFileEntry(const libed2k::shared_file_entry& sf);
};

struct QED2KPeerOptions
{
    quint8  m_nAICHVersion;
    bool    m_bUnicodeSupport;
    quint8  m_nUDPVer;
    quint8  m_nDataCompVer;
    quint8  m_nSupportSecIdent;
    quint8  m_nSourceExchange1Ver;
    quint8  m_nExtendedRequestsVer;
    quint8  m_nAcceptCommentVer;
    bool    m_bNoViewSharedFiles;
    bool    m_bMultiPacket;
    bool    m_bSupportsPreview;

    bool    m_bSupportCaptcha;
    bool    m_bSourceExt2;
    bool    m_bExtMultipacket;
    bool    m_bLargeFiles;
    QED2KPeerOptions(const libed2k::misc_options& mo, const libed2k::misc_options2& mo2);
};

namespace aux
{

class QED2KSession: public QObject, public SessionBase
{
    Q_OBJECT
    Q_DISABLE_COPY(QED2KSession)

public:
    QED2KSession();
    void start();
    bool started() const;
    virtual ~QED2KSession();

    Transfer getTransfer(const QString& hash) const;
    std::vector<Transfer> getTransfers() const;
    qreal getMaxRatioPerTransfer(const QString& hash, bool* use_global) const;
    bool isFilePreviewPossible(const QString& hash) const;
    void changeLabelInSavePath(const Transfer& t, const QString& old_label, const QString& new_label);
    void pauseTransfer(const QString& hash);
    void resumeTransfer(const QString& hash);
    void deleteTransfer(const QString& hash, bool delete_files);
    void recheckTransfer(const QString& hash);
    void setDownloadLimit(const QString& hash, long limit);
    void setUploadLimit(const QString& hash, long limit);
    void setMaxRatioPerTransfer(const QString& hash, qreal ratio);
    void removeRatioPerTransfer(const QString& hash);
    void banIP(QString ip);
    QHash<QString, TrackerInfos> getTrackersInfo(const QString &hash) const;
    void setDownloadRateLimit(long rate);
    void setUploadRateLimit(long rate);
    bool hasActiveTransfers() const;

    libed2k::session* delegate() const;

private:
    QScopedPointer<libed2k::session> m_session;
    libed2k::session_settings m_settings;
    libed2k::fingerprint m_finger;
    QScopedPointer<QTimer> m_alerts_timer;

public slots:
	void startUpTransfers();
	void configureSession();
	void enableIPFilter(const QString &filter_path, bool force=false);

	/**
	  * number parameters were ignored on zero value
	 */
	void searchFiles(const QString& strQuery,
	                quint64 nMinSize,
	                quint64 nMaxSize,
	                unsigned int nSources,
                    unsigned int nCompleteSources,
	                QString strFileType,
	                QString strFileExt,
	                QString strMediaCodec,
	                quint32 nMediaLength,
	                quint32 nMediaBitrate);

	/**
	  * set found file hash here
	 */
	void searchRelatedFiles(QString strHash);
	/**
	  * you can search more results only when previous call return succuss
	 */
	void searchMoreResults();

    libed2k::peer_connection_handle getPeer(const libed2k::net_identifier& np);

private slots:
    void readAlerts();

signals:
    void serverNameResolved(QString strName);
    void serverConnectionInitialized(unsigned int nClientId);
    void serverConnectionClosed(QString strError);
    void serverStatus(int nFiles, int nUsers);
    void serverMessage(QString strMessage);
    void serverIdentity(QString strName, QString strDescription);
    void searchResult(const libed2k::net_identifier& np, const QString& hash, 
                      const std::vector<QED2KSearchResultEntry>& vRes, bool bMoreResult);

    //peer signals
    void peerConnected(const libed2k::net_identifier& np,
                       const QString& hash,
                       bool bActive);

    void peerDisconnected(const libed2k::net_identifier& np, const QString& hash, const libed2k::error_code ec);

    /**
      * incoming message signal
     */
    void peerMessage(const libed2k::net_identifier& np, const QString& hash, const QString& strMessage);
    void peerCaptchaRequest(const libed2k::net_identifier& np, const QString& hash, const QPixmap& pm);
    void peerCaptchaResult(const libed2k::net_identifier& np, const QString& hash, quint8 nResult);

    /**
      * peer rejected our query for files and possibly directories
     */
    void peerSharedFilesAccessDenied(const libed2k::net_identifier& np, const QString& hash);

    /**
      * requested peers files - all
     */
    void peerSharedFiles(const libed2k::net_identifier& np, const QString& hash,
                         const std::vector<QED2KSearchResultEntry>& vRes);

    /**
      * requested peers shared directories
     */
    void peerSharedDirectories(const libed2k::net_identifier& np, const QString& hash, const QStringList& strList);

    /**
      * requested peers files from sprecified directory
     */
    void peerSharedDirectoryFiles(const libed2k::net_identifier& np, const QString& hash,
                                  const QString& strDirectory, const std::vector<QED2KSearchResultEntry>& vRes);

    void addedTransfer(Transfer t);
    void pausedTransfer(Transfer t);
    void resumedTransfer(Transfer t);
    void deletedTransfer(QString hash);
    void transferAboutToBeRemoved(Transfer t);
};

}

typedef NullSessionProxy<aux::QED2KSession> QED2KSession;

#endif //__QED2KSESSION_
