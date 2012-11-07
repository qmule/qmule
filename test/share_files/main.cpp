#ifdef TEST
#include <QtTest/QTest>
#include "share_files_test.h"

QTEST_MAIN(share_files_test)

#else

#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

#endif

