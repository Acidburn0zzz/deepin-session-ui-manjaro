#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QTextCodec>
#include <QDebug>
#include <QSettings>
#include <QStringList>

#include "logowidget.h"
LogoWidget::LogoWidget(QWidget* parent)
    : QFrame(parent)
{
    initUI();
}

void LogoWidget::initUI() {
    setFixedSize(240, 40);

    m_logoLabel = new QLabel();
    m_logoLabel->setObjectName("Logo");
    m_logoLabel->setFixedSize(150, 38);

    m_logoVersionLabel = new QLabel;
    m_logoVersionLabel->setObjectName("LogoVersion");

    this->setObjectName("LogoWidget");

    m_logoRightSideLayout = new QVBoxLayout;
    m_logoRightSideLayout->setMargin(0);
    m_logoRightSideLayout->setSpacing(0);
    m_logoRightSideLayout->addWidget(m_logoVersionLabel);
    m_logoRightSideLayout->addStretch();

    m_logoLayout = new QHBoxLayout;
    m_logoLayout->setMargin(0);
    m_logoLayout->setSpacing(0);
    m_logoLayout->addWidget(m_logoLabel);
    m_logoLayout->addLayout(m_logoRightSideLayout);
    m_logoLayout->addStretch();
    setLayout(m_logoLayout);

    QString systemVersion = getVersion();
    m_logoVersionLabel->setText(systemVersion);
}

QString LogoWidget::getVersion() {
    QString versionNumber;

    QFile lsbRelease( "/etc/lsb-release" );
    if ( !lsbRelease.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
        versionNumber = "Manjaro Linux";
    } else {
        QString version = "Rolling";
        QString codename = "Manjaro Linux";
        QTextStream in( &lsbRelease );
        while ( !in.atEnd() )
        {
            QString line = in.readLine();
            if ( line.startsWith("DISTRIB_RELEASE="))
                version = line.trimmed().split("=").value(1);
            if ( line.startsWith("DISTRIB_CODENAME="))
                codename = line.trimmed().split("=").value(1);
        }
        lsbRelease.close();
        versionNumber = version + " " + codename;
    }

    qDebug() << "Manjaro-Deepin Version: " << versionNumber;

    QFont versionFont; QFontMetrics fm(versionFont);
    int width=fm.width(versionNumber);
    setFixedSize(150+width, 40);
    return versionNumber;
}

LogoWidget::~LogoWidget()
{
}
