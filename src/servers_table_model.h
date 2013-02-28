#ifndef SERVERS_TABLE_MODEL_H
#define SERVERS_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <libed2k/file.hpp>

class servers_table_model : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit servers_table_model(QObject *parent = 0);    
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    /**
      * save/load model to server.met file
     */
    void save();
    void load();
private:
    libed2k::server_met server_met;
signals:
    
public slots:
    
};

#endif // SERVERS_TABLE_MODEL_H
