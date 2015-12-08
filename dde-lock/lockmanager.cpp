#include "userwidget.h"
#include "passwdedit.h"
#include "util_updateui.h"
#include "dbus/dbusinputdevices.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QDebug>
#include <QMap>

#include "lockmanager.h"

LockManager::LockManager(QWidget* parent)
    : QFrame(parent)
{

    initUI();
    initBackend();
    updateUI();
    initConnect();


    loadMPRIS();
    leaveEvent(nullptr);
}
void LockManager::initConnect() {

    connect(m_passwordEdit, &PassWdEdit::keybdLayoutButtonClicked, this, &LockManager::keybdLayoutWidgetPosit);
    connect(m_controlWidget, &ControlWidget::shutdownClicked, this, &LockManager::shutdownMode);
    connect(m_requireShutdownWidget, &ShutdownWidget::shutDownWidgetAction, [this] (const ShutdownWidget::Actions action) {
        switch (action) {
        case ShutdownWidget::RequireRestart:    m_action = Restart;         break;
        case ShutdownWidget::RequireShutdown:   m_action = Shutdown;        break;
        case ShutdownWidget::RequireSuspend:    m_action = Suspend;         break;
        }

        passwordMode();
    });
    connect(qApp, &QApplication::aboutToQuit, [this]{
        m_hotZoneInterface->EnableZoneDetected(true);
    });

    connect(m_passwordEdit, &PassWdEdit::leftKeyPressed, this, &LockManager::leftKeyPressed);
    connect(m_passwordEdit, &PassWdEdit::rightKeyPressed, this, &LockManager::rightKeyPressed);
}

void LockManager::keybdLayoutWidgetPosit() {
    m_keybdArrowWidget->show(m_passwordEdit->x() + 123, m_passwordEdit->y() + m_passwordEdit->height() - 15);
}

void LockManager::leftKeyPressed() {
    if (!m_requireShutdownWidget->isHidden()) {
        m_requireShutdownWidget->leftKeySwitch();
    }
}

void LockManager::rightKeyPressed() {
    if (!m_requireShutdownWidget->isHidden()) {
        m_requireShutdownWidget->rightKeySwitch();
    }
}
void LockManager::initUI() {
    setFixedSize(qApp->desktop()->size());
    setFocusPolicy(Qt::NoFocus);

    m_timeWidget = new TimeWidget(this);
    m_timeWidget->setFixedSize(400, 300);

    m_userWidget = new UserWidget(this);
    recordPid();
    m_passwordEdit = new PassWdEdit(this);
    m_passwordEdit->setEnterBtnStyle(":/img/unlock_normal.png", ":/img/unlock_normal.png", ":/img/unlock_press.png");
    m_passwordEdit->setFocusPolicy(Qt::StrongFocus);
    m_passwordEdit->setFocus();

    m_requireShutdownWidget = new ShutdownWidget(this);
    m_requireShutdownWidget->hide();
    m_requireShutdownWidget->setFixedWidth(width());
    m_requireShutdownWidget->setFixedHeight(300);


    m_controlWidget = new ControlWidget(this);
    m_controlWidget->hideMusicControlWidget();
    m_controlWidget->setUserSwitchEnable(m_userWidget->count() > 1);


    QHBoxLayout *passwdLayout = new QHBoxLayout;
    passwdLayout->setMargin(0);
    passwdLayout->setSpacing(0);
    passwdLayout->addStretch();
    passwdLayout->addWidget(m_passwordEdit);
    passwdLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addStretch();
    mainLayout->addLayout(passwdLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);
    showFullScreen();
    activateWindow();
    updateWidgetsPosition();
    updateStyle(":/skin/lock.qss", this);

    m_lockInter = new DBusLockService(LOCKSERVICE_NAME, LOCKSERVICE_PATH, QDBusConnection::systemBus(), this);
    qDebug() << "DBusLockService" << m_lockInter->IsLiveCD(m_userWidget->currentUser());
    connect(m_passwordEdit, &PassWdEdit::submit, this, &LockManager::unlock);
}

