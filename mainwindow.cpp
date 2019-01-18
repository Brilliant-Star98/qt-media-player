#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <VLCQtWidgets/WidgetSeek.h>
#include <VLCQtWidgets/WidgetVideo.h>
#include <VLCQtCore/Enums.h>
#include <VLCQtCore/VideoDelegate.h>
#include <VLCQtCore/Common.h>
#include <VLCQtCore/Instance.h>
#include <VLCQtCore/Media.h>
#include <VLCQtCore/MediaPlayer.h>
#include <QVBoxLayout>
#include <QDir>
#include <Windows.h>
#include <QPalette>
#include <QDebug>
#include <QPixmap>
#include <QMessageBox>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _media(0)
{
    ui->setupUi(this);
    //environment updating
    ui->menuBar->setVisible(false);
    ui->mainToolBar->setVisible(false);
    ui->statusBar->setVisible(false);

    m_vlcPlayerCount = 0;
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    this->raise();
    this->setFocus();
    QApplication::setOverrideCursor(Qt::BlankCursor);


    showFullScreen();

    qDebug() << this->width() << ' ' << this->height();
    ui->widget->setGeometry(QRect(-9, -9, this->width() + 18, this->height() + 18));

    //AllowSetForegroundWindow(NULL);
    //SetForegroundWindow((HWND)winId());
    //setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    h_PlayLayout = new QBoxLayout(QBoxLayout::LeftToRight, ui->widget);
    h_PlayLayout->setGeometry(QRect(-9, -9, this->width() + 18, this->height() + 18));

    // Create Media Folder
    QDir dir(QDir::currentPath() + "/media");
    if (!dir.exists())
        dir.mkpath(".");

    _instance = new VlcInstance(VlcCommon::args(), this);
    _vplayer = new VlcMediaPlayer(_instance);

    connect(_vplayer, SIGNAL(end()), this, SLOT(playFinished()));

    //Download Configuration
    pManager = new QNetworkAccessManager();
    QObject::connect(pManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(configureReceived(QNetworkReply*)));

    manager = new DownloadManager();
    QObject::connect(manager, SIGNAL(finished()), this, SLOT(finishedConfigureDownload()));

    //create a timer
    p_PlayTimer = new QTimer(this);
    //setup signal and slot
    QObject::connect(p_PlayTimer, SIGNAL(timeout()), this, SLOT(playNextOrder()));

    p_OrderPlayTimer = new QTimer(this);

    QObject::connect(p_OrderPlayTimer, SIGNAL(timeout()), this, SLOT(playNextInSameOrder()));

    p_OrderAllPlayTimer = new QTimer(this);

    QObject::connect(p_OrderAllPlayTimer, SIGNAL(timeout()), this, SLOT(playAllInSameOrder()));

    m_CurrentPlayOrder = 0;
    m_CurrentPlayInSameOrder = 0;
    coor_count = 0;

    initialize();

}

MainWindow::~MainWindow()
{
    delete _instance;
    delete ui;
}

void MainWindow::initialize()
{

    loadSettings();

    //ui->txtUrl->setText(mConfigureURL);
    //ui->txtPeriod->setText(QString::number(mCheckingPeriod));

    // Setup Max Client Count
    //mMaxClients = 100;
    //mClientCount = 0;
    m_IsDownloadingStatus = false;
    m_CurrentPlayOrder = 0;

    //updateConnectedClientCount();

    //mDisplayItems.clear();

    m_ConfigureInitialized = false;

    if (m_confUrl.length() != 0 && m_CurrentConfigure.length() != 0)
        getConfigure();

}


