class MyTabWidget : public QTabWidget
{
public:
    MyTabWidget(QWidget* parent = 0) : QTabWidget(parent)
    { }
    ~MyTabWidget()
    {}

    void setTabFontColor(int nTab, const QColor& color)
    {
        tabBar()->setTabTextColor(nTab, color);
    }
};
