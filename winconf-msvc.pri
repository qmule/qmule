RC_FILE = qbittorrent.rc

# Enable Wide characters
DEFINES += TORRENT_USE_WPATH

#Adapt the lib names/versions accordingly
CONFIG(debug, debug|release) {
  LIBS += libtorrentd.lib \
          libboost_system-vc90-mt-gd-1_49.lib \
          libboost_filesystem-vc90-mt-gd-1_49.lib \
          libboost_thread-vc90-mt-gd-1_49.lib \
          libed2kd.lib
} else {
  LIBS += libtorrent.lib \
          libboost_system-vc90-mt-1_49.lib \
          libboost_filesystem-vc90-mt-1_49.lib \
          libboost_thread-vc90-mt-1_49.lib \		  
          libed2k.lib
}

LIBS += advapi32.lib shell32.lib cryptopp.lib cryptlib.lib
LIBS += libeay32MD.lib ssleay32MD.lib
LIBS += PowrProf.lib
