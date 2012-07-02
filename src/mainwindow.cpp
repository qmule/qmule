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

#include <QtGlobal>
#if defined(Q_WS_X11) && defined(QT_DBUS_LIB)
#include <QDBusConnection>
#include "notifications.h"
#endif

#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QTimer>
#include <QDesktopServices>
#include <QStatusBar>
#include <QClipboard>
#include <QCloseEvent>
#include <QShortcut>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QDockWidget>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

#include <libed2k/log.hpp>

#include "transport/session.h"
#include "mainwindow.h"
//#include "transferlistwidget.h"
#include "misc.h"
#include "torrentcreatordlg.h"
#include "downloadfromurldlg.h"
#include "torrentadditiondlg.h"
//#include "searchengine.h"
#ifdef RSS_ENABLE
#include "rss_imp.h"
#include "rsssettings.h"
#endif
#include "about_imp.h"
#include "trackerlogin.h"
#include "options_imp.h"
#include "speedlimitdlg.h"
#include "preferences.h"
#include "trackerlist.h"
#include "peerlistwidget.h"
#include "torrentpersistentdata.h"
#include "transferlistfilterswidget.h"
#include "propertieswidget.h"
#include "statusbar.h"
#include "hidabletabwidget.h"
#include "qinisettings.h"
#include "torrentimportdlg.h"
//#include "torrentmodel.h"
#include "executionlog.h"
#include "iconprovider.h"
#include "status_widget.h"
#include "search_widget.h"
#include "login_dlg.h"
#include "messages_widget.h"
#include "files_widget.h"
#include "status_bar.h"

#include "xcatalog/catalogwidget.h"

#ifdef Q_WS_MAC
#include "qmacapplication.h"
void qt_mac_set_dock_menu(QMenu *menu);
#endif
#include "lineedit.h"
#include "sessionapplication.h"
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
#include "programupdater.h"
#endif
#include "powermanagement.h"

using namespace libtorrent;

#define TIME_TRAY_BALLOON 5000
#define PREVENT_SUSPEND_INTERVAL 60000
#define NOAUTH

/*****************************************************
 *                                                   *
 *                       GUI                         *
 *                                                   *
 *****************************************************/

