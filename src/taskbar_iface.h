#ifndef TASKBAR_IFACE_H
#define TASKBAR_IFACE_H

#include <QObject>
#include <QWidget>
#include <QIcon>

class taskbar_iface : public QObject
{
    Q_OBJECT
public:
    enum TBSTATE { S_NOPROGRESS = 0, S_INTERMEDIATE, S_NORM, S_ERROR, S_PAUSED };

    explicit taskbar_iface(QObject *parent, quint64 total);
    void initialize();
    ~taskbar_iface();
private:
    quint64 m_total;
    class TaskBarIface;
    TaskBarIface* m_pdelegate;
signals:
    
public slots:
    void setState(WId, TBSTATE);
    void setProgress(WId, quint64);
    void setIcon(WId, const QIcon&, const QString&);
    
};

#endif // TASKBAR_IFACE_H
