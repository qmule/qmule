#ifndef ADVANCEDSETTINGS_H
#define ADVANCEDSETTINGS_H

#include <QTableWidget>
#include <QHeaderView>
#include <QSpinBox>
#include <QHostAddress>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QNetworkInterface>
#include <libtorrent/version.hpp>
#include "preferences.h"

enum AdvSettingsCols {PROPERTY, VALUE};
enum AdvSettingsRows {DISK_CACHE, OUTGOING_PORT_MIN, OUTGOING_PORT_MAX, IGNORE_LIMIT_LAN, RECHECK_COMPLETED, LIST_REFRESH, RESOLVE_COUNTRIES, MAX_HALF_OPEN, SUPER_SEEDING, NETWORK_IFACE, MULE_NETWORK_IFACE, NETWORK_ADDRESS, PROGRAM_NOTIFICATIONS, TRACKER_STATUS, TRACKER_PORT,
                    #if defined(Q_WS_X11)
                      USE_ICON_THEME,
                    #endif
                      CONFIRM_DELETE_TORRENT, TRACKER_EXCHANGE,
                      ANNOUNCE_ALL_TRACKERS,
                      ROW_COUNT};

class AdvancedSettings: public QTableWidget {
  Q_OBJECT

private:
  QSpinBox spin_cache, outgoing_ports_min, outgoing_ports_max, spin_list_refresh, spin_maxhalfopen, spin_tracker_port;
  QCheckBox cb_ignore_limits_lan, cb_recheck_completed, cb_resolve_countries,
  cb_super_seeding, cb_program_notifications, cb_tracker_status, cb_confirm_torrent_deletion,
  cb_enable_tracker_ext;
  QComboBox combo_iface;  
  QComboBox combo_iface_mule;
#if defined(Q_WS_X11)
  QCheckBox cb_use_icon_theme;
#endif
  QCheckBox cb_announce_all_trackers;
  QRegExpValidator m_rev;
  QLineEdit txt_network_address;

public:
  AdvancedSettings(QWidget *parent=0): QTableWidget(parent)
  {
    // Set visual appearance
    m_rev.setRegExp(QRegExp("\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}"));
    txt_network_address.setValidator(&m_rev);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setColumnCount(2);
    QStringList header;
    header << tr("Setting") << tr("Value", "Value set for this setting");
    setHorizontalHeaderLabels(header);
    setColumnWidth(0, width()/2);
    horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->setVisible(false);
    setRowCount(ROW_COUNT);
    // Load settings
    loadAdvancedSettings();
  }

  ~AdvancedSettings() {
  }

public slots:
  void saveAdvancedSettings() {
    Preferences pref;
    // Disk write cache
    pref.setDiskCacheSize(spin_cache.value());
    // Outgoing ports
    pref.setOutgoingPortsMin(outgoing_ports_min.value());
    pref.setOutgoingPortsMax(outgoing_ports_max.value());
    // Ignore limits on LAN
    pref.ignoreLimitsOnLAN(cb_ignore_limits_lan.isChecked());
    // Recheck torrents on completion
    pref.recheckTorrentsOnCompletion(cb_recheck_completed.isChecked());
    // Transfer list refresh interval
    pref.setRefreshInterval(spin_list_refresh.value());
    // Peer resolution
    pref.resolvePeerCountries(cb_resolve_countries.isChecked());
    // Max Half-Open connections
    pref.setMaxHalfOpenConnections(spin_maxhalfopen.value());
    // Super seeding
    pref.enableSuperSeeding(cb_super_seeding.isChecked());
    // Network interface
    if (combo_iface.currentIndex() == 0) {
      // All interfaces (default)
      pref.setNetworkInterface(QString::null);
    }
    else
    {
      pref.setNetworkInterface(combo_iface.currentText());
    }

    if (combo_iface_mule.currentIndex() == 0)
    {
        // All interfaces (default)
        pref.setNetworkInterfaceMule(QString::null);
    }
    else
    {
        pref.setNetworkInterfaceMule(combo_iface_mule.currentText());
    }

    // Network address
    QHostAddress addr(txt_network_address.text().trimmed());
    if (addr.isNull())
      pref.setNetworkAddress("");
    else
      pref.setNetworkAddress(addr.toString());
    // Program notification
    pref.useProgramNotification(cb_program_notifications.isChecked());
    // Tracker
    pref.setTrackerEnabled(cb_tracker_status.isChecked());
    pref.setTrackerPort(spin_tracker_port.value());
    // Icon theme
#if defined(Q_WS_X11)
    pref.useSystemIconTheme(cb_use_icon_theme.isChecked());
#endif
    pref.setConfirmTorrentDeletion(cb_confirm_torrent_deletion.isChecked());
    // Tracker exchange
    pref.setTrackerExchangeEnabled(cb_enable_tracker_ext.isChecked());
    pref.setAnnounceToAllTrackers(cb_announce_all_trackers.isChecked());
  }

signals:
  void settingsChanged();

private:
  void setRow(int row, const QString &property, QSpinBox* editor) {
    setItem(row, PROPERTY, new QTableWidgetItem(property));
    bool ok; Q_UNUSED(ok);
    ok = connect(editor, SIGNAL(valueChanged(int)), SIGNAL(settingsChanged()));
    Q_ASSERT(ok);
    setCellWidget(row, VALUE, editor);
  }

