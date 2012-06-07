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

#include <QtCore/QSettings>
#include <QtGui/QDesktopServices>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>

#ifdef _WINDLL
#include <ActiveQt/QAxFactory>
#endif

#include "loadhelper.h"
#include "catalogwidget.h"
#include "catalog.h"
#include "foldermodel.h"
#include "filemodel.h"
#include "fileitemdelegate.h"
#include "reportdialog.h"
#include "tools.h"
#include "config.h"
#include "ui_catalogwidget.h"

#ifdef _WINDLL
QAXFACTORY_BEGIN("{64468e37-9ecc-4042-8bda-3e06b9a85999}",
                 "{1467b275-6491-41fa-9dd2-bd4515f6662c}")
	QAXCLASS(XCatalogWidget)
QAXFACTORY_END()
#endif

XCatalogWidget::XCatalogWidget( QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::XCatalogWidget),
    m_catalog(new XCatalog(this)),
    m_searchMode(false)
{
    ui->setupUi(this);
    setWindowTitle( tr("xCatalog v%1").arg(XCFG_VERSION) );

    // load settings, restore window geometry/position
    QSettings settings(XCFG_APPNAME_CFG, XCFG_APPNAME);
    restoreGeometry(settings.value(XCFG_WINDOW_GEOMETRY).toByteArray());
    ui->splitter->restoreState(settings.value(XCFG_WINDOW_STATE).toByteArray());
    int sortRole = settings.value(XCFG_FILES_SORT_ROLE, FileModel::DateRole).toInt();

    // setup search box
    //ui->searchBox->setButtonPixmap(FancyLineEdit::Right, QPixmap(QLatin1String(":/images/search.png")) );
    //ui->searchBox->setButtonVisible(FancyLineEdit::Right, true);

    // setup filter box
    ui->filterBox->setButtonPixmap(FancyLineEdit::Left, QPixmap(QLatin1String(":/images/filter.png")) );
    ui->filterBox->setButtonVisible(FancyLineEdit::Left, true);
    ui->filterBox->setVisible(false);

    connect(ui->filterBox, SIGNAL(filterCleared()),
            ui->filterBox, SLOT(hide()));

    connect(m_catalog, SIGNAL(filesFetched(XFolder*)),
            this, SLOT(onFilesReady(XFolder*)) );

    connect(m_catalog, SIGNAL(fileDetailsFetched(XFile*)),
            this, SLOT(onFileDetailsReady(XFile*)) );

    connect(m_catalog, SIGNAL(thumbnailFetched(XFile*)),
            this, SLOT(onThumbnailReady(XFile*)), Qt::QueuedConnection );

    connect(m_catalog, SIGNAL(searchComplete(XFolder*)),
            this, SLOT(onSearchComplete(XFolder*)) );

    connect (m_catalog, SIGNAL(error(int,void*)),
             this, SLOT(onError(int,void*)) );

    connect(m_catalog, SIGNAL(allFoldersFetched(XFolder*)),
            this, SLOT(onAllFoldersFetched(XFolder*)), Qt::QueuedConnection );

    ui->filesView->installEventFilter(this);

    // QWebView link delegation
    connect(ui->fileDesc->page(), SIGNAL(linkClicked(QUrl)),
            this, SLOT(onLinkClicked(QUrl)) );
    ui->fileDesc->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    ui->foldersView->setModel( new FolderModel(m_catalog) );

    // file list view and model
    FileModel *sourceFileModel = new FileModel(m_catalog);
    QSortFilterProxyModel *proxyFileModel = new QSortFilterProxyModel;
    proxyFileModel->setSourceModel(sourceFileModel);
    proxyFileModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyFileModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyFileModel->setDynamicSortFilter(true);
    ui->filesView->setModel(proxyFileModel);
    ui->filesView->setItemDelegate(new FileItemDelegate);
    connect(ui->filterBox, SIGNAL(filterChanged(QString)),
            this, SLOT(filterChanged(QString)) );
    ui->filesView->header()->setVisible(false);

    connect(ui->foldersView, SIGNAL(activated(QModelIndex)),
            this, SLOT(onFolderSelected(QModelIndex)) );

    connect(ui->filesView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(onFileSelected(QModelIndex)) );
    connect(ui->filesView, SIGNAL(activated(QModelIndex)),
            this, SLOT(onFileSelected(QModelIndex)) );

    connect(ui->foldersView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onCategorySelectionChange(QItemSelection,QItemSelection)) );

    connect(ui->searchBox, SIGNAL(returnPressed()),
            this, SLOT(execSearch()) );

    connect(ui->searchButton, SIGNAL(clicked()),
            this, SLOT(execSearch()) );

    // file list menu
    QMenu *flMenu = new QMenu(ui->fileListButton);
    QAction* action = NULL;

    action = new QAction(QIcon(QLatin1String(":/images/refresh_folders.png")), tr("Обновить список категорий"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(refreshFolderView()));
    flMenu->addAction(action);

    action = new QAction(QIcon(QLatin1String(":/images/refresh_files.png")), tr("Обновить список файлов"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(refreshFileView()));
    flMenu->addAction(action);

    flMenu->addSeparator();

    QActionGroup *sortGroup = new QActionGroup(flMenu);

    action = new QAction(QIcon(QLatin1String(":/images/sort-alphabet.png")), tr("Сортировать по имени"), this);
    action->setCheckable(true);
    if ( Qt::DisplayRole == sortRole )
        action->setChecked(true);
    connect(action, SIGNAL(triggered()), this, SLOT(sortByName()));
    sortGroup->addAction(action);
    flMenu->addAction(action);

    action = new QAction(QIcon(QLatin1String(":/images/sort-number.png")), tr("Сортировать по дате"), this);
    action->setCheckable(true);
    if ( FileModel::DateRole == sortRole )
        action->setChecked(true);
    connect(action, SIGNAL(triggered()), this, SLOT(sortByDate()));
    sortGroup->addAction(action);
    flMenu->addAction(action);

    ui->fileListButton->setMenu(flMenu);
    if ( Qt::DisplayRole == sortRole )
        sortByName();
    else
        sortByDate();

    // load templates
    m_templateFileInfo = Util::readFileContents(QLatin1String(":/resources/fileinfo.template"), "utf-8");
    m_templateLoading = Util::readFileContents(QLatin1String(":/resources/loading.template"), "utf-8");
    m_templateErrorLoading = Util::readFileContents(QLatin1String(":/resources/error_loading.template"), "utf-8");

    // load all folders at startup
    m_catalog->fetchFoldersRecursive(m_catalog->rootFolder());
}

XCatalogWidget::~XCatalogWidget()
{
    QSettings settings(XCFG_APPNAME_CFG, XCFG_APPNAME);
    settings.setValue(XCFG_WINDOW_GEOMETRY, saveGeometry());
    settings.setValue(XCFG_WINDOW_STATE, ui->splitter->saveState());

    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel*>(ui->filesView->model());
    //settings.setValue(XCFG_FILES_SORT_ORDER, model->sortOrder());
    settings.setValue(XCFG_FILES_SORT_ROLE, model->sortRole());

    delete ui;
    delete m_catalog;
}

void XCatalogWidget::onFilesReady( XFolder *parent )
{
    ui->filesView->model()->revert();
    ui->foldersView->setDisabled(false);

    static bool isFirst = true;

    // select first file on startup
    if ( isFirst && parent->files().count() > 0 ) {
        QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel*>(ui->filesView->model());
        XFile *file = FileModel::indexToFile(model->mapToSource(model->index(0,0)));
        m_catalog->setSelectedFile(file);
        m_catalog->fetchFileDetails(file);
        ui->filesView->selectionModel()->setCurrentIndex(model->index(0,0), QItemSelectionModel::SelectCurrent);
        isFirst = false;
    }
}

void XCatalogWidget::onFileDetailsReady( XFile *file )
{
    if ( m_catalog->selectedFile() == file ) {
        setFileDetails(file);
    }
}

void XCatalogWidget::onThumbnailReady( XFile *file )
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel*>(ui->filesView->model());

    if ( m_catalog->selectedFolder() == NULL ||
         m_catalog->selectedFolder()->files().indexOf(file) == -1 )
        return;

    QModelIndex sourceIndex;
    if ( m_searchMode )
        sourceIndex = model->sourceModel()->index( m_catalog->selectedFolder()->files().indexOf(file), 0);
    else
        sourceIndex = model->sourceModel()->index( file->parent()->files().indexOf(file), 0 );
    QModelIndex proxyIndex = model->mapFromSource(sourceIndex);

    ui->filesView->update(proxyIndex);
}

