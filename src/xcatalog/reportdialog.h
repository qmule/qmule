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

#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class ReportDialog;
}

class QNetworkReply;
class QNetworkAccessManager;
class QButtonGroup;

class ReportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog(QWidget *parent = 0);
    ~ReportDialog();

    void setPostId( uint postId ) { m_postId = postId; }

private slots:
    void onOk();
    void reportSent( QNetworkReply *reply );

private:
    Ui::ReportDialog *ui;
    QButtonGroup *m_reasonGroup;
    uint m_postId;
    QNetworkAccessManager* m_nam;

void addReason( const QString &text );
};

#endif // REPORTDIALOG_H
