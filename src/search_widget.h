#ifndef SEARCH_WIDGET_H
#define SEARCH_WIDGET_H

#include <QWidget>
#include "ui_search_widget.h"
#include "search_widget_delegate.h"
#include "qtlibed2k/qed2ksession.h"

QT_BEGIN_NAMESPACE
class QSortFilterProxyModel;
class QStandardItemModel;
QT_END_NAMESPACE

class SWDelegate;
class search_filter;

enum RESULT_TYPE
{
    RT_FILES,
    RT_CLIENTS,
    RT_FOLDERS,
    RT_USER_DIRS
};

struct UserDir
{
    UserDir() : bExpanded(false), bFilled(false), dirPath("") {}

    bool    bExpanded;
    bool    bFilled;
    QString dirPath;
    std::vector<QED2KSearchResultEntry> vecFiles;
};

struct SearchResult
{
    SearchResult(RESULT_TYPE type, const std::vector<QED2KSearchResultEntry>& vRes) : 
        resultType(type), vecResults(vRes), vecUserDirs(), netPoint() {}
    SearchResult(RESULT_TYPE type, const std::vector<QED2KSearchResultEntry>& vRes, const std::vector<UserDir> userDirs, const libed2k::net_identifier& np) : 
        resultType(type), vecResults(vRes), vecUserDirs(userDirs), netPoint(np) {}

    RESULT_TYPE resultType;
    std::vector<QED2KSearchResultEntry> vecResults;
    std::vector<UserDir> vecUserDirs;
    libed2k::net_identifier netPoint;
};

class search_widget : public QWidget , private Ui::search_widget
{
    Q_OBJECT

private:
    QMenu* menuResults;
    QAction* closeAll;
    QMenu* menuSubResults;
    QAction* defValue;
    QAction* defKilos;
    QAction* defMegas;
    QTabBar* tabSearch;

    std::vector<SearchResult> searchItems;
    std::vector<libed2k::net_identifier> connectedPeers;

    int nCurTabSearch;
    int nSortedColumn;

    bool moreSearch;
    QIcon iconSerachActive;
    QIcon iconSearchResult;
    QIcon iconUserFiles;
    QScopedPointer<QStandardItemModel> model;
    QScopedPointer<QSortFilterProxyModel> filterModel;
    SWDelegate* itemDelegate;
    search_filter* searchFilter;

    QMenu* userMenu;
    QAction* userUpdate;
    QAction* userDetails;
    QAction* userAddToFriends;
    QAction* userSendMessage;
    QAction* userBrowseFiles;

    QIcon iconAny;
    QIcon iconArchive;
    QIcon iconAudio;
    QIcon iconCDImage;
    QIcon iconPicture;
    QIcon iconProgram;
    QIcon iconVideo;
    QIcon iconDocument;
    QIcon iconCollection;
    QIcon iconFolder;
    QIcon iconUser;

public:
    search_widget(QWidget *parent = 0);
    ~search_widget();

private:
    void addCondRow();
    void clearSearchTable();
    void showErrorParamMsg(int numParam);
    void setUserPicture(const libed2k::net_identifier& np, QIcon& icon);
    bool findSelectedUser(QED2KSearchResultEntry& entry);
    void fillFileValues(int row, const QED2KSearchResultEntry& fileEntry, const QModelIndex& parent = QModelIndex());

private slots:
    void itemCondClicked(QTableWidgetItem* item);
    void sortChanged(int logicalIndex, Qt::SortOrder order);
    void startSearch();
    void continueSearch();
    void processSearchResult(const libed2k::net_identifier& np,
    		                 const QString& hash,
    		                 const std::vector<QED2KSearchResultEntry>& vRes, bool bMoreResult);
    void closeTab(int index);
    void selectTab(int nTabNum);
    void setSizeType();
    void searchTextChanged(const QString text);
    void applyFilter(QString filter);
    void setFilterType(SWDelegate::Column column);
    void displayListMenu(const QPoint&);
    void initPeer();
    void sendMessage();
    void peerConnected(const libed2k::net_identifier& np, const QString&, bool bActive);
    void peerDisconnected(const libed2k::net_identifier& np, const QString&, const libed2k::error_code ec);
    void resultSelectionChanged(const QItemSelection& sel, const QItemSelection& unsel);
    void download();
    void requestUserDirs();
    void processUserDirs(const libed2k::net_identifier& np, const QString& hash, const QStringList& strList);
    void processUserFiles(const libed2k::net_identifier& np, const QString& hash,
                          const QString& strDirectory, const std::vector<QED2KSearchResultEntry>& vRes);
    void itemCollapsed(const QModelIndex& index);
    void itemExpanded(const QModelIndex& index);

signals:
    void sendMessage(const QString& user_name, const libed2k::net_identifier& np);
};

#endif // SEARCH_WIDGET_H
