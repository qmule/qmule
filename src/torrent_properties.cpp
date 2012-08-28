#include <QDesktopServices>
#include <QAction>
#include <QMessageBox>
#include <QMenu>

#include "misc.h"
#include "transport/session.h"
#include <libtorrent/version.hpp>
#include "downloadedpiecesbar.h"
#include "pieceavailabilitybar.h"
#include "proplistdelegate.h"
#include "torrentcontentmodel.h"
#include "torrentcontentfiltermodel.h"
#include "torrent_properties.h"

using namespace libtorrent;

torrent_properties::torrent_properties(QWidget *parent, Transfer& transfer)
    : QDialog(parent), h(transfer)
{
    setupUi(this);

    // Set Properties list model
    PropListModel = new TorrentContentFilterModel();
    filesList->setModel(PropListModel);
    PropDelegate = new PropListDelegate();
    filesList->setItemDelegate(PropDelegate);
    filesList->setSortingEnabled(true);

    // Downloaded pieces progress bar
    downloaded_pieces = new DownloadedPiecesBar(this);
    ProgressHLayout->insertWidget(1, downloaded_pieces);
    // Pieces availability bar
    pieces_availability = new PieceAvailabilityBar(this);
    ProgressHLayout_2->insertWidget(1, pieces_availability);

    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(selectAllButton, SIGNAL(clicked()), PropListModel, SLOT(selectAll()));
    connect(selectNoneButton, SIGNAL(clicked()), PropListModel, SLOT(selectNone()));
    connect(filesList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openDoubleClickedFile(QModelIndex)));
    connect(PropListModel, SIGNAL(filteredFilesChanged()), this, SLOT(filteredFilesChanged()));
    connect(PropDelegate, SIGNAL(filteredFilesChanged()), this, SLOT(filteredFilesChanged()));
    connect(filesList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayFilesListMenu(const QPoint&)));

    loadTorrentInfos();

    // Dynamic data refresher
    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(loadDynamicData()));
    refreshTimer->start(3000); // 3sec
}

torrent_properties::~torrent_properties()
{

}

void torrent_properties::loadTorrentInfos() 
{
    if(!h.is_valid()) 
        return;

    try 
    {
        // Save path
        save_path->setText(h.save_path());
        // Hash
        hash_lbl->setText(h.hash());
        PropListModel->model()->clear();
        if (h.has_metadata()) 
        {
            // Creation date
            lbl_creationDate->setText(h.creation_date());
            // Pieces size
            pieceSize_lbl->setText(misc::friendlyUnit(h.piece_length()));
            // Comment
            comment_text->setHtml(misc::parseHtmlLinks(h.comment()));
            // List files in torrent
            PropListModel->model()->setupModelData(h.get_info());
        }
    } 
    catch(invalid_handle& e) 
    {
    }
    // Load dynamic data
    loadDynamicData();
}

void torrent_properties::loadDynamicData() 
{
    if(!h.is_valid()) 
        return;

    try 
    {
        // Transfer infos
        wasted->setText(misc::friendlyUnit(h.total_failed_bytes()+h.total_redundant_bytes()));
        upTotal->setText(misc::friendlyUnit(h.all_time_upload()) + " ("+misc::friendlyUnit(h.total_payload_upload())+" "+tr("this session")+")");
        dlTotal->setText(misc::friendlyUnit(h.all_time_download()) + " ("+misc::friendlyUnit(h.total_payload_download())+" "+tr("this session")+")");

        if (h.upload_limit() <= 0)
            lbl_uplimit->setText(QString::fromUtf8("-"));
        else
            lbl_uplimit->setText(misc::friendlyUnit(h.upload_limit())+tr("/s", "/second (i.e. per second)"));

        if (h.download_limit() <= 0)
            lbl_dllimit->setText(QString::fromUtf8("-"));
        else
            lbl_dllimit->setText(misc::friendlyUnit(h.download_limit())+tr("/s", "/second (i.e. per second)"));

        QString elapsed_txt = misc::userFriendlyDuration(h.active_time());
        if (h.is_seed()) 
            elapsed_txt += " ("+tr("Seeded for %1", "e.g. Seeded for 3m10s").arg(misc::userFriendlyDuration(h.seeding_time()))+")";
        lbl_elapsed->setText(elapsed_txt);

        if (h.connections_limit() > 0)
            lbl_connections->setText(QString::number(h.num_connections())+" ("+tr("%1 max", "e.g. 10 max").arg(QString::number(h.connections_limit()))+")");
        else
            lbl_connections->setText(QString::number(h.num_connections()));

        // Update next announce time
        reannounce_lbl->setText(h.next_announce());
        
        // Update ratio info
        const qreal ratio = Session::instance()->getRealRatio(h.hash());
        if (ratio > Session::MAX_RATIO)
            shareRatio->setText(QString::fromUtf8("-"));
        else
            shareRatio->setText(QString(QByteArray::number(ratio, 'f', 2)));

        if (!h.is_seed()) 
        {
            showPiecesDownloaded(true);
            // Downloaded pieces
            bitfield bf(h.get_info().num_pieces(), 0);
            h.downloading_pieces(bf);
            downloaded_pieces->setProgress(h.pieces(), bf);
            // Pieces availability
            if (h.has_metadata() && !h.is_paused() && !h.is_queued() && !h.is_checking()) 
            {
                showPiecesAvailability(true);
                std::vector<int> avail;
                h.piece_availability(avail);
                pieces_availability->setAvailability(avail);
                avail_average_lbl->setText(QString::number(h.distributed_copies(), 'f', 3));
            } 
            else 
            {
                showPiecesAvailability(false);
            }
            // Progress
            qreal progress = h.progress()*100.;
            if (progress > 99.94 && progress < 100.)
                progress = 99.9;
            progress_lbl->setText(QString::number(progress, 'f', 1)+"%");
        } 
        else 
        {
            showPiecesAvailability(false);
            showPiecesDownloaded(false);
        }

        // Files progress
        if (h.is_valid() && h.has_metadata()) 
        {
            qDebug("Updating priorities in files tab");
            filesList->setUpdatesEnabled(false);
            std::vector<size_type> fp;
            h.file_progress(fp);
            PropListModel->model()->updateFilesPriorities(h.file_priorities());
            PropListModel->model()->updateFilesProgress(fp);
            filesList->setUpdatesEnabled(true);
        }
    }
    catch(invalid_handle e) 
    {
    }
}