void MainWindow::finishedConfigureDownload()
{
    qDebug() << "Finished Configure Download!";
    //ui->downloading_label->setVisible(false);
    playNextOrder();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Q){
        p_PlayTimer->stop();
        this->close();
    }
    else if(event->key() == Qt::Key_L)
    {
        loadSettings();
        Settings *m_ConfigureDig = new Settings(this, m_confUrl);
        m_ConfigureDig->setWindowFlags(Qt::SplashScreen);
        if(m_ConfigureDig->exec() == QDialog::Accepted)
        {
            m_confUrl = m_ConfigureDig->getConfigureUrl();
            qDebug() << m_confUrl;
            //Download configure url
//            DownloadManager *d_Manager = new DownloadManager(this);
//            QUrl url(m_confUrl);
//            d_Manager->append(url);
            this->raise();
            this->setFocus();
            QApplication::setOverrideCursor(Qt::BlankCursor);

            m_OrderItems.clear();
            m_CurrentPlayOrder = 0;
            m_CurrentPlayInSameOrder = 0;
            coor_count = 0;
            p_PlayTimer->stop();
            p_OrderPlayTimer->stop();
            p_OrderAllPlayTimer->stop();
            getConfigure();
        }
    }
}

QStringList MainWindow::getContents(QString body, QString name)
{
    QStringList out;

    body = body.replace("\t", "");
    body = body.replace("\n", "");

    if(body.indexOf("<" + name + ">") >= 0)
    {
        QStringList contents_tmp = body.split("<" + name + ">");

        foreach(QString part, contents_tmp)
        {
            if(part.indexOf("</" + name + ">") > 0)
            {
                QStringList part_lst = part.split("</" + name + ">");
                if(part_lst.length() > 1)
                    out.push_back(QString(part_lst.at(0)));
            }
        }
    }

    return out;
}

void MainWindow::getConfigure()
{
    //send request to get new configure
    mRequest.setUrl(m_confUrl);
    pManager->get(mRequest);
}

void MainWindow::configureReceived(QNetworkReply *reply)
{
    if(reply->error()){
        qDebug() << reply->errorString();
        return;
    }

    QString answer = reply->readAll();

    //Update new Configuration
    updateConfigure(answer);

}

