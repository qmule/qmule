#include "misc.h"
#include "ed2k_link_maker.h"

#include <QClipboard>
#include <QUrl>


ed2k_link_maker::ed2k_link_maker(QString fileName, QString hash, quint64 fileSize, QWidget *parent)
    : QDialog(parent), m_fileName(fileName), m_hash(hash), m_fileSize(fileSize)
{
    setupUi(this);

    if (!m_fileName.length() || !m_hash.length())
        return;

    connect(checkForum, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));
    connect(checkSize, SIGNAL(stateChanged(int)), this, SLOT(checkChanged(int)));
    connect(btnCopy, SIGNAL(clicked()), this, SLOT(putToClipboard()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    createED2KLink();
}

ed2k_link_maker::~ed2k_link_maker()
{

}

void ed2k_link_maker::createED2KLink()
{
    editLink->clear();
    checkSize->setDisabled(true);

    QString size = QString::number(m_fileSize);

    QString link = "ed2k://|file|" + QString(QUrl::toPercentEncoding(m_fileName)) + "|" +
        size + "|" + m_hash + "|/";

    if (checkForum->checkState() == Qt::Checked)
    {
        link = "[u][b][url=" + link + "]" + m_fileName + "[/url][/b][/u]";
        checkSize->setEnabled(true);
        if (checkSize->checkState() == Qt::Checked)
            link += " " + misc::friendlyUnit(m_fileSize);
    }
    editLink->appendPlainText(link);
}

void ed2k_link_maker::checkChanged(int state)
{
    createED2KLink();
}

void ed2k_link_maker::putToClipboard()
{
    QString text = editLink->toPlainText();
    if (text.length())
    {
        QClipboard *cb = QApplication::clipboard();
        cb->setText(text);
    }
}

void ed2k_link_maker::close()
{
    accept();
}