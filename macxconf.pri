PREFIX = /usr/local
BINDIR = /usr/local/bin
DATADIR = /usr/local/share

# Use pkg-config to get all necessary libtorrent DEFINES
CONFIG += link_pkgconfig
PKGCONFIG += libtorrent-rasterbar

# Special include/libs paths (macports)
INCLUDEPATH += /usr/include/openssl /usr/include /opt/local/include/boost /opt/local/include
INCLUDEPATH += $$(LIBED2K_ROOT)/include
LIBS += -L/opt/local/lib
LIBS += -L$$(LIBED2K_ROOT)/lib -led2k

# OpenSSL lib
LIBS += -lssl -lcrypto
# Boost system lib
LIBS += -lboost_system-mt
# Boost filesystem lib (Not needed for libtorrent >= 0.16.0)
LIBS += -lboost_filesystem-mt -lboost_thread-mt
# Carbon
LIBS += -framework Carbon -framework IOKit
QMAKE_CXXFLAGS += -fpermissive
document_icon.path = Contents/Resources
document_icon.files = Icons/qMuleDocument.icns

QMAKE_BUNDLE_DATA += document_icon
ICON = Icons/qmule_mac.icns
QMAKE_INFO_PLIST = Info.plist

DEFINES += WITH_GEOIP_EMBEDDED
DEFINES += LIBED2K_USE_BOOST_DATE_TIME
message("On Mac OS X, GeoIP database must be embedded.")