void MainWindow::updateConfigure(QString conf)
{
    //Update new Configuration
    m_CurrentConfigure = conf;
    saveSettings();
    QStringList playlist = getContents(m_CurrentConfigure, "playlist");
    qDebug() << playlist.at(0);

    QStringList arguments;
    if(playlist.length() == 1)
    {
        //Parse Playlist
        QString playlist_contents = (QString)(playlist.at(0));
        QStringList itemlist = getContents(playlist_contents, "item");

        foreach(QString item_contents, itemlist)
        {
           //Parse a Item(Order)
           QStringList orderId_contents = getContents(item_contents, "order");
           QStringList embed_contents = getContents(item_contents, "embed");
           QStringList coordinate_contents = getContents(item_contents, "coordinates");
           QStringList display_contents;
           QStringList duration_contents;
           QStringList startDate_contents;
           QStringList endDate_contents;
           QStringList startTime_contents;
           QStringList endTime_contents;
           QString real_display_content;
           QDate sdate, edate;
           QTime stime, etime;
           PlayDate pDate;

           QString coorRectStr = coordinate_contents.at(0);
           QRect coorRect = getCoordinateRect(coorRectStr);

           int new_order_number = QString(orderId_contents.at(0)).toInt();

           //Create new order
           OrderItem *new_order = new OrderItem();
           new_order->setOrderNumber(QString(orderId_contents.at(0)).toInt());

           if (embed_contents.length() == 1){
                QString entrylist_content = (QString)embed_contents.at(0);
                QStringList entrylist = getContents(entrylist_content, "entry");

                foreach (QString item_entrylist, entrylist) {

                    display_contents = getContents(item_entrylist, "file");
                    duration_contents = getContents(item_entrylist, "duration");

                    startDate_contents = getContents(item_entrylist, "startdate");
                    endDate_contents = getContents(item_entrylist, "stopdate");
                    startTime_contents = getContents(item_entrylist, "starttime");
                    endTime_contents = getContents(item_entrylist, "stoptime");
                    sdate = QDate::fromString(QString(startDate_contents.at(0)), "yyyy-MM-dd");
                    edate = QDate::fromString(QString(endDate_contents.at(0)), "yyyy-MM-dd");
                    stime = QTime::fromString(QString(startTime_contents.at(0)), "hh:mm:ss");
                    etime = QTime::fromString(QString(endTime_contents.at(0)), "hh:mm:ss");
                    pDate.init(sdate, edate, stime, etime);

                    if(orderId_contents.length() < 0 || duration_contents.length() < 0)
                        continue;

                    new_order->setOrderDuration(QString(duration_contents.at(0)).toInt());
                    new_order->setOrderStatus("embed");

                    if(display_contents.length() == 1)
                    {
                        real_display_content = display_contents.at(0);
                        real_display_content.remove('\r');
                        qDebug() << display_contents.at(0);
                        arguments.push_back(real_display_content);
                        new_order->pushMedia(real_display_content);
                        new_order->pushCoordinate(coorRect);
                        new_order->pushPlayDate(pDate);
                    }
                    else
                    {
                        new_order->pushMedia("");
                        new_order->pushCoordinate(coorRect);
                        new_order->pushPlayDate(pDate);
                    }
                }
           }
           else{
               duration_contents = getContents(item_contents, "duration");
               startDate_contents = getContents(item_contents, "startdate");
               endDate_contents = getContents(item_contents, "stopdate");
               startTime_contents = getContents(item_contents, "starttime");
               endTime_contents = getContents(item_contents, "stoptime");
               sdate = QDate::fromString(QString(startDate_contents.at(0)), "yyyy-MM-dd");
               edate = QDate::fromString(QString(endDate_contents.at(0)), "yyyy-MM-dd");
               stime = QTime::fromString(QString(startTime_contents.at(0)), "hh:mm:ss");
               etime = QTime::fromString(QString(endTime_contents.at(0)), "hh:mm:ss");
               pDate.init(sdate, edate, stime, etime);

               if(orderId_contents.length() < 0 || duration_contents.length() < 0)
                   continue;

               new_order->setOrderDuration(QString(duration_contents.at(0)).toInt());
               new_order->setOrderStatus("");

               qDebug() << "order : " << new_order->getOrderNumber();
               qDebug() << "duration : " << new_order->getOrderDuration();

               display_contents = getContents(item_contents, "file");

               if(display_contents.length() == 1)
               {
                   real_display_content = display_contents.at(0);
                   real_display_content.remove('\r');
                   qDebug() << display_contents.at(0);
                   arguments.push_back(real_display_content);
                   new_order->pushMedia(real_display_content);
                   new_order->pushCoordinate(coorRect);
                   new_order->pushPlayDate(pDate);
               }
               else
               {
                   new_order->pushMedia("");
                   new_order->pushCoordinate(coorRect);
                   new_order->pushPlayDate(pDate);
               }
           }

           //Checking with previous order
           if(!m_OrderItems.empty()){
                OrderItem *prev_order = m_OrderItems.back();
                int prev_order_number = prev_order->getOrderNumber();
                if(prev_order_number == new_order_number && prev_order->getOrderStatus() != "embed"){
                    //add media to previous order
                    duration_contents = getContents(item_contents, "duration");
                    duration_contents = getContents(item_contents, "duration");
                    startDate_contents = getContents(item_contents, "startdate");
                    endDate_contents = getContents(item_contents, "stopdate");
                    startTime_contents = getContents(item_contents, "starttime");
                    endTime_contents = getContents(item_contents, "stoptime");
                    sdate = QDate::fromString(QString(startDate_contents.at(0)), "yyyy-MM-dd");
                    edate = QDate::fromString(QString(endDate_contents.at(0)), "yyyy-MM-dd");
                    stime = QTime::fromString(QString(startTime_contents.at(0)), "hh:mm:ss");
                    etime = QTime::fromString(QString(endTime_contents.at(0)), "hh:mm:ss");
                    pDate.init(sdate, edate, stime, etime);

                    if(orderId_contents.length() < 0 || duration_contents.length() < 0)
                        continue;

                    prev_order->setOrderDuration(QString(duration_contents.at(0)).toInt());

                    qDebug() << "order : " << prev_order->getOrderNumber();
                    qDebug() << "duration : " << prev_order->getOrderDuration();

                    display_contents = getContents(item_contents, "file");

                    if(display_contents.length() == 1)
                    {
                        real_display_content = display_contents.at(0);
                        real_display_content.remove('\r');
                        qDebug() << display_contents.at(0);
                        arguments.push_back(real_display_content);
                        prev_order->pushMedia(real_display_content);
                        prev_order->pushCoordinate(coorRect);
                        prev_order->pushPlayDate(pDate);
                    }
                    else
                    {
                        prev_order->pushMedia("");
                        prev_order->pushCoordinate(coorRect);
                        prev_order->pushPlayDate(pDate);
                    }
                }
                else{
                    m_OrderItems.push_back(new_order);
                }
           }
           else{
               m_OrderItems.push_back(new_order);
           }

        }
    }

    manager->clearHistory();
    manager->append(arguments);

}

