#ifndef COLLECTION_SAVE_DLG_H
#define COLLECTION_SAVE_DLG_H

#include <QDialog>
#include <libed2k/file.hpp>
#include "ui_collection_save_dlg.h"

struct FileData
{
    QString             file_name;
    libed2k::md4_hash   file_hash;
    libed2k::size_type  file_size;
};

class collection_save_dlg : public QDialog, public Ui::collection_save_dlg
{
    Q_OBJECT

private:
    std::vector<FileData> file_data;
    QString dirPath;
    QChar separator;

public:
    collection_save_dlg(QWidget *parent, QString path);
    collection_save_dlg(QWidget *parent, std::vector<FileData>& new_file_data, QString name);
    ~collection_save_dlg();
    void init();

private:
    void addFilesRow(int row);

private slots:
    void selectDirectory();
    void dowload();
    void cancel();
    void checkDir(const QString& dir_name);
    void selectionChanged();
};

#endif // COLLECTION_SAVE_DLG_H
