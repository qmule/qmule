#include "delay.h"

Delay::Delay(int mseconds) : m_mseconds(mseconds)
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
}

Delay::~Delay()
{

}

void Delay::execute(boost::function<void()> f)
{
    m_delegate = f;

    if (m_timer.isActive())
    {
        m_timer.setInterval(m_mseconds);
    }
    else
    {
        m_timer.start(m_mseconds);
    }
}

void Delay::cancel()
{
    if (m_timer.isActive()) m_timer.stop();
}

void Delay::on_timeout()
{
    m_delegate();
}
