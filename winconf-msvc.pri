RC_FILE = qmule.rc

# Enable Wide characters
DEFINES += TORRENT_USE_WPATH

#Adapt the lib names/versions accordingly
CONFIG(debug, debug|release) {
  LIBS += libtorrentd.lib \
          libboost_system-vc90-mt-gd-1_49.lib \
          libboost_filesystem-vc90-mt-gd-1_49.lib \
          libboost_thread-vc90-mt-gd-1_49.lib \
          libed2kd.lib \
          libeay32MD.lib\
          ssleay3MD2.lib
} else {
  LIBS += libtorrent.lib \
          libboost_system-vc90-mt-1_49.lib \
          libboost_filesystem-vc90-mt-1_49.lib \
          libboost_thread-vc90-mt-1_49.lib \		  
          libed2k.lib\
          libeay32MD.lib\
          ssleay32MD.lib
}

CONFIG(win7){
  LIBS += ole32.lib
}

LIBS += advapi32.lib shell32.lib cryptlib.lib user32.lib
LIBS += PowrProf.lib
