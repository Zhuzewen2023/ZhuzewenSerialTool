#ifndef WIDGET_H
#define WIDGET_H

#include <QPushButton>
#include <QSerialPort>
#include <QTimer>
#include <QWidget>
#include <qcheckbox.h>
#include "myserialcombobox.h"
#include "customthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_closeOpenSerial_clicked();

    void on_pushButton_sendText_clicked();

    void on_SerialData_ready_to_read();

    void on_checkBox_sendInterval_stateChanged(int arg1);

    void on_pushButton_cleanAccept_clicked();

    void on_pushButton_saveAccept_clicked();

    void on_getSysTime_timer_timeRefresh();

    void on_checkBox_receiveTime_stateChanged(int arg1);

    void on_checkBox_hexShow_clicked(bool checked);

    void on_updateSendStatusLabel_currentIndexChanged(int arg);

    void on_pushButton_hidePanel_clicked(bool checked);

    void on_pushButton_hideHistory_clicked(bool checked);

    void on_refreshSerialName_refresh();

    //void on_pushButtonSendText1_clicked();

    void on_command_button_clicked();

    void on_checkBox_loopSend_clicked(bool checked);

    void buttons_handler();

    void on_pushButton_reset_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_load_clicked();

private:
    Ui::Widget *ui;
    QSerialPort* serialPort;
    int writeCntTotal;
    int readCntTotal;
    QString sendBak;
    bool serialStatus;
    QTimer* timer;
    QTimer* getSysTime_timer;
    QString myTime;
    int receiveTimeOrNot;
    bool hexShowClicked;
    //构造一个数组存放多文本的按键
    QList<QPushButton *> buttons; //放QPushButton指针进去
    QList<QLineEdit *> lineEdits;
    QList<QCheckBox *> checkBoxs;

    QTimer* buttonsCtrlTimer;
     int buttonIndex;
     CustomThread* mythread;

    void getSysTime();
    void convertTextToHex();
    void convertHexToText();
};
#endif // WIDGET_H
