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

#include <libed2k/log.hpp>

#include "transport/session.h"
#include "mainwindow.h"
#include "misc.h"
#include "torrentcreatordlg.h"
#include "downloadfromurldlg.h"
#include "torrentadditiondlg.h"
#include "trackerlogin.h"
#include "options_imp.h"
#include "speedlimitdlg.h"
#include "preferences.h"
#include "peerlistwidget.h"
#include "torrentpersistentdata.h"
#include "transferlistfilterswidget.h"
#include "hidabletabwidget.h"
#include "qinisettings.h"
#include "torrentimportdlg.h"
#include "executionlog.h"
#include "iconprovider.h"
#include "status_widget.h"
#include "search_widget.h"
#include "messages_widget.h"
#include "files_widget.h"
#include "status_bar.h"
#include "collection_save_dlg.h"

#include "xcatalog/catalogwidget.h"

#ifdef Q_WS_MAC
#include "qmacapplication.h"
void qt_mac_set_dock_menu(QMenu *menu);
#endif
#include "lineedit.h"
#include "sessionapplication.h"
#include "powermanagement.h"

using namespace libtorrent;

#define TIME_TRAY_BALLOON 5000
#define PREVENT_SUSPEND_INTERVAL 60000

/*****************************************************
 *                                                   *
 *                       GUI                         *
 *                                                   *
 *****************************************************/