void XCatalogWidget::onFolderSelected( const QModelIndex &index )
{
    XFolder *folder = FolderModel::indexToFolder(index);
    if ( NULL == folder ) {
        qDebug("Warning: selected index hasn't stored folder");
        return;
    }

    ui->pathLabel->setText(m_catalog->createFolderPath(folder));
    ui->fileDesc->setHtml("");

    ui->foldersView->setDisabled(true);

    if ( NULL != m_catalog->selectedFolder() ) {
        m_catalog->selectedFolder()->freeThumbnails();
    }

    if ( m_searchMode ) {
        delete m_catalog->selectedFolder();
        m_searchMode = false;
    }

    m_catalog->setSelectedFolder(folder);
    //m_catalog->setSelectedFolder(NULL);
    m_catalog->fetchFiles(folder);
}

void XCatalogWidget::onFileSelected( const QModelIndex &index )
{
    XFile* file = ui->filesView->model()->data(index, FileModel::XFileRole).value<XFile*>();
    if ( NULL == file ) {
        qDebug("Warning: selected index hasn't stored file");
        return;
    }

    m_catalog->setSelectedFile(file);
    QString path = QString("%1->%2").arg(m_catalog->createFolderPath(file->parent()), file->name());
    ui->pathLabel->setText( path );

    if ( file->info().isEmpty() ) {
        ui->fileDesc->setHtml(m_templateLoading);
        m_catalog->fetchFileDetails(file);
        return;
    } else {
        setFileDetails(file);
    }
}

