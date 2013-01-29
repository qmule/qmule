#ifndef __DELAY_H__
#define __DELAY_H__

#include <boost/function.hpp>
#include <QObject>
#include <QTimer>

class Delay : QObject
{
    Q_OBJECT
public:
    Delay(int mseconds);
    ~Delay();
    void execute(boost::function<void()>);
    void cancel();
private:
    int m_mseconds;
    QTimer m_timer;
    boost::function<void()> m_delegate;
private slots:
    void on_timeout();
};

#endif
