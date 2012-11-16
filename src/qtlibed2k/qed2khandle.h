#ifndef __QED2kHANDLE__
#define __QED2kHANDLE__

#include <transport/transfer_base.h>
#include <libed2k/transfer_handle.hpp>
#include <QString>

class QED2KHandle: public TransferBase
{
public:
    QED2KHandle();
    explicit QED2KHandle(const libed2k::transfer_handle& h);
    const libed2k::transfer_handle& delegate() const { return m_delegate; }

    bool operator==(const TransferBase& t) const;
    bool operator<(const TransferBase& t) const;

    QString hash() const;
    QString name() const;

    QString save_path() const;
    QString firstFileSavePath() const;
    QString creation_date() const;
    QString comment() const;
    QString next_announce() const;
    TransferStatus status() const;
    TransferInfo get_info() const;
    int queue_position() const;
    float distributed_copies() const;
    int num_files() const;
    int upload_limit() const;
    int download_limit() const;
    QString current_tracker() const;
    bool is_valid() const;
    bool is_seed() const;
    bool is_paused() const;
    bool is_queued() const;
    bool has_metadata() const;
    bool priv() const;
    bool super_seeding() const;
    bool is_sequential_download() const;
    void downloading_pieces(TransferBitfield& bf) const;
    void piece_availability(std::vector<int>& avail) const;
    std::vector<int> piece_priorities() const;
    TransferSize piece_length() const;
    bool extremity_pieces_first() const;
    void file_progress(std::vector<TransferSize>& fp) const;
    std::vector<int> file_priorities() const;
    QString filepath_at(unsigned int index) const;
    QString filename_at(unsigned int index) const;
    TransferSize filesize_at(unsigned int index) const;
    std::vector<int> file_extremity_pieces_at(unsigned int index) const;
    QStringList url_seeds() const;
    QStringList absolute_files_path() const;
    void get_peer_info(std::vector<PeerInfo>& peers) const;
    std::vector<AnnounceEntry> trackers() const;

    void pause() const;
    void resume() const;
    void move_storage(const QString& path) const;
    void rename_file(int index, const QString& new_name) const;
    void prioritize_files(const std::vector<int>& priorities) const;
    void prioritize_extremity_pieces(bool p) const;
    void prioritize_extremity_pieces(bool p, unsigned int index) const;
    void set_tracker_login(const QString& login, const QString& passwd) const;
    void flush_cache() const;
    void force_recheck() const;
    void force_reannounce() const;
    void add_url_seed(const QString& url) const;
    void remove_url_seed(const QString& url) const;
    void connect_peer(const PeerEndpoint& ep) const;
    void set_peer_upload_limit(const PeerEndpoint& ep, long limit) const;
    void set_peer_download_limit(const PeerEndpoint& ep, long limit) const;
    void add_tracker(const AnnounceEntry& url) const;
    void replace_trackers(const std::vector<AnnounceEntry>& trackers) const;
    void queue_position_up() const;
    void queue_position_down() const;
    void queue_position_top() const;
    void queue_position_bottom() const;
    void super_seeding(bool ss) const;
    void set_sequential_download(bool sd) const;
    void save_resume_data() const;
    void set_upload_mode(bool b) const;

private:
    libed2k::transfer_handle m_delegate;
};


#endif //__QED2kHANDLE__
