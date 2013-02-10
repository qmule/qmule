/*
 * Bittorrent Client using Qt4 and libtorrent.
 * Copyright (C) 2006  Christophe Dumez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 *
 * Contact : chris@qbittorrent.org
 */

#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <math.h>
#include "misc.h"
#include "preferences.h"
#include "qtorrenthandle.h"
#include "torrentpersistentdata.h"
#include <libtorrent/version.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/peer_info.hpp>
#if LIBTORRENT_VERSION_MINOR < 15
#include <boost/date_time/posix_time/posix_time_types.hpp>
#endif

#ifdef Q_WS_WIN
#include <Windows.h>
#endif

using namespace libtorrent;
using namespace std;

#if LIBTORRENT_VERSION_MINOR < 16
static QString boostTimeToQString(const boost::posix_time::ptime &boostDate) {
  if (boostDate.is_not_a_date_time()) return "";
  struct std::tm tm;
  try {
    tm = boost::posix_time::to_tm(boostDate);
  } catch(std::exception e) {
      return "";
  }
  const time_t t = mktime(&tm);
  const QDateTime dt = QDateTime::fromTime_t(t);
  return dt.toString(Qt::DefaultLocaleLongDate);
}
#endif

QTorrentHandle::QTorrentHandle(const torrent_handle& h): torrent_handle(h) {}

bool QTorrentHandle::operator==(const TransferBase& t) const {
    const QTorrentHandle* pt = dynamic_cast<const QTorrentHandle*>(&t);
    return (pt && *pt == *this);
}

bool QTorrentHandle::operator<(const TransferBase& t) const {
    const QTorrentHandle* pt = dynamic_cast<const QTorrentHandle*>(&t);
    return (pt && *pt < *this);
}

//
// Getters
//

QString QTorrentHandle::hash() const {
  return misc::toQString(torrent_handle::info_hash());
}

QString QTorrentHandle::name() const {
  QString name = TorrentPersistentData::getName(hash());
  if (name.isEmpty()) {
    name = misc::toQStringU(torrent_handle::name());
  }
  return name;
}

QString QTorrentHandle::creation_date() const {
#if LIBTORRENT_VERSION_MINOR > 15
  boost::optional<time_t> t = torrent_handle::get_torrent_info().creation_date();
  if (t)
      return QDateTime::fromTime_t(*t).toString(Qt::DefaultLocaleLongDate);
  return "";
#else
  boost::optional<boost::posix_time::ptime> boostDate = torrent_handle::get_torrent_info().creation_date();
  if (boostDate) {
      return boostTimeToQString(*boostDate);
  }
  return "";
#endif
}

QString QTorrentHandle::next_announce() const {
#if LIBTORRENT_VERSION_MINOR > 15
  return misc::userFriendlyDuration(torrent_handle::status(0x0).next_announce.total_seconds());
#else
  return misc::userFriendlyDuration(torrent_handle::status().next_announce.total_seconds());
#endif
}

qlonglong QTorrentHandle::next_announce_s() const {
#if LIBTORRENT_VERSION_MINOR > 15
  return torrent_handle::status(0x0).next_announce.total_seconds();
#else
  return torrent_handle::status().next_announce.total_seconds();
#endif
}

QString QTorrentHandle::current_tracker() const {
#if LIBTORRENT_VERSION_MINOR > 15
  return misc::toQString(torrent_handle::status(0x0).current_tracker);
#else
  return misc::toQString(torrent_handle::status().current_tracker);
#endif
}

bool QTorrentHandle::is_paused() const {
#if LIBTORRENT_VERSION_MINOR > 15
  torrent_status st = torrent_handle::status(0x0);
  return st.paused && !st.auto_managed;
#else
  return torrent_handle::is_paused() && !torrent_handle::is_auto_managed();
#endif
}

bool QTorrentHandle::is_queued() const {
#if LIBTORRENT_VERSION_MINOR > 15
  torrent_status st = torrent_handle::status(0x0);
  return st.paused && st.auto_managed;
#else
  return torrent_handle::is_paused() && torrent_handle::is_auto_managed();
#endif
}

size_type QTorrentHandle::total_size() const {
  return torrent_handle::get_torrent_info().total_size();
}

size_type QTorrentHandle::piece_length() const {
  return torrent_handle::get_torrent_info().piece_length();
}

