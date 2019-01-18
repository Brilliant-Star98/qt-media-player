#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QLabel>

#include "downloadmanager.h"
#include "settings.h"
#include "orderitem.h"
#include "playdate.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>

#define     IS_IMAGE 1
#define     IS_VIDEO 2
#define     APP_NAME "SignageServer@bstar"
#define     LAST_UPDATED_TIME "LastUpdatedTime"
#define     LAST_CONFIGURATION  "LastConfiguration"
#define     CONFIGURE_URL "ConfigureURL"
#define     CHECKING_PERIOD "CheckingPeriod"

namespace Ui {
class MainWindow;
}

class VlcInstance;
class VlcMediaPlayer;
class VlcWidgetVideo;
class VlcMedia;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    QStringList getContents(QString body, QString name);
    int getMediaType(QString);
    QRect getCoordinateRect(QString);


protected:
    void keyPressEvent(QKeyEvent *event);
    void getConfigure();
    void updateConfigure(QString conf);
    void playImage(QString, QLabel*, int, int);
    void playVideo(QString, QRect);
    void playVideoConcurrently(QString, QRect, VlcMediaPlayer*);
    void clearWidgets(QLayout *);
    void loadSettings();
    void saveSettings();
    void initialize();

private:
    QString                         m_confUrl;
    QString                         m_CurrentConfigure;
    QString                         m_LastUpdatedTime;

private:
    Ui::MainWindow                  *ui;
    //Vlc classes
    VlcInstance                     *_instance;
    VlcMedia                        *_media;
    VlcMediaPlayer                  *_vplayer;
    QList<VlcMediaPlayer*>          m_playerItems;

    QNetworkAccessManager           *pManager;
    QNetworkRequest                 mRequest;
    QList<OrderItem*>               m_OrderItems;
    QStringList                     n_MediaList; //For several medias in one order
    DownloadManager                 *manager;

    QTimer*                         p_PlayTimer;
    QTimer*                         p_OrderPlayTimer; // serveral plays for one order
    QTimer*                         p_OrderAllPlayTimer; //to show on one screen for one order

    QRect                           i_Rect;
    QBoxLayout                      *h_PlayLayout; //to add videos for one screen

    int                             m_CurrentPlayOrder;
    int                             m_CurrentPlayInSameOrder;
    int                             m_vlcPlayerCount;
    int                             coor_count;
    bool                            m_IsDownloadingStatus;
    bool                            m_ConfigureInitialized;

private slots:
    void finishedConfigureDownload();
    void configureReceived(QNetworkReply*);
    void playNextOrder(); //plays next media
    void playNextInSameOrder(); //plays next media in embed of same order
    void playAllInSameOrder();  //plays all medias in same order
    void playFinished(); //if vlc player playing finished
};

#endif // MAINWINDOW_H
