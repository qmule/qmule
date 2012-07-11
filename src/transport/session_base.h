
#ifndef __SESSION_BASE_H__
#define __SESSION_BASE_H__

#include <transport/transfer.h>
#include <qtlibtorrent/trackerinfos.h>

class SessionBase
{
public:
    static const qreal MAX_RATIO;

    virtual ~SessionBase() {};
    virtual Transfer getTransfer(const QString& hash) const = 0;
    virtual std::vector<Transfer> getTransfers() const = 0;
    virtual qreal getMaxRatioPerTransfer(const QString& hash, bool* use_global) const = 0;
    virtual bool isFilePreviewPossible(const QString& hash) const = 0;
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
    // implemented methods
    qreal getRealRatio(const QString& hash) const;
    virtual void readAlerts() = 0;
    virtual void saveTempFastResumeData() = 0;
};

#endif
