#ifndef SESSIONBUTTON
#define SESSIONBUTTON
#include <QObject>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QtWidgets>
#include <QFocusEvent>
#include <QGraphicsDropShadowEffect>

#include "util_signalmanager.h"

class SessionButton: public QPushButton
{
    Q_OBJECT
public:
    SessionButton(QString text, QString buttonId, QWidget* parent=0);
    ~SessionButton();
protected:
    void paintEvent(QPaintEvent* event);
    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
signals:
    void buttonAction(QString id);
    void updateStyle();
public slots:
    bool isChecked() const;
    void setHover(bool isHover);
    void setChecked(bool checked);
    void setButtonMutex(QString buttonName);
    void setButtonHoverMutex(QString buttonName);
    inline QString buttonId() {
        return m_iconLabel->objectName();
    }
private:
    void initUI();
    void initConnect();
    void addTextShadow();

    bool m_checked = false;
    QString m_buttonId;
    QString m_buttonText;
    QLabel* m_iconLabel;
    QLabel* m_contentTextLabel;
    QVBoxLayout* m_Layout;
    QHBoxLayout* m_ButtonLayout;
};
#endif // SESSIONBUTTON