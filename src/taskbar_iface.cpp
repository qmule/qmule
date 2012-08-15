#include "taskbar_iface.h"

#if (defined Q_WS_WIN) && (defined WIN7_SDK)
#include <WinSDKVer.h>

#if _WIN32_MAXVER >= 0x0601

#include <shlobj.h>

class taskbar_iface::TaskBarIface
{
    friend class taskbar_iface;
private:
    ITaskbarList3*  m_taskbarInterface;
    ~TaskBarIface()
    {
        if (m_taskbarInterface) m_taskbarInterface->Release();
    }

public:
    TaskBarIface() : m_taskbarInterface(NULL)
    {
        HRESULT hr = CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, reinterpret_cast<void**> (&(m_taskbarInterface)));

        if (SUCCEEDED(hr))
        {
            hr = m_taskbarInterface->HrInit();

            if (FAILED(hr))
            {
                m_taskbarInterface->Release();
                m_taskbarInterface = NULL;
            }
        }
    }

    void setState(WId w, TBSTATE tb_state)
    {
        if (m_taskbarInterface)
        {
            switch(tb_state)
            {
            case S_INTERMEDIATE:
                m_taskbarInterface->SetProgressState(w, TBPF_INDETERMINATE);
                break;
            case S_NORM:
                m_taskbarInterface->SetProgressState(w, TBPF_NORMAL);
                break;
            case S_ERROR:
                m_taskbarInterface->SetProgressState(w, TBPF_ERROR);
                break;
            case S_PAUSED:
                m_taskbarInterface->SetProgressState(w, TBPF_PAUSED);
                break;
            default:
                m_taskbarInterface->SetProgressState(w, TBPF_NOPROGRESS);
                break;
            }
        }
    }

    void setProgress(WId w, quint64 completed, quint64 total)
    {
        if (m_taskbarInterface)
        {
           m_taskbarInterface->SetProgressValue(w, (completed < total)?completed:total, total);
        }
    }

    void setOverlayIcon(WId w, const QIcon& icon, const QString& description)
    {
        if (m_taskbarInterface)
        {
            HICON overlay_icon = icon.isNull() ? NULL : icon.pixmap(48).toWinHICON();
            m_taskbarInterface->SetOverlayIcon(w, overlay_icon, description.toStdWString().c_str());
            if (overlay_icon)
            {
                DestroyIcon(overlay_icon);
            }
        }
    }
};

#else

// old windows
class taskbar_iface::TaskBarIface
{
public:
    TaskBarIface(){}
    void setState(WId w, TBSTATE tb_state){}
    void setProgress(WId w, quint64 completed, quint64 total) {}
    void setOverlayIcon(WId w, const QIcon& icon, const QString& description){}
};

#endif

#else
// linux
class taskbar_iface::TaskBarIface
{
public:
    TaskBarIface(){}
    void setState(WId w, TBSTATE tb_state){}
    void setProgress(WId w, quint64 completed, quint64 total) {}
    void setOverlayIcon(WId w, const QIcon& icon, const QString& description){}
};

#endif


taskbar_iface::taskbar_iface(QObject *parent, quint64 total) :
    QObject(parent), m_total(total)
{
}

taskbar_iface::~taskbar_iface()
{
    if (m_pdelegate) delete m_pdelegate;
}

void taskbar_iface::initialize()
{
    m_pdelegate = new taskbar_iface::TaskBarIface;
}

void taskbar_iface::setState(WId w, TBSTATE tb_state)
{
    if (m_pdelegate) m_pdelegate->setState(w, tb_state);
}

void taskbar_iface::setProgress(WId w, quint64 completed)
{
    if (m_pdelegate) m_pdelegate->setProgress(w, completed, m_total);
}

void taskbar_iface::setIcon(WId w, const QIcon& icon, const QString& description)
{
    if (m_pdelegate) m_pdelegate->setOverlayIcon(w, icon, description);
}
