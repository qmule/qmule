# Adapt these paths on Windows

#Point this to the boost include folder
INCLUDEPATH += $$(BOOST_ROOT)
#Point this to the libtorrent include folder
INCLUDEPATH += $$(LIBTORRENT_ROOT)/include
#Point this to the zlib include folder(libtorrent's if you used that)
INCLUDEPATH += $$(LIBTORRENT_ROOT)/zlib
#Point this to the openssl include folder
INCLUDEPATH += $$(OPENSSL_ROOT)/include
#Point this to the ed2klib include folder
INCLUDEPATH += $$(LIBED2K_ROOT)/include

#Point this to the openssl lib folder
LIBS += -L$$(OPENSSL_ROOT)/lib/VC
#Point this to the libtorrent lib folder
LIBS += -L$$(LIBTORRENT_ROOT)/lib
#Point this to the boost lib folder
LIBS += -L$$(BOOST_ROOT)/stage/lib

#Point this to the ed2klib lib folder
Debug:LIBS += -L$$(LIBED2K_ROOT)/Win32/Debug
Release:LIBS += -L$$(LIBED2K_ROOT)/Win32/Release
Release:DEFINES += _BIND_TO_CURRENT_VCLIBS_VERSION=1

DEFINES += "\"BUILDDATE=\\\"$$system(date /t && time /t)\\\"\""

# LIBTORRENT/LIBED2K DEFINES
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

win7 {
  DEFINES += _WIN32_WINNT=0x0601
} else:wista {
  DEFINES += _WIN32_WINNT=0x0600
} else {
  DEFINES += _WIN32_WINNT=0x0500
}


DEFINES += _WIN32_IE=0x0500
DEFINES += __USE_W32_SOCKETS
DEFINES += WITH_SHIPPED_GEOIP_H

CONFIG(debug, debug|release) {
  DEFINES += TORRENT_DEBUG
  DEFINES += LIBED2K_DEBUG
} else {
  DEFINES += NDEBUG
}

CONFIG(win7) {
  DEFINES += WIN7_SDK
}

win32-g++ {
  include(winconf-mingw.pri)
}
else {
  include(winconf-msvc.pri)
}

DEFINES += WITH_GEOIP_EMBEDDED
message("On Windows, GeoIP database must be embedded.")