int QTorrentHandle::num_pieces() const {
  return torrent_handle::get_torrent_info().num_pieces();
}

bool QTorrentHandle::extremity_pieces_first() const {
  const torrent_info& t = get_torrent_info();
  std::vector<int> piece_priorities = torrent_handle::piece_priorities();

  for (int index = 0; index < t.num_files(); ++index) {
#if LIBTORRENT_VERSION_MINOR > 15
    QString path = misc::toQStringU(t.file_at(index).path);
#else
    QString path = misc::toQStringU(t.file_at(index).path.string());
#endif
    const QString ext = misc::file_extension(path);
    if (misc::isPreviewable(ext) && torrent_handle::file_priority(index) > 0) {
      const std::vector<int> extremities = file_extremity_pieces_at(index);
      int sum_prio = 0;
      foreach (int e, extremities) sum_prio += piece_priorities[e];
      if (sum_prio == 7 * extremities.size()) return true;
    }
  }

  return false;
}

QString QTorrentHandle::save_path() const {
#if LIBTORRENT_VERSION_MINOR > 15
  return misc::toQStringU(torrent_handle::save_path()).replace("\\", "/");
#else
  return misc::toQStringU(torrent_handle::save_path().string()).replace("\\", "/");
#endif
}

QStringList QTorrentHandle::url_seeds() const {
  QStringList res;
  try {
    const std::set<std::string> existing_seeds = torrent_handle::url_seeds();
    std::set<std::string>::const_iterator it;
    for (it = existing_seeds.begin(); it != existing_seeds.end(); it++) {
      qDebug("URL Seed: %s", it->c_str());
      res << misc::toQString(*it);
    }
  } catch(std::exception e) {
    std::cout << "ERROR: Failed to convert the URL seed" << std::endl;
  }
  return res;
}

bool QTorrentHandle::has_filtered_pieces() const {
  const std::vector<int> piece_priorities = torrent_handle::piece_priorities();
  foreach (const int priority, piece_priorities) {
    if (priority == 0)
      return true;
  }
  return false;
}

int QTorrentHandle::num_files() const {
  return torrent_handle::get_torrent_info().num_files();
}

QString QTorrentHandle::filename_at(unsigned int index) const {
  Q_ASSERT(index < (unsigned int)torrent_handle::get_torrent_info().num_files());
#if LIBTORRENT_VERSION_MINOR > 15
  return misc::fileName(filepath_at(index));
#else
  return misc::toQStringU(torrent_handle::get_torrent_info().file_at(index).path.leaf());
#endif
}

size_type QTorrentHandle::filesize_at(unsigned int index) const {
  Q_ASSERT(index < (unsigned int)torrent_handle::get_torrent_info().num_files());
  return torrent_handle::get_torrent_info().file_at(index).size;
}

QString QTorrentHandle::filepath_at(unsigned int index) const {
#if LIBTORRENT_VERSION_MINOR > 15
  return misc::toQStringU(torrent_handle::get_torrent_info().file_at(index).path);
#else
  return misc::toQStringU(torrent_handle::get_torrent_info().file_at(index).path.string());
#endif
}

QString QTorrentHandle::orig_filepath_at(unsigned int index) const {
#if LIBTORRENT_VERSION_MINOR > 15
  return misc::toQStringU(torrent_handle::get_torrent_info().orig_files().at(index).path);
#else
  return misc::toQStringU(torrent_handle::get_torrent_info().orig_files().at(index).path.string());
#endif
}

std::vector<int> QTorrentHandle::file_extremity_pieces_at(unsigned int index) const {
  const torrent_info& t = get_torrent_info();
  const int num_pieces = t.num_pieces();
  const int piece_size = t.piece_length();
  const file_entry& file = t.file_at(index);

  // Determine the first and last piece of the file
  int first_piece = floor((file.offset + 1) / (float)piece_size);
  Q_ASSERT(first_piece >= 0 && first_piece < num_pieces);

  int num_pieces_in_file = ceil(file.size / (float)piece_size);
  int last_piece = first_piece + num_pieces_in_file - 1;
  Q_ASSERT(last_piece >= 0 && last_piece < num_pieces);

  const int preview_size = 10 * 1024 * 1024; // 10M
  const int preview_pieces = std::min<int>(ceil(preview_size / (float)piece_size), num_pieces_in_file);
  Q_ASSERT(preview_pieces > 0 && preview_pieces <= num_pieces_in_file);

  std::vector<int> result;
  for (int p = 0; p < preview_pieces; ++p) {
    result.push_back(first_piece + p);
    result.push_back(last_piece - p);
  }
  return result;
}

