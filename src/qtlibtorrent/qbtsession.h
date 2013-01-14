/*
 * Bittorrent Client using Qt4 and libtorrent.
 * Copyright (C) 2006  Christophe Dumez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * Contact : chris@qbittorrent.org
 */
#ifndef __BITTORRENT_H__
#define __BITTORRENT_H__

#include <QHash>
#include <QUrl>
#include <QStringList>
#ifdef DISABLE_GUI
#include <QCoreApplication>
#else
#include <QApplication>
#include <QPalette>
#endif
#include <QPointer>
#include <QTimer>

#include <libtorrent/version.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/ip_filter.hpp>

#include <transport/session_base.h>

#include "qtracker.h"
#include "qtorrenthandle.h"
#include "trackerinfos.h"

#define MAX_SAMPLES 20

class DownloadThread;
class FilterParserThread;
#ifndef RSS_ENABLE
class HttpServer;
#endif
class BandwidthScheduler;
class ScanFoldersModel;
class DNSUpdater;

namespace aux
{

class QBtSession : public SessionBase
{
    Q_OBJECT
    Q_DISABLE_COPY(QBtSession)

private:

  enum shutDownAction { NO_SHUTDOWN, SHUTDOWN_COMPUTER, SUSPEND_COMPUTER };

public:
  explicit QBtSession();
  void start();
  void stop();
  bool started() const;
  ~QBtSession();

  QTorrentHandle getTorrentHandle(const QString &hash) const;
  std::vector<libtorrent::torrent_handle> getTorrents() const;
  Transfer getTransfer(const QString& hash) const;
  std::vector<Transfer> getTransfers() const;
  qreal getPayloadDownloadRate() const;
  qreal getPayloadUploadRate() const;
  SessionStatus getSessionStatus() const;
  QHash<QString, TrackerInfos> getTrackersInfo(const QString &hash) const;
  bool hasDownloadingTorrents() const;
  inline QStringList getPeerBanMessages() const { return peerBanMessages; }
  inline libtorrent::session* getSession() const { return s; }
  inline bool useTemporaryFolder() const { return !defaultTempPath.isEmpty(); }
  inline QString getDefaultSavePath() const { return defaultSavePath; }
  inline ScanFoldersModel* getScanFoldersModel() const {  return m_scanFolders; }
  inline bool isDHTEnabled() const { return DHTEnabled; }
  inline bool isLSDEnabled() const { return LSDEnabled; }
  inline bool isPexEnabled() const { return PeXEnabled; }
  inline bool isQueueingEnabled() const { return queueingEnabled; }

