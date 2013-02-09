#-------------------------------------------------
#
# Project created by QtCreator 2012-07-16T20:50:14
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = qtConsole
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += ../../src/wgetter.h
SOURCES += main.cpp \
           ../../src/wgetter.cpp

win32{
  	INCLUDEPATH += $$(LIBED2K_ROOT)\\include $$(CRYPTOPP_ROOT)
	LIBS += -L$$(CRYPTOPP_ROOT)/Win32/DLL_Output/Debug -L$$(CRYPTOPP_ROOT)/Win32/Output/Debug 
	LIBS += -L$$(LIBED2K_ROOT)\\win32\\debug libed2kd.lib advapi32.lib  cryptlib.lib shell32.lib
}

unix:!macx {
    INCLUDEPATH += $$(LIBED2K_ROOT)/include
	LIBS += -L$$(LIBED2K_ROOT)/lib -led2k -lcrypto++
  }

win32{

DEFINES += BOOST_ALL_NO_LIB
DEFINES += BOOST_ASIO_HASH_MAP_BUCKETS=1021
DEFINES += BOOST_EXCEPTION_DISABLE
DEFINES += BOOST_SYSTEM_STATIC_LINK=1
DEFINES += BOOST_THREAD_USE_LIB
DEFINES += BOOST_THREAD_USE_LIB=1
DEFINES += TORRENT_USE_OPENSSL
DEFINES += UNICODE
DEFINES += WIN32
DEFINES += WIN32_LEAN_AND_MEAN
DEFINES += _CRT_SECURE_NO_DEPRECATE
DEFINES += _FILE_OFFSET_BITS=64
DEFINES += _SCL_SECURE_NO_DEPRECATE
DEFINES += _UNICODE
DEFINES += _WIN32
DEFINES += _WIN32_WINNT=0x0500
DEFINES += _WIN32_IE=0x0500
DEFINES += __USE_W32_SOCKETS
DEFINES += WITH_SHIPPED_GEOIP_H
}
