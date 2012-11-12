#ifndef FILES_WIDGET_H
#define FILES_WIDGET_H

#include <QWidget>
#include <QFileIconProvider>
#include "ui_files_widget.h"
#include "misc.h"

class files_widget : public QWidget, public Ui::files_widget
{
    Q_OBJECT

public:
    files_widget(QWidget *parent = 0);
    ~files_widget();

private:
    QMenu*   filesMenu;
    QAction* filesExchDir;
    QAction* filesExchSubdir;
    QAction* filesUnexchDir;
    QAction* filesUnexchSubdir;
public slots:
    void putToClipboard();
};

#endif // FILES_WIDGET_H
