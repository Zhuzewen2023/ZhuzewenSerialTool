#ifndef CUSTOMTHREAD_H
#define CUSTOMTHREAD_H

#include <QWidget>
#include <QThread>

class CustomThread : public QThread
{
    Q_OBJECT

protected:
    void run() override;
public:
    CustomThread(QWidget* parent);

signals:
    void threadTimeOut();

};

#endif // CUSTOMTHREAD_H
