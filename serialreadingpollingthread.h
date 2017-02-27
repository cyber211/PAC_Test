#ifndef SERIALREADINGPOLLINGTHREAD_H
#define SERIALREADINGPOLLINGTHREAD_H

#include <QThread>
#include <QtCore>
#include <QObject>

using namespace std;


class SerialReadingPollingThread : public QThread
{
    Q_OBJECT
public:
    explicit SerialReadingPollingThread(QObject *parent = 0);

private:

signals:

public slots:


private slots:
   void run();

};

#endif // SERIALREADINGPOLLINGTHREAD_H