// Constructor
MainWindow::MainWindow(QWidget *parent, QStringList torrentCmdLine) : QMainWindow(parent), m_posInitialized(false), force_exit(false) {
  setupUi(this);

  Preferences pref;
  ui_locked = pref.isUILocked();
  setWindowTitle(tr("qBittorrent %1", "e.g: qBittorrent v0.x").arg(QString::fromUtf8(VERSION)));
  displaySpeedInTitle = pref.speedInTitleBar();
  // Clean exit on log out
  connect(static_cast<SessionApplication*>(qApp), SIGNAL(sessionIsShuttingDown()), this, SLOT(deleteBTSession()));
  // Setting icons

  icon_TrayConn.addFile(":/emule/TrayConnected.ico", QSize(22, 22));
  icon_TrayConn.addFile(":/emule/TrayConnected.ico", QSize(16, 16));
  icon_TrayConn.addFile(":/emule/TrayConnected.ico", QSize(32, 32));

  icon_TrayDisconn.addFile(":/emule/TrayDisconnected.ico", QSize(22, 22));
  icon_TrayDisconn.addFile(":/emule/TrayDisconnected.ico", QSize(16, 16));
  icon_TrayDisconn.addFile(":/emule/TrayDisconnected.ico", QSize(32, 32));

  icon_NewMsg.addFile(":/emule/statusbar/MessagePending.ico", QSize(22, 22));
  icon_NewMsg.addFile(":/emule/statusbar/MessagePending.ico", QSize(16, 16));
  icon_NewMsg.addFile(":/emule/statusbar/MessagePending.ico", QSize(32, 32));

  icon_CurTray = icon_TrayDisconn;

  QIcon mainWndIcon;
  mainWndIcon.addFile(":/emule/newmule.png", QSize(22, 22));
  mainWndIcon.addFile(":/emule/newmule.png", QSize(16, 16));
  mainWndIcon.addFile(":/emule/newmule.png", QSize(32, 32));

  this->setWindowIcon(mainWndIcon);

  actionOpen->setIcon(IconProvider::instance()->getIcon("list-add"));
  actionDownload_from_URL->setIcon(IconProvider::instance()->getIcon("insert-link"));
  actionSet_upload_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/seeding.png")));
  actionSet_download_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/download.png")));
  actionSet_global_upload_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/seeding.png")));
  actionSet_global_download_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/download.png")));
  actionCreate_torrent->setIcon(IconProvider::instance()->getIcon("document-edit"));
  actionAbout->setIcon(IconProvider::instance()->getIcon("help-about"));
  actionBugReport->setIcon(IconProvider::instance()->getIcon("tools-report-bug"));
  actionDecreasePriority->setIcon(IconProvider::instance()->getIcon("go-down"));
  actionDelete->setIcon(IconProvider::instance()->getIcon("list-remove"));
  actionDocumentation->setIcon(IconProvider::instance()->getIcon("help-contents"));
  actionDonate_money->setIcon(IconProvider::instance()->getIcon("wallet-open"));
  actionExit->setIcon(IconProvider::instance()->getIcon("application-exit"));
  actionIncreasePriority->setIcon(IconProvider::instance()->getIcon("go-up"));
  actionLock_qBittorrent->setIcon(IconProvider::instance()->getIcon("object-locked"));
  actionPause->setIcon(IconProvider::instance()->getIcon("media-playback-pause"));
  actionPause_All->setIcon(IconProvider::instance()->getIcon("media-playback-pause"));
  actionStart->setIcon(IconProvider::instance()->getIcon("media-playback-start"));
  actionStart_All->setIcon(IconProvider::instance()->getIcon("media-playback-start"));
  action_Import_Torrent->setIcon(IconProvider::instance()->getIcon("document-import"));
//  menuAuto_Shutdown_on_downloads_completion->setIcon(IconProvider::instance()->getIcon("application-exit"));

  QMenu *startAllMenu = new QMenu(this);
  startAllMenu->addAction(actionStart_All);
  actionStart->setMenu(startAllMenu);
  QMenu *pauseAllMenu = new QMenu(this);
  pauseAllMenu->addAction(actionPause_All);
  actionPause->setMenu(pauseAllMenu);
  QMenu *lockMenu = new QMenu(this);
  QAction *defineUiLockPasswdAct = lockMenu->addAction(tr("Set the password..."));
  connect(defineUiLockPasswdAct, SIGNAL(triggered()), this, SLOT(defineUILockPassword()));
  actionLock_qBittorrent->setMenu(lockMenu);
  // Creating Bittorrent session
  connect(Session::instance(), SIGNAL(fullDiskError(Transfer, QString)), 
          this, SLOT(fullDiskError(Transfer, QString)));
  connect(Session::instance(), SIGNAL(finishedTransfer(Transfer)),
          this, SLOT(finishedTorrent(Transfer)));
  connect(Session::instance(), SIGNAL(trackerAuthenticationRequired(Transfer)),
          this, SLOT(trackerAuthenticationRequired(Transfer)));
  connect(Session::instance(), SIGNAL(newDownloadedTransfer(QString, QString)),
          this, SLOT(processDownloadedFiles(QString, QString)));
  connect(Session::instance(), SIGNAL(downloadFromUrlFailure(QString, QString)),
          this, SLOT(handleDownloadFromUrlFailure(QString, QString)));
  connect(Session::instance(), SIGNAL(alternativeSpeedsModeChanged(bool)),
          this, SLOT(updateAltSpeedsBtn(bool)));
  connect(Session::instance(), SIGNAL(recursiveDownloadPossible(QTorrentHandle)),
          this, SLOT(askRecursiveTorrentDownloadConfirmation(QTorrentHandle)));
#ifdef Q_WS_MAC
  connect(static_cast<QMacApplication*>(qApp), SIGNAL(newFileOpenMacEvent(QString)), this, SLOT(processParams(QString)));
#endif

  qDebug("create tabWidget");

  menuStatus = new QMenu(this);
  menuStatus->setObjectName(QString::fromUtf8("menuStatus"));
  menuStatus->addAction(actionStatus);
  menuStatus->addAction(actionFiles);
  menuStatus->addAction(actionMessages);
  menuStatus->addAction(actionOptions);
  menuStatus->addSeparator();

  actionTools->setMenu(menuStatus);
  if(QToolButton * btn = qobject_cast<QToolButton *>(toolBar->widgetForAction(actionTools)))
    btn->setPopupMode(QToolButton::InstantPopup);
  
  //tabs = new HidableTabWidget();
  //connect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_changed(int)));
  //vSplitter = new QSplitter(Qt::Horizontal);
  //vSplitter->setChildrenCollapsible(false);
  //hSplitter = new QSplitter(Qt::Vertical);
  //hSplitter->setChildrenCollapsible(true);
  //hSplitter->setContentsMargins(0, 4, 0, 0);

  // Transfer List tab
  dock = new QDockWidget(this, Qt::Popup);
  QWidget* titleWidget = new QWidget(this);
  dock->setTitleBarWidget(titleWidget);
  addDockWidget(Qt::TopDockWidgetArea, dock);
  dock->setFloating(false);
  dock->setFeatures(0);
  transfer_List = new transfer_list(centralwidget, this);
  transfer_List->setParent(dock);
  dock->setWidget(transfer_List);
  
  status = new status_widget(this);
  search = new search_widget(this);
  catalog = new XCatalogWidget(this);
  messages = new messages_widget(this);
  files = new files_widget(this);

  vboxLayout->addWidget(dock);
  vboxLayout->addWidget(status);
  vboxLayout->addWidget(search);
  vboxLayout->addWidget(catalog);
  vboxLayout->addWidget(messages);
  vboxLayout->addWidget(files);

  connect(actionStatus, SIGNAL(triggered()), this, SLOT(on_actionStatus_triggerd()));
  connect(actionTransfer, SIGNAL(triggered()), this, SLOT(on_actionTransfer_triggerd()));
  connect(actionSearch, SIGNAL(triggered()), this, SLOT(on_actionSearch_triggerd()));
  connect(actionCatalog, SIGNAL(triggered()), this, SLOT(on_actionCatalog_triggerd()));
  connect(actionMessages, SIGNAL(triggered()), this, SLOT(on_actionMessages_triggerd()));  
  connect(actionFiles, SIGNAL(triggered()), this, SLOT(on_actionFiles_triggerd()));  
  connect(search, SIGNAL(sendMessage(const QString&, const libed2k::net_identifier&)), this, SLOT(startChat(const QString&, const libed2k::net_identifier&)));  

  connect(messages, SIGNAL(newMessage()), this, SLOT(startMessageFlickering()));
  connect(messages, SIGNAL(stopMessageNotification()), this, SLOT(stopMessageFlickering()));
  flickerTimer = new QTimer(this);
  connect(flickerTimer, SIGNAL(timeout()), SLOT(on_flickerTimer()));

  on_actionCatalog_triggerd();
#ifndef NOAUTH
  actionStatus->setDisabled(true);
  actionTransfer->setDisabled(true);
  actionSearch->setDisabled(true);
  actionCatalog->setDisabled(true);
  menuStatus->setDisabled(true);
#endif

  //properties = new PropertiesWidget(hSplitter, this, transferList);
  //transferListFilters = new TransferListFiltersWidget(vSplitter, transferList);
  //hSplitter->addWidget(switcher1);
  //hSplitter->addWidget(transfer_List);
  //hSplitter->addWidget(transferList);
  //transferList2->hide();
  //hSplitter->addWidget(properties);
  //vSplitter->addWidget(transferListFilters);
  //vSplitter->addWidget(hSplitter);
  //vSplitter->setCollapsible(0, true);
  //vSplitter->setCollapsible(1, false);
  //tabs->addTab(vSplitter, IconProvider::instance()->getIcon("folder-remote"), tr("Transfers"));

  
  // Name filter
  /*
  search_filter = new LineEdit();
//  connect(search_filter, SIGNAL(textChanged(QString)), transferList, SLOT(applyNameFilter(QString)));
  QAction *searchFilterAct = toolBar->insertWidget(actionLock_qBittorrent, search_filter);
  search_filter->setFixedWidth(200);
  QWidget *spacer = new QWidget(this);
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  toolBar->insertWidget(searchFilterAct, spacer);
*/

//  prioSeparator = toolBar->insertSeparator(actionDecreasePriority);
//  prioSeparatorMenu = menu_Edit->insertSeparator(actionDecreasePriority);

  // Transfer list slots
//  connect(actionStart, SIGNAL(triggered()), transferList, SLOT(startSelectedTorrents()));
//  connect(actionStart_All, SIGNAL(triggered()), QBtSession::instance(), SLOT(resumeAllTorrents()));
//  connect(actionPause, SIGNAL(triggered()), transferList, SLOT(pauseSelectedTorrents()));
//  connect(actionPause_All, SIGNAL(triggered()), QBtSession::instance(), SLOT(pauseAllTorrents()));
//  connect(actionDelete, SIGNAL(triggered()), transferList, SLOT(deleteSelectedTorrents()));
//  connect(actionIncreasePriority, SIGNAL(triggered()), transferList, SLOT(increasePrioSelectedTorrents()));
//  connect(actionDecreasePriority, SIGNAL(triggered()), transferList, SLOT(decreasePrioSelectedTorrents()));
//  connect(actionToggleVisibility, SIGNAL(triggered()), this, SLOT(toggleVisibility()));
//  connect(actionMinimize, SIGNAL(triggered()), SLOT(minimizeWindow()));

  m_pwr = new PowerManagement(this);
  preventTimer = new QTimer(this);
  connect(preventTimer, SIGNAL(timeout()), SLOT(checkForActiveTorrents()));

  // Configure BT session according to options
  loadPreferences(false);

  // Start connection checking timer
  guiUpdater = new QTimer(this);
  connect(guiUpdater, SIGNAL(timeout()), this, SLOT(updateGUI()));
  guiUpdater->start(2000);
  // Accept drag 'n drops
  setAcceptDrops(true);
  createKeyboardShortcuts();
  // Create status bar
  statusBar = new status_bar(this, QMainWindow::statusBar());

#ifdef Q_WS_MAC
  setUnifiedTitleAndToolBarOnMac(true);
#endif

  // View settings
  actionTop_tool_bar->setChecked(pref.isToolbarDisplayed());
  actionSpeed_in_title_bar->setChecked(pref.speedInTitleBar());
#ifdef RSS_ENABLE
  actionRSS_Reader->setChecked(RssSettings().isRSSEnabled());
#endif
  actionSearch_engine->setChecked(pref.isSearchEnabled());
  actionExecution_Logs->setChecked(pref.isExecutionLogEnabled());
//  displaySearchTab(actionSearch_engine->isChecked());
//  displayRSSTab(actionRSS_Reader->isChecked());
  on_actionExecution_Logs_triggered(actionExecution_Logs->isChecked());

  // Auto shutdown actions
  QActionGroup * autoShutdownGroup = new QActionGroup(this);
  autoShutdownGroup->setExclusive(true);
  autoShutdownGroup->addAction(actionAutoShutdown_Disabled);
  autoShutdownGroup->addAction(actionAutoExit_qBittorrent);
  autoShutdownGroup->addAction(actionAutoShutdown_system);
  autoShutdownGroup->addAction(actionAutoSuspend_system);
#if !defined(Q_WS_X11) || defined(QT_DBUS_LIB)
  actionAutoShutdown_system->setChecked(pref.shutdownWhenDownloadsComplete());
  actionAutoSuspend_system->setChecked(pref.suspendWhenDownloadsComplete());
#else
  actionAutoShutdown_system->setDisabled(true);
  actionAutoSuspend_system->setDisabled(true);
#endif
  actionAutoExit_qBittorrent->setChecked(pref.shutdownqBTWhenDownloadsComplete());

  if (!autoShutdownGroup->checkedAction())
    actionAutoShutdown_Disabled->setChecked(true);

  // Load Window state and sizes
  readSettings();

  if (!ui_locked) {
    if (pref.startMinimized() && systrayIcon)
      showMinimized();
    else {
      show();
      activateWindow();
      raise();
    }
  }

//  properties->readSettings();

  // Start watching the executable for updates
  executable_watcher = new QFileSystemWatcher();
  connect(executable_watcher, SIGNAL(fileChanged(QString)), this, SLOT(notifyOfUpdate(QString)));
  executable_watcher->addPath(qApp->applicationFilePath());

  // Resume unfinished torrents
  Session::instance()->startUpTransfers();
  // Add torrent given on command line
  processParams(torrentCmdLine);

  // Populate the transfer list
//  transferList->getSourceModel()->populate();

  // Update the number of torrents (tab)
  updateNbTorrents();
  //connect(transferList->getSourceModel(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(updateNbTorrents()));
  //connect(transferList->getSourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(updateNbTorrents()));
  //connect(transferList2->getSourceModel(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(updateNbTorrents()));
  //connect(transferList2->getSourceModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(updateNbTorrents()));

  qDebug("GUI Built");
#ifdef Q_WS_WIN
  if (!pref.neverCheckFileAssoc() && (!Preferences::isTorrentFileAssocSet() || !Preferences::isMagnetLinkAssocSet())) {
    if (QMessageBox::question(0, tr("Torrent file association"),
                             tr("qBittorrent is not the default application to open torrent files or Magnet links.\nDo you want to associate qBittorrent to torrent files and Magnet links?"),
                             QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
      Preferences::setTorrentFileAssoc(true);
      Preferences::setMagnetLinkAssoc(true);
    } else {
      pref.setNeverCheckFileAssoc();
    }
  }
#endif
#ifdef Q_WS_MAC
  qt_mac_set_dock_menu(getTrayIconMenu());
#endif
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
  // Check for update
/*  if (pref.isUpdateCheckEnabled()) {
    ProgramUpdater *updater = new ProgramUpdater(this);
    connect(updater, SIGNAL(updateCheckFinished(bool, QString)), SLOT(handleUpdateCheckFinished(bool, QString)));
    updater->checkForUpdates();
  }*/
#endif

  // Make sure the Window is visible if we don't have a tray icon
  if (!systrayIcon && isHidden()) {
    show();
    activateWindow();
    raise();
  }

  icon_disconnected.addFile(QString::fromUtf8(":/emule/ConnectDoBig.png"), QSize(), QIcon::Normal, QIcon::Off);
  icon_connected.addFile(QString::fromUtf8(":/emule/ConnectDrop.png"), QSize(), QIcon::Normal, QIcon::Off);
  icon_connecting.addFile(QString::fromUtf8(":/emule/ConnectStop.png"), QSize(), QIcon::Normal, QIcon::Off);
  connectioh_state = csDisconnected;
  authTimer = new QTimer(this);
  connect(authTimer, SIGNAL(timeout()), this, SLOT(startAuthByTimer()));
  connect(this, SIGNAL(signalAuth(const QString&, const QString&)), SLOT(on_auth(const QString&, const QString&)), Qt::BlockingQueuedConnection);
  
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverNameResolved(QString)), this, SLOT(ed2kServerNameResolved(QString)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverConnectionInitialized(unsigned int)), this, SLOT(ed2kConnectionInitialized(unsigned int)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverStatus(int, int)), this, SLOT(ed2kServerStatus(int, int)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverMessage(QString)), this, SLOT(ed2kServerMessage(QString)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverIdentity(QString, QString)), this, SLOT(ed2kIdentity(QString, QString)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverConnectionFailed(QString)), this, SLOT(ed2kConnectionFailed(QString)));

  authRequest();
}

