
#ifndef __TRANSFER_BASE_H__
#define __TRANSFER_BASE_H__

#include <QString>

#include "misc.h"
#include <libtorrent/torrent_handle.hpp>
#include <libed2k/transfer_handle.hpp>

enum TransferState
{
    qt_queued_for_checking,
    qt_checking_files,
    qt_downloading_metadata,
    qt_downloading,
    qt_finished,
    qt_seeding,
    qt_allocating,
    qt_checking_resume_data,
    qt_unhandled_state
};

template<typename StateType>
TransferState libstate2tstate(const StateType& st)
{
    TransferState ts;

    switch (st.state)
    {
        case StateType::queued_for_checking:
            ts = qt_queued_for_checking;
            break;
        case StateType::checking_files:
            ts = qt_checking_files;
            break;
        case StateType::downloading_metadata:
            ts = qt_downloading_metadata;
            break;
        case StateType::downloading:
            ts = qt_downloading;
            break;
        case StateType::finished:
            ts = qt_seeding;
            break;
        case StateType::seeding:
            ts = qt_seeding;
            break;
        case StateType::allocating:
            ts = qt_allocating;
            break;
        case StateType::checking_resume_data:
            ts = qt_checking_resume_data;
            break;
        default:
            ts = qt_unhandled_state;
    }

    return (ts);
}

typedef libtorrent::size_type TransferSize;

typedef libtorrent::bitfield TransferBitfield;

template<class BField>
TransferBitfield bitfield2TBF(const BField& bf)
{
    return (TransferBitfield(bf.bytes(), bf.size()));
}

struct TransferStatus
{
    TransferState state;
    bool paused;
    float progress;
    int progress_ppm;
    QString error;

    //boost::posix_time::time_duration next_announce;
    //boost::posix_time::time_duration announce_interval;

    QString current_tracker;

    TransferSize total_download;
    TransferSize total_upload;
    TransferSize total_payload_download;
    TransferSize total_payload_upload;
    TransferSize total_failed_bytes;
    TransferSize total_redundant_bytes;

    int download_rate;
    int upload_rate;

    int download_payload_rate;
    int upload_payload_rate;

    int num_seeds;
    int num_peers;
    int num_complete;
    int num_incomplete;
    int list_seeds;
    int list_peers;
    int connect_candidates;

    TransferBitfield pieces;
    int num_pieces;
    TransferSize total_done;
    TransferSize total_wanted_done;
    TransferSize total_wanted;
    int distributed_full_copies;
    int distributed_fraction;

    float distributed_copies;
    int block_size;

    int num_uploads;
    int num_connections;
    int uploads_limit;
    int connections_limit;
    //storage_mode_t storage_mode;

    int up_bandwidth_queue;
    int down_bandwidth_queue;
    TransferSize all_time_upload;
    TransferSize all_time_download;
    int active_time;
    int finished_time;
    int seeding_time;

    int seed_rank;

    int last_scrape;

    bool has_incoming;

    int sparse_regions;

    bool seed_mode;
    bool upload_mode;
    int priority;
};