void torrent_properties::showPiecesDownloaded(bool show) 
{
    downloaded_pieces_lbl->setVisible(show);
    downloaded_pieces->setVisible(show);
    progress_lbl->setVisible(show);
    if (show || (!show && !pieces_availability->isVisible()))
        line_2->setVisible(show);
}

void torrent_properties::showPiecesAvailability(bool show) 
{
    avail_pieces_lbl->setVisible(show);
    pieces_availability->setVisible(show);
    avail_average_lbl->setVisible(show);
    if (show || (!show && !downloaded_pieces->isVisible()))
        line_2->setVisible(show);
}

void torrent_properties::openDoubleClickedFile(QModelIndex index) 
{
    if (!index.isValid()) 
        return;

    if (!h.is_valid() || !h.has_metadata()) 
        return;

    if (PropListModel->getType(index) == TorrentContentModelItem::TFILE) 
    {
        int i = PropListModel->getFileIndex(index);
        const QDir saveDir(h.save_path());
        const QString filename = h.filepath_at(i);
        const QString file_path = QDir::cleanPath(saveDir.absoluteFilePath(filename));
        qDebug("Trying to open file at %s", qPrintable(file_path));
        // Flush data
        h.flush_cache();
        if (QFile::exists(file_path))
            QDesktopServices::openUrl(QUrl::fromLocalFile(file_path));
        else
            QMessageBox::warning(this, tr("I/O Error"), tr("This file does not exist yet."));
    } 
    else 
    {
        // FOLDER
        QStringList path_items;
        path_items << index.data().toString();
        QModelIndex parent = PropListModel->parent(index);

        while(parent.isValid()) 
        {
            path_items.prepend(parent.data().toString());
            parent = PropListModel->parent(parent);
        }
        const QDir saveDir(h.save_path());
        const QString filename = path_items.join(QDir::separator());
        const QString file_path = QDir::cleanPath(saveDir.absoluteFilePath(filename));
        qDebug("Trying to open folder at %s", qPrintable(file_path));
        // Flush data
        h.flush_cache();
        if (QFile::exists(file_path)) 
            QDesktopServices::openUrl(QUrl::fromLocalFile(file_path));
        else
            QMessageBox::warning(this, tr("I/O Error"), tr("This folder does not exist yet."));
    }
}

void torrent_properties::displayFilesListMenu(const QPoint&) 
{
    QMenu myFilesLlistMenu;
    QModelIndexList selectedRows = filesList->selectionModel()->selectedRows(0);
    QMenu subMenu;
    if (!h.is_seed()) 
    {
        subMenu.setTitle(tr("Priority"));
        subMenu.addAction(actionNot_downloaded);
        subMenu.addAction(actionNormal);
        subMenu.addAction(actionHigh);
        subMenu.addAction(actionMaximum);
        myFilesLlistMenu.addMenu(&subMenu);
    }
    // Call menu
    const QAction *act = myFilesLlistMenu.exec(QCursor::pos());
    if (act) 
    {
        int prio = 1;
        if (act == actionHigh)
            prio = prio::HIGH;
        else
            if (act == actionMaximum)
                prio = prio::MAXIMUM;
            else
                if (act == actionNot_downloaded)
                    prio = prio::IGNORED;
        qDebug("Setting files priority");
        foreach (QModelIndex index, selectedRows) 
        {
            qDebug("Setting priority(%d) for file at row %d", prio, index.row());
            PropListModel->setData(PropListModel->index(index.row(), PRIORITY, index.parent()), prio);
        }
        // Save changes
        filteredFilesChanged();
    }
}

void torrent_properties::filteredFilesChanged() 
{
  if (h.is_valid()) 
    applyPriorities();
}

bool torrent_properties::applyPriorities() 
{
    qDebug("Saving files priorities");
    const std::vector<int> priorities = PropListModel->model()->getFilesPriorities(h.get_info().num_files());
    // Save first/last piece first option state
    bool first_last_piece_first = h.first_last_piece_first();
    // Prioritize the files
    qDebug("prioritize files: %d", priorities[0]);
    h.prioritize_files(priorities);
    // Restore first/last piece first option if necessary
    if (first_last_piece_first)
        h.prioritize_first_last_piece(true);
    return true;
}

void torrent_properties::close()
{
    accept();
}