//play next order
void MainWindow::playNextOrder()
{

    if(_vplayer->state() == Vlc::Playing)
        _vplayer->stop();

    qDebug() << "Current Play Order : " << m_CurrentPlayOrder + 1;
    p_PlayTimer->stop();

    QString fileName;


    n_MediaList = m_OrderItems.at(m_CurrentPlayOrder)->getAllMedia();
    QRect coorRect = m_OrderItems.at(m_CurrentPlayOrder)->getCoordinate(0);

    clearWidgets(h_PlayLayout);
    if(n_MediaList.length() == 1){

        fileName = m_OrderItems.at(m_CurrentPlayOrder)->getMedia(1);

        qDebug() << "Playing file Name : " << fileName;
        qDebug() << "Playing file Rect : " << coorRect;
        qDebug() << "Playing duration : "  << m_OrderItems.at(m_CurrentPlayOrder)->getOrderDuration();
        PlayDate pd = m_OrderItems.at(m_CurrentPlayOrder)->getPlayDate(0);
        qDebug() << "Available time : " << pd.startDate << ' ' << pd.endDate << ' ' << pd.startTime << ' ' << pd.endTime;

        QUrl n_FileUrl(fileName);
        QString fn = n_FileUrl.fileName();

        if (getMediaType(fn) == IS_IMAGE){
            qDebug() << ui->widget->width() << ui->widget->height();
            QLabel *imagewidget = new QLabel(ui->widget);
            imagewidget->setGeometry(QRect(coorRect.x() - 9, coorRect.y() - 9, coorRect.width(), coorRect.height()));

            //this->setCentralWidget(v_PlayLayout);
            //ui->widget->setLayout(v_PlayLayout);
            playImage(fn, imagewidget, coorRect.width(), coorRect.height());
            //h_PlayLayout->addWidget(imagewidget);
        }
        else if(getMediaType(fn) == IS_VIDEO){
            qDebug() << ui->widget->width() << ui->widget->height();
            playVideo(fn, coorRect);
        }
        p_PlayTimer->start(1000 * m_OrderItems.at(m_CurrentPlayOrder)->getOrderDuration());
        m_CurrentPlayOrder ++;
    }
    else{
        if(m_OrderItems.at(m_CurrentPlayOrder)->getOrderStatus() == "embed"){
            m_CurrentPlayInSameOrder = 0;
            i_Rect = coorRect;
            playNextInSameOrder();
        }
        else{
            coor_count = 0;
            playAllInSameOrder();
        }
    }

    if(m_CurrentPlayOrder >= m_OrderItems.length())
        m_CurrentPlayOrder = 0;
}