template<typename TS>
TransferStatus transfer_status2TS(const TS& t)
{
    TransferStatus ts;
    ts.state                   = libstate2tstate(t);
    ts.paused                  = t.paused;
    ts.progress                = t.progress;
    ts.progress_ppm            = t.progress_ppm;
    ts.error                   = misc::toQStringU(t.error);
    //boost::posix_time::time_duration next_announce;
    //boost::posix_time::time_duration announce_interval;

    ts.current_tracker         = misc::toQStringU(t.current_tracker);
    ts.total_download          = t.total_download;
    ts.total_upload            = t.total_upload;
    ts.total_payload_download  = t.total_payload_download;
    ts.total_payload_upload    = t.total_payload_upload;
    ts.total_failed_bytes      = t.total_failed_bytes;
    ts.total_redundant_bytes   = t.total_redundant_bytes;
    ts.download_rate           = t.download_rate;
    ts.upload_rate             = t.upload_rate;
    ts.download_payload_rate   = t.download_payload_rate;
    ts.upload_payload_rate     = t.upload_payload_rate;
    ts.num_seeds               = t.num_seeds;
    ts.num_peers               = t.num_peers;
    ts.num_complete            = t.num_complete;
    ts.num_incomplete          = t.num_incomplete;
    ts.list_seeds              = t.list_seeds;
    ts.list_peers              = t.list_peers;
    ts.connect_candidates      = t.connect_candidates;
    ts.pieces                  = bitfield2TBF(t.pieces);
    ts.num_pieces              = t.num_pieces;
    ts.total_done              = t.total_done;
    ts.total_wanted_done       = t.total_wanted_done;
    ts.total_wanted            = t.total_wanted;
    ts.distributed_full_copies = t.distributed_full_copies;
    ts.distributed_fraction    = t.distributed_fraction;
    ts.distributed_copies      = t.distributed_copies;
    ts.block_size              = t.block_size;
    ts.num_uploads             = t.num_uploads;
    ts.num_connections         = t.num_connections;
    ts.uploads_limit           = t.uploads_limit;
    ts.connections_limit       = t.connections_limit;
    //storage_mode_t storage_mode;
    ts.up_bandwidth_queue      = t.up_bandwidth_queue;
    ts.down_bandwidth_queue    = t.down_bandwidth_queue;
    ts.all_time_upload         = t.all_time_upload;
    ts.all_time_download       = t.all_time_download;
    ts.active_time             = t.active_time;
    ts.finished_time           = t.finished_time;
    ts.seeding_time            = t.seeding_time;
    ts.seed_rank               = t.seed_rank;
    ts.last_scrape             = t.last_scrape;
    ts.has_incoming            = t.has_incoming;
    ts.sparse_regions          = t.sparse_regions;
    ts.seed_mode               = t.seed_mode;
    ts.upload_mode             = t.upload_mode;
    ts.priority                = t.priority;
    return (ts);
}

struct PeerInfo
{
    int connection_type;
    QString client;
    float   progress;
    int payload_down_speed;
    int payload_up_speed;
    TransferSize total_download;
    TransferSize total_upload;
    libed2k::tcp::endpoint ip;
    char country[2];
};

template<typename PF>
PeerInfo peer_info2PInfo(const PF& p)
{
    PeerInfo pf;
    pf.connection_type      = p.connection_type;
    pf.client               = misc::toQStringU(p.client);
    pf.progress             = p.progress;
    pf.payload_down_speed   = p.payload_down_speed;
    pf.payload_up_speed     = p.payload_up_speed;
    pf.total_download       = p.total_download;
    pf.total_upload         = p.total_upload;
    pf.ip                   = p.ip;
    pf.country[0]           = p.country[0];
    pf.country[1]           = p.country[1];
    return pf;
}

typedef libtorrent::torrent_info TransferInfo;
typedef libtorrent::announce_entry AnnounceEntry;
typedef boost::asio::ip::tcp::endpoint PeerEndpoint;

/**
 * Abstract data transfer.
 */
class TransferBase
{
public:
    virtual ~TransferBase(){}
    virtual bool operator==(const TransferBase& t) const = 0;
    virtual bool operator<(const TransferBase& t) const = 0;

