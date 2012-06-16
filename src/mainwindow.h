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

#ifndef GUI_H
#define GUI_H

#include <QProcess>
#include <QSystemTrayIcon>
#include <QPointer>

#include <libed2k/is_https_auth.hpp>

#include "qtlibed2k/qed2ksession.h"
#include "transport/transfer.h"
#include "ui_mainwindow.h"
#include "qtorrenthandle.h"
#include "transfer_list.h"

class QBtSession;
class downloadFromURL;
//class SearchEngine;
#ifdef RSS_ENABLE
class RSSImp;
#endif
class about;
class options_imp;
class TransferListWidget;
class TransferListFiltersWidget;
class PropertiesWidget;
class status_bar;
class consoleDlg;
class about;
class TorrentCreatorDlg;
class downloadFromURL;
class HidableTabWidget;
class LineEdit;
class ExecutionLog;
class PowerManagement;
class status_widget;
class search_widget;
class XCatalogWidget;
class messages_widget;
class files_widget;

QT_BEGIN_NAMESPACE
class QCloseEvent;
class QFileSystemWatcher;
class QShortcut;
class QSplitter;
class QTabWidget;
class QTimer;
class QVBoxLayout;
QT_END_NAMESPACE

class MainWindow : public QMainWindow, private Ui::MainWindow{
  Q_OBJECT

friend class callback_wrapper;

public:

  enum ConeectionState
  {
      csDisconnected = 1,
      csConnecting   = 2,
      csConnected    = 3
  };

