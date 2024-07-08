#include "customthread.h"

void CustomThread::run()
{
    while(1)
    {
        msleep(1000);
        emit threadTimeOut();
    }
}

CustomThread::CustomThread(QWidget *parent):QThread(parent)
{

}