// Constructor
MainWindow::MainWindow(QSplashScreen* sscrn, QWidget *parent, QStringList torrentCmdLine) : QMainWindow(parent), m_posInitialized(false), force_exit(false)
{
  setupUi(this);
  QApplication::setOverrideCursor(Qt::WaitCursor);
  m_sscrn.reset(sscrn);
  m_bDisconnectBtnPressed = false;
  m_last_file_error = QDateTime::currentDateTime().addSecs(-1); // imagine last file error event was 1 seconds in past
  m_tbar.reset(new taskbar_iface(this, 99));
#ifdef Q_WS_WIN
  m_nTaskbarButtonCreated = RegisterWindowMessage(L"TaskbarButtonCreated");
#else
  m_nTaskbarButtonCreated = 0;
#endif
  Preferences pref;
  pref.migrate();
  ui_locked = pref.isUILocked();
  setWindowTitle(misc::productName());
  displaySpeedInTitle = pref.speedInTitleBar();
  // Clean exit on log out
  connect(static_cast<SessionApplication*>(qApp), SIGNAL(sessionIsShuttingDown()), this, SLOT(deleteSession()));
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
  actionSet_upload_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/seeding.png")));
  actionSet_download_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/download.png")));
  actionSet_global_upload_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/seeding.png")));
  actionSet_global_download_limit->setIcon(QIcon(QString::fromUtf8(":/Icons/skin/download.png")));
  actionCreate_torrent->setIcon(IconProvider::instance()->getIcon("document-edit"));
  actionBugReport->setIcon(IconProvider::instance()->getIcon("tools-report-bug"));
  actionDecreasePriority->setIcon(IconProvider::instance()->getIcon("go-down"));
  actionDelete->setIcon(IconProvider::instance()->getIcon("list-remove"));
  actionDocumentation->setIcon(IconProvider::instance()->getIcon("help-contents"));
  actionExit->setIcon(IconProvider::instance()->getIcon("application-exit"));
  actionIncreasePriority->setIcon(IconProvider::instance()->getIcon("go-up"));
  actionLock_qMule->setIcon(IconProvider::instance()->getIcon("object-locked"));
  actionPause->setIcon(IconProvider::instance()->getIcon("media-playback-pause"));
  actionPause_All->setIcon(IconProvider::instance()->getIcon("media-playback-pause"));
  actionStart->setIcon(IconProvider::instance()->getIcon("media-playback-start"));
  actionStart_All->setIcon(IconProvider::instance()->getIcon("media-playback-start"));
  action_Import_Torrent->setIcon(IconProvider::instance()->getIcon("document-import"));

  QMenu *startAllMenu = new QMenu(this);
  startAllMenu->addAction(actionStart_All);
  actionStart->setMenu(startAllMenu);
  QMenu *pauseAllMenu = new QMenu(this);
  pauseAllMenu->addAction(actionPause_All);
  actionPause->setMenu(pauseAllMenu);
  QMenu *lockMenu = new QMenu(this);
  QAction *defineUiLockPasswdAct = lockMenu->addAction(tr("Set the password..."));
  connect(defineUiLockPasswdAct, SIGNAL(triggered()), this, SLOT(defineUILockPassword()));
  actionLock_qMule->setMenu(lockMenu);
  if (!m_sscrn.isNull())
      m_sscrn->showMessage(tr("Create sessions..."), Qt::AlignLeft | Qt::AlignBottom);
  // Creating Bittorrent session
  connect(Session::instance(), SIGNAL(fileError(Transfer, QString)),
          this, SLOT(fileError(Transfer, QString)));
  connect(Session::instance(), SIGNAL(addedTransfer(Transfer)),
          this, SLOT(addedTransfer(Transfer)));
  connect(Session::instance(), SIGNAL(finishedTransfer(Transfer)),
          this, SLOT(finishedTransfer(Transfer)));
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
  menuStatus->addAction(actionOpenDownloadPath);

  actionTools->setMenu(menuStatus);
  if(QToolButton * btn = qobject_cast<QToolButton *>(toolBar->widgetForAction(actionTools)))
    btn->setPopupMode(QToolButton::InstantPopup);

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
  statusBar = new status_bar(this, QMainWindow::statusBar());


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
  connect(search, SIGNAL(addFriend(const QString&, const libed2k::net_identifier&)), this, SLOT(addFriend(const QString&, const libed2k::net_identifier&)));
  connect(transfer_List, SIGNAL(sendMessage(const QString&, const libed2k::net_identifier&)), this, SLOT(startChat(const QString&, const libed2k::net_identifier&)));
  connect(transfer_List, SIGNAL(addFriend(const QString&, const libed2k::net_identifier&)), this, SLOT(addFriend(const QString&, const libed2k::net_identifier&)));

  // load from catalog link, temporary without deferred proxy
  connect(catalog, SIGNAL(ed2kLinkEvent(QString,bool)), Session::instance(), SLOT(addLink(QString,bool)));
  connect(catalog, SIGNAL(filePreviewEvent(QString)), Session::instance(), SLOT(playLink(QString)));

  connect(messages, SIGNAL(newMessage()), this, SLOT(startMessageFlickering()));
  connect(messages, SIGNAL(stopMessageNotification()), this, SLOT(stopMessageFlickering()));
  connect(statusBar, SIGNAL(stopMessageNotification()), this, SLOT(stopMessageFlickering()));
  flickerTimer = new QTimer(this);
  connect(flickerTimer, SIGNAL(timeout()), SLOT(on_flickerTimer()));
  on_actionCatalog_triggerd();


  m_pwr = new PowerManagement(this);
  preventTimer = new QTimer(this);
  connect(preventTimer, SIGNAL(timeout()), SLOT(checkForActiveTorrents()));

  // Configure session according to options
  loadPreferences(false);

  // Start connection checking timer
  guiUpdater = new QTimer(this);
  connect(guiUpdater, SIGNAL(timeout()), this, SLOT(updateGUI()));
  guiUpdater->start(2000);
  // Accept drag 'n drops
  setAcceptDrops(true);
  createKeyboardShortcuts();

#ifdef Q_WS_MAC
  setUnifiedTitleAndToolBarOnMac(true);
#endif

  // View settings
  actionTop_tool_bar->setChecked(pref.isToolbarDisplayed());
  actionSpeed_in_title_bar->setChecked(pref.speedInTitleBar());
  actionExecution_Logs->setChecked(pref.isExecutionLogEnabled());
  on_actionExecution_Logs_triggered(actionExecution_Logs->isChecked());

  // Auto shutdown actions
  QActionGroup * autoShutdownGroup = new QActionGroup(this);
  autoShutdownGroup->setExclusive(true);
  autoShutdownGroup->addAction(actionAutoShutdown_Disabled);
  autoShutdownGroup->addAction(actionAutoExit_qMule);
  autoShutdownGroup->addAction(actionAutoShutdown_system);
  autoShutdownGroup->addAction(actionAutoSuspend_system);
#if !defined(Q_WS_X11) || defined(QT_DBUS_LIB)
  actionAutoShutdown_system->setChecked(pref.shutdownWhenDownloadsComplete());
  actionAutoSuspend_system->setChecked(pref.suspendWhenDownloadsComplete());
#else
  actionAutoShutdown_system->setDisabled(true);
  actionAutoSuspend_system->setDisabled(true);
#endif
  actionAutoExit_qMule->setChecked(pref.shutdownqBTWhenDownloadsComplete());

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

  // Start watching the executable for updates
  executable_watcher = new QFileSystemWatcher();
  connect(executable_watcher, SIGNAL(fileChanged(QString)), this, SLOT(notifyOfUpdate(QString)));
  executable_watcher->addPath(qApp->applicationFilePath());

  connect(Session::instance(), SIGNAL(beginLoadSharedFileSystem()), this, SLOT(on_beginLoadSharedFileSystem()));
  connect(Session::instance(), SIGNAL(endLoadSharedFileSystem()), this, SLOT(on_endLoadSharedFileSystem()));

  if (!m_sscrn.isNull())
      m_sscrn->showMessage(tr("Startup transfers..."), Qt::AlignLeft | Qt::AlignBottom);
  // Resume unfinished torrents
  Session::instance()->startUpTransfers();
  // Add torrent given on command line
  processParams(torrentCmdLine);

  qDebug("GUI Built");
#ifdef Q_WS_MAC
  qt_mac_set_dock_menu(getTrayIconMenu());
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
  m_info_dlg.reset(new is_info_dlg(this));
  m_updater.reset(new silent_updater(VERSION_MAJOR, VERSION_MINOR, VERSION_UPDATE, VERSION_BUILD, this));

  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverNameResolved(QString)), this, SLOT(ed2kServerNameResolved(QString)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverConnectionInitialized(quint32, quint32, quint32)), this, SLOT(ed2kConnectionInitialized(quint32, quint32, quint32)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverStatus(int, int)), this, SLOT(ed2kServerStatus(int, int)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverMessage(QString)), this, SLOT(ed2kServerMessage(QString)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverIdentity(QString, QString)), this, SLOT(ed2kIdentity(QString, QString)));
  connect(Session::instance()->get_ed2k_session(), SIGNAL(serverConnectionClosed(QString)), this, SLOT(ed2kConnectionClosed(QString)));

  connect(Session::instance(), SIGNAL(newConsoleMessage(const QString&)), status, SLOT(addHtmlLogMessage(const QString&)));

  //Tray actions.
  connect(actionToggleVisibility, SIGNAL(triggered()), this, SLOT(toggleVisibility()));
  connect(actionStart_All, SIGNAL(triggered()), Session::instance(), SLOT(resumeAllTransfers()));
  connect(actionPause_All, SIGNAL(triggered()), Session::instance(), SLOT(pauseAllTransfers()));
  if (!m_sscrn.isNull())
      m_sscrn->showMessage(tr("Startup sessions..."), Qt::AlignLeft | Qt::AlignBottom);
  Session::instance()->start();
}