void MainWindow::deleteBTSession()
{
  guiUpdater->stop();
  Session::drop();
  m_pwr->setActivityState(false);
  // Save window size, columns size
  writeSettings();
  // Accept exit
  qApp->exit();
}

// Destructor
MainWindow::~MainWindow() {
  qDebug("GUI destruction");
  hide();
#ifdef Q_WS_MAC
  // Workaround to avoid bug http://bugreports.qt.nokia.com/browse/QTBUG-7305
  setUnifiedTitleAndToolBarOnMac(false);
#endif
//  disconnect(tabs, SIGNAL(currentChanged(int)), this, SLOT(tab_changed(int)));
  // Delete other GUI objects
  if(executable_watcher)
    delete executable_watcher;
  delete statusBar;
//  delete search_filter;
//  delete transferList;
  delete guiUpdater;
  if (createTorrentDlg)
    delete createTorrentDlg;
  if (m_executionLog)
    delete m_executionLog;
  if (aboutDlg)
    delete aboutDlg;
  if (options)
    delete options;
  if (downloadFromURLDialog)
    delete downloadFromURLDialog;

  if(systrayCreator) {
    delete systrayCreator;
  }
  if (systrayIcon) {
    delete systrayIcon;
  }
  if (myTrayIconMenu) {
    delete myTrayIconMenu;
  }
//  delete tabs;
  // Keyboard shortcuts
  delete switchSearchShortcut;
  delete switchSearchShortcut2;
  delete switchTransferShortcut;
  delete switchRSSShortcut;
  IconProvider::drop();
  // Delete Session::instance() object
  m_pwr->setActivityState(false);
  qDebug("Deleting Session::instance()");
  Session::drop();
  qDebug("Exiting GUI destructor...");
}

void MainWindow::defineUILockPassword() {
  QString old_pass_md5 = Preferences().getUILockPasswordMD5();
  if (old_pass_md5.isNull()) old_pass_md5 = "";
  bool ok = false;
  QString new_clear_password = QInputDialog::getText(this, tr("UI lock password"), tr("Please type the UI lock password:"), QLineEdit::Password, old_pass_md5, &ok);
  if (ok) {
    new_clear_password = new_clear_password.trimmed();
    if (new_clear_password.size() < 3) {
      QMessageBox::warning(this, tr("Invalid password"), tr("The password should contain at least 3 characters"));
      return;
    }
    if (new_clear_password != old_pass_md5) {
      Preferences().setUILockPassword(new_clear_password);
    }
    QMessageBox::information(this, tr("Password update"), tr("The UI lock password has been successfully updated"));
  }
}

void MainWindow::on_actionLock_qBittorrent_triggered() {
  Preferences pref;
  // Check if there is a password
  if (pref.getUILockPasswordMD5().isEmpty()) {
    // Ask for a password
    bool ok = false;
    QString clear_password = QInputDialog::getText(this, tr("UI lock password"), tr("Please type the UI lock password:"), QLineEdit::Password, "", &ok);
    if (!ok) return;
    pref.setUILockPassword(clear_password);
  }
  // Lock the interface
  ui_locked = true;
  pref.setUILocked(true);
  myTrayIconMenu->setEnabled(false);
  hide();
}

void MainWindow::displayRSSTab(bool enable) {
/*  if (enable) {
    // RSS tab
    if(!rssWidget) {
      rssWidget = new RSSImp(tabs);
      int index_tab = tabs->addTab(rssWidget, tr("RSS"));
      tabs->setTabIcon(index_tab, IconProvider::instance()->getIcon("application-rss+xml"));
    }
  } else {
    if(rssWidget) {
      delete rssWidget;
    }
  }*/
}

void MainWindow::displaySearchTab(bool enable) {
/*  if (enable) {
    // RSS tab
    if(!searchEngine) {
      searchEngine = new SearchEngine(this);
      tabs->insertTab(1, searchEngine, IconProvider::instance()->getIcon("edit-find"), tr("Search"));
    }
  } else {
    if(searchEngine) {
      delete searchEngine;
    }
  }*/
}

void MainWindow::updateNbTorrents() {
//  tabs->setTabText(0, tr("Transfers (%1)").arg(transferList->getSourceModel()->rowCount()));
}

void MainWindow::on_actionWebsite_triggered() const {
  QDesktopServices::openUrl(QUrl(QString::fromUtf8("http://www.qbittorrent.org")));
}

void MainWindow::on_actionDocumentation_triggered() const {
  QDesktopServices::openUrl(QUrl(QString::fromUtf8("http://doc.qbittorrent.org")));
}

void MainWindow::on_actionBugReport_triggered() const {
  QDesktopServices::openUrl(QUrl(QString::fromUtf8("http://bugs.qbittorrent.org")));
}

void MainWindow::tab_changed(int new_tab) {
  Q_UNUSED(new_tab);
  // We cannot rely on the index new_tab
  // because the tab order is undetermined now
/*  if(tabs->currentWidget() == vSplitter) {
    qDebug("Changed tab to transfer list, refreshing the list");
    properties->loadDynamicData();
    return;
  }
  if(tabs->currentWidget() == searchEngine) {
    qDebug("Changed tab to search engine, giving focus to search input");
    searchEngine->giveFocusToSearchInput();
  }*/
}

void MainWindow::writeSettings() {
  QIniSettings settings(QString::fromUtf8("qBittorrent"), QString::fromUtf8("qBittorrent"));
  settings.beginGroup(QString::fromUtf8("MainWindow"));
  settings.setValue("geometry", saveGeometry());
  // Splitter size
//  settings.setValue(QString::fromUtf8("vsplitterState"), vSplitter->saveState());
  settings.endGroup();
//  properties->saveSettings();
}

void MainWindow::readSettings() {
  QIniSettings settings(QString::fromUtf8("qBittorrent"), QString::fromUtf8("qBittorrent"));
  settings.beginGroup(QString::fromUtf8("MainWindow"));
  if(settings.contains("geometry")) {
    if(restoreGeometry(settings.value("geometry").toByteArray()))
      m_posInitialized = true;
  }
  const QByteArray splitterState = settings.value("vsplitterState").toByteArray();
/*  if(splitterState.isEmpty()) {
    // Default sizes
    vSplitter->setSizes(QList<int>() << 120 << vSplitter->width()-120);
  } else {
    vSplitter->restoreState(splitterState);
  }*/
  settings.endGroup();
}

