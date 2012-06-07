//this file is part of xCatalog
//Copyright (C) 2011 xCatalog Team
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtGui/QRadioButton>
#include <QtGui/QMovie>
#include <QtGui/QButtonGroup>

#include "reportdialog.h"
#include "ui_reportdialog.h"

#define XCAT_REPORT_URL "http://city.is74.ru/forum/report_xcat.php?ver=2&p=%1"

ReportDialog::ReportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportDialog),
    m_nam(NULL)
{
    QFlags<Qt::WindowType> flags = windowFlags() | Qt::CustomizeWindowHint;
    flags &= ~(Qt::WindowCloseButtonHint|Qt::WindowContextHelpButtonHint);
    setWindowFlags( flags );
    ui->setupUi(this);
    ui->infoLabel->hide();

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOk()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));

    // generate list of options dynamically
    m_reasonGroup = new QButtonGroup(this);
    m_reasonGroup->setExclusive(true);
    addReason(tr("Заголовок отсутствует"));
    addReason(tr("Содержимое не соответствует заявленному"));
}

void ReportDialog::addReason( const QString &text )
{
    QRadioButton *button = new QRadioButton(text, ui->groupBox);
    m_reasonGroup->addButton(button, m_reasonGroup->buttons().size());
    ui->groupBox->layout()->addWidget(button);
}

ReportDialog::~ReportDialog()
{
    delete ui;

    if ( m_nam ) delete m_nam;
    if ( m_reasonGroup ) delete m_reasonGroup;
}

void ReportDialog::onOk()
{
    QAbstractButton *button = m_reasonGroup->checkedButton();
    if ( NULL == button ) {
        ui->infoLabel->show();
        return;
    }

    QString reason = button->text();

    // set progress label
    QMovie *movie = new QMovie(":/images/loading.gif");
    ui->infoLabel->setMovie(movie);
    movie->start();
    ui->infoLabel->show();

    // send request
    QUrl url( QString::fromUtf8(XCAT_REPORT_URL).arg(m_postId) );
    QByteArray postData = "reason=" + reason.toUtf8().toPercentEncoding();

    m_nam = new QNetworkAccessManager;
    connect(m_nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(reportSent(QNetworkReply*)));
    QNetworkRequest request(url);
    request.setHeader( QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded" );
    m_nam->post(request, postData);
}

void ReportDialog::reportSent( QNetworkReply *reply )
{
    reply->deleteLater();
    close();
}
