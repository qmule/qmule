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
    RT_FOLDERS
};

struct SearchResult
{
    SearchResult(const std::vector<QED2KSearchResultEntry>& vRes, RESULT_TYPE type) : resultType(type), vecResults(vRes) {}
    RESULT_TYPE resultType;
    std::vector<QED2KSearchResultEntry> vecResults;
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
    bool moreSearch;
    QIcon iconSerachActive;
    QIcon iconSearchResult;
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

public:
    search_widget(QWidget *parent = 0);
    ~search_widget();

private:
    void addCondRow();
    void clearSearchTable();
    void showErrorParamMsg(int numParam);
    void setUserPicture(const libed2k::net_identifier& np, QIcon& icon);
    bool findSelectedUser(QED2KSearchResultEntry& entry);

private slots:
    void itemCondClicked(QTableWidgetItem* item);
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

signals:
    void sendMessage(const QString& user_name, const libed2k::net_identifier& np);
};

#endif // SEARCH_WIDGET_H
