# Global
TEMPLATE = app
CONFIG += qt thread

# Windows specific configuration
win32 {
  include(../winconf.pri)  
}

# Mac specific configuration
macx {
  include(../macxconf.pri)
}

# Unix specific configuration
unix:!macx {
  include(../unixconf.pri)
}

# eCS(OS/2) specific configuration
os2 {
  include(../os2conf.pri)
}

nox {
  QT -= gui
  TARGET = qmule-nox
  DEFINES += DISABLE_GUI
} else {
  TARGET = qmule 
}
QT += network
QT += webkit
QT += xml

# Vars
LANG_PATH = lang
ICONS_PATH = Icons

CONFIG(debug, debug|release):message(Project is built in DEBUG mode.)
CONFIG(release, debug|release):message(Project is built in RELEASE mode.)

# Disable debug output in release mode
CONFIG(release, debug|release) {
   message(Disabling debug output.)
   DEFINES += QT_NO_DEBUG_OUTPUT
}

# Disable authentication
CONFIG(amd1) {
   DEFINES += AMD1
}

# VERSION DEFINES
include(../version.pri)

DEFINES += QT_NO_CAST_TO_ASCII
# Fast concatenation (Qt >= 4.6)
DEFINES += QT_USE_FAST_CONCATENATION QT_USE_FAST_OPERATOR_PLUS

# Fixes compilation with Boost >= v1.46 where boost
# filesystem v3 is the default.
DEFINES += BOOST_FILESYSTEM_VERSION=2

INCLUDEPATH += $$PWD


# Resource files
RESOURCES += icons.qrc \
            lang.qrc \
            Icons/emule/emule.qrc

# Source code
usesystemqtsingleapplication {
  nox {
    CONFIG += qtsinglecoreapplication
  } else {
    CONFIG += qtsingleapplication
  }
} else {
  nox {
    include(qtsingleapp/qtsinglecoreapplication.pri)
  } else {
    include(qtsingleapp/qtsingleapplication.pri)
  }
}

include(qtlibtorrent/qtlibtorrent.pri)
include(tracker/tracker.pri)
include (preferences/preferences.pri)
include(transport/transport.pri)
include(xcatalog/xcatalog.pri)
include(qtlibed2k/qtlibed2k.pri)
include(session_fs_models/session_fs_models.pri)

!nox {
  include(lineedit/lineedit.pri)
  include(properties/properties.pri)
  include(torrentcreator/torrentcreator.pri)
  include(geoip/geoip.pri)
  include(powermanagement/powermanagement.pri)
}

HEADERS += misc.h \
           downloadthread.h \
           stacktrace.h \
           torrentpersistentdata.h \
           filesystemwatcher.h \
           scannedfoldersmodel.h \
           qinisettings.h \
           smtp.h

SOURCES += main.cpp \
           downloadthread.cpp \
           scannedfoldersmodel.cpp \
           misc.cpp \
           smtp.cpp

HEADERS +=  mainwindow.h\
          transferlistwidget.h \
          transferlistdelegate.h \
          transferlistfilterswidget.h \
          torrentcontentmodel.h \
          torrentcontentmodelitem.h \
          torrentcontentfiltermodel.h \
          deletionconfirmationdlg.h \
          reverseresolution.h \
          ico.h \
          speedlimitdlg.h \
          previewselect.h \
          previewlistdelegate.h \
          downloadfromurldlg.h \
          torrentadditiondlg.h \
          trackerlogin.h \
          hidabletabwidget.h \
          sessionapplication.h \
          torrentimportdlg.h \
          executionlog.h \
          iconprovider.h \
          updownratiodlg.h \
          loglistwidget.h \
          transfer_list.h \
          status_widget.h  \
          search_widget.h \
          search_widget_delegate.h \
          search_filter.h \
          messages_widget.h \
          add_friend.h \
          files_widget.h \
          status_bar.h \
          clicked_label.h \
          collection_save_dlg.h \
          infodlg.h \
          silent_updater.h\
          taskbar_iface.h \
          user_properties.h \
          torrent_properties.h \
          ed2k_link_maker.h \
          delay.h \
          wgetter.h

SOURCES += mainwindow.cpp \
         ico.cpp \
         transferlistwidget.cpp \
         torrentcontentmodel.cpp \
         torrentcontentmodelitem.cpp \
         torrentcontentfiltermodel.cpp \
         torrentadditiondlg.cpp \
         sessionapplication.cpp \
         torrentimportdlg.cpp \
         executionlog.cpp \
         previewselect.cpp \
         iconprovider.cpp \
         updownratiodlg.cpp \
         loglistwidget.cpp \
         transfer_list.cpp \
         status_widget.cpp  \
         search_widget.cpp \
         search_filter.cpp \
         messages_widget.cpp \
         add_friend.cpp \
         files_widget.cpp \
         status_bar.cpp \
         collection_save_dlg.cpp \
         infodlg.cpp \
         silent_updater.cpp\
         taskbar_iface.cpp \
         user_properties.cpp \
         torrent_properties.cpp \
         ed2k_link_maker.cpp \
         delay.cpp \
         wgetter.cpp

  macx {
    HEADERS += qmacapplication.h 
    SOURCES += qmacapplication.cpp 
  }

  FORMS += mainwindow.ui \
           preview.ui \
           login.ui \
           downloadfromurldlg.ui \
           torrentadditiondlg.ui \
           bandwidth_limit.ui \
           updownratiodlg.ui \
           confirmdeletiondlg.ui \
           torrentimportdlg.ui \
           executionlog.ui \
           status_widget.ui \
           search_widget.ui \
           messages_widget.ui \
           add_friend.ui \
           files_widget.ui \
           status_bar.ui \
           collection_save_dlg.ui \
           infodlg.ui \
           user_properties.ui \
           torrent_properties.ui \
           ed2k_link_maker.ui

DESTDIR = .

# OS specific config
OTHER_FILES += ../winconf.pri ../macxconf.pri ../unixconf.pri ../os2conf.pri
# compiler specific config
OTHER_FILES += ../winconf-mingw.pri ../winconf-msvc.pri
# version file
OTHER_FILES += ../version.pri

# Translations
TRANSLATIONS = $$LANG_PATH/qmule_en.ts \
               $$LANG_PATH/qmule_ru.ts 
