#ifndef CLICKED_LABEL_H
#define CLICKED_LABEL_H

#include <QLabel>
#include <QMouseEvent>

class clicked_label : public QLabel
{
    Q_OBJECT
public:
    clicked_label(QWidget* parent = 0): QLabel(parent)
    {
    }
    
    ~clicked_label()
    {
    }

signals:
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent * e)
    {
        if (e->button() == Qt::LeftButton)
        {
            emit doubleClicked();
            QLabel::mouseDoubleClickEvent(e);
        }
    }
};

#endif