TransferStatus QTorrentHandle::status() const
{
#if LIBTORRENT_VERSION_MINOR > 15
  return transfer_status2TS(torrent_handle::status(0x0));
#else
  return transfer_status2TS(torrent_handle::status());
#endif
}

TransferState QTorrentHandle::state() const
{
    return status().state;
}

libtorrent::torrent_info QTorrentHandle::get_info() const {
    return torrent_handle::get_torrent_info();
}

QString QTorrentHandle::creator() const {
  return misc::toQStringU(torrent_handle::get_torrent_info().creator());
}

QString QTorrentHandle::comment() const {
  return misc::toQStringU(torrent_handle::get_torrent_info().comment());
}

// Return a list of absolute paths corresponding
// to all files in a torrent
QStringList QTorrentHandle::absolute_files_path() const {
  QDir saveDir(save_path());
  QStringList res;
  for (int i = 0; i<num_files(); ++i) {
    res << QDir::cleanPath(saveDir.absoluteFilePath(filepath_at(i)));
  }
  return res;
}

QStringList QTorrentHandle::absolute_files_path_uneeded() const {
  QDir saveDir(save_path());
  QStringList res;
  std::vector<int> fp = torrent_handle::file_priorities();
  for (uint i = 0; i < fp.size(); ++i) {
    if (fp[i] == 0) {
      const QString file_path = QDir::cleanPath(saveDir.absoluteFilePath(filepath_at(i)));
      if (file_path.contains(".unwanted"))
        res << file_path;
    }
  }
  return res;
}

void QTorrentHandle::get_peer_info(std::vector<PeerInfo>& peers) const
{
    std::vector<libtorrent::peer_info> lt_peers;
    torrent_handle::get_peer_info(lt_peers);
    std::transform(lt_peers.begin(), lt_peers.end(), std::back_inserter(peers), peer_info2PInfo<libtorrent::peer_info>);
}
std::vector<libtorrent::announce_entry> QTorrentHandle::trackers() const {
    return torrent_handle::trackers();
}
void QTorrentHandle::flush_cache() const {
    torrent_handle::flush_cache();
}
void QTorrentHandle::force_recheck() const {
    torrent_handle::force_recheck();
}
void QTorrentHandle::force_reannounce() const {
    torrent_handle::force_reannounce();
}
void QTorrentHandle::connect_peer(const PeerEndpoint& ep) const {
    torrent_handle::connect_peer(ep);
}
void QTorrentHandle::set_peer_upload_limit(const PeerEndpoint& ep, long limit) const {
    torrent_handle::set_peer_upload_limit(ep, limit);
}
void QTorrentHandle::set_peer_download_limit(const PeerEndpoint& ep, long limit) const {
    torrent_handle::set_peer_download_limit(ep, limit);
}
void QTorrentHandle::add_tracker(const libtorrent::announce_entry& url) const {
    torrent_handle::add_tracker(url);
}
void QTorrentHandle::replace_trackers(const std::vector<libtorrent::announce_entry>& trackers) const {
    torrent_handle::replace_trackers(trackers);
}
void QTorrentHandle::queue_position_up() const {
    torrent_handle::queue_position_up();
}
void QTorrentHandle::queue_position_down() const {
    torrent_handle::queue_position_down();
}
void QTorrentHandle::queue_position_top() const {
    torrent_handle::queue_position_top();
}
void QTorrentHandle::queue_position_bottom() const {
    torrent_handle::queue_position_bottom();
}
void QTorrentHandle::super_seeding(bool ss) const {
    torrent_handle::super_seeding(ss);
}
void QTorrentHandle::set_sequential_download(bool sd) const {
    torrent_handle::set_sequential_download(sd);
}

bool QTorrentHandle::has_missing_files() const {
  const QStringList paths = absolute_files_path();
  foreach (const QString &path, paths) {
    if (!QFile::exists(path)) return true;
  }
  return false;
}