void MainWindow::balloonClicked() {
  if (isHidden()) {
    show();
    if (isMinimized()) {
      showNormal();
    }
    raise();
    activateWindow();
  }
}

// called when a torrent has finished
void MainWindow::finishedTorrent(const Transfer& h) const {
  if (!TorrentPersistentData::isSeed(h.hash()))
    showNotificationBaloon(tr("Download completion"), tr("%1 has finished downloading.", "e.g: xxx.avi has finished downloading.").arg(h.name()));
}

// Notification when disk is full
void MainWindow::fullDiskError(const Transfer& h, QString msg) const {
  if (!h.is_valid()) return;
  showNotificationBaloon(tr("I/O Error", "i.e: Input/Output Error"), tr("An I/O error occured for torrent %1.\n Reason: %2", "e.g: An error occured for torrent xxx.avi.\n Reason: disk is full.").arg(h.name()).arg(msg));
}

void MainWindow::createKeyboardShortcuts() {
  actionCreate_torrent->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+N")));
  actionOpen->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+O")));
  actionExit->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+Q")));
  switchTransferShortcut = new QShortcut(QKeySequence(tr("Alt+1", "shortcut to switch to first tab")), this);
  connect(switchTransferShortcut, SIGNAL(activated()), this, SLOT(displayTransferTab()));
  switchSearchShortcut = new QShortcut(QKeySequence(tr("Alt+2", "shortcut to switch to third tab")), this);
  connect(switchSearchShortcut, SIGNAL(activated()), this, SLOT(displaySearchTab()));
  switchSearchShortcut2 = new QShortcut(QKeySequence(tr("Ctrl+F", "shortcut to switch to search tab")), this);
  connect(switchSearchShortcut2, SIGNAL(activated()), this, SLOT(displaySearchTab()));
  switchRSSShortcut = new QShortcut(QKeySequence(tr("Alt+3", "shortcut to switch to fourth tab")), this);
  connect(switchRSSShortcut, SIGNAL(activated()), this, SLOT(displayRSSTab()));
  actionDocumentation->setShortcut(QKeySequence("F1"));
//  actionOptions->setShortcut(QKeySequence(QString::fromUtf8("Alt+O")));
#ifdef Q_WS_MAC
  actionDelete->setShortcut(QKeySequence("Ctrl+Backspace"));
#else
  actionDelete->setShortcut(QKeySequence(QString::fromUtf8("Del")));
#endif
  actionStart->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+S")));
  actionStart_All->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+Shift+S")));
  actionPause->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+P")));
  actionPause_All->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+Shift+P")));
  actionDecreasePriority->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+-")));
  actionIncreasePriority->setShortcut(QKeySequence(QString::fromUtf8("Ctrl++")));
#ifdef Q_WS_MAC
  actionMinimize->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+M")));
  addAction(actionMinimize);
#endif
}

// Keyboard shortcuts slots
void MainWindow::displayTransferTab() const {
//  tabs->setCurrentWidget(transferList);
}

void MainWindow::displaySearchTab() const {
//  if(searchEngine)
//    tabs->setCurrentWidget(searchEngine);
}

void MainWindow::displayRSSTab() const {
//  if(rssWidget)
//    tabs->setCurrentWidget(rssWidget);
}

// End of keyboard shortcuts slots

void MainWindow::askRecursiveTorrentDownloadConfirmation(const QTorrentHandle &h) {
  Preferences pref;
  if (pref.recursiveDownloadDisabled()) return;
  // Get Torrent name
  QString torrent_name;
  try {
    torrent_name = h.name();
  } catch(invalid_handle&) {
    return;
  }
  QMessageBox confirmBox(QMessageBox::Question, tr("Recursive download confirmation"), tr("The torrent %1 contains torrent files, do you want to proceed with their download?").arg(torrent_name));
  QPushButton *yes = confirmBox.addButton(tr("Yes"), QMessageBox::YesRole);
  /*QPushButton *no = */confirmBox.addButton(tr("No"), QMessageBox::NoRole);
  QPushButton *never = confirmBox.addButton(tr("Never"), QMessageBox::NoRole);
  confirmBox.exec();
  if (confirmBox.clickedButton() == 0) return;
  if (confirmBox.clickedButton() == yes) {
    Session::instance()->get_torrent_session()->recursiveTorrentDownload(h);
    return;
  }
  if (confirmBox.clickedButton() == never) {
    pref.disableRecursiveDownload();
  }
}

void MainWindow::handleDownloadFromUrlFailure(QString url, QString reason) const {
  // Display a message box
  showNotificationBaloon(tr("Url download error"), tr("Couldn't download file at url: %1, reason: %2.").arg(url).arg(reason));
}

void MainWindow::on_actionSet_global_upload_limit_triggered() {
  qDebug("actionSet_global_upload_limit_triggered");
  bool ok;
#if LIBTORRENT_VERSION_MINOR > 15
    int cur_limit = Session::instance()->get_torrent_session()->getSession()->settings().upload_rate_limit; // TODO - check settings source
#else
    int cur_limit = Session::instance()->get_torrent_session()->getSession()->upload_rate_limit();
#endif
  const long new_limit = SpeedLimitDialog::askSpeedLimit(&ok, tr("Global Upload Speed Limit"), cur_limit);
  if (ok) {
    qDebug("Setting global upload rate limit to %.1fKb/s", new_limit/1024.);
    Session::instance()->setUploadRateLimit(new_limit);
    if (new_limit <= 0)
      Preferences().setGlobalUploadLimit(-1);
    else
      Preferences().setGlobalUploadLimit(new_limit/1024.);
  }
}

void MainWindow::on_actionSet_global_download_limit_triggered() {
  qDebug("actionSet_global_download_limit_triggered");
  bool ok;
#if LIBTORRENT_VERSION_MINOR > 15
    int cur_limit = Session::instance()->get_torrent_session()->getSession()->settings().download_rate_limit;
#else
    int cur_limit = Session::instance()->get_torrent_session()->getSession()->download_rate_limit();
#endif
  const long new_limit = SpeedLimitDialog::askSpeedLimit(&ok, tr("Global Download Speed Limit"), cur_limit);
  if (ok) {
    qDebug("Setting global download rate limit to %.1fKb/s", new_limit/1024.);
    Session::instance()->setDownloadRateLimit(new_limit);
    if (new_limit <= 0)
      Preferences().setGlobalDownloadLimit(-1);
    else
      Preferences().setGlobalDownloadLimit(new_limit/1024.);
  }
}

// Necessary if we want to close the window
// in one time if "close to systray" is enabled
void MainWindow::on_actionExit_triggered() {
  force_exit = true;
  close();
}

/*QWidget* MainWindow::getCurrentTabWidget() const {
  if(isMinimized() || !isVisible())
    return 0;
  if(tabs->currentIndex() == 0)
    return transferList;
  return tabs->currentWidget();
}*/

void MainWindow::setTabText(int index, QString text) const {
//  tabs->setTabText(index, text);
}

bool MainWindow::unlockUI() {
  bool ok = false;
  QString clear_password = QInputDialog::getText(this, tr("UI lock password"), tr("Please type the UI lock password:"), QLineEdit::Password, "", &ok);
  if (!ok) return false;
  Preferences pref;
  QString real_pass_md5 = pref.getUILockPasswordMD5();
  QCryptographicHash md5(QCryptographicHash::Md5);
  md5.addData(clear_password.toLocal8Bit());
  QString password_md5 = md5.result().toHex();
  if (real_pass_md5 == password_md5) {
    ui_locked = false;
    pref.setUILocked(false);
    myTrayIconMenu->setEnabled(true);
    return true;
  }
  QMessageBox::warning(this, tr("Invalid password"), tr("The password is invalid"));
  return false;
}

void MainWindow::notifyOfUpdate(QString) {
  // Delete the executable watcher
  delete executable_watcher;
  executable_watcher = 0;
}

// Toggle Main window visibility
void MainWindow::toggleVisibility(QSystemTrayIcon::ActivationReason e) {
  if (e == QSystemTrayIcon::Trigger || e == QSystemTrayIcon::DoubleClick) {
    if (isHidden()) {
      if (ui_locked) {
        // Ask for UI lock password
        if (!unlockUI())
          return;
      }
      show();
      if (isMinimized()) {
        if (isMaximized()) {
          showMaximized();
        }else{
          showNormal();
        }
      }
      raise();
      activateWindow();
    }else{
      hide();
    }
  }
  actionToggleVisibility->setText(isVisible() ? tr("Hide") : tr("Show"));
}

