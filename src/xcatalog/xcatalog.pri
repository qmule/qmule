INCLUDEPATH += $$PWD

HEADERS += $$PWD/abstractcatalogdataprovider.h \
    $$PWD/catalog.h \
    $$PWD/catalogwidget.h \
    $$PWD/config.h \
    $$PWD/constants.h \
    $$PWD/3rdparty/lineedit/fancylineedit.h \
    $$PWD/fileitemdelegate.h \
    $$PWD/filemodel.h \
    $$PWD/3rdparty/lineedit/filterlineedit.h \
    $$PWD/folder.h \
    $$PWD/foldermodel.h \
    $$PWD/reportdialog.h \
    $$PWD/3rdparty/sdtp/sdtp.h \
    $$PWD/sdtpcatalogdataprovider.h \
    $$PWD/tools.h

SOURCES += $$PWD/abstractcatalogdataprovider.cpp \
    $$PWD/catalog.cpp \
    $$PWD/catalogwidget.cpp \
    $$PWD/3rdparty/lineedit/fancylineedit.cpp \
    $$PWD/fileitemdelegate.cpp \
    $$PWD/filemodel.cpp \
    $$PWD/3rdparty/lineedit/filterlineedit.cpp \
    $$PWD/folder.cpp \
    $$PWD/foldermodel.cpp \
    $$PWD/reportdialog.cpp \
    $$PWD/3rdparty/sdtp/sdtp.cpp \
    $$PWD/sdtpcatalogdataprovider.cpp \
    $$PWD/tools.cpp

FORMS += $$PWD/catalogwidget.ui \
    $$PWD/reportdialog.ui

RESOURCES += $$PWD/xcatalog.qrc
