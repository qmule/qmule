
#ifndef __SESSION_BASE_H__
#define __SESSION_BASE_H__

#include <QHash>
#include <QDebug>
#include <QPalette>
#include <QApplication>

#include <vector>
#include <queue>
#include <boost/bind.hpp>
#include <libtorrent/session_status.hpp>
#include <libed2k/add_transfer_params.hpp>
#include <libed2k/session_status.hpp>

#include "transport/transfer.h"
#include "qtlibtorrent/trackerinfos.h"

struct ErrorCode
{
    typedef boost::system::error_code LibCode;
    typedef std::string UICode;

    ErrorCode& operator=(const LibCode& ec) {
        m_libCode = ec;
        m_uiCode = "";
        return *this;
    }
    ErrorCode& operator=(const UICode& ec) {
        m_uiCode = ec;
        m_libCode = LibCode();
        return *this;
    }

    operator bool() const { return m_libCode || !m_uiCode.empty(); }
    operator LibCode&() { return m_libCode; }

    std::string message() const { return m_libCode ? m_libCode.message() : m_uiCode; }

    LibCode m_libCode;
    std::string m_uiCode;
};

struct SessionStatus
{
    int payload_upload_rate;
    int payload_download_rate;

    SessionStatus() : payload_upload_rate(0), payload_download_rate(0) {}
    SessionStatus(const libtorrent::session_status& s) :
        payload_upload_rate(s.payload_upload_rate), payload_download_rate(s.payload_download_rate) {}
    SessionStatus(const libed2k::session_status& s) :
        payload_upload_rate(s.payload_upload_rate), payload_download_rate(s.payload_download_rate) {}
};

const int MAX_LOG_MESSAGES = 100;

class SessionBase : public QObject
{
    Q_OBJECT
public:
    static const qreal MAX_RATIO;

    virtual void start() = 0;
    virtual void stop()  = 0;

    virtual bool started() const = 0;
    virtual ~SessionBase() {};

    virtual Transfer getTransfer(const QString& hash) const = 0;
    virtual std::vector<Transfer> getTransfers() const = 0;
    virtual qreal getMaxRatioPerTransfer(const QString& hash, bool* use_global) const = 0;
    virtual SessionStatus getSessionStatus() const = 0;
    virtual void changeLabelInSavePath(
        const Transfer& t, const QString& old_label, const QString& new_label) = 0;
    virtual void deleteTransfer(const QString& hash, bool delete_files) = 0;
    virtual void recheckTransfer(const QString& hash) = 0;
    virtual void setDownloadLimit(const QString& hash, long limit) = 0;
    virtual void setUploadLimit(const QString& hash, long limit) = 0;
    virtual void setMaxRatioPerTransfer(const QString& hash, qreal ratio) = 0;
    virtual void removeRatioPerTransfer(const QString& hash) = 0;
    virtual void banIP(QString ip) = 0;
    virtual QHash<QString, TrackerInfos> getTrackersInfo(const QString &hash) const = 0;
    virtual void setDownloadRateLimit(long rate) = 0;
    virtual void setUploadRateLimit(long rate) = 0;
    virtual void startUpTransfers() = 0;
    virtual void configureSession() = 0;
    virtual void enableIPFilter(const QString &filter_path, bool force=false) = 0;
    virtual void readAlerts() = 0;
    virtual void saveTempFastResumeData() = 0;
    virtual void saveFastResumeData() = 0;
    virtual QPair<Transfer,ErrorCode> addLink(QString strLink, bool resumed = false) = 0;
    virtual void addTransferFromFile(const QString& filename) = 0;
    virtual QED2KHandle addTransfer(const libed2k::add_transfer_params&) = 0;   //!< ed2k session only

    // implemented methods
    virtual qreal getRealRatio(const QString& hash) const;
    virtual bool hasActiveTransfers() const;
    inline virtual QStringList getConsoleMessages() const { return consoleMessages; }
    virtual void addConsoleMessage(
        QString msg, QColor color=QApplication::palette().color(QPalette::WindowText));
    virtual bool isFilePreviewPossible(const QString& hash) const;
    virtual void autoRunExternalProgram(const Transfer &t);
    virtual QList<QDir> incompleteFiles() const;

public slots:
    virtual void pauseTransfer(const QString& hash);
    virtual void resumeTransfer(const QString& hash);
    virtual void pauseAllTransfers();
    virtual void resumeAllTransfers();
    virtual void cleanUpAutoRunProcess(int);

    /**
      * return minimum progress in transfers
     */
    virtual float progress() const;

signals:
    void addedTransfer(Transfer t);
    void pausedTransfer(Transfer t);
    void resumedTransfer(Transfer t);
    void finishedTransfer(Transfer t);
    void deletedTransfer(QString hash);
    void transferAboutToBeRemoved(Transfer t, bool del_files);
    void savePathChanged(Transfer t);
    void newConsoleMessage(const QString &msg);
    void fileError(Transfer t, QString msg);

private:
    QStringList consoleMessages;
};