void XCatalogWidget::setFileDetails( const XFile *file )
{
    if ( NULL == file ) {
        ui->fileDesc->setHtml(m_templateErrorLoading);
    } else {
        QString html = m_templateFileInfo;

        html.replace("%INFO%", file->info());
        html.replace("%AUTHOR%", file->author());
        html.replace("%DATE%", file->date().toString("dd.MM.yyyy hh:mm"));
        html.replace("%XTOPICID%", QString("%1").arg(file->parent()->id()) );
        html.replace("%XPOSTID%", QString("%1").arg(file->id()) );

#ifdef _WINDLL
        QRegExp rx("<a href=\"(http://fake\\.link/\\?link=[A-Za-z0-9/+=]+)\">(.*)</a>");
        rx.setMinimal(true); // non greeeeedy
        int pos = 0;
        QUrl url;
        int urlCounter = 0;
        while ( (pos = rx.indexIn(html, pos)) != -1 ) {
            // skip all empty <a></a> tags
            QString linkText = rx.cap(2);
            if ( linkText.isEmpty() ) {
                pos += rx.matchedLength();
                continue;
            }

            url = QUrl(rx.cap(1));
            QByteArray data = QByteArray::fromPercentEncoding ( QByteArray::fromBase64( url.queryItemValue("link").toAscii() ) );
            QString ed2kLink = QString::fromUtf8(data.constData());



            if ( ed2kLink.startsWith("ed2k://") ) {
                int startIndex = ed2kLink.indexOf("file|") + 5;
                int length =  ed2kLink.indexOf('|', startIndex) - startIndex;
                QString fileName = ed2kLink.mid(startIndex, length);

                if ( Util::isMovie(fileName) ) {
                    urlCounter++;
                    url.setHost("preview.link");
                    QString previewTemplate = tr(
                        "<a href=\"%ED2KLINK%\" class=\"img-link\" title=\"Просмотр\">"
                            "<img src=\"qrc:/images/preview16.png\" alt=\"Просмотр\">"
                        "</a>&nbsp;");
                    previewTemplate.replace("%ED2KLINK%", url.toString());
                    //rowTemplate.replace("%FNAME%", fileName);
                    html.insert(pos, previewTemplate);
                    pos += previewTemplate.length();
                }
            }

            pos += rx.matchedLength();
        }

        if ( urlCounter == 1 ) {
            QString previewTemplate = tr(
                "<a href=\"%ED2KLINK%\" class=\"cool-button play-button\">"
                "<table><tr>"
                "<td width=\"5\"><img src=\"qrc:/images/preview32.png\" alt=\"Просмотр\"></td>"
                "<td>Просмотр</td>"
                "</tr></table>"
                "</a>");
            previewTemplate.replace("%ED2KLINK%", url.toString());
            html.replace("<!--PREVIEW-->", previewTemplate);
        }
#endif
        ui->fileDesc->setHtml(html);
    }
}

