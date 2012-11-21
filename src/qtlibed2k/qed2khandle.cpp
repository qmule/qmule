#include <libed2k/constants.hpp>

#include "qed2khandle.h"
#include "torrentpersistentdata.h"
#include "misc.h"

#define CATCH(expr) \
try \
{\
    expr \
} \
catch(libed2k::libed2k_exception& e) \
{ \
    qDebug() << "what: " << e.message(); \
}

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
QString QED2KHandle::name() const { return misc::toQStringU(m_delegate.name()); }

QString QED2KHandle::save_path() const
{
    return misc::toQStringU(m_delegate.save_path()).replace("\\", "/"); // why replace ?
}

QString QED2KHandle::firstFileSavePath() const { return save_path(); }
QString QED2KHandle::creation_date() const { return QString(); }
QString QED2KHandle::comment() const { return QString(); }
QString QED2KHandle::next_announce() const { return QString(); }
TransferStatus QED2KHandle::status() const { return transfer_status2TS(m_delegate.status()); }

TransferInfo QED2KHandle::get_info() const
{
	QString strSHA1("0000000000000000000000000000000000000000");
	QByteArray raw = strSHA1.toAscii();
	libtorrent::sha1_hash ret;
	libtorrent::from_hex(raw.constData(), 40, (char*)&ret[0]);
	return TransferInfo(ret);
}

int QED2KHandle::queue_position() const { return 0; }
float QED2KHandle::distributed_copies() const { return 0; }
int QED2KHandle::num_files() const { return 1; }
int QED2KHandle::upload_limit() const { return m_delegate.upload_limit(); }
int QED2KHandle::download_limit() const { return m_delegate.download_limit(); }
QString QED2KHandle::current_tracker() const {	return QString(); }
bool QED2KHandle::is_valid() const { return m_delegate.is_valid(); }
bool QED2KHandle::is_seed() const { return m_delegate.is_seed(); }
bool QED2KHandle::is_paused() const { return m_delegate.is_paused(); }
bool QED2KHandle::is_queued() const { return false; }
bool QED2KHandle::has_metadata() const { return true; }
bool QED2KHandle::priv() const {return false;}
bool QED2KHandle::super_seeding() const {return false;}
bool QED2KHandle::is_sequential_download() const { return m_delegate.is_sequential_download(); }
void QED2KHandle::downloading_pieces(TransferBitfield& bf) const {}
void QED2KHandle::piece_availability(std::vector<int>& avail) const { m_delegate.piece_availability(avail); }
std::vector<int> QED2KHandle::piece_priorities() const { return m_delegate.piece_priorities(); }
TransferSize QED2KHandle::piece_length() const { return libed2k::PIECE_SIZE; }
bool QED2KHandle::extremity_pieces_first() const {
    const QString ext = misc::file_extension(filename_at(0));

    if (!misc::isPreviewable(ext)) return false; // No media file

    const std::vector<int> extremities = file_extremity_pieces_at(0);
    const std::vector<int> piece_priorities = m_delegate.piece_priorities();
    foreach (int e, extremities) if (piece_priorities[e] != 7) return false;
    return true;
}
void QED2KHandle::file_progress(std::vector<TransferSize>& fp) const {
    fp.clear();
    float p = progress();
    TransferSize s = filesize_at(0);
    fp.push_back(p == 1. ? s : s * p);
}
std::vector<int> QED2KHandle::file_priorities() const { return std::vector<int>(); }

QString QED2KHandle::filepath_at(unsigned int index) const
{
    return misc::toQStringU(libed2k::combine_path(m_delegate.save_path(), m_delegate.name()));
}

QString QED2KHandle::filename_at(unsigned int index) const
{
    return misc::toQStringU(m_delegate.name());
}

TransferSize QED2KHandle::filesize_at(unsigned int index) const
{
    Q_ASSERT(index == 0);
    return m_delegate.size();
}

std::vector<int> QED2KHandle::file_extremity_pieces_at(unsigned int index) const {
    Q_ASSERT(index == 0);
    int last_piece = m_delegate.num_pieces() - 1;
    int penult_piece = std::max(last_piece - 1, 0);

    std::vector<int> res;
    res.push_back(0);
    res.push_back(penult_piece);
    res.push_back(last_piece);
    return res;
}

QStringList QED2KHandle::url_seeds() const { return QStringList(); }
QStringList QED2KHandle::absolute_files_path() const {
    QStringList res;
    res << filepath_at(0);
    return res;
}
void QED2KHandle::get_peer_info(std::vector<PeerInfo>& infos) const 
{
    std::vector<libed2k::peer_info> ed_infos;
    m_delegate.get_peer_info(ed_infos);
    std::transform(ed_infos.begin(), ed_infos.end(), std::back_inserter(infos), peer_info2PInfo<libed2k::peer_info>);
}
std::vector<AnnounceEntry> QED2KHandle::trackers() const { return std::vector<AnnounceEntry>(); }
void QED2KHandle::pause() const { m_delegate.pause(); }
void QED2KHandle::resume() const { m_delegate.resume(); }
void QED2KHandle::move_storage(const QString& new_path) const {
    if (QDir(save_path()) == QDir(new_path))
        return;

    TorrentPersistentData::setPreviousSavePath(hash(), save_path());
    // Create destination directory if necessary
    // or move_storage() will fail...
    QDir().mkpath(new_path);
    // Actually move the storage
    m_delegate.move_storage(new_path.toUtf8().constData());
}
void QED2KHandle::rename_file(int index, const QString& new_name) const {
    m_delegate.rename_file(new_name.toUtf8().constData());
}
void QED2KHandle::prioritize_files(const std::vector<int>& priorities) const {}
void QED2KHandle::prioritize_extremity_pieces(bool p) const {
    prioritize_extremity_pieces(p, 0);
}
void QED2KHandle::prioritize_extremity_pieces(bool p, unsigned int index) const {
    Q_ASSERT(index == 0);

    int prio = p ? 7 : 1;
    const std::vector<int> extremities = file_extremity_pieces_at(index);
    foreach (int e, extremities)
        m_delegate.set_piece_priority(e, prio);
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
void QED2KHandle::set_upload_mode(bool b) const { m_delegate.set_upload_mode(b); }
