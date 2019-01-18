#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <QStringList>
#include <QList>
#include "playdate.h"

class PlayDate;

class OrderItem
{
public:
    OrderItem();

    void    init();

    void    pushMedia(QString);
    void    pushCoordinate(QRect);
    void    pushPlayDate(PlayDate);

    QString getMedia(int);
    QStringList getAllMedia();
    QRect getCoordinate(int);
    PlayDate getPlayDate(int);

    int     getOrderNumber();
    void    setOrderNumber(int);

    int     getOrderDuration();
    void    setOrderDuration(int);

    int     getMediaCount();

    void    setOrderStatus(QString);
    QString getOrderStatus();


private:
    int             mOrderNumber;
    int             mDuration;
    QStringList     mPlayList;
    QString         mOrderStatus;
    QList<QRect>    mCoordinateList;
    QList<PlayDate> mDateList;
};

#endif // ORDERITEM_H
