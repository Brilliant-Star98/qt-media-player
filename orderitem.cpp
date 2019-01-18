#include "orderitem.h"

OrderItem::OrderItem()
{
    init();
}

void OrderItem::init()
{
    mOrderNumber = -1;
    mDuration = -1;
    mPlayList.clear();
}

void OrderItem::pushMedia(QString media)
{
    mPlayList.push_back(media);
}

void OrderItem::pushCoordinate(QRect coorRect)
{
    mCoordinateList.push_back(coorRect);
}

void OrderItem::pushPlayDate(PlayDate pDate)
{
    mDateList.push_back(pDate);
}

QString OrderItem::getMedia(int displayIndex)
{
    // DisplayIndex start from 1
    if(displayIndex <= mPlayList.length())
        return mPlayList.at(displayIndex-1);
    else
        return "";
}

int OrderItem::getOrderNumber()
{
    return mOrderNumber;
}

void OrderItem::setOrderNumber(int num)
{
    mOrderNumber = num;
}

int OrderItem::getOrderDuration()
{
    return mDuration;
}

void OrderItem::setOrderDuration(int num)
{
    mDuration = num;
}

int OrderItem::getMediaCount()
{
    return mPlayList.length();
}

QStringList OrderItem::getAllMedia()
{
    return mPlayList;
}

QRect OrderItem::getCoordinate(int index)
{
    return mCoordinateList.at(index);
}

PlayDate OrderItem::getPlayDate(int index)
{
    return mDateList.at(index);
}

void OrderItem::setOrderStatus(QString sta)
{
    mOrderStatus = sta;
}

QString OrderItem::getOrderStatus()
{
    return mOrderStatus;
}