void LockManager::updateWidgetsPosition() {
    qDebug() << "lockManager: positon" <<this->width() << this->height();
    const int width = this->width();
    const int height = this->height();
    m_userWidget->setFixedWidth(width);
    m_timeWidget->move(48, height - m_timeWidget->height() - 36); // left 48px and bottom 36px
    m_userWidget->move(0, (height - m_userWidget->height()) / 2 - 95);
    m_requireShutdownWidget->move(0,  (height - m_requireShutdownWidget->height())/2 - 50);
    m_controlWidget->move(width - m_controlWidget->width() - 50,
                          height - m_controlWidget->height() - 36); // margin right 50 margin bottom 36

}

void LockManager::leaveEvent(QEvent *) {
    QList<QScreen *> screenList = qApp->screens();
    QPoint mousePoint = QCursor::pos();
    for (const QScreen *screen : screenList)
    {
        if (screen->geometry().contains(mousePoint))
        {
            const QRect &geometry = screen->geometry();
            setFixedSize(geometry.size());
            emit screenChanged(geometry);
            return;
        }
    }
}

void LockManager::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Escape:
        if (!m_requireShutdownWidget->isHidden()) {
            m_requireShutdownWidget->hide();
            m_userWidget->show();
            m_passwordEdit->show();
        }
#ifdef QT_DEBUG
//        qApp->quit();   break;
#endif
            default:;
    }
}

void LockManager::mouseReleaseEvent(QMouseEvent *e)
{
    qDebug() << "ReleaseEvent";
    m_action = Unlock;
    passwordMode();
    if (e->button() == Qt::LeftButton) {
        if (!m_keybdArrowWidget->isHidden()) {
            m_keybdArrowWidget->hide();
        }
    }
}

void LockManager::loadMPRIS()
{
    if (m_mprisInter)
        m_mprisInter->deleteLater();

    QDBusInterface *dbusInter = new QDBusInterface("org.freedesktop.DBus", "/", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);
    if (!dbusInter)
        return;

    QDBusReply<QStringList> response = dbusInter->call("ListNames");
    const QStringList &serviceList = response.value();
    QString service = QString();
    for (const QString &serv : serviceList)
    {
        if (!serv.startsWith("org.mpris.MediaPlayer2."))
            continue;
        service = serv;
        break;
    }

    if (service.isEmpty())
        return;

    qDebug() << "got service: " << service;

    m_mprisInter = new DBusMediaPlayer2(service, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), this);
    m_controlWidget->bindDBusService(m_mprisInter);
    m_controlWidget->showMusicControlWidget();
}

void LockManager::recordPid() {
    qDebug() << "DDE-lock remember PID" << qApp->applicationPid();
    const QString &username = m_userWidget->currentUser();
    QFile lockPIdFile(QString("%1%2%3").arg("/home/").arg(username).arg("/.dlockpid"));

    if (lockPIdFile.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream pidInfo(&lockPIdFile);
        pidInfo << qApp->applicationPid();

        lockPIdFile.close();
    }
}

void LockManager::unlock()
{
    if (!m_requireShutdownWidget->isHidden()) {
        m_requireShutdownWidget->shutdownAction();
        return;
    }

    m_userWidget->showLoadingAni();

    qDebug() << "unlock" << m_userWidget->currentUser() << m_passwordEdit->getText();
    const QString &username = m_userWidget->currentUser();
    const QString &password = m_passwordEdit->getText();

    QDBusPendingReply<bool> result = m_lockInter->UnlockCheck(username, password);
    result.waitForFinished();

    qDebug() << "dde-lock unlock^^^" << m_lockInter->isValid()
             << m_lockInter->lastError()
             << result.error() << result.value();

    if (result.error().type() != QDBusError::NoError || !result.value())
    {
        // Auth fail
        m_userWidget->hideLoadingAni();
        m_passwordEdit->setAlert(true, tr("Wrong Password"));

        return;
    }

    // Auth success
    switch (m_action)
    {
    case Unlock:        qApp->quit();                              break;
    case Restart:       m_sessionManagerIter->RequestReboot();      break;
    case Shutdown:      m_sessionManagerIter->RequestShutdown();    break;
    case Suspend:       m_sessionManagerIter->RequestSuspend();     break;
    }
}