  // Construct / Destruct
  MainWindow(QWidget *parent=0, QStringList torrentCmdLine=QStringList());
  ~MainWindow();
  // Methods
  //QWidget* getCurrentTabWidget() const;
  TransferListWidget* getTransferList() const { return transfer_List->getTransferList(); }
  QMenu* getTrayIconMenu();
//  PropertiesWidget *getProperties() const { return properties; }

public slots:
  void trackerAuthenticationRequired(const Transfer& h);
  void setTabText(int index, QString text) const;
  void showNotificationBaloon(QString title, QString msg) const;
  void downloadFromURLList(const QStringList& urls);
  void updateAltSpeedsBtn(bool alternative);
  void updateNbTorrents();
  void deleteBTSession();
  void on_actionOpen_triggered();
  void emitAuthSignal(const std::string& strRes, const boost::system::error_code& error);
  void addToLog(QString log_message);

protected slots:
  // GUI related slots
  void dropEvent(QDropEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void toggleVisibility(QSystemTrayIcon::ActivationReason e = QSystemTrayIcon::Trigger);
  void on_actionAbout_triggered();
  void on_actionCreate_torrent_triggered();
  void on_actionWebsite_triggered() const;
  void on_actionBugReport_triggered() const;
  void balloonClicked();
  void writeSettings();
  void readSettings();
  void on_actionExit_triggered();
  void createTrayIcon();
  void fullDiskError(const Transfer& h, QString msg) const;
  void handleDownloadFromUrlFailure(QString, QString) const;
  void createSystrayDelayed();
  void tab_changed(int);
  void on_actionLock_qBittorrent_triggered();
  void defineUILockPassword();
  bool unlockUI();
  void notifyOfUpdate(QString);
  void showConnectionSettings();
  void minimizeWindow();
  // Keyboard shortcuts
  void createKeyboardShortcuts();
  void displayTransferTab() const;
  void displaySearchTab() const;
  void displayRSSTab() const;
  // Torrent actions
  void on_actionSet_global_upload_limit_triggered();
  void on_actionSet_global_download_limit_triggered();
  void on_actionDocumentation_triggered() const;  
  void updateGUI();
  void loadPreferences(bool configure_session=true);
  void processParams(const QString& params);
  void processParams(const QStringList& params);
  void addTorrent(QString path);
  void addUnauthenticatedTracker(const QPair<Transfer,QString> &tracker);
  void processDownloadedFiles(QString path, QString url);
  void finishedTorrent(const Transfer& h) const;
  void askRecursiveTorrentDownloadConfirmation(const QTorrentHandle &h);
  // Options slots
  void on_actionOptions_triggered();
  void optionsSaved();
  // HTTP slots
  void on_actionDownload_from_URL_triggered();
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
  void handleUpdateCheckFinished(bool update_available, QString new_version);
  void handleUpdateInstalled(QString error_msg);
#endif

  void on_actionStatus_triggerd();
  void on_actionTransfer_triggerd();
  void on_actionSearch_triggerd();
  void on_actionCatalog_triggerd();
  void on_actionConnect_triggered();
  void on_actionMessages_triggerd();
  void on_actionFiles_triggerd();

protected:
  void closeEvent(QCloseEvent *);
  void showEvent(QShowEvent *);
  bool event(QEvent * event);
  void displayRSSTab(bool enable);
  void displaySearchTab(bool enable);

private:
  QIcon getSystrayIcon() const;
  void selectWidget(int num);

private:
  QFileSystemWatcher *executable_watcher;
  // Bittorrent
  QList<QPair<Transfer,QString> > unauthenticated_trackers; // Still needed?
  // GUI related
  bool m_posInitialized;
  QTimer *guiUpdater;
  //HidableTabWidget *tabs;
  status_bar* statusBar;
  QPointer<options_imp> options;
  QPointer<consoleDlg> console;
  QPointer<about> aboutDlg;
  QPointer<TorrentCreatorDlg> createTorrentDlg;
  QPointer<downloadFromURL> downloadFromURLDialog;
  QPointer<QSystemTrayIcon> systrayIcon;
  QPointer<QTimer> systrayCreator;
  QPointer<QMenu> myTrayIconMenu;

  QMenu* menuStatus;
  
  QWidget* transfer;
  transfer_list* transfer_List;
  QDockWidget *dock;
  status_widget* status;
  search_widget* search;
  XCatalogWidget* catalog;
  messages_widget* messages;
  files_widget* files;

  QString userName;
  QString userPassword;

  //TransferListFiltersWidget *transferListFilters;
  //PropertiesWidget *properties;
  bool displaySpeedInTitle;
  bool force_exit;
  bool ui_locked;
  LineEdit *search_filter;
  // Keyboard shortcuts
  QShortcut *switchSearchShortcut;
  QShortcut *switchSearchShortcut2;
  QShortcut *switchTransferShortcut;
  QShortcut *switchRSSShortcut;
  // Widgets
  QAction *prioSeparator;
  QAction *prioSeparatorMenu;
  //QSplitter *hSplitter;
  //QSplitter *vSplitter;
  // Search
  //QPointer<SearchEngine> searchEngine;
  // RSS
#ifdef RSS_ENABLE
  QPointer<RSSImp> rssWidget;
#endif
  // Execution Log
  QPointer<ExecutionLog> m_executionLog;
  // Power Management
  PowerManagement *m_pwr;
  QTimer *preventTimer;
  QTimer *authTimer;
  QTimer *flickerTimer;
  libed2k::auth_runner ar;

  QIcon icon_disconnected;
  QIcon icon_connected;
  QIcon icon_connecting;

  QIcon icon_TrayConn;
  QIcon icon_TrayDisconn;
  QIcon icon_NewMsg;
  QIcon icon_CurTray;

  ConeectionState connectioh_state;

private slots:
    void on_actionSearch_engine_triggered();
    void on_actionRSS_Reader_triggered();
    void on_actionSpeed_in_title_bar_triggered();
    void on_actionTop_tool_bar_triggered();
    void on_action_Import_Torrent_triggered();
    void on_actionDonate_money_triggered();
    void on_actionExecution_Logs_triggered(bool checked);
    void on_actionAutoExit_qBittorrent_toggled(bool );
    void on_actionAutoSuspend_system_toggled(bool );
    void on_actionAutoShutdown_system_toggled(bool );
    // Check for active torrents and set preventing from suspend state
    void checkForActiveTorrents();
    void on_auth(const std::string& strRes, const boost::system::error_code& error);
    void authRequest();
    void startAuthByTimer();
    void startChat(const QString& user_name, const libed2k::net_identifier& np);
    void startMessageFlickering();
    void stopMessageFlickering();
    void on_flickerTimer();

    void ed2kServerNameResolved(QString strServer);
    void ed2kConnectionInitialized(unsigned int nClientId);
    void ed2kServerStatus(int nFiles, int nUsers);
    void ed2kServerMessage(QString strMessage);
    void ed2kIdentity(QString strName, QString strDescription);
    void ed2kConnectionFailed(QString strError);

signals:
    void signalAuth(const std::string& strRes, const boost::system::error_code& error);
};

class callback_wrapper
{
public:
    static MainWindow* window;
    static void on_auth(const std::string& strRes, const boost::system::error_code& error)
    { 
        qDebug("emit Auth signal");
        //QMetaObject::invokeMethod(window, "on_auth", Qt::QueuedConnection, Q_ARG(const std::string&, strRes), 
        //    Q_ARG(const boost::system::error_code&, error));
        window->emitAuthSignal(strRes, error); 
    }
};

#endif