// Display About Dialog
void MainWindow::on_actionAbout_triggered() {
  //About dialog
  if (aboutDlg) {
    aboutDlg->setFocus();
  } else {
    aboutDlg = new about(this);
  }
}

void MainWindow::showEvent(QShowEvent *e) {
  qDebug("** Show Event **");

//  if(getCurrentTabWidget() == transferList) {
//    properties->loadDynamicData();
  //}

  e->accept();

  // Make sure the window is initially centered
  if (!m_posInitialized) {
    move(misc::screenCenter(this));
    m_posInitialized = true;
  }
}

// Called when we close the program
void MainWindow::closeEvent(QCloseEvent *e) {
  Preferences pref;
  const bool goToSystrayOnExit = pref.closeToTray();
  if (!force_exit && systrayIcon && goToSystrayOnExit && !this->isHidden()) {
    hide();
    e->accept();
    return;
  }

  if (pref.confirmOnExit() && Session::instance()->hasActiveTransfers())
  {
    if (e->spontaneous() || force_exit) {
      if (!isVisible())
        show();
      QMessageBox confirmBox(QMessageBox::Question, tr("Exiting qBittorrent"),
                             tr("Some files are currently transferring.\nAre you sure you want to quit qBittorrent?"),
                             QMessageBox::NoButton, this);
      QPushButton *noBtn = confirmBox.addButton(tr("No"), QMessageBox::NoRole);
      QPushButton *yesBtn = confirmBox.addButton(tr("Yes"), QMessageBox::YesRole);
      QPushButton *alwaysBtn = confirmBox.addButton(tr("Always"), QMessageBox::YesRole);
      confirmBox.setDefaultButton(yesBtn);
      confirmBox.exec();
      if (!confirmBox.clickedButton() || confirmBox.clickedButton() == noBtn) {
        // Cancel exit
        e->ignore();
        force_exit = false;
        return;
      }
      if (confirmBox.clickedButton() == alwaysBtn) {
        // Remember choice
        Preferences().setConfirmOnExit(false);
      }
    }
  }
  hide();
  if (systrayIcon) {
    // Hide tray icon
    systrayIcon->hide();
  }
  // Save window size, columns size
  writeSettings();
  // Accept exit
  e->accept();
  qApp->exit();
}

// Display window to create a torrent
void MainWindow::on_actionCreate_torrent_triggered() {
  if (createTorrentDlg) {
    createTorrentDlg->setFocus();
  } else {
    createTorrentDlg = new TorrentCreatorDlg(this);
    connect(createTorrentDlg, SIGNAL(torrent_to_seed(QString)), this, SLOT(addTorrent(QString)));
  }
}

bool MainWindow::event(QEvent * e) {
  switch(e->type()) {
  case QEvent::WindowStateChange: {
    qDebug("Window change event");
    //Now check to see if the window is minimised
    if (isMinimized()) {
      qDebug("minimisation");
      if (systrayIcon && Preferences().minimizeToTray()) {
        qDebug("Has active window: %d", (int)(qApp->activeWindow() != 0));
        // Check if there is a modal window
        bool has_modal_window = false;
        foreach (QWidget *widget, QApplication::allWidgets()) {
          if (widget->isModal()) {
            has_modal_window = true;
            break;
          }
        }
        // Iconify if there is no modal window
        if (!has_modal_window) {
          qDebug("Minimize to Tray enabled, hiding!");
          e->accept();
          QTimer::singleShot(0, this, SLOT(hide()));
          return true;
        }
      }
    }
    break;
  }
#ifdef Q_WS_MAC
  case QEvent::ToolBarChange: {
    qDebug("MAC: Received a toolbar change event!");
    bool ret = QMainWindow::event(e);

    qDebug("MAC: new toolbar visibility is %d", !actionTop_tool_bar->isChecked());
    actionTop_tool_bar->toggle();
    Preferences().setToolbarDisplayed(actionTop_tool_bar->isChecked());
    return ret;
  }
#endif
  default:
    break;
  }
  return QMainWindow::event(e);
}

// Action executed when a file is dropped
void MainWindow::dropEvent(QDropEvent *event) {
  event->acceptProposedAction();
  QStringList files;
  if (event->mimeData()->hasUrls()) {
    const QList<QUrl> urls = event->mimeData()->urls();
    foreach (const QUrl &url, urls) {
      if (!url.isEmpty()) {
        if (url.scheme().compare("file", Qt::CaseInsensitive) == 0)
          files << url.toLocalFile();
        else
          files << url.toString();
      }
    }
  } else {
    files = event->mimeData()->text().split(QString::fromUtf8("\n"));
  }
  // Add file to download list
  Preferences pref;
  const bool useTorrentAdditionDialog = pref.useAdditionDialog();
  foreach (QString file, files) {
    qDebug("Dropped file %s on download list", qPrintable(file));
    if (misc::isUrl(file)) {
      Session::instance()->downloadFromUrl(file);	// TODO - check it for using only in torrent
      continue;
    }
    // Bitcomet or Magnet link
    if (file.startsWith("bc://bt/", Qt::CaseInsensitive)) {
      qDebug("Converting bc link to magnet link");
      file = misc::bcLinkToMagnet(file);
    }
    if (file.startsWith("magnet:", Qt::CaseInsensitive)) {
      if (useTorrentAdditionDialog) {
        torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
        dialog->showLoadMagnetURI(file);
      } else {
        Session::instance()->addMagnetUri(file);	// TODO - check it fir using only in torrent
      }
      continue;
    }
    // Local file
    if (useTorrentAdditionDialog) {
      torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
      if (file.startsWith("file:", Qt::CaseInsensitive))
        file = QUrl(file).toLocalFile();
      dialog->showLoad(file);
    }else{
      Session::instance()->addTorrent(file);	// TODO - possibly it is torrent only
    }
  }
}

// Decode if we accept drag 'n drop or not
void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  foreach (const QString &mime, event->mimeData()->formats()) {
    qDebug("mimeData: %s", mime.toLocal8Bit().data());
  }
  if (event->mimeData()->hasFormat(QString::fromUtf8("text/plain")) || event->mimeData()->hasFormat(QString::fromUtf8("text/uri-list"))) {
    event->acceptProposedAction();
  }
}

/*****************************************************
 *                                                   *
 *                     Torrent                       *
 *                                                   *
 *****************************************************/

// Display a dialog to allow user to add
// torrents to download list
void MainWindow::on_actionOpen_triggered() {
  Preferences pref;
  QIniSettings settings(QString::fromUtf8("qBittorrent"), QString::fromUtf8("qBittorrent"));
  // Open File Open Dialog
  // Note: it is possible to select more than one file
  const QStringList pathsList = QFileDialog::getOpenFileNames(0,
                                                              tr("Open Torrent Files"), settings.value(QString::fromUtf8("MainWindowLastDir"), QDir::homePath()).toString(),
                                                              tr("Torrent Files")+QString::fromUtf8(" (*.torrent)"));
  if (!pathsList.empty()) {
    const bool useTorrentAdditionDialog = pref.useAdditionDialog();
    const uint listSize = pathsList.size();
    for (uint i=0; i<listSize; ++i) {
      if (useTorrentAdditionDialog) {
        torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
        dialog->showLoad(pathsList.at(i));
      }else{
        Session::instance()->addTorrent(pathsList.at(i));
      }
    }
    // Save last dir to remember it
    QStringList top_dir = pathsList.at(0).split(QDir::separator());
    top_dir.removeLast();
    settings.setValue(QString::fromUtf8("MainWindowLastDir"), top_dir.join(QDir::separator()));
  }
}

void MainWindow::on_actionStatus_triggerd() 
{
    selectWidget(1);
}

void MainWindow::on_actionCatalog_triggerd() 
{
    selectWidget(2);
}

void MainWindow::on_actionTransfer_triggerd() 
{
    selectWidget(3);
}

void MainWindow::on_actionSearch_triggerd() 
{
    selectWidget(4);
}

void MainWindow::on_actionMessages_triggerd() 
{
    selectWidget(5);
}

void MainWindow::on_actionFiles_triggerd() 
{
    selectWidget(6);
}

void MainWindow::on_auth_result(const std::string& strRes, const boost::system::error_code& ec)
{
    qDebug("MainWindow::on_auth_result: %s", strRes.c_str());
    QString strError;

    if (ec)
    {
        strError = QString::fromLocal8Bit(ec.message().c_str());
    }

    emit signalAuth(QString::fromUtf8(strRes.c_str(), strRes.size()), strError);
}

