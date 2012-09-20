
#ifndef __TRANSFER_BASE_H__
#define __TRANSFER_BASE_H__

#include <QString>

#include <libtorrent/torrent_handle.hpp>
#include <libed2k/transfer_handle.hpp>

typedef libtorrent::torrent_status::state_t TransferState;
typedef libtorrent::torrent_status TransferStatus;
typedef libtorrent::size_type TransferSize;
typedef libtorrent::torrent_info TransferInfo;
typedef libtorrent::bitfield TransferBitfield;
typedef libtorrent::peer_info PeerInfo;
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
};

#endif