//For plaing embed medias in one order
void MainWindow::playNextInSameOrder()
{
    if(_vplayer->state() == Vlc::Playing)
        _vplayer->stop();



    p_OrderPlayTimer->stop();
    clearWidgets(h_PlayLayout);

    PlayDate pd = m_OrderItems.at(m_CurrentPlayOrder)->getPlayDate(m_CurrentPlayInSameOrder);
    QString fileName = n_MediaList.at(m_CurrentPlayInSameOrder);
    qDebug() << "Playing file Name : " << fileName;
    qDebug() << "Playing file Rect : " << i_Rect;
    qDebug() << "Available time : " << pd.startDate << ' ' << pd.endDate << ' ' << pd.startTime << ' ' << pd.endTime;

    QUrl n_FileUrl(fileName);
    QString fn = n_FileUrl.fileName();

    if (getMediaType(fn) == IS_IMAGE){
        QLabel *imagewidget = new QLabel(ui->widget);
        imagewidget->setGeometry(QRect(i_Rect.x(), i_Rect.y(), i_Rect.width(), i_Rect.height()));
        playImage(fn, imagewidget, i_Rect.width(), i_Rect.height());
        //h_PlayLayout->addWidget(imagewidget);
    }
    else if(getMediaType(fn) == IS_VIDEO){
        qDebug() << ui->widget->width() << ui->widget->height();
         qDebug() << i_Rect.width() << i_Rect.height();
        playVideo(fn, i_Rect);
    }

    m_CurrentPlayInSameOrder ++;
    qDebug() << m_OrderItems.at(m_CurrentPlayOrder)->getOrderDuration();
    p_OrderPlayTimer->start(1000 * m_OrderItems.at(m_CurrentPlayOrder)->getOrderDuration());

    if(m_CurrentPlayInSameOrder >= n_MediaList.length()){
        p_OrderPlayTimer->stop();
        m_CurrentPlayOrder ++;
        playNextOrder();

    }

}

//For playing all medias in same order
void MainWindow::playAllInSameOrder(){

    if(_vplayer->state() == Vlc::Playing)
        _vplayer->stop();
    if(coor_count >= m_OrderItems.at(m_CurrentPlayOrder)->getMediaCount())
    {
        qDebug() << "media count" << m_OrderItems.at(m_CurrentPlayOrder)->getMediaCount();
        p_OrderAllPlayTimer->stop();
        foreach (VlcMediaPlayer *item, m_playerItems){
            if(item->state() == Vlc::Playing)
                item->stop();
        }
        m_CurrentPlayOrder ++;
        playNextOrder();
        return;
    }
    p_OrderAllPlayTimer->stop();
    clearWidgets(h_PlayLayout);

    m_playerItems.clear();
    m_vlcPlayerCount = 0;
    coor_count = 0;

    foreach (QString item_media, n_MediaList) {

        QRect display_rect = m_OrderItems.at(m_CurrentPlayOrder)->getCoordinate(coor_count);
        PlayDate pd = m_OrderItems.at(m_CurrentPlayOrder)->getPlayDate(coor_count);
        qDebug() << "Playing file Name : " << item_media;
        qDebug() << "Playing file Rect : " << display_rect;
        qDebug() << "Playing duration : " << m_OrderItems.at(m_CurrentPlayOrder)->getOrderDuration();
        qDebug() << "Available time : " << pd.startDate << ' ' << pd.endDate << ' ' << pd.startTime << ' ' << pd.endTime;

        QUrl n_FileUrl(item_media);
        QString fn = n_FileUrl.fileName();

        if (getMediaType(fn) == IS_IMAGE){
            QLabel *imagewidget = new QLabel(ui->widget);
            imagewidget->setGeometry(display_rect);

            playImage(fn, imagewidget, display_rect.width(), display_rect.height());
            imagewidget->show();
            //ui->widget->layout()->addWidget(imagewidget);

        }
        else if(getMediaType(fn) == IS_VIDEO){
            VlcMediaPlayer *vplayer = new VlcMediaPlayer(_instance);
            m_playerItems.push_back(vplayer);
            playVideoConcurrently(fn, display_rect, m_playerItems.at(m_vlcPlayerCount));
            m_vlcPlayerCount ++;
        }
        coor_count ++;
    }

    p_OrderAllPlayTimer->start(1000 * m_OrderItems.at(m_CurrentPlayOrder)->getOrderDuration());
}

int MainWindow::getMediaType(QString fileName)
{
    QString filePath = QDir::currentPath() + "/media/" + fileName;
    QFileInfo fi(fileName);
    QString ext = fi.completeSuffix();
    if(ext == "jpg" || ext == "png" || ext == "gif")
        return IS_IMAGE;
    else if(ext == "mov" || ext == "mp4")
        return IS_VIDEO;
    return 0;
}

