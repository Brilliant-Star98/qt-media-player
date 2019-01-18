#include "playdate.h"

PlayDate::PlayDate()
{

}

void PlayDate::init(QDate sDate, QDate eDate, QTime sTime, QTime eTime)
{
    startDate = sDate;
    endDate = eDate;
    startTime = sTime;
    endTime = eTime;
}

