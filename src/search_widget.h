#ifndef SEARCH_WIDGET_H
#define SEARCH_WIDGET_H

#include <QWidget>
#include <QWebView>
#include <QSortFilterProxyModel>

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
    UserDir(Preferences& pref);

    bool    bExpanded;
    bool    bFilled;
    QString dirPath;
    std::vector<QED2KSearchResultEntry> vecFiles;
    void save(Preferences& pref) const;
};

struct SearchResult
{
    SearchResult(QString request, RESULT_TYPE type, const std::vector<QED2KSearchResultEntry>& vRes) : 
        strRequest(request), resultType(type), vecResults(vRes), vecUserDirs(), netPoint() {}
    SearchResult(QString request, RESULT_TYPE type, const std::vector<QED2KSearchResultEntry>& vRes, const std::vector<UserDir> userDirs, const libed2k::net_identifier& np) : 
        strRequest(request), resultType(type), vecResults(vRes), vecUserDirs(userDirs), netPoint(np) {}
    SearchResult(Preferences& pref);

    QString strRequest;
    RESULT_TYPE resultType;
    std::vector<QED2KSearchResultEntry> vecResults;
    std::vector<UserDir> vecUserDirs;
    libed2k::net_identifier netPoint;
    void save(Preferences& pref) const;
};

class SWTabBar : public QTabBar
{
public:
    SWTabBar(QWidget* parent);
protected:
    void mousePressEvent(QMouseEvent* event);
};

class SWSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    SWSortFilterProxyModel(QObject* parent = 0);
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
    void showOwn(bool f);
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
    bool m_showOwn;
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
    SWTabBar* tabSearch;

    std::vector<SearchResult> searchItems;
    std::vector<libed2k::net_identifier> connectedPeers;

    int nCurTabSearch;
    int nSortedColumn;
    int nSearchesInProgress;

    QIcon iconSerachActive;
    QIcon iconSearchResult;
    QIcon iconUserFiles;
    QScopedPointer<QStandardItemModel> model;
    QScopedPointer<SWSortFilterProxyModel> filterModel;
    SWDelegate* itemDelegate;
    //search_filter* searchFilter;
    QString        m_lastSearchFileType;

    QMenu* userMenu;
    QAction* userUpdate;
    QAction* userDetails;
    QAction* userAddToFriends;
    QAction* userSendMessage;
    QAction* userBrowseFiles;

    QMenu* fileMenu;
    QAction* fileDownload;
    QAction* fileDownloadPause;
    QAction* filePreview;
    QAction* fileSearchRelated;
    QAction* fileED2KLink;

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

    QWebView* torrentSearchView;

public:
    search_widget(QWidget *parent = 0);
    void load();
    void save() const;
    ~search_widget();

private:
    void addCondRow();
    void clearSearchTable();
    void showErrorParamMsg(int numParam);
    void setUserPicture(const libed2k::net_identifier& np, QIcon& icon);
    bool findSelectedUser(QED2KSearchResultEntry& entry);
    void fillFileValues(int row, const QED2KSearchResultEntry& fileEntry, const QModelIndex& parent = QModelIndex());
    bool hasSelectedMedia();
    bool hasSelectedFiles();
    void updateFileActions();

    void processSearchResult(
        const std::vector<QED2KSearchResultEntry>& vRes, boost::optional<bool> obMoreResult);
    Transfer addTransfer(const QModelIndex& index);

    void warnDisconnected();
    void prepareNewSearch(
        const QString& reqType, const QString& reqText, RESULT_TYPE resultType, const QIcon& icon);

private slots:
    void itemCondClicked(QTableWidgetItem* item);
    void sortChanged(int logicalIndex, Qt::SortOrder order);
    void startSearch();
    void continueSearch();
    void cancelSearch();
    void clearSearch();
    void showOwn(int state);
    void searchRelatedFiles();
    void closeTab(int index);
    void selectTab(int nTabNum);
    void closeAllTabs();
    void setSizeType();
    void searchTextChanged(const QString text);
    void applyFilter(QString filter);
    void setFilterType(SWDelegate::Column column);
    void displayListMenu(const QPoint&);
    void initPeer();
    void sendMessage();
    void addToFriends();
    void getUserDetails();
    void peerConnected(const libed2k::net_identifier& np, const QString&, bool bActive);
    void peerDisconnected(const libed2k::net_identifier& np, const QString&, const libed2k::error_code ec);
    void resultSelectionChanged(const QItemSelection& sel, const QItemSelection& unsel);
    Transfer download();
    void downloadPause();
    void preview();
    void createED2KLink();
    void requestUserDirs();
    void processUserDirs(const libed2k::net_identifier& np, const QString& hash, const QStringList& strList);
    void processUserFiles(const libed2k::net_identifier& np, const QString& hash,
                          const QString& strDirectory, const std::vector<QED2KSearchResultEntry>& vRes);
    void processIsModSharedFiles(const libed2k::net_identifier& np, const QString& hash, const QString& dir_hash,
                                 const std::vector<QED2KSearchResultEntry>& vRes);
    void itemCollapsed(const QModelIndex& index);
    void itemExpanded(const QModelIndex& index);
    void displayHSMenu(const QPoint&);

    void ed2kSearchFinished(const libed2k::net_identifier& np,const QString& hash,
                            const std::vector<QED2KSearchResultEntry>& vRes, bool bMoreResult);
    void torrentSearchFinished(bool ok);
    void addedTransfer(Transfer t);
    void deletedTransfer(const QString& hash);

signals:
    void sendMessage(const QString& user_name, const libed2k::net_identifier& np);
    void addFriend(const QString& user_name, const libed2k::net_identifier& np);
};

class SWItemModel : public QStandardItemModel
{
public:
    SWItemModel(int rows, int columns, QObject * parent = 0):
        QStandardItemModel(rows, columns, parent){}
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
};

#endif // SEARCH_WIDGET_H