  virtual void saveTempFastResumeData();
  virtual void readAlerts();

public slots:
  void addTransferFromFile(const QString& filename);
  QED2KHandle addTransfer(const libed2k::add_transfer_params&);
  QTorrentHandle addTorrent(QString path, bool fromScanDir = false, QString from_url = QString(), bool resumed = false);
  QPair<Transfer,ErrorCode> addLink(QString strLink, bool resumed = false);
  void loadSessionState();
  void saveSessionState();
  void downloadFromUrl(const QString &url);
  void deleteTransfer(const QString& hash, bool delete_files);
  void startUpTransfers();
  void recheckTransfer(const QString& hash);
  void useAlternativeSpeedsLimit(bool alternative);
  void preAllocateAllFiles(bool b);
  void saveFastResumeData();
  void enableIPFilter(const QString &filter_path, bool force=false);
  void disableIPFilter();
  void setQueueingEnabled(bool enable);
  void handleDownloadFailure(QString url, QString reason);
  void downloadUrlAndSkipDialog(QString url, QString save_path=QString(), QString label=QString());
  // Session configuration - Setters
  void setListeningPort(int port);
  void setMaxConnections(int maxConnec);
  void setMaxConnectionsPerTorrent(int max);
  void setMaxUploadsPerTorrent(int max);
  void setDownloadRateLimit(long rate);
  void setUploadRateLimit(long rate);
  void setGlobalMaxRatio(qreal ratio);
  qreal getGlobalMaxRatio() const { return global_ratio_limit; }
  qreal getMaxRatioPerTransfer(const QString& hash, bool* use_global) const;
  void setMaxRatioPerTransfer(const QString& hash, qreal ratio);
  void removeRatioPerTransfer(const QString& hash);
  void setDHTPort(int dht_port);
  void setProxySettings(libtorrent::proxy_settings proxySettings);
  void setSessionSettings(const libtorrent::session_settings &sessionSettings);
  void startTorrentsInPause(bool b);
  void setDefaultTempPath(QString temppath);
  void setAppendLabelToSavePath(bool append);
  void appendLabelToTorrentSavePath(const QTorrentHandle &h);
  void changeLabelInSavePath(const Transfer& t, const QString& old_label, const QString& new_label);
  void appendqBextensionToTorrent(const QTorrentHandle &h, bool append);
  void setAppendqBExtension(bool append);
  void applyEncryptionSettings(libtorrent::pe_settings se);
  void setDownloadLimit(const QString& hash, long val);
  void setUploadLimit(const QString& hash, long val);
  void enableUPnP(bool b);
  void enableLSD(bool b);
  bool enableDHT(bool b);
  void addPeerBanMessage(QString msg, bool from_ipfilter);
  void processDownloadedFile(QString, QString);
  void addMagnetSkipAddDlg(QString uri);
  void downloadFromURLList(const QStringList& urls);
  void configureSession();
  void banIP(QString ip);
  void recursiveTorrentDownload(const QTorrentHandle &h);

private:
  QString getSavePath(const QString &hash, bool fromScanDir = false, QString filePath = QString::null, QString root_folder=QString::null);
  bool loadFastResumeData(const QString &hash, std::vector<char> &buf);
  void loadTorrentSettings(QTorrentHandle &h);
  void loadTorrentTempData(QTorrentHandle &h, QString savePath, bool magnet);
  libtorrent::add_torrent_params initializeAddTorrentParams(const QString &hash);
  libtorrent::entry generateFilePriorityResumeData(boost::intrusive_ptr<libtorrent::torrent_info> &t, const std::vector<int> &fp);
  void updateRatioTimer();

private slots:
  void addTorrentsFromScanFolder(QStringList&);
  void processBigRatios();
  void exportTorrentFiles(QString path);  
  void sendNotificationEmail(const QTorrentHandle &h);
  void mergeTorrents(QTorrentHandle &h_ex, boost::intrusive_ptr<libtorrent::torrent_info> t);
  void exportTorrentFile(const QTorrentHandle &h);
  void initWebUi();
  void handleIPFilterParsed(int ruleCount);
  void handleIPFilterError();

signals:
  void addedTorrent(const QTorrentHandle& h);
  void deletedTorrent(const QString &hash);
  void torrentAboutToBeRemoved(const QTorrentHandle &h, bool del_files);
  void pausedTorrent(const QTorrentHandle& h);
  void finishedTorrent(const QTorrentHandle& h);
  void trackerError(const QString &hash, QString time, QString msg);
  void trackerAuthenticationRequired(const QTorrentHandle& h);
  void newDownloadedTorrent(QString path, QString url);
  void updateFileSize(const QString &hash);
  void downloadFromUrlFailure(QString url, QString reason);
  void torrentFinishedChecking(const QTorrentHandle& h);
  void metadataReceived(const QTorrentHandle &h);
  void newBanMessage(const QString &msg);
  void alternativeSpeedsModeChanged(bool alternative);
  void recursiveTorrentDownloadPossible(const QTorrentHandle &h);
  void ipFilterParsed(bool error, int ruleCount);
  void listenSucceeded();

private:
  // Bittorrent
  libtorrent::session *s;
  QPointer<BandwidthScheduler> bd_scheduler;
  QMap<QUrl, QPair<QString, QString> > savepathLabel_fromurl; // Use QMap for compatibility with Qt < 4.7: qHash(QUrl)
  QHash<QString, QHash<QString, TrackerInfos> > trackersInfos;
  QHash<QString, QString> savePathsToRemove;
  QStringList torrentsToPausedAfterChecking;
  QTimer resumeDataTimer;
  // Ratio
  QPointer<QTimer> BigRatioTimer;
  // HTTP
  DownloadThread* downloader;
  // File System
  ScanFoldersModel *m_scanFolders;
  // Console / Log
  QStringList peerBanMessages;
  // Settings
  bool preAllocateAll;
  bool addInPause;
  qreal global_ratio_limit;
  int high_ratio_action;
  bool LSDEnabled;
  bool DHTEnabled;
  int current_dht_port;
  bool PeXEnabled;
  bool queueingEnabled;
  bool appendLabelToSavePath;
  bool torrentExport;
  bool appendqBExtension;
  QString defaultSavePath;
  QString defaultTempPath;
  // IP filtering
  QPointer<FilterParserThread> filterParser;
  QString filterPath;
#ifdef RSS_ENABLE
  // Web UI
  QPointer<HttpServer> httpServer;
#endif
  QList<QUrl> url_skippingDlg;
  // GeoIP
#ifndef DISABLE_GUI
  bool geoipDBLoaded;
  bool resolve_countries;
#endif
  // Tracker
  QPointer<QTracker> m_tracker;
  shutDownAction m_shutdownAct;
  // Port forwarding
  libtorrent::upnp *m_upnp;
  libtorrent::natpmp *m_natpmp;
  // DynDNS
  DNSUpdater *m_dynDNSUpdater;
};

}

typedef DeferredSessionProxy<aux::QBtSession> QBtSession;

#endif