void MainWindow::deleteSession()
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
MainWindow::~MainWindow()
{
  qDebug("GUI destruction");
  hide();
#ifdef Q_WS_MAC
  // Workaround to avoid bug http://bugreports.qt.nokia.com/browse/QTBUG-7305
  setUnifiedTitleAndToolBarOnMac(false);
#endif
  // Delete other GUI objects
  if(executable_watcher)
    delete executable_watcher;
  delete statusBar;
  delete guiUpdater;
  if (createTorrentDlg)
    delete createTorrentDlg;
  if (m_executionLog)
    delete m_executionLog;
  if (options)
    delete options;
  if(systrayCreator) {
    delete systrayCreator;
  }
  if (systrayIcon) {
    delete systrayIcon;
  }
  if (myTrayIconMenu) {
    delete myTrayIconMenu;
  }

  // Keyboard shortcuts
  delete switchTransferShortcut;
  delete hideShortcut;

  IconProvider::drop();
  // Delete Session::instance() object
  m_pwr->setActivityState(false);
  qDebug() << "Saving session filesystem";
  Session::instance()->dropDirectoryTransfers();
  Session::instance()->saveFileSystem();
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

void MainWindow::on_actionLock_qMule_triggered() {
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

void MainWindow::on_actionWebsite_triggered() const {
  QDesktopServices::openUrl(QUrl(QString::fromUtf8("http://www.is74.ru")));
}

void MainWindow::on_actionDocumentation_triggered() const {
  QDesktopServices::openUrl(QUrl(QString::fromUtf8("http://is74.ru")));
}

void MainWindow::on_actionBugReport_triggered() const {
  QDesktopServices::openUrl(QUrl(QString::fromUtf8("http://is74.ru")));
}

void MainWindow::tab_changed(int new_tab)
{
  Q_UNUSED(new_tab);
}

void MainWindow::writeSettings() {
  Preferences settings;
  settings.beginGroup(QString::fromUtf8("MainWindow"));
  settings.setValue("geometry", saveGeometry());
  settings.endGroup();
  settings.setMigrationStage(false);
}

void MainWindow::readSettings() {
  Preferences settings;
  settings.beginGroup(QString::fromUtf8("MainWindow"));
  if(settings.contains("geometry")) {
    if(restoreGeometry(settings.value("geometry").toByteArray()))
      m_posInitialized = true;
  }
  const QByteArray splitterState = settings.value("vsplitterState").toByteArray();
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

// called when a transfer has started
void MainWindow::addedTransfer(const Transfer& h) const {
  if (TorrentPersistentData::getAddedDate(h.hash()).secsTo(QDateTime::currentDateTime()) <= 1
      && !h.is_seed())
    showNotificationBaloon(
      tr("Download starting"),
      tr("%1 has started downloading.", "e.g: xxx.avi has started downloading.").arg(h.name()));
}

// called when a transfer has finished
void MainWindow::finishedTransfer(const Transfer& h) const {
  if (!TorrentPersistentData::isSeed(h.hash()))
    showNotificationBaloon(
      tr("Download completion"),
      tr("%1 has finished downloading.", "e.g: xxx.avi has finished downloading.").arg(h.name()));
}

// Notification when disk is full and other disk errors
void MainWindow::fileError(const Transfer& h, QString msg)
{
    QDateTime cdt = QDateTime::currentDateTime();

    if (m_last_file_error.secsTo(cdt) > 1)
    {
        showNotificationBaloon(
            tr("I/O Error"),
            tr("An I/O error occured for %1.\nReason: %2").arg(h.name()).arg(msg));
    }

    m_last_file_error = cdt;
}

void MainWindow::createKeyboardShortcuts() {
  actionCreate_torrent->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+N")));
  actionOpen->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+O")));
  actionExit->setShortcut(QKeySequence(QString::fromUtf8("Ctrl+Q")));
  switchTransferShortcut = new QShortcut(QKeySequence(tr("Alt+1", "shortcut to switch to first tab")), this);
  connect(switchTransferShortcut, SIGNAL(activated()), this, SLOT(displayTransferTab()));
  hideShortcut = new QShortcut(QKeySequence(QString::fromUtf8("Esc")), this);
  connect(hideShortcut, SIGNAL(activated()), this, SLOT(hide()));
  actionDocumentation->setShortcut(QKeySequence("F1"));

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

void MainWindow::setTabText(int, QString) const {
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
      stopMessageFlickering();
    }else{
      hide();
    }
  }
  actionToggleVisibility->setText(isVisible() ? tr("Hide") : tr("Show"));
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

  SessionStatus status = Session::instance()->getSessionStatus();

  // has active transfers or sessions speed > 0 (we have incoming peers)
  if (pref.confirmOnExit() && (Session::instance()->hasActiveTransfers() || (status.payload_download_rate > 0) || (status.payload_upload_rate > 0)))
  {
    if (e->spontaneous() || force_exit) {
      if (!isVisible())
        show();
      QMessageBox confirmBox(QMessageBox::Question, tr("Exiting qMule"),
                             tr("Some files are currently transferring.\nAre you sure you want to quit qMule?"),
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
    switch(e->type()) 
    {
        case QEvent::WindowStateChange: 
        {
            qDebug("Window change event");
            //Now check to see if the window is minimised
            if (isMinimized()) 
            {
                qDebug("minimisation");
                if (systrayIcon && Preferences().minimizeToTray()) 
                {
                    qDebug("Minimize to Tray enabled, hiding!");
                    e->accept();
                    QTimer::singleShot(0, this, SLOT(hide()));
                    return true;
                }
            }
            break;
        }
#ifdef Q_WS_MAC
        case QEvent::ToolBarChange: 
        {
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

#ifdef Q_WS_WIN
bool MainWindow::winEvent(MSG * message, long * result)
{
    if (message->message == m_nTaskbarButtonCreated)
    {
        qDebug() << "initialize task bar";
        m_tbar->initialize();
        m_tbar->setState(winId(), taskbar_iface::S_NOPROGRESS);
        return true;
    }

    return false;
}
#endif

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
        Session::instance()->addLink(file);	// TODO - check it fir using only in torrent
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
      Session::instance()->addLink(file);	// TODO - possibly it is torrent only
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
void MainWindow::on_actionOpen_triggered()
{
  Preferences settings;
  // Open File Open Dialog
  // Note: it is possible to select more than one file
  const QStringList pathsList = QFileDialog::getOpenFileNames(0,
                                                              tr("Open Torrent Files"), settings.value(QString::fromUtf8("MainWindowLastDir"), QDir::homePath()).toString(),
                                                              tr("Torrent Files")+QString::fromUtf8(" (*.torrent)"));
  if (!pathsList.empty()) {
    const bool useTorrentAdditionDialog = settings.useAdditionDialog();
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
    selectWidget(wStatus);
}

void MainWindow::on_actionCatalog_triggerd()
{
    selectWidget(wCatalog);
}

void MainWindow::on_actionTransfer_triggerd()
{
    selectWidget(wTransfer);
}

void MainWindow::on_actionSearch_triggerd()
{
    selectWidget(wSearch);
}

void MainWindow::on_actionMessages_triggerd()
{
    selectWidget(wMessages);
}

void MainWindow::on_actionFiles_triggerd()
{
    selectWidget(wFiles);
}

void MainWindow::selectWidget(Widgets wNum)
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

    switch (wNum)
    {
        case wStatus:
        {
            status->show();
            break;
        }
        case wCatalog:
        {
            actionCatalog->setChecked(true);
            catalog->show();
            break;
        }
        case wTransfer:
        {
            actionTransfer->setChecked(true);
            dock->show();
            break;
        }
        case wSearch:
        {
            actionSearch->setChecked(true);
            search->show();
            break;
        }
        case wMessages:
        {
            messages->show();
            break;
        }
        case wFiles:
        {
            files->show();
            break;
        }
    }
}

void MainWindow::activateControls(bool status)
{
    actionStatus->setDisabled(!status);
    actionTransfer->setDisabled(!status);
    actionSearch->setDisabled(!status);
    actionCatalog->setDisabled(!status);
    menuStatus->setDisabled(!status);
}

void MainWindow::on_actionConnect_triggered()
{
    QMessageBox confirmBox(QMessageBox::Question, tr("Server connection"),
                           tr("Do you want to break network connection?"),
                           QMessageBox::NoButton, this);

    confirmBox.addButton(tr("No"), QMessageBox::NoRole);
    QPushButton *yesBtn = confirmBox.addButton(tr("Yes"), QMessageBox::YesRole);
    confirmBox.setDefaultButton(yesBtn);
    m_bDisconnectBtnPressed = false;

    switch (connectioh_state)
    {
        case csDisconnected:
        {
            Session::instance()->get_ed2k_session()->startServerConnection();
            break;
        }
        case csConnecting:
        case csConnected:
        {
            confirmBox.exec();
            if (confirmBox.clickedButton() && confirmBox.clickedButton() == yesBtn)
            {
                m_bDisconnectBtnPressed = true; // mark user press button
                Session::instance()->get_ed2k_session()->stopServerConnection();
            }

            break;
        }
        default:
            break;
    }
}

// As program parameters, we can get paths or urls.
// This function parse the parameters and call
// the right addTorrent function, considering
// the parameter type.
void MainWindow::processParams(const QString& params_str) {
  processParams(params_str.split(" ", QString::SkipEmptyParts));
}

void MainWindow::processParams(const QStringList& params)
{
  Preferences pref;    
  const bool useTorrentAdditionDialog = pref.useAdditionDialog();
  qDebug() << "process params: " << params;

  foreach (QString param, params)
  {
    param = param.trimmed();

    if (misc::isUrl(param))
    {
      Session::instance()->downloadFromUrl(param);
    }
    else
    {
      if (param.startsWith("bc://bt/", Qt::CaseInsensitive))
      {
        qDebug("Converting bc link to magnet link");
        param = misc::bcLinkToMagnet(param);
      }

      if (param.startsWith("magnet:", Qt::CaseInsensitive))
      {
        if (useTorrentAdditionDialog)
        {
          torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
          dialog->showLoadMagnetURI(param);
        }
        else
        {
          Session::instance()->addLink(param);
        }
      }
      else if (param.startsWith("ed2k://", Qt::CaseInsensitive))
      {
          Session::instance()->addLink(param);
      }
      else
      {
        // for torrent we run dialog when it option activated
        if (useTorrentAdditionDialog)
        {
            if (param.endsWith(".emulecollection"))
            {
                collection_save_dlg dialog(this, param);
                dialog.exec();
            }
            else
            {
                torrentAdditionDialog *dialog = new torrentAdditionDialog(this);
                dialog->showLoad(param);
            }
        }
        else
        {
          Session::instance()->addTransferFromFile(param);
        }
      }
    }
  }
}

void MainWindow::addTorrent(QString path)
{
  Session::instance()->addTorrent(path);
}

void MainWindow::processDownloadedFiles(QString path, QString url)
{
  Preferences settings;
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
  actionLock_qMule->setVisible(newSystrayIntegration);
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
    }
  }

  // Torrent properties
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
  SessionStatus status = Session::instance()->getSessionStatus();

  // update global informations
  if (systrayIcon) {
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
    QString html = "<div style='background-color: #678db2; color: #fff;height: 18px; font-weight: bold; margin-bottom: 5px;'>";
    html += tr("qMule");
    html += "</div>";
    html += "<div style='vertical-align: baseline; height: 18px;'>";
    html += "<img src=':/Icons/skin/download.png'/>&nbsp;" +
        tr("DL speed: %1 KiB/s", "e.g: Download speed: 10 KiB/s")
        .arg(QString::number(status.payload_download_rate/1024., 'f', 1));
    html += "</div>";
    html += "<div style='vertical-align: baseline; height: 18px;'>";
    html += "<img src=':/Icons/skin/seeding.png'/>&nbsp;" +
        tr("UP speed: %1 KiB/s", "e.g: Upload speed: 10 KiB/s")
        .arg(QString::number(status.payload_upload_rate/1024., 'f', 1));
    html += "</div>";
#else
    // OSes such as Windows do not support html here
    QString html =tr("DL speed: %1 KiB/s", "e.g: Download speed: 10 KiB/s")
        .arg(QString::number(status.payload_download_rate/1024., 'f', 1));
    html += "\n";
    html += tr("UP speed: %1 KiB/s", "e.g: Upload speed: 10 KiB/s")
        .arg(QString::number(status.payload_upload_rate/1024., 'f', 1));
#endif
    systrayIcon->setToolTip(html); // tray icon
  }
  if (displaySpeedInTitle) {
    setWindowTitle(
        tr("[D: %1/s, U: %2/s] qMule %3", "D = Download; U = Upload; %3 is qMule version")
        .arg(misc::friendlyUnit(status.payload_download_rate))
        .arg(misc::friendlyUnit(status.payload_upload_rate))
        .arg(QString::fromUtf8(VERSION)));
  }

  statusBar->setUpDown(status.payload_upload_rate, status.payload_download_rate);
  Session::instance()->playPendingMedia();
  m_tbar->setProgress(winId(), Session::instance()->progress()*100);
  m_tbar->setState(winId(), Session::instance()->hasActiveTransfers()?taskbar_iface::S_NORM:taskbar_iface::S_PAUSED);
}

void MainWindow::showNotificationBaloon(QString title, QString msg) const
{
  if (!Preferences().useProgramNotification()) return;

  // forward all notifications to the console
  addConsoleMessage(msg);

#if defined(Q_WS_X11) && defined(QT_DBUS_LIB)
  org::freedesktop::Notifications notifications("org.freedesktop.Notifications",
                                                "/org/freedesktop/Notifications",
                                                QDBusConnection::sessionBus());
  if (notifications.isValid()) {
    QVariantMap hints;
    hints["desktop-entry"] = "qMule";
    QDBusPendingReply<uint> reply = notifications.Notify("qMule", 0, "qMule", title,
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
  Preferences settings;
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
        Session::instance()->addLink(url);
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
  /* disable useless actions
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
  */
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
    connect(options, SIGNAL(status_changed()), files, SLOT(optionsChanged()));
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
    setWindowTitle(tr("qMule %1", "e.g: qMule v0.x").arg(QString::fromUtf8(VERSION)));
}

void MainWindow::on_action_Import_Torrent_triggered()
{
  TorrentImportDlg::importTorrent();
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

void MainWindow::on_actionAutoExit_qMule_toggled(bool enabled)
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

void MainWindow::addConsoleMessage(const QString& msg, QColor color /*=QApplication::palette().color(QPalette::WindowText)*/) const
{
    status->addHtmlLogMessage("<font color='grey'>"+ QDateTime::currentDateTime().toString(QString::fromUtf8("dd/MM/yyyy hh:mm:ss")) + "</font> - <font color='" + color.name() + "'><i>" + msg + "</i></font>");
}

void MainWindow::ed2kServerNameResolved(QString strServer)
{
    status->addLogMessage(QString("Server address: ") + strServer);
    status->serverAddress(strServer);
}

void MainWindow::ed2kConnectionInitialized(quint32 client_id, quint32 tcp_flags, quint32 aux_port)
{
    qDebug() << Q_FUNC_INFO;
    status->updateConnectedInfo();
    statusBar->setConnected(true);
    actionConnect->setIcon(icon_connected);
    actionConnect->setText(tr("Disconnecting"));
    connectioh_state = csConnected;

    icon_CurTray = icon_TrayConn;
    if (systrayIcon) {
        systrayIcon->setIcon(getSystrayIcon());
    }

    m_info_dlg->start();    // start message watcher
    //temporary commented
    //m_updater->start();

    QString log_msg("Client ID: ");
    QString id;
    id.setNum(client_id);
    log_msg += id;
    status->addLogMessage(log_msg);
    status->clientID(client_id);
    statusBar->setStatusMsg(log_msg);
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

void MainWindow::ed2kConnectionClosed(QString strError)
{
    status->addLogMessage(strError);
    setDisconnectedStatus();
    statusBar->setStatusMsg(strError);
}


void MainWindow::startChat(const QString& user_name, const libed2k::net_identifier& np)
{
    on_actionMessages_triggerd();
    messages->startChat(user_name, np);
}

void MainWindow::addFriend(const QString& user_name, const libed2k::net_identifier& np)
{
    on_actionMessages_triggerd();
    messages->addFriend(user_name, np);
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
    if (flickerTimer->isActive())
    {
        icon_CurTray = icon_TrayConn;
        if (systrayIcon) {
            systrayIcon->setIcon(getSystrayIcon());
        }
        statusBar->setNewMessageImg(0);
        messages->setNewMessageImg(0);

        flickerTimer->stop();

        selectWidget(wMessages);
    }
}

void MainWindow::setDisconnectedStatus()
{
    actionConnect->setIcon(icon_disconnected);
    actionConnect->setText(tr("Connecting"));
    connectioh_state = csDisconnected;
    status->setDisconnectedInfo();
    statusBar->reset();
    icon_CurTray = icon_TrayDisconn;
    if (systrayIcon) {
        systrayIcon->setIcon(getSystrayIcon());
    }
}

void MainWindow::on_actionOpenDownloadPath_triggered()
{
    Preferences pref;
    QDesktopServices::openUrl(QUrl::fromLocalFile(pref.getSavePath()));
}

void MainWindow::on_beginLoadSharedFileSystem()
{
    if (!m_sscrn.isNull())
        m_sscrn->showMessage(tr("Begin load shared filesystem..."), Qt::AlignLeft | Qt::AlignBottom);
}

void MainWindow::on_endLoadSharedFileSystem()
{
    if (!m_sscrn.isNull())
    {
        m_sscrn->showMessage(tr("Shared filesystem loading was completed..."), Qt::AlignLeft | Qt::AlignBottom);        
        m_sscrn.reset();
    }

    QApplication::restoreOverrideCursor();
#ifdef Q_WS_WIN
    Preferences pref;
    if (!pref.neverCheckFileAssoc() &&
          (!Preferences::isTorrentFileAssocSet() ||
           !Preferences::isLinkAssocSet("Magnet") ||
           !Preferences::isEmuleFileAssocSet()) ||
           !Preferences::isLinkAssocSet("ed2k"))
    {
        if (QMessageBox::question(0, tr("Torrent file association"),
                                 tr("qMule is not the default application to open torrent files, Magnet links or eMule collections.\nDo you want to associate qMule to torrent files, Magnet links and eMule collections?"),
                                 QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
        {
            Preferences::setTorrentFileAssoc(true);
            Preferences::setLinkAssoc("Magnet", true);
            Preferences::setLinkAssoc("ed2k", true);
            Preferences::setEmuleFileAssoc(true);
            Preferences::setCommonAssocSection(true); // enable common section
        }
        else
        {
            pref.setNeverCheckFileAssoc();
        }
    }
#endif
}
