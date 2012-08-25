#include <QFileDialog>

#include "transport/session.h"
#include "misc.h"
#include "collection_save_dlg.h"

collection_save_dlg::collection_save_dlg(QWidget *parent, QString path)
    : QDialog(parent)
{
    setupUi(this);

    QString collectionName;    
    if (path.lastIndexOf("/") >=0 )
        collectionName = path.right(path.size() - path.lastIndexOf("/") - 1);
    else if (path.lastIndexOf("\\") >=0 )
        collectionName = path.right(path.size() - path.lastIndexOf("\\") - 1);
    else
        collectionName = path;

    this->setWindowTitle(collectionName);
    collectionName.replace(".emulecollection","");
    lineDirName->setText(collectionName);        

    if (QFile::exists(path))
    {
        // emule collections in libed2k works in local codepage
        libed2k::emule_collection ecoll = libed2k::emule_collection::fromFile(path.toLocal8Bit().constData());

        int row = 0;
        foreach(const libed2k::emule_collection_entry& ece, ecoll.m_files)
        {
            FileData fData;
            fData.file_name = QString::fromUtf8(ece.m_filename.c_str());
            fData.file_hash = ece.m_filehash;
            fData.file_size = ece.m_filesize;

            file_data.push_back(fData);
        }
    }
    init();
}

collection_save_dlg::collection_save_dlg(QWidget *parent, std::vector<FileData>& new_file_data, QString name)
    : QDialog(parent)
{
    setupUi(this);

    this->setWindowTitle(name);
    lineDirName->setText(name);
    
    file_data = new_file_data;
    init();
}

void collection_save_dlg::init()
{
    imgCollection->setPixmap(QIcon(":/emule/common/FileTypeEmuleCollection.ico").pixmap(16, 16));

    QStringList labels;
    labels << tr("File name") << tr("File size") << tr("Hash");
    tableFiles->setHorizontalHeaderLabels(labels);

    connect(btnSelect, SIGNAL(clicked()), this, SLOT(selectDirectory()));
    connect(btnDowload, SIGNAL(clicked()), this, SLOT(dowload()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(cancel()));
    connect(lineDirName, SIGNAL(textChanged(const QString&)), this, SLOT(checkDir(const QString&)));
    connect(tableFiles, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));

    tableFiles->setRowCount(file_data.size());
    int row = 0;
    foreach(const FileData& file, file_data)
    {
        addFilesRow(row);

        tableFiles->item(row, 0)->setText(file.file_name);            
        tableFiles->item(row, 1)->setText(misc::friendlyUnit(file.file_size));
        tableFiles->item(row, 2)->setText(QString::fromUtf8(file.file_hash.toString().c_str()));
        row++;
    }
    labelList->setText(tr("Collection list") + "(" + QString::number(file_data.size()) + ")");

    tableFiles->selectAll();

    // disable button only nothing to say
    if (row == 0) btnDowload->setDisabled(true);
    Preferences pref;
    dirPath = pref.getSavePath();
    separator = '/';
    QDir incoming(dirPath);
    labelPath->setText(incoming.filePath(lineDirName->text()));
}

collection_save_dlg::~collection_save_dlg()
{
}

void collection_save_dlg::addFilesRow(int row)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableFiles->setItem(row, 0, item);
    
    item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableFiles->setItem(row, 1, item);

    item = new QTableWidgetItem;
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    tableFiles->setItem(row, 2, item);
}

void collection_save_dlg::selectDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), dirPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.size())
    {
        if (dir.indexOf("/") > 0)
        {
            if (dir.lastIndexOf("/") == (dir.length() - 1) )
                dir = dir.left(dir.length() - 1);
            separator = '/';
        }
        if (dir.indexOf("\\") > 0)
        {
            if (dir.lastIndexOf("\\") == (dir.length() - 1) )
                dir = dir.left(dir.length() - 1);
            separator = '\\';
        }
        
        dirPath = dir;
        labelPath->setText(dirPath + separator + lineDirName->text());
        if (lineDirName->text().length())
            btnDowload->setEnabled(true);
        else
            btnDowload->setDisabled(true);
    }
}

void collection_save_dlg::dowload()
{
    QString dir_path = labelPath->text();

    QDir dir(dir_path);
    if (!dir.exists())
    {
        dir.mkpath(dir_path);
        dir = QDir(dir_path);
    }

    int last_row = -1;
    foreach(QTableWidgetItem* const item, tableFiles->selectedItems())
    {
        int row = item->row();
        if (row == last_row)
            continue;
        libed2k::add_transfer_params atp;
        QString filepath = dir.filePath(file_data[row].file_name);
        atp.file_path = filepath.toUtf8();
        atp.file_hash = file_data[row].file_hash;        
        atp.file_size = file_data[row].file_size;
        Session::instance()->get_ed2k_session()->delegate()->add_transfer(atp);

        last_row = row;
    }
    accept();
}

void collection_save_dlg::cancel()
{
    reject();
}

void collection_save_dlg::checkDir(const QString& dir_name)
{
    QString dirName = dir_name;;
    dirName = dirName.replace("/", "").replace("\\", "");
    if (dirName != dir_name)
        lineDirName->setText(dirName);

    labelPath->setText(dirPath + separator + lineDirName->text());
    if (dirName.length())
        if (tableFiles->selectedItems().size())
        {
            btnDowload->setEnabled(true);
            return;
        }

    btnDowload->setDisabled(true);
}

void collection_save_dlg::selectionChanged()
{
    QString dir_name = lineDirName->text();

    if (dir_name.length())
        if (tableFiles->selectedItems().size())
        {
            btnDowload->setEnabled(true);
            return;
        }

    btnDowload->setDisabled(true);
}
