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
#include <QSplashScreen>

#include "qtlibed2k/qed2ksession.h"
#include "transport/transfer.h"
#include "ui_mainwindow.h"
#include "qtorrenthandle.h"
#include "transfer_list.h"
#include "infodlg.h"
#include "silent_updater.h"
#include "taskbar_iface.h"
#include "wgetter.h"

class downloadFromURL;
class options_imp;
class TransferListWidget;
class TransferListFiltersWidget;
class status_bar;
class consoleDlg;
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

  enum Widgets
  {
      wStatus       = 1,
      wCatalog      = 2,
      wTransfer     = 3,
      wSearch       = 4,
      wMessages     = 5,
      wFiles        = 6
  };

  // Construct / Destruct
  MainWindow(QSplashScreen* sscrn, QWidget *parent=0, QStringList torrentCmdLine=QStringList());
  ~MainWindow();
  TransferListWidget* getTransferList() const { return transfer_List->getTransferList(); }
  QMenu* getTrayIconMenu();

public slots:
  void trackerAuthenticationRequired(const Transfer& h);
  void setTabText(int index, QString text) const;
  void showNotificationBaloon(QString title, QString msg) const;
  void downloadFromURLList(const QStringList& urls);
  void updateAltSpeedsBtn(bool alternative);
  void deleteSession();
  void on_actionOpen_triggered();
  void addConsoleMessage(
      const QString& msg, QColor color = QApplication::palette().color(QPalette::WindowText)) const;

protected slots:
  // GUI related slots
  void dropEvent(QDropEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);
  void toggleVisibility(QSystemTrayIcon::ActivationReason e = QSystemTrayIcon::Trigger);
  void on_actionCreate_torrent_triggered();
  void on_actionWebsite_triggered() const;
  void on_actionBugReport_triggered() const;
  void balloonClicked();
  void writeSettings();
  void readSettings();
  void on_actionExit_triggered();
  void createTrayIcon();
  void fileError(const Transfer& h, QString msg);
  void handleDownloadFromUrlFailure(QString, QString) const;
  void createSystrayDelayed();
  void tab_changed(int);
  void on_actionLock_qMule_triggered();
  void defineUILockPassword();
  bool unlockUI();
  void notifyOfUpdate(QString);
  void showConnectionSettings();
  void minimizeWindow();
  // Keyboard shortcuts
  void createKeyboardShortcuts();
  void displayTransferTab() const;
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
  void addedTransfer(const Transfer& h) const;
  void finishedTransfer(const Transfer& h) const;
  void askRecursiveTorrentDownloadConfirmation(const QTorrentHandle &h);
  // Options slots
  void on_actionOptions_triggered();
  void optionsSaved();


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

#ifdef Q_WS_WIN
  bool winEvent(MSG * message, long * result);
#endif

private:
  QIcon getSystrayIcon() const;
  void selectWidget(Widgets wNum);

private:
  void activateControls(bool status);
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
  QPointer<TorrentCreatorDlg> createTorrentDlg;
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

  bool displaySpeedInTitle;
  bool force_exit;
  bool ui_locked;
  LineEdit *search_filter;
  // Keyboard shortcuts
  QShortcut *switchTransferShortcut;
  QShortcut *hideShortcut;
  // Widgets
  QAction *prioSeparator;
  QAction *prioSeparatorMenu;
  // Execution Log
  QPointer<ExecutionLog> m_executionLog;
  // Power Management
  PowerManagement *m_pwr;
  QTimer *preventTimer;
  QTimer *flickerTimer;
  QScopedPointer<is_info_dlg> m_info_dlg;
  QScopedPointer<silent_updater> m_updater;
  QScopedPointer<taskbar_iface>  m_tbar;
  QScopedPointer<QSplashScreen>  m_sscrn;
  QScopedPointer<wgetter>        m_ipf_getter;
  unsigned int m_nTaskbarButtonCreated;

  QIcon icon_disconnected;
  QIcon icon_connected;
  QIcon icon_connecting;

  QIcon icon_TrayConn;
  QIcon icon_TrayDisconn;
  QIcon icon_NewMsg;
  QIcon icon_CurTray;

  ConeectionState connectioh_state;
  bool            m_bDisconnectBtnPressed;
  QDateTime       m_last_file_error;

private slots:
    void on_actionSpeed_in_title_bar_triggered();
    void on_actionTop_tool_bar_triggered();
    void on_action_Import_Torrent_triggered();
    void on_actionExecution_Logs_triggered(bool checked);
    void on_actionAutoExit_qMule_toggled(bool );
    void on_actionAutoSuspend_system_toggled(bool );
    void on_actionAutoShutdown_system_toggled(bool );
    // Check for active torrents and set preventing from suspend state
    void checkForActiveTorrents();
    void startChat(const QString& user_name, const libed2k::net_identifier& np);
    void addFriend(const QString& user_name, const libed2k::net_identifier& np);
    void startMessageFlickering();
    void stopMessageFlickering();
    void on_flickerTimer();
    void setDisconnectedStatus();

    void ed2kServerNameResolved(QString strServer);
    void ed2kConnectionInitialized(quint32 client_id, quint32 tcp_flags, quint32 aux_port);
    void ed2kServerStatus(int nFiles, int nUsers);
    void ed2kServerMessage(QString strMessage);
    void ed2kIdentity(QString strName, QString strDescription);
    void ed2kConnectionClosed(QString strError);

    void on_actionOpenDownloadPath_triggered();
    void beginLoadSharedFileSystem();
    void endLoadSharedFileSystem();

    void new_version_ready(int,int,int,int);
    void current_version_obsolete(int,int,int,int);
};

#endif