void MainWindow::selectWidget(int num)
{
    actionCatalog->setChecked(false);
    actionTransfer->setChecked(false);
    actionSearch->setChecked(false);

    dock->hide();
    status->hide();
    search->hide();
    catalog->hide();
    messages->hide();
    files->hide();

    switch (num)
    {
        case 1://status
        {
            status->show();
            break;
        }
        case 2://catalog
        {
            actionCatalog->setChecked(true);
            catalog->show();
            break;
        }
        case 3://transfer
        {
            actionTransfer->setChecked(true);
            dock->show();
            break;
        }
        case 4://search
        {
            actionSearch->setChecked(true);
            search->show();
            break;
        }
        case 5://messages
        {
            messages->show();
            break;
        }
        case 6://files
        {
            files->show();
            break;
        }
    }
}

void MainWindow::on_actionConnect_triggered() 
{
    QMessageBox msgBox;
    msgBox.setText(tr("Do you want to break network connection?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);

    switch (connectioh_state)
    {
        case csDisconnected:
        {
            authRequest();
            break;
        }
        case csConnecting:
        {
            if (msgBox.exec() == QMessageBox::Ok)
            {
                actionConnect->setIcon(icon_disconnected);
                connectioh_state = csDisconnected;
                ar.stop();
            }
            break;
        }
        case csConnected:
        {
            if (msgBox.exec() == QMessageBox::Ok)
            {
                actionConnect->setIcon(icon_disconnected);
                connectioh_state = csDisconnected;
                status->setDisconnectedInfo();
                statusBar->reset();
                icon_CurTray = icon_TrayDisconn;
                if (systrayIcon) {
                    systrayIcon->setIcon(getSystrayIcon());
                }
            }
            break;
        }
    }
}

// As program parameters, we can get paths or urls.
// This function parse the parameters and call
// the right addTorrent function, considering
// the parameter type.
void MainWindow::processParams(const QString& params_str) {
  processParams(params_str.split("|", QString::SkipEmptyParts));
}

void MainWindow::processParams(const QStringList& params) {
  Preferences pref;
  const bool useTorrentAdditionDialog = pref.useAdditionDialog();
  foreach (QString param, params) {
    param = param.trimmed();
    if (misc::isUrl(param)) {
      Session::instance()->downloadFromUrl(param);
    }else{
      if (param.startsWith("bc://bt/", Qt::CaseInsensitive)) {
        qDebug("Converting bc link to magnet link");
        param = misc::bcLinkToMagnet(param);
      }
      if (param.startsWith("magnet:", Qt::CaseInsensitive)) {
        if (useTorrentAdditionDialog) {
          torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
          dialog->showLoadMagnetURI(param);
        } else {
          Session::instance()->addMagnetUri(param);
        }
      } else {
        if (useTorrentAdditionDialog) {
          torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
          dialog->showLoad(param);
        }else{
          Session::instance()->addTorrent(param);
        }
      }
    }
  }
}

void MainWindow::addTorrent(QString path)
{
  Session::instance()->addTorrent(path);
}

void MainWindow::processDownloadedFiles(QString path, QString url) {
  QIniSettings settings(QString::fromUtf8("qBittorrent"), QString::fromUtf8("qBittorrent"));
  const bool useTorrentAdditionDialog = settings.value(QString::fromUtf8("Preferences/Downloads/AdditionDialog"), true).toBool();
  if (useTorrentAdditionDialog) {
    torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
    dialog->showLoad(path, url);
  }
  else
  {
    Session::instance()->addTorrent(path, false, url);
  }
}

void MainWindow::optionsSaved() {
  loadPreferences();
}

// Load program preferences
void MainWindow::loadPreferences(bool configure_session)
{
  Session::instance()->addConsoleMessage(tr("Options were saved successfully."));
  const Preferences pref;
  const bool newSystrayIntegration = pref.systrayIntegration();
  actionLock_qBittorrent->setVisible(newSystrayIntegration);
  if (newSystrayIntegration != (systrayIcon!=0)) {
    if (newSystrayIntegration) {
      // create the trayicon
      if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        if (!configure_session) { // Program startup
          systrayCreator = new QTimer(this);
          connect(systrayCreator, SIGNAL(timeout()), this, SLOT(createSystrayDelayed()));
          systrayCreator->setSingleShot(true);
          systrayCreator->start(2000);
          qDebug("Info: System tray is unavailable, trying again later.");
        }  else {
          qDebug("Warning: System tray is unavailable.");
        }
      } else {
        createTrayIcon();
      }
    } else {
      // Destroy trayicon
      delete systrayIcon;
      delete myTrayIconMenu;
    }
  }
  // Reload systray icon
  if (newSystrayIntegration && systrayIcon) {
    systrayIcon->setIcon(getSystrayIcon());
  }
  // General
  if (pref.isToolbarDisplayed()) {
    toolBar->setVisible(true);
  } else {
    // Clear search filter before hiding the top toolbar
    search_filter->clear();
    toolBar->setVisible(false);
  }

  if (pref.preventFromSuspend())
  {
    preventTimer->start(PREVENT_SUSPEND_INTERVAL);
  }
  else
  {
    preventTimer->stop();
    m_pwr->setActivityState(false);
  }

  const uint new_refreshInterval = pref.getRefreshInterval();
  getTransferList()->setRefreshInterval(new_refreshInterval);
  getTransferList()->setAlternatingRowColors(pref.useAlternatingRowColors());

//  properties->getFilesList()->setAlternatingRowColors(pref.useAlternatingRowColors());
//  properties->getTrackerList()->setAlternatingRowColors(pref.useAlternatingRowColors());
//  properties->getPeerList()->setAlternatingRowColors(pref.useAlternatingRowColors());
  // Queueing System
  if (pref.isQueueingSystemEnabled()) {
    if (!actionDecreasePriority->isVisible()) {
      getTransferList()->hidePriorityColumn(false);
      actionDecreasePriority->setVisible(true);
      actionIncreasePriority->setVisible(true);
      prioSeparator->setVisible(true);
      //prioSeparatorMenu->setVisible(true);
    }
  } else {
    if (actionDecreasePriority->isVisible()) {
      getTransferList()->hidePriorityColumn(true);
      actionDecreasePriority->setVisible(false);
      actionIncreasePriority->setVisible(false);
      //prioSeparator->setVisible(false);
      //prioSeparatorMenu->setVisible(false);
    }
  }

  // Torrent properties
  //properties->reloadPreferences();

  // Icon provider
#if defined(Q_WS_X11)
  IconProvider::instance()->useSystemIconTheme(pref.useSystemIconTheme());
#endif

  if (configure_session)
    Session::instance()->configureSession();

  qDebug("GUI settings loaded");
}

void MainWindow::addUnauthenticatedTracker(const QPair<Transfer,QString> &tracker) {
  // Trackers whose authentication was cancelled
  if (unauthenticated_trackers.indexOf(tracker) < 0) {
    unauthenticated_trackers << tracker;
  }
}

// Called when a tracker requires authentication
void MainWindow::trackerAuthenticationRequired(const Transfer& h) {
  if (unauthenticated_trackers.indexOf(QPair<Transfer,QString>(h, h.current_tracker())) < 0) {
    // Tracker login
    new trackerLogin(this, h);
  }
}

// Check connection status and display right icon
void MainWindow::updateGUI() {
  // update global informations
  if (systrayIcon) {
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
    QString html = "<div style='background-color: #678db2; color: #fff;height: 18px; font-weight: bold; margin-bottom: 5px;'>";
    html += tr("qBittorrent");
    html += "</div>";
    html += "<div style='vertical-align: baseline; height: 18px;'>";
    html += "<img src=':/Icons/skin/download.png'/>&nbsp;"+tr("DL speed: %1 KiB/s", "e.g: Download speed: 10 KiB/s").arg(QString::number(Session::instance()->get_torrent_session()->getPayloadDownloadRate()/1024., 'f', 1));
    html += "</div>";
    html += "<div style='vertical-align: baseline; height: 18px;'>";
    html += "<img src=':/Icons/skin/seeding.png'/>&nbsp;"+tr("UP speed: %1 KiB/s", "e.g: Upload speed: 10 KiB/s").arg(QString::number(Session::instance()->get_torrent_session()->getPayloadUploadRate()/1024., 'f', 1));
    html += "</div>";
#else
    // OSes such as Windows do not support html here
    QString html =tr("DL speed: %1 KiB/s", "e.g: Download speed: 10 KiB/s").arg(QString::number(Session::instance()->get_torrent_session()->getPayloadDownloadRate()/1024., 'f', 1));
    html += "\n";
    html += tr("UP speed: %1 KiB/s", "e.g: Upload speed: 10 KiB/s").arg(QString::number(Session::instance()->get_torrent_session()->getPayloadUploadRate()/1024., 'f', 1));
#endif
    systrayIcon->setToolTip(html); // tray icon
  }
  if (displaySpeedInTitle) {
    setWindowTitle(tr("[D: %1/s, U: %2/s] qBittorrent %3", "D = Download; U = Upload; %3 is qBittorrent version").arg(misc::friendlyUnit(Session::instance()->get_torrent_session()->getSessionStatus().payload_download_rate)).arg(misc::friendlyUnit(Session::instance()->get_torrent_session()->getSessionStatus().payload_upload_rate)).arg(QString::fromUtf8(VERSION)));
  }
}

void MainWindow::showNotificationBaloon(QString title, QString msg) const {
  if (!Preferences().useProgramNotification()) return;
#if defined(Q_WS_X11) && defined(QT_DBUS_LIB)
  org::freedesktop::Notifications notifications("org.freedesktop.Notifications",
                                                "/org/freedesktop/Notifications",
                                                QDBusConnection::sessionBus());
  if (notifications.isValid()) {
    QVariantMap hints;
    hints["desktop-entry"] = "qBittorrent";
    QDBusPendingReply<uint> reply = notifications.Notify("qBittorrent", 0, "qbittorrent", title,
                                                         msg, QStringList(), hints, -1);
    reply.waitForFinished();
    if (!reply.isError())
      return;
  }
#endif
  if (systrayIcon && QSystemTrayIcon::supportsMessages())
    systrayIcon->showMessage(title, msg, QSystemTrayIcon::Information, TIME_TRAY_BALLOON);
}

/*****************************************************
 *                                                   *
 *                      Utils                        *
 *                                                   *
 *****************************************************/

void MainWindow::downloadFromURLList(const QStringList& url_list) {
  QIniSettings settings(QString::fromUtf8("qBittorrent"), QString::fromUtf8("qBittorrent"));
  const bool useTorrentAdditionDialog = settings.value(QString::fromUtf8("Preferences/Downloads/AdditionDialog"), true).toBool();
  foreach (QString url, url_list) {
    if (url.startsWith("bc://bt/", Qt::CaseInsensitive)) {
      qDebug("Converting bc link to magnet link");
      url = misc::bcLinkToMagnet(url);
    }
    if (url.startsWith("magnet:", Qt::CaseInsensitive)) {
      if (useTorrentAdditionDialog) {
        torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
        dialog->showLoadMagnetURI(url);
      } else {
        Session::instance()->addMagnetUri(url);
      }
    } else {
      Session::instance()->downloadFromUrl(url);
    }
  }
}

/*****************************************************
 *                                                   *
 *                     Options                       *
 *                                                   *
 *****************************************************/

void MainWindow::createSystrayDelayed() {
  static int timeout = 20;
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    // Ok, systray integration is now supported
    // Create systray icon
    createTrayIcon();
    delete systrayCreator;
  } else {
    if (timeout) {
      // Retry a bit later
      systrayCreator->start(2000);
      --timeout;
    } else {
      // Timed out, apparently system really does not
      // support systray icon
      delete systrayCreator;
      // Disable it in program preferences to
      // avoid trying at earch startup
      Preferences().setSystrayIntegration(false);
    }
  }
}