#define DEFER0(call)                                            \
    if (!S::started())                                          \
        m_deferred.push(boost::bind(&S::call, this));           \
    else S::call()

#define DEFER1(call, arg1)                                              \
    if (!S::started())                                                  \
        m_deferred.push(boost::bind(&S::call, this, arg1));             \
    else S::call(arg1)

#define DEFER2(call, arg1, arg2)                                        \
    if (!S::started())                                                  \
        m_deferred.push(boost::bind(&S::call, this, arg1, arg2));       \
    else S::call(arg1, arg2)

#define DEFER3(call, arg1, arg2, arg3)                                  \
    if (!S::started())                                                  \
        m_deferred.push(boost::bind(&S::call, this, arg1, arg2, arg3)); \
    else S::call(arg1, arg2, arg3)

#define FORWARD_RETURN(call, def)               \
    if (!S::started()) return def;              \
    else return S::call

#define FORWARD_RETURN1(call, arg1, def)        \
    if (!S::started()) return def;              \
    else return S::call(arg1)

#define FORWARD_RETURN2(call, arg1, arg2, def)  \
    if (!S::started()) return def;              \
    else return S::call(arg1, arg2)

#define DEFER_RETURN1(call, arg1, def)                                  \
    if (!S::started()) {                                                \
        m_deferred.push(boost::bind(&S::call, this, arg1));             \
        return def;                                                     \
    }                                                                   \
    else return S::call(arg1)

#define DEFER_RETURN2(call, arg1, arg2, def)                            \
    if (!S::started()) {                                                \
        m_deferred.push(boost::bind(&S::call, this, arg1, arg2));       \
        return def;                                                     \
    }                                                                   \
    else return S::call(arg1, arg2)

template <typename S>
class DeferredSessionProxy : public S
{
public:
    void start()
    {
        S::start();
        while(!m_deferred.empty()) {
            m_deferred.front()();
            m_deferred.pop();
        }
    }

    Transfer getTransfer(const QString& hash) const {
        FORWARD_RETURN(getTransfer(hash), Transfer()); }
    std::vector<Transfer> getTransfers() const {
        FORWARD_RETURN(getTransfers(), std::vector<Transfer>()); }
    qreal getMaxRatioPerTransfer(const QString& hash, bool* use_global) const {
        FORWARD_RETURN(getMaxRatioPerTransfer(hash, use_global), 0); }
    SessionStatus getSessionStatus() const { FORWARD_RETURN(getSessionStatus(), SessionStatus()); }
    void changeLabelInSavePath(
        const Transfer& t, const QString& old_label, const QString& new_label) {
        DEFER3(changeLabelInSavePath, t, old_label, new_label); }
    void deleteTransfer(const QString& hash, bool delete_files) {
        DEFER2(deleteTransfer, hash, delete_files); }
    void recheckTransfer(const QString& hash) { DEFER1(recheckTransfer, hash); }
    void setDownloadLimit(const QString& hash, long limit) { DEFER2(setDownloadLimit, hash, limit); }
    void setUploadLimit(const QString& hash, long limit) { DEFER2(setUploadLimit, hash, limit); }
    void setMaxRatioPerTransfer(const QString& hash, qreal ratio) {
        DEFER2(setMaxRatioPerTransfer, hash, ratio); }
    void removeRatioPerTransfer(const QString& hash) { DEFER1(removeRatioPerTransfer, hash); }
    void banIP(QString ip) { DEFER1(banIP, ip); }
    QHash<QString, TrackerInfos> getTrackersInfo(const QString &hash) const {
        FORWARD_RETURN(getTrackersInfo(hash), (QHash<QString, TrackerInfos>())); }
    void setDownloadRateLimit(long rate) { DEFER1(setDownloadRateLimit, rate); }
    void setUploadRateLimit(long rate) { DEFER1(setUploadRateLimit, rate); }
    bool hasActiveTransfers() const { FORWARD_RETURN(hasActiveTransfers(), false); }
    void startUpTransfers() { DEFER0(startUpTransfers); }
    void configureSession() { DEFER0(configureSession); }
    void enableIPFilter(const QString &filter_path, bool force=false) {
        DEFER2(enableIPFilter, filter_path, force); }
    void readAlerts() { DEFER0(readAlerts); }
    void saveTempFastResumeData() { DEFER0(saveTempFastResumeData); }
    void saveFastResumeData() { DEFER0(saveFastResumeData); }
    QPair<Transfer,ErrorCode> addLink(QString strLink, bool resumed = false) {
        DEFER_RETURN2(addLink, strLink, resumed, (QPair<Transfer,ErrorCode>())); }
    void addTransferFromFile(const QString& filename) { DEFER1(addTransferFromFile, filename); }
    QED2KHandle addTransfer(const libed2k::add_transfer_params& atp) {
        DEFER_RETURN1(addTransfer, atp, QED2KHandle()); }
    qreal getRealRatio(const QString& hash) const { FORWARD_RETURN(getRealRatio(hash), 0); }
private:
    std::queue<boost::function<void()> > m_deferred;
};

#endif