int QTorrentHandle::queue_position() const {
  if (torrent_handle::queue_position() < 0)
    return -1;
  return torrent_handle::queue_position()+1;
}

int QTorrentHandle::num_uploads() const {
#if LIBTORRENT_VERSION_MINOR > 15
  return torrent_handle::status(0x0).num_uploads;
#else
  return torrent_handle::status().num_uploads;
#endif
}

bool QTorrentHandle::is_valid() const {
    return torrent_handle::is_valid();
}

bool QTorrentHandle::is_seed() const {
  // Affected by bug http://code.rasterbar.com/libtorrent/ticket/402
  //return torrent_handle::is_seed();
  // May suffer from approximation problems
  //return (progress() == 1.);
  // This looks safe
  //torrent_status::state_t st = state();
  //return (st == torrent_status::finished || st == torrent_status::seeding);
  // Or maybe like this?
#if LIBTORRENT_VERSION_MINOR > 15
  return torrent_handle::status(0x0).is_seeding;
#else
  return torrent_handle::is_seed();
#endif
}

bool QTorrentHandle::is_auto_managed() const {
#if LIBTORRENT_VERSION_MINOR > 15
  torrent_status status = torrent_handle::status(0x0);
  return status.auto_managed;
#else
  return torrent_handle::is_auto_managed();
#endif
}

bool QTorrentHandle::is_sequential_download() const {
#if LIBTORRENT_VERSION_MINOR > 15
  torrent_status status = torrent_handle::status(0x0);
  return status.sequential_download;
#else
  return torrent_handle::is_sequential_download();
#endif
}

int QTorrentHandle::upload_limit() const {
    return torrent_handle::upload_limit();
}

int QTorrentHandle::download_limit() const {
    return torrent_handle::download_limit();
}

bool QTorrentHandle::priv() const {
  return torrent_handle::get_torrent_info().priv();
}

bool QTorrentHandle::super_seeding() const {
    return torrent_handle::super_seeding();
}

QString QTorrentHandle::firstFileSavePath() const {
  Q_ASSERT(has_metadata());
  QString fsave_path = TorrentPersistentData::getSavePath(hash());
  if (fsave_path.isEmpty())
    fsave_path = save_path();
  fsave_path.replace("\\", "/");
  if (!fsave_path.endsWith("/"))
    fsave_path += "/";
  fsave_path += filepath_at(0);
  // Remove .!qB extension
  if (fsave_path.endsWith(".!qB", Qt::CaseInsensitive))
    fsave_path.chop(4);
  return QFileInfo(fsave_path).path();
}

bool QTorrentHandle::has_error() const {
#if LIBTORRENT_VERSION_MINOR > 15
  torrent_status st = torrent_handle::status(0x0);
  return st.paused && !st.error.empty();
#else
  return torrent_handle::is_paused() && !torrent_handle::status().error.empty();
#endif
}

QString QTorrentHandle::error() const {
#if LIBTORRENT_VERSION_MINOR > 15
  return misc::toQString(torrent_handle::status(0x0).error);
#else
  return misc::toQString(torrent_handle::status().error);
#endif
}

void QTorrentHandle::downloading_pieces(bitfield &bf) const {
  std::vector<partial_piece_info> queue;
  torrent_handle::get_download_queue(queue);
  for (std::vector<partial_piece_info>::const_iterator it=queue.begin(); it!= queue.end(); it++) {
    bf.set_bit(it->piece_index);
  }
  return;
}

void QTorrentHandle::piece_availability(std::vector<int>& avail) const {
    torrent_handle::piece_availability(avail);
}

std::vector<int> QTorrentHandle::piece_priorities() const {
    return torrent_handle::piece_priorities();
}

bool QTorrentHandle::has_metadata() const {
#if LIBTORRENT_VERSION_MINOR > 15
  return torrent_handle::status(query_distributed_copies).has_metadata;
#else
  return torrent_handle::has_metadata();
#endif
}

float QTorrentHandle::distributed_copies() const {
#if LIBTORRENT_VERSION_MINOR > 15
return torrent_handle::status(0x0).distributed_copies;
#else
  return torrent_handle::status().distributed_copies;
#endif
}