void LockManager::initBackend() {
    m_hotZoneInterface = new DBusHotzone("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone",
                                                        QDBusConnection::sessionBus(), this);
    m_hotZoneInterface->EnableZoneDetected(false);



    DBusInputDevices * dbusInputDevices = new DBusInputDevices(this);
    foreach (InputDevice device, dbusInputDevices->infos()) {
        if (device.deviceType == "keyboard") {
            m_keyboardLayoutInterface = new DBusKeyboard(this);
            break;
        }
    }

    m_keybdLayoutNameList = m_keyboardLayoutInterface->userLayoutList();

    for (int i = 0; i < m_keybdLayoutNameList.length(); i++) {
        QDBusPendingReply<QString> tmpValue =  m_keyboardLayoutInterface->GetLayoutDesc(m_keybdLayoutNameList[i]);
        tmpValue.waitForFinished();

        keybdLayoutDescList << tmpValue;
        m_keybdInfoMap.insert(m_keybdLayoutNameList[i], tmpValue);
    }
    qDebug() << "QStringList" << m_keybdLayoutNameList;
    m_passwordEdit->updateKeybdLayoutUI(keybdLayoutDescList);

    m_sessionManagerIter = new DBusSessionManagerInterface("com.deepin.SessionManager", "/com/deepin/SessionManager",
                                                           QDBusConnection::sessionBus(), this);
}

void LockManager::updateUI() {
    m_keybdLayoutWidget = new KbLayoutWidget(keybdLayoutDescList);
    m_keybdLayoutWidget->setFixedWidth(DDESESSIONCC::PASSWDLINEEIDT_WIDTH);

    m_keybdArrowWidget = new DArrowRectangle(DArrowRectangle::ArrowTop, this);

    m_keybdArrowWidget->setBackgroundColor(QColor(0, 0, 0, 78));
    m_keybdArrowWidget->setBorderColor(QColor(0, 0, 0, 100));
    m_keybdArrowWidget->setMargin(1);
    m_keybdArrowWidget->setArrowX(13);
    m_keybdArrowWidget->setArrowWidth(12);
    m_keybdArrowWidget->setArrowHeight(6);
    m_keybdArrowWidget->setContent(m_keybdLayoutWidget);
    m_keybdLayoutWidget->setParent(m_keybdArrowWidget);
    m_keybdLayoutWidget->show();
    m_keybdArrowWidget->move(m_passwordEdit->x() + 123, m_passwordEdit->y() + m_passwordEdit->height() - 15);
    m_keybdArrowWidget->hide();

    connect(m_keybdLayoutWidget, &KbLayoutWidget::setButtonClicked, this, &LockManager::setCurrentKeyboardLayout);
    connect(m_keybdLayoutWidget, &KbLayoutWidget::setButtonClicked, m_keybdArrowWidget, &DArrowRectangle::hide);
}

void LockManager::setCurrentKeyboardLayout(QString keyboard_value) {
    QMap<QString, QString>::iterator lookupIt;

    for (lookupIt = m_keybdInfoMap.begin(); lookupIt != m_keybdInfoMap.end(); ++lookupIt) {
        if (lookupIt.value() == keyboard_value && m_keyboardLayoutInterface->currentLayout() != lookupIt.key()) {

            m_keyboardLayoutInterface->setCurrentLayout(lookupIt.key());
        }
    }

}

void LockManager::passwordMode()
{
    m_userWidget->show();
    m_passwordEdit->show();
    m_passwordEdit->setFocus();
    m_requireShutdownWidget->hide();

    if (m_action == Suspend)
    {
        m_sessionManagerIter->RequestSuspend().waitForFinished();
        return;
    }

    if (m_action == Restart) {
        m_passwordEdit->setAlert(true, tr("Enter your password to restart"));
        m_passwordEdit->setEnterBtnStyle(":/img/restartIcon_normal.png", ":/img/restartIcon_normal.png", ":/img/restartIcon_press.png");
    } else if (m_action == Shutdown) {
        m_passwordEdit->setAlert(true, tr("Enter your passwrod to shutdown"));
        m_passwordEdit->setEnterBtnStyle(":/img/shutdownIcon_normal.png", ":/img/shutdownIcon_normal.png", ":/img/shutdownIcon_press.png");
    } else if (m_action == Unlock) {
        m_passwordEdit->setEnterBtnStyle(":/img/unlock_normal.png", ":/img/unlock_normal.png", ":/img/unlock_press.png");
    }
}

void LockManager::shutdownMode()
{
    m_userWidget->hide();
    m_passwordEdit->hide();
    m_requireShutdownWidget->show();
}