void MainWindow::playImage(QString fileName, QLabel *imagewidget, int width, int height)
{
    if(_vplayer->state() == Vlc::Playing)
        _vplayer->stop();

    QString filePath = QDir::currentPath() + "/media/" + fileName;
    QPixmap pic(filePath);

    QPixmap scaled = pic.scaled( width, height, Qt::IgnoreAspectRatio, Qt::FastTransformation );
    imagewidget->setPixmap(scaled);
    imagewidget->show();

    /*
    ui->imagewidget->setScaledContents(true);
    ui->imagewidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    */
}

void MainWindow::playVideo(QString fileName, QRect coorRect)
{

    QString filePath = QDir::currentPath() + "/media/" + fileName;
    VlcWidgetVideo *_vlcWidget;

    _vlcWidget = new VlcWidgetVideo(ui->widget);
    _vlcWidget->setGeometry(QRect(coorRect.x() - 9, coorRect.y() - 9, coorRect.width() + 18, coorRect.height() + 18));

    _vplayer->setVideoWidget(_vlcWidget);
    _vlcWidget->setMediaPlayer(_vplayer);


    VlcMedia *vmedia = new VlcMedia(filePath, true, _instance);
    if(_vplayer->state() == Vlc::Playing)
        _vplayer->stop();
    _vplayer->open(vmedia);
    _vlcWidget->show();
    delete vmedia;
}

void MainWindow::playVideoConcurrently(QString fileName, QRect coorRect, VlcMediaPlayer *vplayer)
{

    QString filePath = QDir::currentPath() + "/media/" + fileName;
    VlcWidgetVideo *_vlcWidget;

    _vlcWidget = new VlcWidgetVideo(ui->widget);
    _vlcWidget->setGeometry(QRect(coorRect.x() - 9, coorRect.y() - 9, coorRect.width() + 18, coorRect.height() + 18));

    vplayer->setVideoWidget(_vlcWidget);
    _vlcWidget->setMediaPlayer(vplayer);

    VlcMedia *vmedia = new VlcMedia(filePath, true, _instance);
    if(vplayer->state() == Vlc::Playing)
        vplayer->stop();
    vplayer->open(vmedia);
    _vlcWidget->show();
    delete vmedia;
}

void MainWindow::playFinished()
{
    _vplayer->stop();
    //delete _vplayer;
}

QRect MainWindow::getCoordinateRect(QString rec)
{
    QStringList strList;
    strList = rec.split(',');
    int position[4], c = 0;
    foreach(QString item, strList){
        position[c] = item.toInt();
        c ++;
    }
    QRect rect;
    rect.setX(position[0]);
    rect.setY(position[1]);
    rect.setWidth(position[2] - position[0]);
    rect.setHeight(position[3] - position[1]);
    return rect;
}

void MainWindow::clearWidgets(QLayout *layout)
{

    QLayoutItem *item;
    while((item = layout->takeAt(0)))
    {
        if (item->layout())
        {
            clearWidgets(item->layout());
            delete item->layout();
        }

        if (item->widget())
        {
            delete item->widget();
        }
    }
}

void MainWindow::loadSettings()
{
    QSettings settings(APP_NAME, QSettings::IniFormat);

    m_confUrl = settings.value(CONFIGURE_URL, "http://52.40.117.153/cache/files/343.php").toString();
    //mCheckingPeriod = settings.value(CHECKING_PERIOD, 10).toInt();
    m_LastUpdatedTime = settings.value(LAST_UPDATED_TIME).toString();
    m_CurrentConfigure = settings.value(LAST_CONFIGURATION).toString();
}

void MainWindow::saveSettings()
{
    QSettings settings(APP_NAME, QSettings::IniFormat);

    settings.setValue(CONFIGURE_URL, m_confUrl);
    //settings.setValue(CHECKING_PERIOD, mCheckingPeriod);
    settings.setValue(LAST_UPDATED_TIME, m_LastUpdatedTime);
    settings.setValue(LAST_CONFIGURATION, m_CurrentConfigure);
}