void QTorrentHandle::file_progress(std::vector<size_type>& fp) const {
  torrent_handle::file_progress(fp, torrent_handle::piece_granularity);
}

std::vector<int> QTorrentHandle::file_priorities() const {
    return torrent_handle::file_priorities();
}

//
// Setters
//

void QTorrentHandle::pause() const {
  torrent_handle::auto_managed(false);
  torrent_handle::pause();
  torrent_handle::save_resume_data();
}

void QTorrentHandle::resume() const {
  if (has_error())
    torrent_handle::clear_error();

  const QString torrent_hash = hash();
  bool has_persistant_error = TorrentPersistentData::hasError(torrent_hash);
  TorrentPersistentData::setErrorState(torrent_hash, false);
  bool temp_path_enabled = Preferences().isTempPathEnabled();
  if (has_persistant_error && temp_path_enabled) {
    // Torrent was supposed to be seeding, checking again in final destination
    qDebug("Resuming a torrent with error...");
    const QString final_save_path = TorrentPersistentData::getSavePath(torrent_hash);
    qDebug("Torrent final path is: %s", qPrintable(final_save_path));
    if (!final_save_path.isEmpty())
      move_storage(final_save_path);
  }
  torrent_handle::auto_managed(true);
  torrent_handle::resume();
  if (has_persistant_error && temp_path_enabled) {
    // Force recheck
    torrent_handle::force_recheck();
  }
}

void QTorrentHandle::remove_url_seed(const QString& seed) const {
  torrent_handle::remove_url_seed(seed.toStdString());
}

void QTorrentHandle::add_url_seed(const QString& seed) const {
  const std::string str_seed = seed.toStdString();
  qDebug("calling torrent_handle::add_url_seed(%s)", str_seed.c_str());
  torrent_handle::add_url_seed(str_seed);
}

void QTorrentHandle::set_tracker_login(const QString& username, const QString& password) const {
  torrent_handle::set_tracker_login(std::string(username.toLocal8Bit().constData()), std::string(password.toLocal8Bit().constData()));
}

void QTorrentHandle::move_storage(const QString& new_path) const {
  if (QDir(save_path()) == QDir(new_path))
    return;

  TorrentPersistentData::setPreviousSavePath(hash(), save_path());
  // Create destination directory if necessary
  // or move_storage() will fail...
  QDir().mkpath(new_path);
  // Actually move the storage
  torrent_handle::move_storage(new_path.toUtf8().constData());
}

bool QTorrentHandle::save_torrent_file(const QString& path) const {
  if (!has_metadata()) return false;

  const torrent_info& t = torrent_handle::get_torrent_info();

  entry meta = bdecode(t.metadata().get(),
                       t.metadata().get() + t.metadata_size());
  entry torrent_entry(entry::dictionary_t);
  torrent_entry["info"] = meta;
  if (!torrent_handle::trackers().empty())
    torrent_entry["announce"] = torrent_handle::trackers().front().url;

  vector<char> out;
  bencode(back_inserter(out), torrent_entry);
  QFile torrent_file(path);
  if (!out.empty() && torrent_file.open(QIODevice::WriteOnly)) {
    torrent_file.write(&out[0], out.size());
    torrent_file.close();
    return true;
  }

  return false;
}

void QTorrentHandle::file_priority(int index, int priority) const {
  vector<int> priorities = torrent_handle::file_priorities();
  if (priorities[index] != priority) {
    priorities[index] = priority;
    prioritize_files(priorities);
  }
}

