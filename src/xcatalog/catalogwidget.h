//this file is part of xCatalog
//Copyright (C) 2011 xCatalog Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef CATALOG_WIDGET_H
#define CATALOG_WIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QWidget>
#include <QtGui/QItemSelection>

class XCatalog;
class XFolder;
class XFile;

namespace Ui {
    class XCatalogWidget;
}

class XCatalogWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(XCatalogWidget)

#ifdef _WINDLL
    Q_CLASSINFO("ClassID", "{45d10b88-8b68-4ee6-940b-983ba8bf5c49}")
    Q_CLASSINFO("InterfaceID", "{e0df9386-ae7c-4d07-aee5-3b7a9630fe86}")
    Q_CLASSINFO("EventsID", "{844ceebf-4554-4f2d-b2e8-dc8421d2ada6}")
    Q_CLASSINFO("RegisterObject", "yes")
#endif

public:
    explicit XCatalogWidget( QWidget *parent = 0 );
    ~XCatalogWidget();

signals:
    void ed2kLinkEvent( QString ed2kLink );
    void filePreviewEvent( QString ed2kLink );

private slots:
    void onFilesReady( XFolder *folder );
    void onFileDetailsReady( XFile *file );
    void onThumbnailReady( XFile *file );

    void onAllFoldersFetched( XFolder* folder );
    void onFolderSelected( const QModelIndex &index );
    void onFileSelected( const QModelIndex &index );
    void onLinkClicked( const QUrl &link );
    void refreshFileView();
    void refreshFolderView();
    void filterChanged( const QString &filter );
    void sortByName();
    void sortByDate();
    void onCategorySelectionChange( const QItemSelection & selected, const QItemSelection & deselected );
    void execSearch();
    void onSearchComplete( XFolder *result );
    void onError( int type, void* param );

protected:
    void setFileDetails( const XFile *file );
    bool eventFilter( QObject *obj, QEvent *event );

    Ui::XCatalogWidget *ui;
    XCatalog *m_catalog;
    QString m_templateFileInfo;
    QString m_templateErrorLoading;
    QString m_templateLoading;
    bool m_searchMode;
};

#endif // CATALOG_WIDGET_H
