#include <QHBoxLayout>
#include <QEvent>
#include <QWheelEvent>

#include "controlwidget.h"

ControlWidget::ControlWidget(QWidget *parent)
    : QWidget(parent)
{
    m_songName = new QLabel;
    m_songName->setAlignment(Qt::AlignCenter);
    m_songName->setStyleSheet("color:white;"
//                              "background-color:red;"
                              "font-size:15px;"
                              "margin:0 35px 10px 5px;");
    m_volumeNums = new QLabel;
    m_volumeNums->hide();
    m_volumeNums->setStyleSheet("color:white;"
//                                "background-color:red;"
                                "font-size:13px;");
    m_prevSong = new DImageButton;
    m_prevSong->setNormalPic(":/icons/previous_normal.png");
    m_prevSong->setHoverPic(":/icons/previous_hover.png");
    m_prevSong->setPressPic(":/icons/previous_press.png");

    m_pauseSong = new DImageButton;
    m_pauseSong->setNormalPic(":/icons/pause_normal.png");
    m_pauseSong->setHoverPic(":/icons/pause_hover.png");
    m_pauseSong->setPressPic(":/icons/pause_press.png");

    m_nextSong = new DImageButton;
    m_nextSong->setNormalPic(":/icons/next_normal.png");
    m_nextSong->setHoverPic(":/icons/next_hover.png");
    m_nextSong->setPressPic(":/icons/next_press.png");
    m_volume = new DImageButton;
    m_volume->setNormalPic(":/icons/volume_normal.png");
    m_volume->setHoverPic(":/icons/volume_hover.png");
    m_volume->setPressPic(":/icons/volume_press.png");
    m_volume->installEventFilter(this);


    m_userswitch = new DImageButton;
    m_userswitch->setNormalPic(":/icons/userswitch_normal.png");
    m_userswitch->setHoverPic(":/icons/userswitch_hover.png");
    m_userswitch->setPressPic(":/icons/userswitch_press.png");


    m_shutdown = new DImageButton;
    m_shutdown->setNormalPic(":/icons/shutdown_normal.png");
    m_shutdown->setHoverPic(":/icons/shutdown_hover.png");
    m_shutdown->setPressPic(":/icons/shutdown_press.png");


    QHBoxLayout *volumeLayout = new QHBoxLayout;
    volumeLayout->addWidget(m_volumeNums);
    volumeLayout->setSpacing(0);
    volumeLayout->setMargin(0);

    QWidget *volumeWidget = new QWidget;
    volumeWidget->setFixedWidth(30);
    volumeWidget->setLayout(volumeLayout);

    QHBoxLayout *songCtrls = new QHBoxLayout;
    songCtrls->addWidget(m_prevSong);
    songCtrls->addWidget(m_pauseSong);
    songCtrls->addWidget(m_nextSong);
    songCtrls->addWidget(m_volume);
    songCtrls->addWidget(volumeWidget);
    songCtrls->setSpacing(0);
    songCtrls->setMargin(0);

    QVBoxLayout *songLayout = new QVBoxLayout;
    songLayout->addWidget(m_songName);
    songLayout->addStretch();
    songLayout->addLayout(songCtrls);
    songLayout->setSpacing(0);
    songLayout->setMargin(0);

    m_songControlWidget = new QWidget;
    m_songControlWidget->setLayout(songLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addStretch();
    mainLayout->addWidget(m_songControlWidget);
    mainLayout->setAlignment(m_songControlWidget, Qt::AlignBottom);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_userswitch);
    mainLayout->setAlignment(m_userswitch, Qt::AlignBottom);
    mainLayout->addWidget(m_shutdown);
    mainLayout->setAlignment(m_shutdown, Qt::AlignBottom);

    setLayout(mainLayout);
    setFixedSize(500, 150);

    connect(m_shutdown, &DImageButton::clicked, this, &ControlWidget::shutdownClicked);
    connect(m_userswitch, &DImageButton::clicked, this, &ControlWidget::switchToGreeter);
}