void XCatalogWidget::onLinkClicked( const QUrl &link )
{
    const QString action = link.host();

    // any link
    if ( action == "fake.link" )
    {
        QByteArray data = QByteArray::fromPercentEncoding ( QByteArray::fromBase64( link.queryItemValue("link").toAscii() ) );
        QString realLink = QString::fromUtf8(data.constData());

#ifdef _WINDLL
        if ( realLink.startsWith("ed2k://") ) {
            emit ed2kLinkEvent(realLink);
        } else {
            Util::openUrl(realLink);
        }
#else
        Util::openUrl(realLink);
#endif

        return;
    }

#ifdef _WINDLL
    // preview link
    if ( action == "preview.link" )
    {
		QByteArray data = QByteArray::fromPercentEncoding ( QByteArray::fromBase64( link.queryItemValue("link").toAscii() ) );
		QString realLink = QString::fromUtf8(data.constData());

        emit ed2kLinkEvent(realLink);
        emit filePreviewEvent(realLink);

        return;
    }
#endif

    // abuse report link
    else if ( action == "report.link" )
    {
        bool ok = false;
        uint postid = link.queryItemValue("postid").toUInt(&ok);
        if ( ok ) {
            ReportDialog reportDlg(this);
            reportDlg.setPostId(postid);
            reportDlg.exec();
        }

        return;
    }

    // refresh interface link
    else if ( action == "reload.link" )
    {
        int type = link.queryItemValue("type").toInt();
        switch ( type ) {
        case FoldersRecursive:
            refreshFolderView();
            break;
        case Files:
            refreshFileView();
            break;
        case FileDetails:
            m_catalog->fetchFileDetails(m_catalog->selectedFile());
            break;
        }

        return;
    }

    // unknown link
    Util::openUrl( link.toString() );
}

void XCatalogWidget::refreshFileView()
{
    if ( m_searchMode )
        return;

    ui->fileDesc->setHtml("");

    if ( NULL == m_catalog->selectedFolder() )
        return;

    ui->foldersView->setDisabled(true);
    m_catalog->selectedFolder()->clearFiles();
    m_catalog->fetchFiles(m_catalog->selectedFolder());
}

void XCatalogWidget::refreshFolderView()
{
    m_catalog->reset();
    m_catalog->fetchFoldersRecursive(m_catalog->rootFolder());
}

void XCatalogWidget::filterChanged( const QString &filter )
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel*>(ui->filesView->model());
    model->setFilterFixedString(filter);
}

void XCatalogWidget::sortByName()
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel*>(ui->filesView->model());
    model->setSortRole(Qt::DisplayRole);
    ui->filesView->header()->setSortIndicator(0, Qt::AscendingOrder);

    ui->fileListButton->setIcon(QIcon(QLatin1String(":/images/sort-alphabet.png")));
}

void XCatalogWidget::sortByDate()
{
    QSortFilterProxyModel *model = qobject_cast<QSortFilterProxyModel*>(ui->filesView->model());
    model->setSortRole(FileModel::DateRole);
    ui->filesView->header()->setSortIndicator(0, Qt::DescendingOrder);

    ui->fileListButton->setIcon(QIcon(QLatin1String(":/images/sort-number.png")));
}

