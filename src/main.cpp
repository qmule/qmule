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

#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QLibraryInfo>

#ifndef DISABLE_GUI
#include <QMessageBox>
#include <QStyleFactory>
#include <QStyle>
#include <QSplashScreen>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QPushButton>
#include <QTimer>
#include "sessionapplication.h"
#include "mainwindow.h"
#include "ico.h"
#else
#include "qtsinglecoreapplication.h"
#include <iostream>
#include <stdio.h>
#endif

#include "preferences.h"
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
#include <signal.h>
#include <execinfo.h>
#include "stacktrace.h"
#endif

#include <stdlib.h>
#include "misc.h"
#include "preferences.h"

class UsageDisplay: public QObject {
  Q_OBJECT

public:
  static void displayUsage(char* prg_name) {
    std::cout << qPrintable(tr("Usage:")) << std::endl;
    std::cout << '\t' << prg_name << " --version: " << qPrintable(tr("displays program version")) << std::endl;
#ifndef DISABLE_GUI
    std::cout << '\t' << prg_name << " --no-splash: " << qPrintable(tr("disable splash screen")) << std::endl;
#endif
    std::cout << '\t' << prg_name << " --help: " << qPrintable(tr("displays this help message")) << std::endl;    
    std::cout << '\t' << prg_name << " " << qPrintable(tr("[files or urls]: downloads the torrents passed by the user (optional)")) << std::endl;
  }
};

#include "main.moc"

#if defined(Q_WS_X11) || defined(Q_WS_MAC)
void sigintHandler(int) {
  signal(SIGINT, 0);
  qDebug("Catching SIGINT, exiting cleanly");
  qApp->exit();
}

void sigtermHandler(int)
{
  signal(SIGTERM, 0);
  qDebug("Catching SIGTERM, exiting cleanly");
  qApp->exit();
}

void sigsegvHandler(int) {
  signal(SIGABRT, 0);
  signal(SIGSEGV, 0);
  std::cerr << "\n\n*************************************************************\n";
  std::cerr << "Catching SIGSEGV, please report a bug at http://tcs.is74.ru/crashrpt.php provide the following backtrace:\n";
  std::cerr << "qMule version: " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_UPDATE << "." << VERSION_BUILD << std::endl;
  print_stacktrace();
  raise(SIGSEGV);
}

void sigabrtHandler(int) {
  signal(SIGABRT, 0);
  signal(SIGSEGV, 0);
  std::cerr << "\n\n*************************************************************\n";
  std::cerr << "Catching SIGABRT, please report a bug at http://tcs.is74.ru/crashrpt.php provide the following backtrace:\n";
  std::cerr << "qMule version: " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_UPDATE << "." << VERSION_BUILD << std::endl;
  print_stacktrace();
  raise(SIGABRT);
}
#endif

#ifdef Q_WS_WIN