void ControlWidget::bindDBusService(DBusMediaPlayer2 *dbusInter)
{
    m_dbusInter = dbusInter;

    connect(m_prevSong, &DImageButton::clicked, m_dbusInter, &DBusMediaPlayer2::Previous);
    connect(m_nextSong, &DImageButton::clicked, m_dbusInter, &DBusMediaPlayer2::Next);
    connect(m_pauseSong, &DImageButton::clicked, m_dbusInter, &DBusMediaPlayer2::PlayPause);
    connect(m_dbusInter, &DBusMediaPlayer2::PlaybackStatusChanged, this, &ControlWidget::changePauseBtnPic);
    connect(m_dbusInter, &DBusMediaPlayer2::VolumeChanged, this, &ControlWidget::changeVolumeBtnPic);
    connect(m_dbusInter, &DBusMediaPlayer2::VolumeChanged, [this] {
        m_volumeNums->setText(QString::number(m_dbusInter->volume() * 100));
    });
    connect(m_dbusInter, &DBusMediaPlayer2::MetadataChanged, [this] {
        const QString text = m_dbusInter->metadata().value("xesam:title").toString();
        QFontMetrics qfm(m_songName->font());

        if (qfm.width(text) > m_songName->width())
            m_songName->setText(qfm.elidedText(text, Qt::ElideRight, m_songName->width()));
        else
            m_songName->setText(text);
    });
    connect(m_volume, &DImageButton::clicked, [this] {
        const double currentVolume = m_dbusInter->volume();

        if (currentVolume) {
            m_lastVolumeNums = currentVolume;
            m_dbusInter->setVolume(0);
        } else {
            m_dbusInter->setVolume(m_lastVolumeNums);
        }
    });

    m_dbusInter->VolumeChanged();
    m_dbusInter->MetadataChanged();
    m_dbusInter->PlaybackStatusChanged();
    m_dbusInter->VolumeChanged();
}

void ControlWidget::hideMusicControlWidget()
{
    m_songControlWidget->hide();
}

void ControlWidget::showMusicControlWidget()
{
    m_songControlWidget->show();
}

bool ControlWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_volume) {
        switch (e->type()) {
        case QEvent::Wheel:     volumeWheelControl(reinterpret_cast<QWheelEvent *>(e));         break;
        case QEvent::Enter:     m_volumeNums->show();       break;
        case QEvent::Leave:     m_volumeNums->hide();       break;
        default:;
        }
    }

    return false;
}

void ControlWidget::volumeWheelControl(const QWheelEvent *e)
{
    const int delta = e->delta() / 60;

    m_lastVolumeNums += double(delta) / 100;
    if (m_lastVolumeNums < 0)
        m_lastVolumeNums = 0;
    if (m_lastVolumeNums > 1.0)
        m_lastVolumeNums = 1.0;

    m_dbusInter->setVolume(m_lastVolumeNums);
}

void ControlWidget::changePauseBtnPic()
{
    if (m_dbusInter->playbackStatus() == "Playing") {
        m_pauseSong->setNormalPic(":/icons/pause_normal.png");
        m_pauseSong->setHoverPic(":/icons/pause_hover.png");
        m_pauseSong->setPressPic(":/icons/pause_press.png");
    } else {
        m_pauseSong->setNormalPic(":/icons/start_normal.png");
        m_pauseSong->setHoverPic(":/icons/start_hover.png");
        m_pauseSong->setPressPic(":/icons/start_press.png");
    }
}

void ControlWidget::changeVolumeBtnPic()
{
    if (m_dbusInter->volume()) {
        m_volume->setNormalPic(":/icons/volume_normal.png");
        m_volume->setHoverPic(":/icons/volume_hover.png");
        m_volume->setPressPic(":/icons/volume_press.png");
    } else {
        m_volume->setNormalPic(":/icons/mute_normal.png");
        m_volume->setHoverPic(":/icons/mute_hover.png");
        m_volume->setPressPic(":/icons/mute_press.png");
    }
}

void ControlWidget::switchToGreeter() {
    m_utilFile = new UtilFile(this);
    m_utilFile->setExpandState(1);

    QProcess *process = new QProcess;
    process->start("dde-switchtogreeter");
    process->waitForFinished();
    process->deleteLater();

    // FIXME: 这儿要是不延时结束的话会闪一下桌面
    QTimer::singleShot(2000, qApp, SLOT(quit()));
}