void QTorrentHandle::prioritize_files(const vector<int> &files) const {
  if ((int)files.size() != torrent_handle::get_torrent_info().num_files()) return;
  qDebug() << Q_FUNC_INFO;
  bool was_seed = is_seed();
  vector<size_type> progress;
  file_progress(progress);
  qDebug() << Q_FUNC_INFO << "Changing files priorities...";
  torrent_handle::prioritize_files(files);
  qDebug() << Q_FUNC_INFO << "Moving unwanted files to .unwanted folder...";
  for (uint i = 0; i < files.size(); ++i) {
    // Move unwanted files to a .unwanted subfolder
    if (files[i] == 0 && progress[i] < filesize_at(i)) {
      QString old_path = filepath_at(i);
      // Make sure the file does not already exists
      if (QFile::exists(QDir(save_path()).absoluteFilePath(old_path))) {
        qWarning() << "File" << old_path << "already exists at destination.";
        qWarning() << "We do not move it to .unwanted folder";
        continue;
      }
      QString old_name = filename_at(i);
      QString parent_path = misc::branchPath(old_path);
      if (parent_path.isEmpty() || QDir(parent_path).dirName() != ".unwanted") {
        QString unwanted_abspath = QDir::cleanPath(save_path()+"/"+parent_path+"/.unwanted");
        qDebug() << "Unwanted path is" << unwanted_abspath;
        bool created = QDir().mkpath(unwanted_abspath);
#ifdef Q_WS_WIN
        qDebug() << "unwanted folder was created:" << created;
        if (created) {
          // Hide the folder on Windows
          qDebug() << "Hiding folder (Windows)";
          wstring win_path =  unwanted_abspath.replace("/","\\").toStdWString();
          DWORD dwAttrs = GetFileAttributesW(win_path.c_str());
          bool ret = SetFileAttributesW(win_path.c_str(), dwAttrs|FILE_ATTRIBUTE_HIDDEN);
          Q_ASSERT(ret != 0); Q_UNUSED(ret);
        }
#else
        Q_UNUSED(created);
#endif
        if (!parent_path.isEmpty() && !parent_path.endsWith("/"))
          parent_path += "/";
        rename_file(i, parent_path+".unwanted/"+old_name);
      }
    }
    // Move wanted files back to their original folder
    qDebug() << Q_FUNC_INFO << "Moving wanted files back from .unwanted folder";
    if (files[i] > 0) {
      QString parent_relpath = misc::branchPath(filepath_at(i));
      if (QDir(parent_relpath).dirName() == ".unwanted") {
        QString old_name = filename_at(i);
        QString new_relpath = misc::branchPath(parent_relpath);
        if (new_relpath.isEmpty())
            rename_file(i, old_name);
        else
            rename_file(i, QDir(new_relpath).filePath(old_name));
        // Remove .unwanted directory if empty
        qDebug() << "Attempting to remove .unwanted folder at " << QDir(save_path() + QDir::separator() + new_relpath).absoluteFilePath(".unwanted");
        QDir(save_path() + QDir::separator() + new_relpath).rmdir(".unwanted");
      }
    }
  }

  if (was_seed && !is_seed()) {
    qDebug() << "Torrent is no longer SEEDING";
    // Save seed status
    TorrentPersistentData::saveSeedStatus(*this);
    // Move to temp folder if necessary
    const Preferences pref;
    if (pref.isTempPathEnabled()) {
      QString tmp_path = pref.getTempPath();
      QString root_folder = TorrentPersistentData::getRootFolder(hash());
      if (!root_folder.isEmpty())
        tmp_path = QDir(tmp_path).absoluteFilePath(root_folder);
      qDebug() << "tmp folder is enabled, move torrent to " << tmp_path << " from " << save_path();
      move_storage(tmp_path);
    }
  }
}

void QTorrentHandle::prioritize_extremity_pieces(bool b, unsigned int file_index) const {
  // Determine the priority to set
  int prio = b ? 7 : torrent_handle::file_priority(file_index);

  const std::vector<int> extremities = file_extremity_pieces_at(file_index);
  foreach (int e, extremities)
    piece_priority(e, prio);
}

void QTorrentHandle::prioritize_extremity_pieces(bool b) const {
  if (!has_metadata()) return;
  // Download first and last pieces first for all media files in the torrent
  const uint nbfiles = num_files();
  for (uint index = 0; index < nbfiles; ++index) {
    const QString path = filepath_at(index);
    const QString ext = misc::file_extension(path);
    if (misc::isPreviewable(ext) && torrent_handle::file_priority(index) > 0) {
        prioritize_extremity_pieces(b, index);
    }
  }
}

void QTorrentHandle::rename_file(int index, const QString& name) const {
  qDebug() << Q_FUNC_INFO << index << name;
  torrent_handle::rename_file(index, std::string(name.toUtf8().constData()));
}

void QTorrentHandle::set_upload_mode(bool b) const { torrent_handle::set_upload_mode(b); }

//
// Operators
//

bool QTorrentHandle::operator ==(const QTorrentHandle& new_h) const {
  return info_hash() == new_h.info_hash();
}
