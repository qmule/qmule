#include <libed2k/constants.hpp>

#include "qed2khandle.h"
#include "misc.h"

QED2KHandle::QED2KHandle()
{
}

QED2KHandle::QED2KHandle(const libed2k::transfer_handle& h): m_delegate(h)
{
}

bool QED2KHandle::operator==(const TransferBase& t) const
{
    const QED2KHandle* pt = dynamic_cast<const QED2KHandle*>(&t);
    return (pt && pt->m_delegate == m_delegate);
}

bool QED2KHandle::operator<(const TransferBase& t) const
{
    const QED2KHandle* pt = dynamic_cast<const QED2KHandle*>(&t);
    return (pt && pt->m_delegate < m_delegate);
}

QString QED2KHandle::hash() const { return misc::toQString(m_delegate.hash()); }
QString QED2KHandle::name() const { return misc::toQStringU(m_delegate.filepath().filename()); }

QString QED2KHandle::save_path() const
{
	return misc::toQStringU(m_delegate.filepath().parent_path().string()).replace("\\", "/");
}

QString QED2KHandle::firstFileSavePath() const { return QString(); }
QString QED2KHandle::creation_date() const { return QString(); }
QString QED2KHandle::comment() const { return QString(); }
QString QED2KHandle::next_announce() const { return QString(); }
TransferState QED2KHandle::state() const { return m_delegate.status().state; }
TransferStatus QED2KHandle::status() const { return m_delegate.status(); }

TransferInfo QED2KHandle::get_info() const
{
	QString strSHA1("0000000000000000000000000000000000000000");
	QByteArray raw = strSHA1.toAscii();
	libtorrent::sha1_hash ret;
	libtorrent::from_hex(raw.constData(), 40, (char*)&ret[0]);
	return TransferInfo(ret);
}

