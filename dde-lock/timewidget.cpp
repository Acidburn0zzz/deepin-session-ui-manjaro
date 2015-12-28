#include "timewidget.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QFontDatabase>

TimeWidget::TimeWidget(QWidget *parent)
    : QWidget(parent)
{
    int id = QFontDatabase::addApplicationFont(":/fonts/fonts/MavenProLight-200.otf");
    const QString fontFamily = QFontDatabase::applicationFontFamilies(id).first();
    const QFont timeFont(fontFamily);

    const QFont dateFont("Source Han Sans SC");

    m_timeLabel = new QLabel;
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setAlignment(Qt::AlignLeft);
    m_timeLabel->setStyleSheet("color:white;"
                               "font-size:68px;");
    m_dateLabel = new QLabel;
    m_dateLabel->setFont(dateFont);
    m_dateLabel->setAlignment(Qt::AlignLeft);
    m_dateLabel->setStyleSheet("color:white;"
                               "padding:3px 0;"
                               "font-size:21px;");
    refreshTime();

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);
    m_refreshTimer->start();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addStretch();
    vLayout->addWidget(m_timeLabel);
    vLayout->addWidget(m_dateLabel);
    vLayout->setSpacing(0);
    vLayout->setMargin(0);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(vLayout);
    mainLayout->addStretch();
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    setLayout(mainLayout);

    connect(m_refreshTimer, &QTimer::timeout, this, &TimeWidget::refreshTime);
}

void TimeWidget::refreshTime()
{
    m_timeLabel->setText(QDateTime::currentDateTime().toString(tr("hh:mm")));
    m_dateLabel->setText(QDateTime::currentDateTime().toString(tr("yyyy-MM-dd dddd")));
}

