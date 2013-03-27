
#include "transfer_base.h"

qreal TransferBase::download_payload_rate() const { return status().download_payload_rate; }
qreal TransferBase::upload_payload_rate() const { return status().upload_payload_rate; }

float TransferBase::progress() const
{
    // libtorrent 0.16: torrent_handle::status(query_accurate_download_counters)
    TransferStatus st = status();
    if (!st.total_wanted)
        return 0.;
    if (st.total_wanted_done == st.total_wanted)
        return 1.;
    float progress = (float) st.total_wanted_done / (float) st.total_wanted;
    return std::min<float>(progress, 1.);
}

int TransferBase::num_seeds() const { return status().num_seeds; }
int TransferBase::num_peers() const { return status().num_peers; }
int TransferBase::num_complete() const { return status().num_complete; }
int TransferBase::num_incomplete() const { return status().num_incomplete; }
int TransferBase::num_connections() const { return status().num_connections; }
int TransferBase::connections_limit() const { return status().connections_limit; }
TransferSize TransferBase::actual_size() const {
    // libtorrent 0.16: torrent_handle::status(query_accurate_download_counters).total_wanted
    return status().total_wanted;
}
TransferSize TransferBase::total_done() const { return status().total_done; }
TransferSize TransferBase::total_wanted_done() const {
    // libtorrent 0.16: torrent_handle::status(query_accurate_download_counters).total_wanted_done
    return status().total_wanted_done;
}
TransferSize TransferBase::total_wanted() const { return status().total_wanted; }
TransferSize TransferBase::total_failed_bytes() const { return status().total_failed_bytes; }
TransferSize TransferBase::total_redundant_bytes() const { return status().total_redundant_bytes; }
TransferSize TransferBase::total_payload_upload() const { return status().total_payload_upload; }
TransferSize TransferBase::total_payload_download() const { return status().total_payload_download; }
TransferSize TransferBase::all_time_upload() const { return status().all_time_upload; }
TransferSize TransferBase::all_time_download() const { return status().all_time_download; }
qlonglong TransferBase::active_time() const { return status().active_time; }
qlonglong TransferBase::seeding_time() const { return status().seeding_time; }
TransferBitfield TransferBase::pieces() const { return status().pieces; }
bool TransferBase::is_checking() const {
    TransferState st = state();
    return st == qt_checking_files || st == qt_checking_resume_data;
}
