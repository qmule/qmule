#ifndef ED2K_LINK_MAKER_H
#define ED2K_LINK_MAKER_H

#include <QDialog>
#include "ui_ed2k_link_maker.h"

class ed2k_link_maker : public QDialog, public Ui::ed2k_link_maker
{
    Q_OBJECT
    QString m_fileName;
    QString m_hash;
    quint64 m_fileSize;

public:
    ed2k_link_maker(QString fileName, QString hash, quint64 fileSize, QWidget *parent = 0);
    ~ed2k_link_maker();

private:
    void createED2KLink();

private slots:
    void checkChanged(int state);
    void putToClipboard();
    void close();
};

#endif // ED2K_LINK_MAKER_H