void customMessageHandler(QtMsgType type, const char *msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;

    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        abort();
    }

    QFile outFile("debuglog.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

#endif
// Main
int main(int argc, char *argv[]) {
  // Create Application
  QString uid = misc::getUserIDString();
#ifdef DISABLE_GUI
  QtSingleCoreApplication app("qMule-"+uid, argc, argv);
#else
  SessionApplication app("qMule-"+uid, argc, argv);
#endif

#ifdef Q_WS_WIN
    qInstallMsgHandler(customMessageHandler);
#endif

  QStringList al = app.arguments();
  al.removeFirst();
  QStringList dataList = al.filter(QRegExp("^[^-]"));

  // Check if qMule is already running for this user
  if (app.isRunning())
  {
    qDebug("qMule is already running for this user.");

    if (!dataList.isEmpty())
    {
      qDebug("Passing program parameters to running instance...");
      qDebug() << dataList;
      app.sendMessage(dataList.join("|"));
    }

    return 0;
  }

  Preferences pref;
#ifndef DISABLE_GUI
  bool no_splash = false;
#endif

  // Load translation
  QString locale = pref.getLocale();
  QTranslator qtTranslator;
  QTranslator translator;

  if (locale.isEmpty()) 
  {
    locale = QLocale::system().name();
    qDebug("locale is empty, use %s", qPrintable(locale));
    pref.setLocale(locale);
    if (locale.isEmpty())
    {
        locale = "en_GB";
    }
  }

  qDebug("Program locale: %s", qPrintable(locale));
  setlocale(LC_CTYPE, ""); // TODO - need attention

  if (qtTranslator.load(
          QString::fromUtf8("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath)
                                                                    )) {
    qDebug("Qt %s locale recognized, using translation.", qPrintable(locale));
  }else{
    qDebug("Qt %s locale unrecognized, using default (en_GB).", qPrintable(locale));
  }
  app.installTranslator(&qtTranslator);
  if (translator.load(QString::fromUtf8(":/lang/qmule_") + locale)) {
    qDebug("%s locale recognized, using translation.", qPrintable(locale));
  }else{
    qDebug("%s locale unrecognized, using default (en_GB).", qPrintable(locale));
  }
  app.installTranslator(&translator);
#ifndef DISABLE_GUI
  if (locale.startsWith("ar")) {
    qDebug("Right to Left mode");
    app.setLayoutDirection(Qt::RightToLeft);
  } else {
    app.setLayoutDirection(Qt::LeftToRight);
  }
#endif
  app.setApplicationName(QString::fromUtf8("qMule"));

  if (!al.filter(QRegExp("^-+help$")).isEmpty())
  {
      UsageDisplay::displayUsage(argv[0]);
      return 0;
  }

  if (!al.filter(QRegExp("^-+version$")).isEmpty())
  {
      std::cout << "qMule " << VERSION << '\n';
      return 0;
  }

#ifndef DISABLE_GUI
  no_splash = !al.filter(QRegExp("^-+no-splash$")).isEmpty();
#endif

  QSplashScreen *splash = 0;
#ifndef DISABLE_GUI
  if (pref.isSlashScreenDisabled())
  {
    no_splash = true;
  }

  if (!no_splash)
  {
    QPixmap splash_img(":/Icons/skin/Logo.png");
    QPainter painter(&splash_img);
    QString prog_name = "qMule";
    QString version = QString("v%1.%2.%3.%4")
            .arg(VERSION_MAJOR)
            .arg(VERSION_MINOR)
            .arg(VERSION_UPDATE)
            .arg(VERSION_BUILD);
    painter.setPen(QPen(Qt::black));
    painter.setFont(QFont("Arial", 22, QFont::Black));
    painter.drawText(175 - painter.fontMetrics().width(prog_name) / 2, 220, prog_name);
    painter.drawText(175 - painter.fontMetrics().width(version) / 2, 220 + painter.fontMetrics().height(), version);
    
    splash = new QSplashScreen(splash_img, Qt::WindowStaysOnTopHint);    
    splash->show();
    app.processEvents();
  }
#endif
  // Set environment variable
  if (qputenv("QMULE", QByteArray(VERSION))) {
    std::cerr << "Couldn't set environment variable...\n";
  }

#ifndef DISABLE_GUI
  app.setStyleSheet("QStatusBar::item { border-width: 0; }");
#endif

#ifndef DISABLE_GUI
  app.setQuitOnLastWindowClosed(false);
#endif
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
  signal(SIGABRT, sigabrtHandler);
  signal(SIGTERM, sigtermHandler);
  signal(SIGINT, sigintHandler);
  signal(SIGSEGV, sigsegvHandler);
#endif

#ifndef DISABLE_GUI
  MainWindow window(splash, 0, dataList);
  QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                   &window, SLOT(processParams(const QString&)));  
  app.setActivationWindow(&window);
#else
  QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                   &loader, SLOT(processParams(const QString&)));
#endif

  LOGGER_INIT(LOG_ALL)

  int ret = app.exec();
  qDebug("Application has exited");
  return ret;
}


