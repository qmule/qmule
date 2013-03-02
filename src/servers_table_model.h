#ifndef SERVERS_TABLE_MODEL_H
#define SERVERS_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <libed2k/file.hpp>

class servers_table_model : public QAbstractTableModel
{
    Q_OBJECT
public:

    enum DisplayColumns
    {
        DC_NAME = 0,
        DC_IP,
        DC_PORT,
        DC_FILES,
        DC_USERS,
        DC_MAX_USERS,
        DC_LOWID_USERS,
        DC_DESCR
    };

    explicit servers_table_model(QObject *parent = 0);    
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void removeServer(const QModelIndex&);
    void addServer(const QString& name, const libed2k::net_identifier& point);
    void clear();

    /**
      * save/load model to server.met file
     */
    void save();
    void load();

    /**
      * public getters
     */
    QString ip(const QModelIndex&) const;
    qint16  port(const QModelIndex&) const;
private:
    libed2k::server_met server_met;
    libed2k::net_identifier m_connected_identifier;

    /**
      * getters to container
     */
    QString ip(int) const;
    qint16  port(int) const;
    QString name(int) const;
    QString description(int) const;
    quint64 users(int) const;
    quint64 files(int) const;
    quint64 soft_files(int) const;
    quint64 hard_files(int) const;
    quint64 max_users(int) const;
    quint64 lowid_users(int) const;
    libed2k::net_identifier identifier(int) const;
signals:
    
public slots:
    
};

#endif // SERVERS_TABLE_MODEL_H
