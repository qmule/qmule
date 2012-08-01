#include <QDateTime>
#include "session_base.h"

const qreal SessionBase::MAX_RATIO = 9999.;

qreal SessionBase::getRealRatio(const QString &hash) const
{
    Transfer h = getTransfer(hash);
    if (!h.is_valid()) {
        return 0.;
    }

    libtorrent::size_type all_time_upload = h.all_time_upload();
    libtorrent::size_type all_time_download = h.all_time_download();
    if (all_time_download == 0 && h.is_seed()) {
        // Purely seeded torrent
        all_time_download = h.total_done();
    }

    if (all_time_download == 0) {
        if (all_time_upload == 0)
            return 0;
        return MAX_RATIO+1;
    }

    qreal ratio = all_time_upload / (float) all_time_download;
    Q_ASSERT(ratio >= 0.);
    if (ratio > MAX_RATIO)
        ratio = MAX_RATIO;
    return ratio;
}

void SessionBase::addConsoleMessage(QString msg, QColor color/*=QApplication::palette().color(QPalette::WindowText)*/)
{
    if (consoleMessages.size() > MAX_LOG_MESSAGES)
    {
        consoleMessages.removeFirst();
    }

    msg = "<font color='grey'>"+ QDateTime::currentDateTime().toString(QString::fromUtf8("dd/MM/yyyy hh:mm:ss")) + "</font> - <font color='" + color.name() + "'><i>" + msg + "</i></font>";
    consoleMessages.append(msg);
    emit newConsoleMessage(msg);
}
