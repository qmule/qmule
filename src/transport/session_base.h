
#ifndef __SESSION_BASE_H__
#define __SESSION_BASE_H__

#include <QHash>

#include <vector>
#include <libtorrent/session_status.hpp>

#include <transport/transfer.h>
#include <qtlibtorrent/trackerinfos.h>

typedef libtorrent::session_status SessionStatus;

class SessionBase
{
public:
    static const qreal MAX_RATIO;

    virtual void start() = 0;
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

    // implemented methods
    virtual qreal getRealRatio(const QString& hash) const;
};

#define FORWARD(call)                           \
    if (!S::started()) return;                  \
    else S::call

#define FORWARD_RETURN(call, def)               \
    if (!S::started()) return def;              \
    else return S::call

template <typename S>
class NullSessionProxy : public S
{
public:
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
        FORWARD(changeLabelInSavePath(t, old_label, new_label)); }
    void pauseTransfer(const QString& hash) { FORWARD(pauseTransfer(hash)); }
    void resumeTransfer(const QString& hash) { FORWARD(resumeTransfer(hash)); }
    void deleteTransfer(const QString& hash, bool delete_files) {
        FORWARD(deleteTransfer(hash, delete_files)); }
    void recheckTransfer(const QString& hash) { FORWARD(recheckTransfer(hash)); }
    void setDownloadLimit(const QString& hash, long limit) { FORWARD(setDownloadLimit(hash, limit)); }
    void setUploadLimit(const QString& hash, long limit) { FORWARD(setUploadLimit(hash, limit)); }
    void setMaxRatioPerTransfer(const QString& hash, qreal ratio) {
        FORWARD(setMaxRatioPerTransfer(hash, ratio)); }
    void removeRatioPerTransfer(const QString& hash) { FORWARD(removeRatioPerTransfer(hash)); }
    void banIP(QString ip) { FORWARD(banIP(ip)); }
    QHash<QString, TrackerInfos> getTrackersInfo(const QString &hash) const {
        FORWARD_RETURN(getTrackersInfo(hash), (QHash<QString, TrackerInfos>())); }
    void setDownloadRateLimit(long rate) { FORWARD(setDownloadRateLimit(rate)); }
    void setUploadRateLimit(long rate) { FORWARD(setUploadRateLimit(rate)); }
    bool hasActiveTransfers() const { FORWARD_RETURN(hasActiveTransfers(), false); }
    void startUpTransfers() { FORWARD(startUpTransfers()); }
    void configureSession() { FORWARD(configureSession()); }
    void enableIPFilter(const QString &filter_path, bool force=false) {
        FORWARD(enableIPFilter(filter_path, force)); }
    void readAlerts() { FORWARD(readAlerts()); }
    void saveTempFastResumeData() { FORWARD(saveTempFastResumeData()); }

    qreal getRealRatio(const QString& hash) const { FORWARD_RETURN(getRealRatio(hash), 0); }
};

#endif
