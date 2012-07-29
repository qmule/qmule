
#ifndef __SESSION_BASE_H__
#define __SESSION_BASE_H__

#include <QHash>
#include <QDebug>

#include <vector>
#include <queue>
#include <libtorrent/session_status.hpp>

#include <transport/transfer.h>
#include <qtlibtorrent/trackerinfos.h>

typedef libtorrent::session_status SessionStatus;

class SessionBase
{
public:
    static const qreal MAX_RATIO;

    virtual void start() = 0;
    virtual void stop()  = 0;

    virtual bool started() const = 0;
    virtual ~SessionBase() {};

    virtual Transfer getTransfer(const QString& hash) const = 0;
    virtual std::vector<Transfer> getTransfers() const = 0;
    virtual qreal getMaxRatioPerTransfer(const QString& hash, bool* use_global) const = 0;
    virtual bool isFilePreviewPossible(const QString& hash) const = 0;
    virtual SessionStatus getSessionStatus() const = 0;
    virtual void changeLabelInSavePath(
        const Transfer& t, const QString& old_label, const QString& new_label) = 0;
    virtual void pauseTransfer(const QString& hash) = 0;
    virtual void resumeTransfer(const QString& hash) = 0;
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
    virtual bool hasActiveTransfers() const = 0;
    virtual void startUpTransfers() = 0;
    virtual void configureSession() = 0;
    virtual void enableIPFilter(const QString &filter_path, bool force=false) = 0;
    virtual void readAlerts() = 0;
    virtual void saveTempFastResumeData() = 0;
    virtual void saveFastResumeData() = 0;
    virtual Transfer addLink(QString strLink, bool resumed = false) = 0;
    virtual void addTransferFromFile(const QString& filename) = 0;

    // implemented methods
    virtual qreal getRealRatio(const QString& hash) const;
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

#define FORWARD_RETURN2(call, arg1, arg2, def)  \
    if (!S::started()) return def;              \
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
    bool isFilePreviewPossible(const QString& hash) const {
        FORWARD_RETURN(isFilePreviewPossible(hash), false); }
    SessionStatus getSessionStatus() const {
        FORWARD_RETURN(getSessionStatus(), SessionStatus()); }
    void changeLabelInSavePath(
        const Transfer& t, const QString& old_label, const QString& new_label) {
        DEFER3(changeLabelInSavePath, t, old_label, new_label); }
    void pauseTransfer(const QString& hash) { DEFER1(pauseTransfer, hash); }
    void resumeTransfer(const QString& hash) { DEFER1(resumeTransfer, hash); }
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

    Transfer addLink(QString strLink, bool resumed = false) { FORWARD_RETURN2(addLink, strLink, resumed, Transfer()); }
    void addTransferFromFile(const QString& filename) { DEFER1(addTransferFromFile, filename); }

    qreal getRealRatio(const QString& hash) const { FORWARD_RETURN(getRealRatio(hash), 0); }

private:
    std::queue<boost::function<void()> > m_deferred;
};

#endif
