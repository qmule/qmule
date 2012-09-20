#ifndef TORRENT_PROPERTIES_H
#define TORRENT_PROPERTIES_H

#include <QDialog>
#include "ui_torrent_properties.h"
#include "transport/transfer.h"

class DownloadedPiecesBar;
class PieceAvailabilityBar;
class TorrentContentFilterModel;
class PropListDelegate;

class torrent_properties : public QDialog, public Ui::torrent_properties
{
    Q_OBJECT

    Transfer h;
    DownloadedPiecesBar *downloaded_pieces;
    PieceAvailabilityBar *pieces_availability;
    TorrentContentFilterModel *PropListModel;
    PropListDelegate *PropDelegate;
    QTimer *refreshTimer;

public:
    torrent_properties(QWidget *parent, Transfer& transfer);
    ~torrent_properties();

private:
    void loadTorrentInfos();
    void showPiecesDownloaded(bool show);
    void showPiecesAvailability(bool show);
    void openDoubleClickedFile(QModelIndex);
    bool applyPriorities();
    
private slots:
    void loadDynamicData();
    void filteredFilesChanged();
    void displayFilesListMenu(const QPoint& pos);
    void close();
};

#endif // TORRENT_PROPERTIES_H
