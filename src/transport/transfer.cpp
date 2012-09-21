#include <libtorrent/peer_id.hpp>
#include <libtorrent/escape_string.hpp>
#include "transport/transfer.h"

//-- Generic transfer handle

Transfer::Transfer() : m_delegate(new QTorrentHandle()) {}

Transfer::Transfer(const QTorrentHandle& h) : m_delegate(new QTorrentHandle(h)) {}

Transfer::Transfer(const QED2KHandle& h) : m_delegate(new QED2KHandle(h)) {}

bool Transfer::operator==(const Transfer& t) const {
    return *m_delegate == *t.m_delegate; }

bool Transfer::operator<(const Transfer& t) const {
    return *m_delegate < *t.m_delegate; }

QTorrentHandle Transfer::torrentHandle() const {
    const QTorrentHandle* h = dynamic_cast<const QTorrentHandle*>(m_delegate.data());
    return h ? *h : QTorrentHandle();
}

QED2KHandle Transfer::ed2kHandle() const
{
    const QED2KHandle* h = dynamic_cast<const QED2KHandle*>(m_delegate.data());
    return h ? *h : QED2KHandle();
}

Transfer::Type Transfer::type() const
{
    if (dynamic_cast<const QTorrentHandle*>(m_delegate.data())) return BITTORRENT;
    else if (dynamic_cast<const QED2KHandle*>(m_delegate.data())) return ED2K;
    return UNDEFINED;
}

QString Transfer::hash() const { return m_delegate->hash(); }

QString Transfer::name() const { return m_delegate->name(); }

QString Transfer::save_path() const { return m_delegate->save_path(); }

QString Transfer::firstFileSavePath() const {
    return m_delegate->firstFileSavePath(); }

QString Transfer::creation_date() const { return m_delegate->creation_date(); }

QString Transfer::comment() const { return m_delegate->comment(); }

QString Transfer::next_announce() const { return m_delegate->next_announce(); }

TransferState Transfer::state() const { return libstate2tstate(m_delegate->state()); }

TransferStatus Transfer::status() const { return transfer_status2TS(m_delegate->status()); }

TransferInfo Transfer::get_info() const { return m_delegate->get_info(); }

qreal Transfer::download_payload_rate() const {
    return m_delegate->download_payload_rate(); }

qreal Transfer::upload_payload_rate() const {
    return m_delegate->upload_payload_rate(); }

int Transfer::queue_position() const { return m_delegate->queue_position(); }

float Transfer::progress() const { return m_delegate->progress(); }

float Transfer::distributed_copies() const { return m_delegate->distributed_copies(); }

int Transfer::num_files() const { return m_delegate->num_files(); }

int Transfer::num_seeds() const { return m_delegate->num_seeds(); }

int Transfer::num_peers() const { return m_delegate->num_peers(); }

int Transfer::num_complete() const { return m_delegate->num_complete(); }

int Transfer::num_incomplete() const { return m_delegate->num_incomplete(); }

int Transfer::num_connections() const { return m_delegate->num_connections(); }

int Transfer::upload_limit() const { return m_delegate->upload_limit(); }

int Transfer::download_limit() const { return m_delegate->download_limit(); }

int Transfer::connections_limit() const { return m_delegate->connections_limit(); }

QString Transfer::current_tracker() const { return m_delegate->current_tracker(); }

TransferSize Transfer::actual_size() const { return m_delegate->actual_size(); }

TransferSize Transfer::total_done() const { return m_delegate->total_done(); }

TransferSize Transfer::total_wanted_done() const { return m_delegate->total_wanted_done(); }

TransferSize Transfer::total_wanted() const { return m_delegate->total_wanted(); }

TransferSize Transfer::total_failed_bytes() const {
    return m_delegate->total_failed_bytes(); }

TransferSize Transfer::total_redundant_bytes() const {
    return m_delegate->total_redundant_bytes(); }

TransferSize Transfer::total_payload_upload() const {
    return m_delegate->total_payload_upload(); }

TransferSize Transfer::total_payload_download() const {
    return m_delegate->total_payload_download(); }

TransferSize Transfer::all_time_upload() const {
    return m_delegate->all_time_upload(); }

TransferSize Transfer::all_time_download() const {
    return m_delegate->all_time_download(); }

qlonglong Transfer::active_time() const { return m_delegate->active_time(); }

qlonglong Transfer::seeding_time() const { return m_delegate->seeding_time(); }