void XCatalogWidget::onCategorySelectionChange( const QItemSelection & selected, const QItemSelection & deselected  )
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    QModelIndexList list = ui->foldersView->selectionModel()->selectedIndexes();

    QString searchPlaceholder;
    if ( list.count() == 0)
        searchPlaceholder = tr("Поиск по всему каталогу");
    else if ( list.count() == 1 ) {
        const XFolder *folder = FolderModel::indexToFolder(list.first());
        searchPlaceholder = tr("Поиск по категории '%1'").arg(folder->name());
    } else {
        searchPlaceholder = tr("Поиск по выбранным категориям");
    }

    ui->searchBox->setPlaceholderText(searchPlaceholder);
}

void XCatalogWidget::execSearch()
{
    // get search query
    QString searchQuery = ui->searchBox->text();
    if ( searchQuery.length() < 3 ) // min 3 characters
        return;

    // get selected categories
    XFolderList selectedFolders;
    QModelIndexList list = ui->foldersView->selectionModel()->selectedIndexes();
    for ( QModelIndexList::const_iterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        XFolder* folder = FolderModel::indexToFolder(*it);
        if ( folder->parent()->id() == 0 ) { // top folder => select children
            selectedFolders += folder->folders();
        } else
            selectedFolders.append( folder );
    }

    ui->searchBox->setEnabled(false);
    ui->searchButton->setEnabled(false);

    m_catalog->execSearch(searchQuery, selectedFolders);
}

void XCatalogWidget::onSearchComplete( XFolder *result )
{

    ui->pathLabel->setText(tr("Результаты поиска"));
    ui->fileDesc->setHtml("");

    if ( NULL != m_catalog->selectedFolder() ) {
        m_catalog->selectedFolder()->freeThumbnails();
    }

    if ( m_searchMode ) {
        XFolder* currentFolder = m_catalog->selectedFolder();
        delete currentFolder;
    }
    m_catalog->setSelectedFolder(result);
    ui->filesView->model()->revert();
    m_searchMode = true;

    ui->searchBox->setEnabled(true);
    ui->searchButton->setEnabled(true);
}

bool XCatalogWidget::eventFilter( QObject *obj, QEvent *event )
{
    if ( ui->filesView == obj && event->type() == QEvent::KeyPress ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ( !keyEvent->text().trimmed().isEmpty() ) {
            ui->filterBox->setVisible(true);
            ui->filterBox->setFocus();
            ui->filterBox->setText(keyEvent->text());
            return true;
        }
        return false;
    } else {
        // standard event processing
        return false;
    }
}

void XCatalogWidget::onError( int type, void* param )
{
    QString html = m_templateErrorLoading;
    QString reason;

    switch ( type ) {
    case FoldersRecursive:
        reason =  QString::fromUtf8("Не удалось загрузить список категорий.");
        break;
    case Files:
        ui->foldersView->setDisabled(false);
        reason =  QString::fromUtf8("Не удалось загрузить список файлов.");
        break;
    case FileDetails:
        reason =  QString::fromUtf8("Не удалось загрузить описание файла.");
        break;
    case Search:
        reason =  QString::fromUtf8("Не удалось выполнить поиск.");
        break;
    default:
        qDebug("UNKNOWN ERROR: type=%d, ptr=%p", type, param);
        return;
    }

    html.replace("%REASON%", reason );
    html.replace("%TYPE%", QString("%1").arg(type) );

    ui->fileDesc->setHtml(html);
}

void XCatalogWidget::onAllFoldersFetched( XFolder* )
{
    ui->filesView->model()->revert();
    ui->foldersView->model()->revert();

    XFolder *folder = m_catalog->folderDict()[400];
    if ( folder ) {
        QModelIndex parent = ui->foldersView->model()->index( m_catalog->rootFolder()->folders().indexOf(folder), 0);
        ui->foldersView->setExpanded(parent, true);
        ui->foldersView->selectionModel()->setCurrentIndex(ui->foldersView->model()->index(0,0, parent), QItemSelectionModel::SelectCurrent);
    }

    folder = m_catalog->folderDict()[200];
    if (  folder ) {
        m_catalog->setSelectedFolder(folder);
        m_catalog->fetchFiles(folder);
    }
}