    virtual QString hash() const = 0;
    virtual QString name() const = 0;
    virtual QString save_path() const = 0;
    virtual QString firstFileSavePath() const = 0;
    virtual QString creation_date() const = 0;
    virtual QString comment() const = 0;
    virtual QString next_announce() const = 0;
    virtual TransferState state() const = 0;
    virtual TransferStatus status() const = 0;
    virtual TransferInfo get_info() const = 0;
    virtual qreal download_payload_rate() const = 0;
    virtual qreal upload_payload_rate() const = 0;
    virtual int queue_position() const = 0;
    virtual float progress() const = 0;
    virtual float distributed_copies() const = 0;
    virtual int num_files() const = 0;
    virtual int num_seeds() const = 0;
    virtual int num_peers() const = 0;
    virtual int num_complete() const = 0;
    virtual int num_incomplete() const = 0;
    virtual int num_connections() const = 0;
    virtual int upload_limit() const = 0;
    virtual int download_limit() const = 0;
    virtual int connections_limit() const = 0;
    virtual QString current_tracker() const = 0;
    virtual TransferSize actual_size() const = 0;
    virtual TransferSize total_done() const = 0;
    virtual TransferSize total_wanted_done() const = 0;
    virtual TransferSize total_wanted() const = 0;
    virtual TransferSize total_failed_bytes() const = 0;
    virtual TransferSize total_redundant_bytes() const = 0;
    virtual TransferSize total_payload_upload() const = 0;
    virtual TransferSize total_payload_download() const = 0;
    virtual TransferSize all_time_upload() const = 0;
    virtual TransferSize all_time_download() const = 0;
    virtual qlonglong active_time() const = 0;
    virtual qlonglong seeding_time() const = 0;
    virtual bool is_valid() const = 0;
    virtual bool is_seed() const = 0;
    virtual bool is_paused() const = 0;
    virtual bool is_queued() const = 0;
    virtual bool is_checking() const = 0;
    virtual bool has_metadata() const = 0;
    virtual bool priv() const = 0;
    virtual bool super_seeding() const = 0;
    virtual bool is_sequential_download() const = 0;
    virtual TransferBitfield pieces() const = 0;
    virtual void downloading_pieces(TransferBitfield& bf) const = 0;
    virtual void piece_availability(std::vector<int>& avail) const = 0;
    virtual TransferSize piece_length() const = 0;
    virtual bool first_last_piece_first() const = 0;
    virtual void file_progress(std::vector<TransferSize>& fp) const = 0;
    virtual std::vector<int> file_priorities() const = 0;
    virtual QString filepath_at(unsigned int index) const = 0;
    virtual QString filename_at(unsigned int index) const = 0;
    virtual TransferSize filesize_at(unsigned int index) const = 0;
    virtual QStringList url_seeds() const = 0;
    virtual QStringList absolute_files_path() const = 0;
    virtual void get_peer_info(std::vector<PeerInfo>& peers) const = 0;
    virtual std::vector<AnnounceEntry> trackers() const = 0;

    virtual void pause() const = 0;
    virtual void resume() const = 0;
    virtual void move_storage(const QString& path) const = 0;
    virtual void rename_file(int index, const QString& new_name) const = 0;
    virtual void prioritize_files(const std::vector<int>& priorities) const = 0;
    virtual void prioritize_first_last_piece(bool p) const = 0;
    virtual void set_tracker_login(const QString& login, const QString& passwd) const = 0;
    virtual void flush_cache() const = 0;
    virtual void force_recheck() const = 0;
    virtual void force_reannounce() const = 0;
    virtual void add_url_seed(const QString& url) const = 0;
    virtual void remove_url_seed(const QString& url) const = 0;
    virtual void connect_peer(const PeerEndpoint& ep) const = 0;
    virtual void set_peer_upload_limit(const PeerEndpoint& ep, long limit) const = 0;
    virtual void set_peer_download_limit(const PeerEndpoint& ep, long limit) const = 0;
    virtual void add_tracker(const AnnounceEntry& url) const = 0;
    virtual void replace_trackers(const std::vector<AnnounceEntry>& trackers) const = 0;
    virtual void queue_position_up() const = 0;
    virtual void queue_position_down() const = 0;
    virtual void queue_position_top() const = 0;
    virtual void queue_position_bottom() const = 0;
    virtual void super_seeding(bool ss) const = 0;
    virtual void set_sequential_download(bool sd) const = 0;
    virtual void set_upload_mode(bool b) const = 0;
};

#endif