bool Transfer::is_valid() const { return m_delegate->is_valid(); }

bool Transfer::is_seed() const { return m_delegate->is_seed(); }

bool Transfer::is_paused() const { return m_delegate->is_paused(); }

bool Transfer::is_queued() const { return m_delegate->is_queued(); }

bool Transfer::is_checking() const { return m_delegate->is_checking(); }

bool Transfer::has_metadata() const { return m_delegate->has_metadata(); }

bool Transfer::priv() const { return m_delegate->priv(); }

bool Transfer::super_seeding() const { return m_delegate->super_seeding(); }

bool Transfer::is_sequential_download() const {
    return m_delegate->is_sequential_download(); }

TransferBitfield Transfer::pieces() const { return bitfield2TBF(m_delegate->pieces()); }

void Transfer::downloading_pieces(TransferBitfield& bf) const {
    m_delegate->downloading_pieces(bf); }

void Transfer::piece_availability(std::vector<int>& avail) const {
    m_delegate->piece_availability(avail); }

TransferSize Transfer::piece_length() const {
    return m_delegate->piece_length(); }

bool Transfer::first_last_piece_first() const {
    return m_delegate->first_last_piece_first(); }

void Transfer::file_progress(std::vector<TransferSize>& fp) const {
    m_delegate->file_progress(fp); }

std::vector<int> Transfer::file_priorities() const {
    return m_delegate->file_priorities(); }

QString Transfer::filepath_at(unsigned int index) const {
    return m_delegate->filepath_at(index); }

QString Transfer::filename_at(unsigned int index) const {
    return m_delegate->filename_at(index); }

TransferSize Transfer::filesize_at(unsigned int index) const {
    return m_delegate->filesize_at(index); }

QStringList Transfer::url_seeds() const { return m_delegate->url_seeds(); }

QStringList Transfer::absolute_files_path() const {
    return m_delegate->absolute_files_path(); }

void Transfer::get_peer_info(std::vector<PeerInfo>& peers) const {
    return m_delegate->get_peer_info(peers); }

std::vector<AnnounceEntry> Transfer::trackers() const {
    return m_delegate->trackers(); }

void Transfer::pause() const { m_delegate->pause(); }

void Transfer::resume() const { m_delegate->resume(); }

void Transfer::move_storage(const QString& path) const { m_delegate->move_storage(path); }

void Transfer::rename_file(unsigned int index, const QString& new_name) const {
    m_delegate->rename_file(index, new_name); }

void Transfer::prioritize_files(const std::vector<int> priorities) const {
    m_delegate->prioritize_files(priorities); }

void Transfer::prioritize_first_last_piece(bool p) const {
    m_delegate->prioritize_first_last_piece(p); }

void Transfer::set_tracker_login(const QString& login, const QString& passwd) const {
    return m_delegate->set_tracker_login(login, passwd); }

void Transfer::flush_cache() const { m_delegate->flush_cache(); }

void Transfer::force_recheck() const { m_delegate->flush_cache(); }

void Transfer::force_reannounce() const { m_delegate->force_reannounce(); }

void Transfer::add_url_seed(const QString& url) const { m_delegate->add_url_seed(url); }

void Transfer::remove_url_seed(const QString& url) const { m_delegate->remove_url_seed(url); }

void Transfer::connect_peer(const PeerEndpoint& ep) const { m_delegate->connect_peer(ep); }

void Transfer::set_peer_upload_limit(const PeerEndpoint& ep, long limit) const {
    m_delegate->set_peer_upload_limit(ep, limit); }

void Transfer::set_peer_download_limit(const PeerEndpoint& ep, long limit) const {
    m_delegate->set_peer_download_limit(ep, limit); }

void Transfer::add_tracker(const AnnounceEntry& url) const { m_delegate->add_tracker(url); }

void Transfer::replace_trackers(const std::vector<AnnounceEntry>& trackers) const {
    m_delegate->replace_trackers(trackers); }

void Transfer::queue_position_up() const { m_delegate->queue_position_up(); }

void Transfer::queue_position_down() const { m_delegate->queue_position_down(); }

void Transfer::queue_position_top() const { m_delegate->queue_position_top(); }

void Transfer::queue_position_bottom() const { m_delegate->queue_position_bottom(); }

void Transfer::super_seeding(bool ss) const { m_delegate->super_seeding(ss); }

void Transfer::set_sequential_download(bool sd) const { m_delegate->set_sequential_download(sd); }
