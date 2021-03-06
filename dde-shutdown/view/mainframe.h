#ifndef ContentFrame
#define ContentFrame
#include <QtWidgets/QWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtCore>
#include "contentwidget.h"

class MainFrame : public QFrame
{
    Q_OBJECT
public:
    MainFrame(QWidget* parent=0);
    ~MainFrame();

    ShutDownFrame* m_shutdownFrame;
    QHBoxLayout* m_Layout;
    QVBoxLayout* m_contentLayout;
    QVBoxLayout* m_contentRightLayout;
signals:
    void OutKeyLeft();
    void OutKeyRight();
    void pressEnterAction();

public slots:
    void changeView(bool a);
    void DirectLeft();
    void DirectRight();
    void EnterAction();
    void hideBtns(const QStringList &btnsName);
    void disableBtns(const QStringList &btnsName);

private:
    void initUI();
    void initConnect();

};

#endif // ContentFrame
