#include "serialreadingpollingthread.h"

#include <QDebug>



SerialReadingPollingThread::SerialReadingPollingThread(QObject *parent) :
    QThread(parent)
{
}


void SerialReadingPollingThread::run()
{

    while(1)
    {
        qDebug()<< "this is serial thread running！";
    }










}

