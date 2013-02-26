TEMPLATE = app

QT += network webkit

RESOURCES += \
    xcatalog.qrc

RC_FILE = xcatalog.rc

SOURCES += main.cpp \
    catalogwidget.cpp \
    foldermodel.cpp \
    folder.cpp \
    catalog.cpp \
    abstractcatalogdataprovider.cpp \
    filemodel.cpp \
    tools.cpp \
    3rdparty/lineedit/filterlineedit.cpp \
    3rdparty/lineedit/fancylineedit.cpp \
    fileitemdelegate.cpp \
    3rdparty/sdtp/sdtp.cpp \
    sdtpcatalogdataprovider.cpp \
    reportdialog.cpp \
    loadhelper.cpp

HEADERS += \
    catalogwidget.h \
    foldermodel.h \
    folder.h \
    config.h \
    catalog.h \
    abstractcatalogdataprovider.h \
    filemodel.h \
    tools.h \
    3rdparty/lineedit/filterlineedit.h \
    3rdparty/lineedit/fancylineedit.h \
    fileitemdelegate.h \
    3rdparty/sdtp/sdtp.h \
    sdtpcatalogdataprovider.h \
    reportdialog.h \
    constants.h \
    loadhelper.h

FORMS += \
    catalogwidget.ui \
    reportdialog.ui

OTHER_FILES += \
    resources/fileinfo.template \
    resources/loading.template \
    resources/error_loading.template \
    resources/common.css









