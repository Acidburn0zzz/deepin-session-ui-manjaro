#include "loginmanager.h"

#include <QtCore/QObject>
#include <QApplication>
#include <QtCore/QFile>
#include <QDesktopWidget>
#include <QDebug>

LoginManager::LoginManager(QWidget* parent)
    : QFrame(parent),
      m_greeter(new QLightDM::Greeter(this))
{
    if (!m_greeter->connectSync())
        qWarning() << "greeter connect fail !!!";

    initUI();
    initConnect();

    m_passWdEdit->updateKeybordLayoutStatus(m_userWidget->currentUser());
    m_sessionWidget->switchToUser(m_userWidget->currentUser());
}

LoginManager::~LoginManager()
{
}

void LoginManager::initUI()
{
    setFixedSize(qApp->desktop()->size());
    setObjectName("LoginManagerTool");
    setFocusPolicy(Qt::StrongFocus);
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
    setFocusPolicy(Qt::NoFocus);

    m_backgroundLabel = new BackgroundLabel(true, this);
    m_sessionWidget = new SessionWidget(this);
    m_sessionWidget->hide();
    m_sessionWidget->move(0, (height() - m_sessionWidget->height()) / 2 - 70); // 中间稍往上的位置
    m_logoWidget = new LogoWidget(this);
    m_logoWidget->move(48, height() - m_logoWidget->height() - 36); // left 48px and bottom 36px
//    m_logoWidget->setStyleSheet("background-color:red;");
    m_switchFrame = new SwitchFrame(this);
    m_switchFrame->move(width() - m_switchFrame->width(), height() - m_switchFrame->height());
    m_userWidget = new UserWidget(this);
    m_userWidget->move(0, (qApp->desktop()->height() - m_userWidget->height()) / 2 - 95); // center and margin-top: -95px
    m_userWidget->setObjectName("UserWidget");
    m_passWdEdit = new PassWdEdit("LoginIcon", this);
    m_passWdEdit->setFocusPolicy(Qt::StrongFocus);
    m_passWdEdit->setFocus();

    m_passWdEditLayout = new QHBoxLayout;
    m_passWdEditLayout->setMargin(0);
    m_passWdEditLayout->setSpacing(0);
    m_passWdEditLayout->addStretch();
    m_passWdEditLayout->addWidget(m_passWdEdit);
    m_passWdEditLayout->addStretch();

    m_Layout = new QVBoxLayout;
    m_Layout->setMargin(0);
    m_Layout->setSpacing(0);
    m_Layout->addStretch();
    m_Layout->addLayout(m_passWdEditLayout);
    m_Layout->addStretch();
    setLayout(m_Layout);
    updateStyle(":/skin/login.qss", this);
    showFullScreen();

    m_passWdEdit->updateKeyboardStatus();
    keyboardLayoutUI();
}

void LoginManager::initConnect()
{
    connect(m_switchFrame, &SwitchFrame::triggerSwitchSession, this, &LoginManager::chooseSessionMode);
    connect(m_switchFrame, &SwitchFrame::triggerSwitchUser, this, &LoginManager::chooseUserMode);
    connect(m_switchFrame, &SwitchFrame::triggerSwitchUser, m_userWidget, &UserWidget::expandWidget);
    connect(m_passWdEdit, &PassWdEdit::submit, this, &LoginManager::login);
    connect(m_sessionWidget, &SessionWidget::sessionChanged, this, &LoginManager::chooseUserMode);
    connect(m_userWidget, &UserWidget::userChanged, m_sessionWidget, &SessionWidget::switchToUser);
    connect(m_userWidget, &UserWidget::userChanged, m_passWdEdit, &PassWdEdit::updateKeybordLayoutStatus);
    connect(m_userWidget, &UserWidget::userChanged, m_passWdEdit, static_cast<void (PassWdEdit::*)()>(&PassWdEdit::setFocus));
    connect(m_greeter, &QLightDM::Greeter::showPrompt, this, &LoginManager::prompt);
    connect(m_greeter, &QLightDM::Greeter::authenticationComplete, this, &LoginManager::authenticationComplete);
    connect(m_passWdEdit, &PassWdEdit::updateKeyboardStatus, this, &LoginManager::keyboardLayoutUI);
    connect(m_passWdEdit, SIGNAL(keybdLayoutButtonClicked()), this, SLOT(keybdLayoutWidget()));
}

void LoginManager::prompt(QString text, QLightDM::Greeter::PromptType type)
{
    qDebug() << "prompt: " << text << type;

    switch (type)
    {
    case QLightDM::Greeter::PromptTypeSecret:   m_greeter->respond(m_passWdEdit->getText().trimmed());     break;
    default:;
    }
}

void LoginManager::authenticationComplete()
{
    qDebug() << "authenticationComplete";

    if (!m_greeter->isAuthenticated()) {
        qWarning() << "auth fail !!!";
        return;
    }

    qDebug() << "session = " << m_sessionWidget->currentSessionName();
    qDebug() << "start session: " << m_greeter->startSessionSync(m_sessionWidget->currentSessionKey());
}

void LoginManager::chooseUserMode()
{
    m_sessionWidget->hide();
    m_userWidget->show();
    m_passWdEdit->show();
}

void LoginManager::chooseSessionMode()
{
    m_sessionWidget->show();
    m_userWidget->hide();
    m_passWdEdit->hide();
}

void LoginManager::keyPressEvent(QKeyEvent* e) {
#ifndef QT_DEBUG
    Q_UNUSED(e)
#else
    if (e->key() == Qt::Key_Escape) {
        qApp->exit();
    }

#endif
}

void LoginManager::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        if (!m_keybdLayoutWidget->isHidden()) {
            m_keybdLayoutWidget->hide();
        }
    }

}

void LoginManager::login()
{
    if (m_greeter->inAuthentication())
        m_greeter->cancelAuthentication();

    const QString &username = m_userWidget->currentUser();
    m_greeter->authenticate(username);
    qDebug() << "choose user: " << username;
    qDebug() << "auth user: " << m_greeter->authenticationUser();
}

void LoginManager::keyboardLayoutUI() {

    QStringList keyboardList;
    QMap<QString, QString>::iterator i ;

    for(i = m_passWdEdit->keybdLayoutMainDescriptionMap.begin(); i != m_passWdEdit->keybdLayoutMainDescriptionMap.end(); i++) {
        if (! m_passWdEdit->keybdLayoutMainDescriptionMap.value(i.key()).isEmpty()) {
            keyboardList << m_passWdEdit->keybdLayoutMainDescriptionMap.value(i.key());
        }
    }

    m_keybdLayoutWidget= new KbLayoutWidget(keyboardList, this);
    m_keybdLayoutWidget->move(m_passWdEdit->x(), 500);
    m_keybdLayoutWidget->hide();
}

void LoginManager::keybdLayoutWidget() {
    if (m_keybdLayoutWidget->isHidden()) {
        m_keybdLayoutWidget->show();
    } else {
        m_keybdLayoutWidget->hide();
    }

}