qreal QED2KHandle::download_payload_rate() const {
    return m_delegate.status().download_payload_rate;
}
qreal QED2KHandle::upload_payload_rate() const {
    return m_delegate.status().upload_payload_rate;
}
int QED2KHandle::queue_position() const { return 0; }
float QED2KHandle::progress() const {
    libed2k::transfer_status st = m_delegate.status();
    if (!st.total_wanted)
        return 0.;
    if (st.total_wanted_done == st.total_wanted)
        return 1.;
    float progress = (float) st.total_wanted_done / (float) st.total_wanted;
    Q_ASSERT(progress >= 0. && progress <= 1.);
    return progress;
}
float QED2KHandle::distributed_copies() const { return 0; }
int QED2KHandle::num_files() const { return 1; }
int QED2KHandle::num_seeds() const { return m_delegate.num_seeds(); }
int QED2KHandle::num_peers() const { return m_delegate.num_peers(); }
int QED2KHandle::num_complete() const { return 0; }
int QED2KHandle::num_incomplete() const {return 0;}
int QED2KHandle::num_connections() const{return 0;}
int QED2KHandle::upload_limit() const {return 0;}
int QED2KHandle::download_limit() const {return 0;}
int QED2KHandle::connections_limit() const {return 0;}
QString QED2KHandle::current_tracker() const {	return QString(); }
TransferSize QED2KHandle::actual_size() const { return m_delegate.status().total_wanted; }
TransferSize QED2KHandle::total_done() const { return m_delegate.status().total_done; }
TransferSize QED2KHandle::total_wanted_done() const { return m_delegate.status().total_wanted_done; }
TransferSize QED2KHandle::total_wanted() const { return m_delegate.status().total_wanted; }
TransferSize QED2KHandle::total_failed_bytes() const {return 0;}
TransferSize QED2KHandle::total_redundant_bytes() const {return 0;}
TransferSize QED2KHandle::total_payload_upload() const {return 0;}
TransferSize QED2KHandle::total_payload_download() const {return 0;}
TransferSize QED2KHandle::all_time_upload() const { return m_delegate.status().all_time_upload; }
TransferSize QED2KHandle::all_time_download() const { return m_delegate.status().all_time_download; }
qlonglong QED2KHandle::active_time() const {return 0;}
qlonglong QED2KHandle::seeding_time() const {return 0;}
bool QED2KHandle::is_valid() const { return m_delegate.is_valid(); }
bool QED2KHandle::is_seed() const { return m_delegate.is_seed(); }
bool QED2KHandle::is_paused() const { return m_delegate.is_paused(); }
bool QED2KHandle::is_queued() const { return false; }
bool QED2KHandle::is_checking() const {return false;}
bool QED2KHandle::has_metadata() const { return true; }
bool QED2KHandle::priv() const {return false;}
bool QED2KHandle::super_seeding() const {return false;}
bool QED2KHandle::is_sequential_download() const { return m_delegate.is_sequential_download(); }
TransferBitfield QED2KHandle::pieces() const { return m_delegate.status().pieces; }
void QED2KHandle::downloading_pieces(TransferBitfield& bf) const {}
void QED2KHandle::piece_availability(std::vector<int>& avail) const { m_delegate.piece_availability(avail); }
TransferSize QED2KHandle::piece_length() const { return libed2k::PIECE_SIZE; }
bool QED2KHandle::first_last_piece_first() const {
    const QString ext = misc::file_extension(filename_at(0));

    if (!misc::isPreviewable(ext)) return false; // No media file

    int last_piece = m_delegate.num_pieces() - 1;
    int penult_piece = std::max(last_piece - 1, 0);
    return m_delegate.piece_priority(0) == 7 &&
        m_delegate.piece_priority(last_piece) == 7 &&
        m_delegate.piece_priority(penult_piece) == 7;
}
void QED2KHandle::file_progress(std::vector<TransferSize>& fp) const {}
std::vector<int> QED2KHandle::file_priorities() const { return std::vector<int>(); }
QString QED2KHandle::filepath_at(unsigned int index) const {
    return misc::toQStringU(m_delegate.filepath().string());
}
QString QED2KHandle::filename_at(unsigned int index) const {
    return misc::toQStringU(m_delegate.filepath().filename());
}
TransferSize QED2KHandle::filesize_at(unsigned int index) const {
    return m_delegate.filesize();
}
QStringList QED2KHandle::url_seeds() const { return QStringList(); }
QStringList QED2KHandle::absolute_files_path() const {
    QStringList res;
    res << filepath_at(0);
    return res;
}
void QED2KHandle::get_peer_info(std::vector<PeerInfo>& infos) const {
    m_delegate.get_peer_info(infos);
}
std::vector<AnnounceEntry> QED2KHandle::trackers() const { return std::vector<AnnounceEntry>(); }
void QED2KHandle::pause() const { m_delegate.pause(); }
void QED2KHandle::resume() const { m_delegate.resume(); }
void QED2KHandle::move_storage(const QString& path) const {}
void QED2KHandle::rename_file(int index, const QString& new_name) const {}
void QED2KHandle::prioritize_files(const std::vector<int>& priorities) const {}
void QED2KHandle::prioritize_first_last_piece(bool p) const
{
    int prio = p ? 7 : 1;

    int last_piece = m_delegate.num_pieces() - 1;
    int penult_piece = std::max(last_piece - 1, 0);
    m_delegate.set_piece_priority(0, prio);
    m_delegate.set_piece_priority(last_piece, prio);
    m_delegate.set_piece_priority(penult_piece, prio);
}
void QED2KHandle::set_tracker_login(const QString& login, const QString& passwd) const {}
void QED2KHandle::flush_cache() const {}
void QED2KHandle::force_recheck() const {}
void QED2KHandle::force_reannounce() const {}
void QED2KHandle::add_url_seed(const QString& url) const {}
void QED2KHandle::remove_url_seed(const QString& url) const {}
void QED2KHandle::connect_peer(const PeerEndpoint& ep) const {}
void QED2KHandle::set_peer_upload_limit(const PeerEndpoint& ep, long limit) const {}
void QED2KHandle::set_peer_download_limit(const PeerEndpoint& ep, long limit) const {}
void QED2KHandle::add_tracker(const AnnounceEntry& url) const {}
void QED2KHandle::replace_trackers(const std::vector<AnnounceEntry>& trackers) const {}
void QED2KHandle::queue_position_up() const {}
void QED2KHandle::queue_position_down() const {}
void QED2KHandle::queue_position_top() const {}
void QED2KHandle::queue_position_bottom() const {}
void QED2KHandle::super_seeding(bool ss) const {}
void QED2KHandle::set_sequential_download(bool sd) const { m_delegate.set_sequential_download(sd); }
void QED2KHandle::save_resume_data() const { m_delegate.save_resume_data(); }
