#include <QDateTime>
#include <libed2k/util.hpp>
#include "status_widget.h"

status_widget::status_widget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    setDisconnectedInfo(QString());

    QString htmlClub = "<a href='http://city.is74.ru/forum/forumdisplay.php?f=134'>" + tr("Visit oslovedy club") + "</a> <br><br>";
    editServerInfo->insertHtml(htmlClub);
}

status_widget::~status_widget()
{

}

void status_widget::addLogMessage(QString log_message)
{
    QDateTime date_time = QDateTime::currentDateTime();
    QString msg = date_time.toString("dd.MM.yyyy hh:mm:ss") + ": " + log_message;
    editJournal->appendHtml(msg);
}

void status_widget::addHtmlLogMessage(const QString& msg)
{
    editJournal->appendHtml(msg);
}

void status_widget::setDisconnectedInfo(const QString& sid)
{
    m_servers.remove(sid);

    if (m_servers.empty())
    {
        editInfo->clear();
        QTextCharFormat charFormat = editInfo->currentCharFormat();
        QTextCharFormat charFormatBold = charFormat;
        charFormatBold.setFontWeight(QFont::Bold);
        editInfo->setCurrentCharFormat(charFormatBold);
        editInfo->appendPlainText(tr("eD2K Network"));
        editInfo->setCurrentCharFormat(charFormat);
        editInfo->appendPlainText(tr("Status:") + "\t" + tr("disconnected"));
    }
    else
        updateConnectedInfo();
}

void status_widget::updateConnectedInfo()
{
    editInfo->clear();

    foreach(QString sid, m_servers.keys())
    {
        QTextCharFormat charFormat = editInfo->currentCharFormat();
        QTextCharFormat charFormatBold = charFormat;
        charFormatBold.setFontWeight(QFont::Bold);

        editInfo->setCurrentCharFormat(charFormatBold);
        editInfo->appendPlainText(tr("eD2K Network"));
        editInfo->setCurrentCharFormat(charFormat);
        editInfo->appendPlainText(tr("Status:") + "\t" + tr("connected"));
        editInfo->appendPlainText(tr("IP:Port:") + "\t");
        QString num;
        num.setNum(m_servers[sid].m_nClientId);
        editInfo->appendPlainText("ID:\t" + num);
        editInfo->appendPlainText(libed2k::isLowId(m_servers[sid].m_nClientId)?"\tLow ID":"\tHigh ID");

        editInfo->setCurrentCharFormat(charFormatBold);
        editInfo->appendPlainText("\n" + tr("eD2K Server"));
        editInfo->setCurrentCharFormat(charFormat);
        editInfo->appendPlainText(tr("Name:") + "\temule.is74.ru");
        editInfo->appendPlainText(tr("Description:"));
        editInfo->appendPlainText(tr("IP:Port:") + "\t" + sid);
        num.setNum(m_servers[sid].m_nUsers);
        editInfo->appendPlainText(tr("Clients:") + "\t" + num);
        num.setNum(m_servers[sid].m_nFiles);
        editInfo->appendPlainText(tr("Files:") + "\t" + num);
    }
}

void status_widget::serverAddress(QString strServer)
{
    m_servers.insert(strServer, server_info());
    updateConnectedInfo();

    QTextCharFormat charFormat = editServerInfo->currentCharFormat();
    QTextCharFormat charFormatBlue = charFormat;
    charFormatBlue.setForeground(QBrush(QColor(Qt::blue)));
    editServerInfo->setCurrentCharFormat(charFormatBlue);
    QDateTime date_time = QDateTime::currentDateTime();
    QString msg = date_time.toString("dd.MM.yyyy hh:mm:ss") + ": " + tr("Connection is established with: ") + strServer;
    editServerInfo->insertPlainText("\n");
    editServerInfo->insertPlainText(msg);
    editServerInfo->setCurrentCharFormat(charFormat);
}

void status_widget::clientID(const QString& sid, unsigned int nClientId)
{
    m_servers[sid].m_nClientId = nClientId;
    updateConnectedInfo();
}

void status_widget::serverInfo(QString strInfo)
{
    editServerInfo->insertPlainText("\n");
    editServerInfo->insertPlainText(strInfo);
}

void status_widget::serverStatus(const QString& sid, int nFiles, int nUsers)
{
    m_servers[sid].m_nFiles = nFiles;
    m_servers[sid].m_nUsers = nUsers;
    updateConnectedInfo();
}