void MainWindow::updateAltSpeedsBtn(bool alternative) {
  actionUse_alternative_speed_limits->setChecked(alternative);
}

QMenu* MainWindow::getTrayIconMenu() {
  if (myTrayIconMenu)
    return myTrayIconMenu;
  // Tray icon Menu
  myTrayIconMenu = new QMenu(this);
  actionToggleVisibility->setText(isVisible() ? tr("Hide") : tr("Show"));
  myTrayIconMenu->addAction(actionToggleVisibility);
  myTrayIconMenu->addSeparator();
  myTrayIconMenu->addAction(actionOpen);
  //myTrayIconMenu->addAction(actionDownload_from_URL);
  myTrayIconMenu->addSeparator();
  const bool isAltBWEnabled = Preferences().isAltBandwidthEnabled();
  updateAltSpeedsBtn(isAltBWEnabled);
  actionUse_alternative_speed_limits->setChecked(isAltBWEnabled);
  myTrayIconMenu->addAction(actionUse_alternative_speed_limits);
  myTrayIconMenu->addAction(actionSet_global_download_limit);
  myTrayIconMenu->addAction(actionSet_global_upload_limit);
  myTrayIconMenu->addSeparator();
  myTrayIconMenu->addAction(actionStart_All);
  myTrayIconMenu->addAction(actionPause_All);
  myTrayIconMenu->addSeparator();
  myTrayIconMenu->addAction(actionExit);
  if (ui_locked)
    myTrayIconMenu->setEnabled(false);
  return myTrayIconMenu;
}

void MainWindow::createTrayIcon() {
  // Tray icon
  systrayIcon = new QSystemTrayIcon(getSystrayIcon(), this);

  systrayIcon->setContextMenu(getTrayIconMenu());
  connect(systrayIcon, SIGNAL(messageClicked()), this, SLOT(balloonClicked()));
  // End of Icon Menu
  connect(systrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggleVisibility(QSystemTrayIcon::ActivationReason)));
  systrayIcon->show();
}

// Display Program Options
void MainWindow::on_actionOptions_triggered() {
  if (options) {
    // Get focus
    options->setFocus();
  } else {
    options = new options_imp(this);
    connect(options, SIGNAL(status_changed()), this, SLOT(optionsSaved()));
  }
}

void MainWindow::on_actionTop_tool_bar_triggered() {
  bool is_visible = static_cast<QAction*>(sender())->isChecked();
  toolBar->setVisible(is_visible);
  Preferences().setToolbarDisplayed(is_visible);
}

void MainWindow::on_actionSpeed_in_title_bar_triggered() {
  displaySpeedInTitle = static_cast<QAction*>(sender())->isChecked();
  Preferences().showSpeedInTitleBar(displaySpeedInTitle);
  if (displaySpeedInTitle)
    updateGUI();
  else
    setWindowTitle(tr("qBittorrent %1", "e.g: qBittorrent v0.x").arg(QString::fromUtf8(VERSION)));
}

void MainWindow::on_actionRSS_Reader_triggered() {
#ifdef RSS_ENABLE
  RssSettings().setRSSEnabled(actionRSS_Reader->isChecked());
  displayRSSTab(actionRSS_Reader->isChecked());
#endif
}

void MainWindow::on_actionSearch_engine_triggered() {
  Preferences().setSearchEnabled(actionSearch_engine->isChecked());
  displaySearchTab(actionSearch_engine->isChecked());
}

void MainWindow::on_action_Import_Torrent_triggered()
{
  TorrentImportDlg::importTorrent();
}

/*****************************************************
 *                                                   *
 *                 HTTP Downloader                   *
 *                                                   *
 *****************************************************/

// Display an input dialog to prompt user for
// an url
void MainWindow::on_actionDownload_from_URL_triggered() {
  if (!downloadFromURLDialog) {
    downloadFromURLDialog = new downloadFromURL(this);
    connect(downloadFromURLDialog, SIGNAL(urlsReadyToBeDownloaded(QStringList)), this, SLOT(downloadFromURLList(QStringList)));
  }
}

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)