  void setRow(int row, const QString &property, QComboBox* editor) {
    setItem(row, PROPERTY, new QTableWidgetItem(property));
    bool ok; Q_UNUSED(ok);
    ok = connect(editor, SIGNAL(currentIndexChanged(int)), SIGNAL(settingsChanged()));
    Q_ASSERT(ok);
    setCellWidget(row, VALUE, editor);
  }

  void setRow(int row, const QString &property, QCheckBox* editor) {
    setItem(row, PROPERTY, new QTableWidgetItem(property));
    bool ok; Q_UNUSED(ok);
    ok = connect(editor, SIGNAL(stateChanged(int)), SIGNAL(settingsChanged()));
    Q_ASSERT(ok);
    setCellWidget(row, VALUE, editor);
  }

  void setRow(int row, const QString &property, QLineEdit* editor) {
    setItem(row, PROPERTY, new QTableWidgetItem(property));
    bool ok; Q_UNUSED(ok);
    ok = connect(editor, SIGNAL(textChanged(QString)), SIGNAL(settingsChanged()));
    Q_ASSERT(ok);
    setCellWidget(row, VALUE, editor);
  }

private slots:
  void loadAdvancedSettings() {
    const Preferences pref;
    // Disk write cache
    spin_cache.setMinimum(1);
    spin_cache.setMaximum(200);
    spin_cache.setValue(pref.diskCacheSize());
    spin_cache.setSuffix(tr(" MiB"));
    setRow(DISK_CACHE, tr("Disk write cache size"), &spin_cache);
    // Outgoing port Min
    outgoing_ports_min.setMinimum(0);
    outgoing_ports_min.setMaximum(65535);
    outgoing_ports_min.setValue(pref.outgoingPortsMin());
    setRow(OUTGOING_PORT_MIN, tr("Outgoing ports (Min) [0: Disabled]"), &outgoing_ports_min);
    // Outgoing port Min
    outgoing_ports_max.setMinimum(0);
    outgoing_ports_max.setMaximum(65535);
    outgoing_ports_max.setValue(pref.outgoingPortsMax());
    setRow(OUTGOING_PORT_MAX, tr("Outgoing ports (Max) [0: Disabled]"), &outgoing_ports_max);
    // Ignore transfer limits on local network
    cb_ignore_limits_lan.setChecked(pref.ignoreLimitsOnLAN());
    setRow(IGNORE_LIMIT_LAN, tr("Ignore transfer limits on local network"), &cb_ignore_limits_lan);
    // Recheck completed torrents
    cb_recheck_completed.setChecked(pref.recheckTorrentsOnCompletion());
    setRow(RECHECK_COMPLETED, tr("Recheck torrents on completion"), &cb_recheck_completed);
    // Transfer list refresh interval
    spin_list_refresh.setMinimum(30);
    spin_list_refresh.setMaximum(99999);
    spin_list_refresh.setValue(pref.getRefreshInterval());
    spin_list_refresh.setSuffix(tr(" ms", " milliseconds"));
    setRow(LIST_REFRESH, tr("Transfer list refresh interval"), &spin_list_refresh);
    // Resolve Peer countries
    cb_resolve_countries.setChecked(pref.resolvePeerCountries());
    setRow(RESOLVE_COUNTRIES, tr("Resolve peer countries (GeoIP)"), &cb_resolve_countries);
    // Max Half Open connections
    spin_maxhalfopen.setMinimum(0);
    spin_maxhalfopen.setMaximum(99999);
    spin_maxhalfopen.setValue(pref.getMaxHalfOpenConnections());
    setRow(MAX_HALF_OPEN, tr("Maximum number of half-open connections [0: Disabled]"), &spin_maxhalfopen);
    // Super seeding
    cb_super_seeding.setChecked(pref.isSuperSeedingEnabled());
    setRow(SUPER_SEEDING, tr("Strict super seeding"), &cb_super_seeding);
    // Network interface
    combo_iface.addItem(tr("Any interface", "i.e. Any network interface"));
    combo_iface_mule.addItem(tr("Any interface", "i.e. Any network interface"));

    const QString current_iface = pref.getNetworkInterface();
    const QString current_iface_mule = pref.getNetworkInterfaceMule();
    int i = 1;
    foreach (const QNetworkInterface& iface, QNetworkInterface::allInterfaces())
    {
        if ((iface.flags() & QNetworkInterface::IsLoopBack) ||
            (!iface.isValid()) ||
            ((iface.flags() & QNetworkInterface::IsUp) == 0))
                continue;
            combo_iface.addItem(iface.humanReadableName());
            combo_iface_mule.addItem(iface.humanReadableName());

      // restore torrent iface
      if (!current_iface.isEmpty() && iface.humanReadableName() == current_iface)
      {
          combo_iface.setCurrentIndex(i);
      }

      // restore mule iface
      if (!current_iface_mule.isEmpty() && iface.humanReadableName() == current_iface_mule)
      {
          combo_iface_mule.setCurrentIndex(i);
      }

      ++i;
    }

    setRow(NETWORK_IFACE, tr("Network Interface (requires restart)"), &combo_iface);
    setRow(MULE_NETWORK_IFACE, tr("Mule Network Interface (requires restart)"), &combo_iface_mule);
    // Network address
    txt_network_address.setText(pref.getNetworkAddress());
    setRow(NETWORK_ADDRESS, tr("IP Address to report to trackers (requires restart)"), &txt_network_address);
    // Program notifications
    cb_program_notifications.setChecked(pref.useProgramNotification());
    setRow(PROGRAM_NOTIFICATIONS, tr("Display program on-screen notifications"), &cb_program_notifications);
    // Tracker State
    cb_tracker_status.setChecked(pref.isTrackerEnabled());
    setRow(TRACKER_STATUS, tr("Enable embedded tracker"), &cb_tracker_status);
    // Tracker port
    spin_tracker_port.setMinimum(1);
    spin_tracker_port.setMaximum(65535);
    spin_tracker_port.setValue(pref.getTrackerPort());
    setRow(TRACKER_PORT, tr("Embedded tracker port"), &spin_tracker_port);
#if defined(Q_WS_X11)
    cb_use_icon_theme.setChecked(pref.useSystemIconTheme());
    setRow(USE_ICON_THEME, tr("Use system icon theme"), &cb_use_icon_theme);
#endif
    // Torrent deletion confirmation
    cb_confirm_torrent_deletion.setChecked(pref.confirmTorrentDeletion());
    setRow(CONFIRM_DELETE_TORRENT, tr("Confirm torrent deletion"), &cb_confirm_torrent_deletion);
    // Tracker exchange
    cb_enable_tracker_ext.setChecked(pref.trackerExchangeEnabled());
    setRow(TRACKER_EXCHANGE, tr("Exchange trackers with other peers"), &cb_enable_tracker_ext);
    // Announce to all trackers
    cb_announce_all_trackers.setChecked(pref.announceToAllTrackers());
    setRow(ANNOUNCE_ALL_TRACKERS, tr("Always announce to all trackers"), &cb_announce_all_trackers);
  }

};

#endif // ADVANCEDSETTINGS_H