void MainWindow::handleUpdateCheckFinished(bool update_available, QString new_version)
{
  if (update_available) {
    if (QMessageBox::question(this, tr("A newer version is available"),
                             tr("A newer version of qBittorrent is available on Sourceforge.\nWould you like to update qBittorrent to version %1?").arg(new_version),
                             QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
      // The user want to update, let's download the update
      ProgramUpdater* updater = dynamic_cast<ProgramUpdater*>(sender());
      connect(updater, SIGNAL(updateInstallFinished(QString)), SLOT(handleUpdateInstalled(QString)));
      updater->updateProgram();
      return;
    }
  }
  sender()->deleteLater();
}

void MainWindow::handleUpdateInstalled(QString error_msg)
{
  if (!error_msg.isEmpty()) {
    QMessageBox::critical(this, tr("Impossible to update qBittorrent"), tr("qBittorrent failed to update, reason: %1").arg(error_msg));
  }
}

#endif

void MainWindow::on_actionDonate_money_triggered()
{
  QDesktopServices::openUrl(QUrl("http://sourceforge.net/donate/index.php?group_id=163414"));
}

void MainWindow::showConnectionSettings()
{
  on_actionOptions_triggered();
  options->showConnectionTab();
}

void MainWindow::minimizeWindow()
{
    setWindowState(windowState() ^ Qt::WindowMinimized);
}

void MainWindow::on_actionExecution_Logs_triggered(bool checked)
{
  if (checked) {
    Q_ASSERT(!m_executionLog);
//    m_executionLog = new ExecutionLog(tabs);
//    int index_tab = tabs->addTab(m_executionLog, tr("Execution Log"));
//    tabs->setTabIcon(index_tab, IconProvider::instance()->getIcon("view-calendar-journal"));
  } else {
    if (m_executionLog)
      delete m_executionLog;
  }
  Preferences().setExecutionLogEnabled(checked);
}

void MainWindow::on_actionAutoExit_qBittorrent_toggled(bool enabled)
{
  qDebug() << Q_FUNC_INFO << enabled;
  Preferences().setShutdownqBTWhenDownloadsComplete(enabled);
}

void MainWindow::on_actionAutoSuspend_system_toggled(bool enabled)
{
  qDebug() << Q_FUNC_INFO << enabled;
  Preferences().setSuspendWhenDownloadsComplete(enabled);
}

void MainWindow::on_actionAutoShutdown_system_toggled(bool enabled)
{
  qDebug() << Q_FUNC_INFO << enabled;
  Preferences().setShutdownWhenDownloadsComplete(enabled);
}

void MainWindow::checkForActiveTorrents()
{
  const TorrentStatusReport report = getTransferList()->getSourceModel()->getTorrentStatusReport();
  if (report.nb_active > 0) // Active torrents are present; prevent system from suspend
    m_pwr->setActivityState(true);
  else
    m_pwr->setActivityState(false);
}

QIcon MainWindow::getSystrayIcon() const
{
#if defined(Q_WS_X11)
  TrayIcon::Style style = Preferences().trayIconStyle();
  switch(style) {
  case TrayIcon::MONO_DARK:
    return QIcon(":/emule/TrayConnected.ico");
  case TrayIcon::MONO_LIGHT:
    return QIcon(":/emule/TrayConnected.ico");
  default:
    break;
  }
#endif
  return icon_CurTray;
}

void MainWindow::on_auth(const QString& strRes, const QString& strError)
{
    qDebug("MainWindow::on_auth");

    if (!strError.isEmpty())
    {
        QString msg = tr("Authentication Error: ") + strError;
        QString msg2 = tr("New authentication attempt in 30 seconds.");
        addToLog(msg);
        addToLog(msg2);
        authTimer->start(30000);

        statusBar->setStatusMsg(msg + " " + msg2);
        return;
    }

    QString str(strError);
    QString result = strRes;

    QString sample("Message type=");
    int nPos = result.indexOf(sample);
    if (nPos >= 0)
    {
        nPos += sample.length();
        result = result.left(nPos) + "\"" + result.mid(nPos , 1) + "\"" + result.right(result.size() - nPos - 1);
    }
    //QString result("<?xml version=\"1.0\"?><DATA><AuthResult>0</AuthResult><Message type=\"1\"><![CDATA[]]></Message><filter><![CDATA[]]></filter><server>emule.is74.ru</server></DATA>");
    QDomDocument doc;
    QString errorStr;
    int errorLine;
    int errorColumn;

    if (!doc.setContent(result, true, &errorStr, &errorLine, &errorColumn)) 
    {
        return;
    }

    QDomElement root = doc.documentElement();
    QDomNode node = root.firstChild();
    int authResult      = -1;
    int authMessageType = -1;
    int authTimeout     = -1;
    QString authMessage;
    QString authFilter;
    QString authServer;

    while (!node.isNull()) 
    {
        if (node.toElement().tagName() == "AuthResult")
            authResult = node.toElement().text().toInt();
        if (node.toElement().tagName() == "Message")
        {
            authMessageType = node.toElement().attribute("type").toInt();
            QDomNode cdata = node.firstChild();
            if ( cdata.isCDATASection() )
	            authMessage = cdata.toCDATASection().data();
        }
        if (node.toElement().tagName() == "filter")
            if ( node.isCDATASection() )
	            authFilter = node.toCDATASection().data();
        if (node.toElement().tagName() == "server")
            authServer = node.toElement().text();
        if (node.toElement().tagName() == "Timeout")
            authResult = node.toElement().text().toInt();
        node = node.nextSibling();
    }

    if (authMessage.length())
    {
        addToLog(tr("Message from authentication server: ") + authMessage);

        if (authMessageType)
        {
            QMessageBox msgBox;
            msgBox.setText(authMessage);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
        }
    }

    switch (authResult)
    {
        case 0:
        {
            QString msg = tr("Authentication comleted");
            addToLog(msg);
            statusBar->setStatusMsg(msg);
            actionConnect->setIcon(icon_connected);
            connectioh_state = csConnected;

            actionStatus->setEnabled(true);
            actionTransfer->setEnabled(true);
            actionSearch->setEnabled(true);
            actionCatalog->setEnabled(true);
            menuStatus->setEnabled(true);

            status->updateConnectedInfo();
            statusBar->setConnected(true);

            icon_CurTray = icon_TrayConn;
            if (systrayIcon) {
                systrayIcon->setIcon(getSystrayIcon());
            }

            break;
        }
        case 1:
        {
            actionConnect->setIcon(icon_disconnected);
            connectioh_state = csDisconnected;
            authTimer->start(1000);
            Preferences pref;
            pref.setISPassword("");
            break;
        }
        case 2:
        {
            break;
        }
    }
   

}

void MainWindow::authRequest()
{
    Preferences pref;

    QString msg = tr("Sending authentication request.");
    authTimer->stop();

    if (pref.getISLogin().length() && pref.getISPassword().length())
    {
        ar.start("el.is74.ru", "auth.php",
                pref.getISLogin().toUtf8().constData(),
                pref.getISPassword().toUtf8().constData(),
                "0.5.6.7",
                boost::bind(&MainWindow::on_auth_result, this, _1, _2));

        addToLog(msg);
        statusBar->setStatusMsg(msg);
        return;
    }

    login_dlg dlg(this, pref.getISLogin(), pref.getISPassword());
    if (dlg.exec() == QDialog::Accepted)
    {
        pref.setISLogin(dlg.getLogin());
        pref.setISPassword(dlg.getPasswd());
        actionConnect->setIcon(icon_connecting);
        connectioh_state = csConnecting;
        ar.start("el.is74.ru", "auth.php",
                pref.getISLogin().toUtf8().constData(),
                pref.getISPassword().toUtf8().constData(),
                "0.5.6.7",
                boost::bind(&MainWindow::on_auth_result, this, _1, _2));

        addToLog(msg);
        statusBar->setStatusMsg(msg);
    }
}

void MainWindow::addToLog(QString log_message)
{
    status->addLogMessage(log_message);
}

void MainWindow::startAuthByTimer()
{
    authRequest();
}

void MainWindow::ed2kServerNameResolved(QString strServer)
{
    status->addLogMessage(QString("Server address: ") + strServer);
    status->serverAddress(strServer);
}

void MainWindow::ed2kConnectionInitialized(unsigned int nClientId)
{
    QString log_msg("Client ID: ");
    QString id;
    id.setNum(nClientId);
    log_msg += id;
    status->addLogMessage(log_msg);
    status->clientID(nClientId);
}

void MainWindow::ed2kServerStatus(int nFiles, int nUsers)
{
    QString log_msg("Number of server files: ");
    QString num;
    num.setNum(nFiles);
    log_msg += num;
    status->addLogMessage(log_msg);
    log_msg = "Number of server users: ";
    num.setNum(nUsers);
    log_msg += num;
    status->addLogMessage(log_msg);

    status->serverStatus(nFiles, nUsers);
    statusBar->setServerInfo(nFiles, nUsers);
}

void MainWindow::ed2kServerMessage(QString strMessage)
{
    status->addLogMessage(strMessage);

    status->serverInfo(strMessage);
}

void MainWindow::ed2kIdentity(QString strName, QString strDescription)
{
    status->addLogMessage(strName);
    status->addLogMessage(strDescription);
}

void MainWindow::ed2kConnectionFailed(QString strError)
{
    status->addLogMessage(strError);
}


void MainWindow::startChat(const QString& user_name, const libed2k::net_identifier& np)
{
    on_actionMessages_triggerd();
    messages->startChat(user_name, np);
}

void MainWindow::startMessageFlickering()
{
    flickerTimer->start(1000);
}

void MainWindow::on_flickerTimer()
{
    static int state = 0;

    statusBar->setNewMessageImg(state + 1);
    messages->setNewMessageImg(state + 1);

    if (state)
        icon_CurTray = icon_NewMsg;
    else
        icon_CurTray = icon_TrayConn;

    if (systrayIcon) {
        systrayIcon->setIcon(getSystrayIcon());
    }

    state = (state + 1) % 2;
}

void MainWindow::stopMessageFlickering()
{
    icon_CurTray = icon_TrayConn;
    if (systrayIcon) {
        systrayIcon->setIcon(getSystrayIcon());
    }
    statusBar->setNewMessageImg(0);
    messages->setNewMessageImg(0);

    flickerTimer->stop();
}
