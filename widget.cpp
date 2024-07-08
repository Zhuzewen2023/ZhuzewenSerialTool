#include "widget.h"
#include "ui_widget.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QThread>
#include "myserialcombobox.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{


    writeCntTotal = 0;
    readCntTotal = 0;
    receiveTimeOrNot = 0;
    hexShowClicked = false;
    sendBak = "感谢使用";
    serialStatus = false;
    ui->setupUi(this);
    this->setLayout(ui->gridLayoutGlobal);

    //定时器
    timer = new QTimer(this);
    buttonsCtrlTimer = new QTimer(this);
    connect(buttonsCtrlTimer, &QTimer::timeout, this, &Widget::buttons_handler);
    mythread = new CustomThread(this);
    //connect(mythread, &CustomeThread::threadTimeOut, this, &Widget::buttons_handler);

    getSysTime_timer = new QTimer(this);
    getSysTime_timer->start(1000);

    ui->comboBox_baud->setCurrentIndex(6);
    ui->comboBox_bit->setCurrentIndex(3);

    serialPort = new QSerialPort(this);

    QList<QSerialPortInfo> serialList = QSerialPortInfo::availablePorts();
    for(QSerialPortInfo serialInfo : serialList)
    {
        //qDebug() << "-----------------------------------------";
        //qDebug() << "portName = " << serialInfo.portName();
        //qDebug() << "serialNumber = " << serialInfo.serialNumber();
        ui->comboBox_serial->addItem(serialInfo.portName());
    }

    QObject::connect(serialPort, &QSerialPort::readyRead, this, &Widget::on_SerialData_ready_to_read);
    connect(timer, &QTimer::timeout, this, [=](){
         on_pushButton_sendText_clicked();
    });

    connect(getSysTime_timer, &QTimer::timeout, this, &Widget::on_getSysTime_timer_timeRefresh);
    QObject::connect(ui->comboBox_serial, SIGNAL(currentIndexChanged(int)), this, SLOT(on_updateSendStatusLabel_currentIndexChanged(int)));
    QObject::connect(ui->comboBox_serial, &MySerialComboBox::refresh, this, &Widget::on_refreshSerialName_refresh);
    //控件初始化
    ui->pushButton_sendText->setEnabled(false);
    ui->checkBox_sendInterval->setEnabled(false);
    ui->checkBox_sendNewLine->setEnabled(false);
    ui->checkBox_sendHex->setEnabled(false);
    ui->checkBox_formatInput->setEnabled(false);
    ui->pushButton_runningMode->setEnabled(false);
    ui->pushButton_downloadMode->setEnabled(false);

    ui->labelSendStatus->setText(ui->comboBox_serial->currentText() + "未打开");


    for(int i = 1; i <= 8; i++)
    {
        QString buttonName = QString("pushButtonSendText%1").arg(i);
        QPushButton * pushButtonFinded = findChild<QPushButton *>(buttonName);
        if(pushButtonFinded != NULL)
        {
            pushButtonFinded->setProperty("buttonId", i);
            buttons.append(pushButtonFinded);
            connect(pushButtonFinded, SIGNAL(clicked()), this, SLOT(on_command_button_clicked()));
        }

        QString lineEditName = QString("lineEditSendText%1").arg(i);
        QLineEdit*  lineEdit = findChild<QLineEdit*>(lineEditName);
        lineEdits.append(lineEdit);

        QString checkBoxName = QString("checkBoxSendText%1").arg(i);
        QCheckBox* checkBox = findChild<QCheckBox*>(checkBoxName);
        checkBoxs.append(checkBox);

    }

}

Widget::~Widget()
{
    delete ui;
    delete serialPort;
    delete timer;
    delete getSysTime_timer;
}


void Widget::on_pushButton_closeOpenSerial_clicked()
{
    if(serialStatus == false)
    {
        //1.选择端口号
        serialPort->setPortName(ui->comboBox_serial->currentText());
        //2.配置波特率
        serialPort->setBaudRate(ui->comboBox_baud->currentText().toInt());
        //3.配置数据位
        int dataBits = ui->comboBox_bit->currentText().toInt();
        qDebug() << "dataBits = " << dataBits;
        switch(dataBits)
        {
               case 5:
                    serialPort->setDataBits(QSerialPort::Data5);
                    qDebug() << "setDataBits 5";
                    break;
               case 6:
                    serialPort->setDataBits(QSerialPort::Data6);
                    qDebug() << "setDataBits 6";
                    break;
               case 7:
                    serialPort->setDataBits(QSerialPort::Data7);
                    qDebug() << "setDataBits 7";
                    break;
               case 8:
                    serialPort->setDataBits(QSerialPort::Data8);
                    qDebug() << "setDataBits 8";
                    break;
               default:
                    serialPort->setDataBits(QSerialPort::UnknownDataBits);
                    qDebug() << "Unknown dataBits";
                    break;

        }
        //4.配置校验位
        QString parity = ui->comboBox_check->currentText();
        qDebug() << "parity = " << parity;
        if(parity == "None")
        {
            serialPort->setParity(QSerialPort::NoParity);
            qDebug() << "set no parity success";
        }else if(parity == "Even")
        {
            serialPort->setParity(QSerialPort::EvenParity);
            qDebug() << "set even parity success";
        }else if(parity == "Mark")
        {
            serialPort->setParity(QSerialPort::MarkParity);
            qDebug() << "set mark parity success";
        }else if(parity == "Odd")
        {
            serialPort->setParity(QSerialPort::OddParity);
            qDebug() << "set odd parity success";
        }else if(parity == "Space")
        {
            serialPort->setParity(QSerialPort::SpaceParity);
            qDebug() << "set space parity success";
        }else
        {
            qDebug() << "set parity error";
        }


        //5.配置停止位
        QString stopBits = ui->comboBox_stop->currentText();
        qDebug() << "stopBits = " << stopBits;

        if(stopBits == "One")
        {
            serialPort->setStopBits(QSerialPort::OneStop);
            qDebug() << "setStopBits OneStop success";
        }else if(stopBits == "One&Half")
        {
            serialPort->setStopBits(QSerialPort::OneAndHalfStop);
            qDebug() << "setStopBits One&HalfStop success";
        }else if(stopBits == "Two")
        {
            serialPort->setStopBits(QSerialPort::TwoStop);
            qDebug() << "setStopBits two stop success";
        }else if(stopBits == "Unknown")
        {
            serialPort->setStopBits(QSerialPort::UnknownStopBits);
            qDebug() << "set stop bits unknown success";
        }else
        {
            qDebug() << "set stop bits error";
        }


        //6.流程控制
        QString flowControl = ui->comboBox_flowCtrl->currentText();
        qDebug() << "flowControl = " << flowControl;


        if(flowControl == "No")
        {
            serialPort->setFlowControl(QSerialPort::NoFlowControl);
            qDebug() << "set no flow control success";
        }else if(flowControl == "Hard")
        {
            serialPort->setFlowControl(QSerialPort::HardwareControl);
            qDebug() << "set hardware control success";
        }else if(flowControl == "Soft")
        {
            serialPort->setFlowControl(QSerialPort::SoftwareControl);
            qDebug() << "set software control success";
        }else if(flowControl == "Unknown")
        {
            serialPort->setFlowControl(QSerialPort::UnknownFlowControl);
            qDebug() << "set unknown flow control success";
        }else
        {
            qDebug() << "set flow control error";
        }

        //打开串口
        if(serialPort->open(QIODevice::ReadWrite))
        {
            qDebug() << "serial open success";
            serialStatus = true;
            ui->comboBox_bit->setEnabled(false);
            ui->comboBox_baud->setEnabled(false);
            ui->comboBox_stop->setEnabled(false);
            ui->comboBox_check->setEnabled(false);
            ui->comboBox_serial->setEnabled(false);
            ui->comboBox_flowCtrl->setEnabled(false);
            ui->pushButton_sendText->setEnabled(true);
            ui->checkBox_sendInterval->setEnabled(true);
            ui->checkBox_sendNewLine->setEnabled(true);
            ui->checkBox_sendHex->setEnabled(true);
            //把按钮改为关闭串口
            ui->pushButton_closeOpenSerial->setText("关闭串口");
            ui->labelSendStatus->setText(ui->comboBox_serial->currentText() + "已打开");
        }else
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("串口打开失败");
            msgBox.setText("  串口可能被占用或已拔出      ");
            msgBox.exec();
        }
    }else
    {
        //关闭串口
        serialPort->close();
        serialStatus = false;
        //把按钮改回打开串口
        ui->pushButton_closeOpenSerial->setText("打开串口");
        ui->comboBox_bit->setEnabled(true);
        ui->comboBox_baud->setEnabled(true);
        ui->comboBox_stop->setEnabled(true);
        ui->comboBox_check->setEnabled(true);
        ui->comboBox_serial->setEnabled(true);
        ui->comboBox_flowCtrl->setEnabled(true);
        ui->pushButton_sendText->setEnabled(false);
        ui->checkBox_sendInterval->setEnabled(false);
        ui->checkBox_sendInterval->setCheckState(Qt::CheckState::Unchecked);
        ui->checkBox_sendNewLine->setEnabled(false);
        ui->checkBox_sendHex->setEnabled(false);
        ui->labelSendStatus->setText(ui->comboBox_serial->currentText() + "已关闭");
    }


}

void Widget::on_pushButton_sendText_clicked()
{
    int writeCnt = 0;
    const char* sendData = NULL;
    sendData = ui->lineEdit_sendText->text().toStdString().c_str();

    if(ui->checkBox_sendHex->isChecked())
    {
        //判断是否是偶数位
        QString tmp = ui->lineEdit_sendText->text(); //获取QString字符串
        QByteArray tmpArray = tmp.toUtf8(); //将QString字符串转换成UTF8编码下的字节数组
        if(tmpArray.size() %2 != 0)
        {
            ui->labelSendStatus->setText("错误的文本输入");
            return;
        }
        //判断是否符合16进制的表达
        for(char c : tmpArray)
        {
            if(!std::isxdigit(c))
            {
                ui->labelSendStatus->setText("输入的不是16进制数");
                return;
            }
        }
        //转换成16进制发送
         //不希望用户输入1，变成字符的1
        QByteArray sendArray = QByteArray::fromHex(tmpArray); //因为用户输入的是16进制的字符串，现在我们需要把他从字节数组转换成真正的16进制数
        if(ui->checkBox_sendNewLine->isChecked())
        {
            sendArray.append("\r\n");
            writeCnt = serialPort->write(sendArray);
        }
        else
        {
            writeCnt = serialPort->write(sendArray);
        }
    }
    else
    {
        if(ui->checkBox_sendNewLine->isChecked())
        {
            QByteArray sendArray(sendData);
            sendArray.append("\r\n");
            writeCnt = serialPort->write(sendArray);
        }
        else
        {
            writeCnt = serialPort->write(sendData);
        }
    }



    //在HEX显示勾选的情况下，再刷新整个接受控件
    if(ui->checkBox_hexShow->isChecked())
    {

        convertHexToText();
        convertTextToHex();
    }

    if(writeCnt == -1)
    {
        qDebug() << "send error";
        ui->labelSendStatus->setText("发送失败");
    }else
    {

        writeCntTotal += writeCnt;
        qDebug() << "sending: " << sendData;

        ui->labelSendStatus->setText("发送成功");
        QString labelText = QString("发送：%1").arg(writeCntTotal);
        ui->labelSentCount->setText(labelText);
        if(strcmp(sendData, sendBak.toStdString().c_str()))
        {
            ui->textEditRecord->append(sendData);
            sendBak = QString(sendData);
        }
    }
}

void Widget::on_SerialData_ready_to_read()
{
    int readCnt = 0;
    QString receiveMsg = serialPort->readAll();
    if(receiveMsg != NULL)
    {
        if(ui->checkBox_autoReturn->isChecked())
        {
            receiveMsg += "\r\n";
        }
        qDebug() << "getMessage: " << receiveMsg;
        if(ui->checkBox_hexShow->isChecked())
        {
            QByteArray tmpHexString = receiveMsg.toUtf8().toHex();
            //读取原来控件上的内容
            QString tmpString = ui->textEditRecv->toPlainText();
            //将原来读取的控件内容和新内容相加
            tmpHexString = tmpString.toUtf8() + tmpHexString;
            //重新显示在控件上
            ui->textEditRecv->setText(QString::fromUtf8(tmpHexString));
            convertHexToText();
            convertTextToHex();
        }
        else
        {
            if(ui->checkBox_receiveTime->checkState() == Qt::Unchecked)
            {
                ui->textEditRecv->insertPlainText(receiveMsg);
            }
            else if(ui->checkBox_receiveTime->checkState() == Qt::Checked)
            {
                getSysTime();
                receiveMsg = "[ " + myTime + " ]" + receiveMsg;
                ui->textEditRecv->insertPlainText(receiveMsg);
            }
        }
        //更改下方接受标签
        readCnt = receiveMsg.size();
        readCntTotal += readCnt;
        QString labelText = QString("接受：%1").arg(readCntTotal);
        ui->labelReceiveCount->setText(labelText);
    }

    ui->textEditRecv->moveCursor(QTextCursor::EndOfLine);
    ui->textEditRecv->ensureCursorVisible();
    ui->textEditRecord->moveCursor(QTextCursor::EndOfLine);
    ui->textEditRecord->ensureCursorVisible();
    //ui->textEditRecv->setFocus();
}

void Widget::on_checkBox_sendInterval_stateChanged(int autoSendStatus)
{
    //勾上是2，没勾上是0
    qDebug() << "state changed " << autoSendStatus;
    if(autoSendStatus)
    {
        ui->lineEdit_sendText->setEnabled(false);
        ui->lineEdit_sendInterval->setEnabled(false);
        timer->start(ui->lineEdit_sendInterval->text().toInt());
    }else
    {
        timer->stop();
        ui->lineEdit_sendText->setEnabled(true);
        ui->lineEdit_sendInterval->setEnabled(true);
    }
}

void Widget::on_pushButton_cleanAccept_clicked()
{
    ui->textEditRecv->setText("");
}

void Widget::on_pushButton_saveAccept_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"), "E:/BaiduSyncdisk/Embedded/myQtLearningProject/serialData.txt", tr("Text file (*.txt)"));
    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "file open error";
            QMessageBox fileOpenFailed_warning;
            fileOpenFailed_warning.setWindowTitle("警告：文件打开失败");
            fileOpenFailed_warning.setText("无法保存文件：文件创建或打开失败");
            fileOpenFailed_warning.setIcon(QMessageBox::Warning);
            return;
        }
        else
        {
            QTextStream out(&file);
            out << ui->textEditRecv->toPlainText();
            file.close();
        }
    }
}

void Widget::on_getSysTime_timer_timeRefresh()
{
    getSysTime();
    //qDebug() << myTime;
    ui->labelTime->setText(myTime);
}

void Widget::getSysTime()
{
    //获取时间

    QDateTime currentTime = QDateTime::currentDateTime();
    QDate date = currentTime.date();
    int year = date.year();
    int month = date.month();
    int day = date.day();
    QTime time = currentTime.time();
    int hour = time.hour();
    int minute = time.minute();
    int second = time.second();
    myTime = QString("%1年%02月%03日 %04:%05:%06").arg(year, 4, 10, QChar('0')).arg(month, 2, 10, QChar('0')).arg(day, 2, 10, QChar('0')).arg(hour, 2, 10, QChar('0')).arg(minute, 2, 10, QChar('0')).arg(second, 2, 10, QChar('0'));


}

void Widget::convertTextToHex()
{
    QString text = ui->textEditRecv->toPlainText();
    QByteArray byteArray = text.toUtf8(); //将文本转换为UTF-8字节数组
    QByteArray hexByteArray = byteArray.toHex(); //转换成HEX
    QString hexString = QString::fromUtf8(hexByteArray).toUpper();
    QString lastShow;
    for(int i = 0; i < hexString.size(); i+= 2)
    {
        lastShow += hexString.mid(i, 2) + " ";
    }
    qDebug() << "lastShow: " << lastShow;
    ui->textEditRecv->setText(lastShow);
}

void Widget::convertHexToText()
{
    QString hexText = ui->textEditRecv->toPlainText();
    //hexText.replace(QRegExp(" "), ""); //移除空格
    QByteArray hexText_byteArray = hexText.toUtf8();
    //将16进制文本转换为字节数组
    QByteArray byteArray = QByteArray::fromHex(hexText_byteArray);
    QString textString = QString::fromUtf8(byteArray);
    qDebug() << textString;
    ui->textEditRecv->setText(textString);

}

void Widget::on_checkBox_receiveTime_stateChanged(int receiveTimeState)
{
    qDebug() << "receiveTimeState: " << receiveTimeState;
    receiveTimeOrNot = receiveTimeState;
}

void Widget::on_checkBox_hexShow_clicked(bool checked)
{
    if(checked)
    {
        hexShowClicked = true;
        convertTextToHex();
    }
    else
    {
        hexShowClicked = false;
        convertHexToText();
    }
}

void Widget::on_updateSendStatusLabel_currentIndexChanged(int arg)
{
    ui->labelSendStatus->setText(ui->comboBox_serial->currentText() + "未打开");
}



void Widget::on_pushButton_hidePanel_clicked(bool checked)
{
    if(checked)
    {
        qDebug() << "pushButton_hidePaned_clicked_checked" << checked << "值";
        ui->pushButton_hidePanel->setText("拓展面板");
        ui->groupBoxTexts->hide();
    }
    else
    {
        qDebug() << "pushButton_hidePaned_clicked_checked" << checked << "值";
        ui->pushButton_hidePanel->setText("隐藏面板");
        ui->groupBoxTexts->show();
    }
}

void Widget::on_pushButton_hideHistory_clicked(bool checked)
{
    if(checked)
    {
        qDebug() << "pushButton_hideHistory_clicked_checked" << checked << "值";
        ui->pushButton_hideHistory->setText("显示历史");
        ui->groupBoxRecord->hide();
    }
    else
    {
        qDebug() << "pushButton_hideHistory_clicked_checked" << checked << "值";
        ui->pushButton_hideHistory->setText("隐藏历史");
        ui->groupBoxRecord->show();
    }
}

void Widget::on_refreshSerialName_refresh()
{
    ui->comboBox_serial->clear();
    QList<QSerialPortInfo> serialList = QSerialPortInfo::availablePorts();
    for(QSerialPortInfo serialInfo : serialList)
    {
        qDebug() << "get refresh() signal, serialInfo = " << serialInfo.portName();

        ui->comboBox_serial->addItem(serialInfo.portName());
    }
    ui->labelSendStatus->setText("串口刷新");
}

void Widget::on_command_button_clicked()
{
    /*
    sender() 函数：这是Qt中 QObject 类的一个成员函数，它返回发出信号的对象的指针。
    在信号和槽的上下文中，当你连接了一个信号到一个槽，并在槽函数内部调用 sender() 时，它会返回发出该信号的对象。
    但是，sender() 返回的是一个 QObject * 类型的指针，这意味着它是一个指向 QObject 或其任何子类的通用指针。
    qobject_cast<T *>(obj)：这是Qt提供的一个用于在Qt对象之间进行安全类型转换的模板函数。
    如果 obj 指向的对象可以转换为类型 T（或其子类），则 qobject_cast 会返回一个指向 T 的指针；
    否则，它会返回 nullptr。
    这是类型安全的，因为它在运行时检查对象的实际类型，并避免了像C++中的 static_cast 或 dynamic_cast（后者在Qt对象上可能不总是安全的）那样可能导致的类型不匹配错误。
    */
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    int num = btn->property("buttonId").toInt(); //这里前面用setproperty设置好了buttonId的属性值为i
    //qDebug() << num;
    QString lineEditName = QString("lineEditSendText%1").arg(num);
    QString checkBoxName = QString("checkBoxSendText%1").arg(num);
    qDebug() << "lineEditName: " << lineEditName;
    qDebug() << "checkBoxName: " << checkBoxName;
    QLineEdit * lineEdit = findChild<QLineEdit *>(lineEditName);
    if(lineEdit->text().size() > 0)
    {
        ui->lineEdit_sendText->setText(lineEdit->text());
        QCheckBox * checkBox = findChild<QCheckBox *>(checkBoxName);
        if(checkBox)
        {
            ui->checkBox_sendHex->setChecked(checkBox->isChecked());
        }
        on_pushButton_sendText_clicked();
    }
}

/*
void Widget::on_pushButtonSendText1_clicked()
{
    if(ui->checkBoxSendText1->isChecked())
    {
        ui->checkBox_sendHex->setCheckState(Qt::Checked);
    }
    else
    {
        ui->checkBox_sendHex->setCheckState(Qt::Unchecked);
    }
    ui->lineEdit_sendText->setText(ui->lineEditSendText1->text());
    on_pushButton_sendText_clicked();

}
*/

void Widget::buttons_handler()
{
    if(buttonIndex < buttons.size())
    {

        QPushButton* btnTmp = buttons[buttonIndex];
        emit btnTmp->clicked();
        buttonIndex++;
        //QThread::usleep(ui->spinBox_loopSend->text().toInt()*1000); //这里会导致主程序卡死
    }
    else
    {
        buttonIndex = 0;
    }


}

void Widget::on_checkBox_loopSend_clicked(bool checked)
{
    qDebug() << "loopSend_clicked checked" << checked;
    if(checked)
    {
        ui->spinBox_loopSend->setEnabled(false);
        buttonsCtrlTimer->start(ui->spinBox_loopSend->text().toInt());
        //mythread->start();
    }
    else
    {
        ui->spinBox_loopSend->setEnabled(true);
        buttonsCtrlTimer->stop();
        //mythread->terminate();
    }
}

void Widget::on_pushButton_reset_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("提示");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("请确认是否重置？重置后将不可逆！");
    //msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    QPushButton* yesButton = msgBox.addButton("是", QMessageBox::YesRole);
    QPushButton* noButton = msgBox.addButton("否", QMessageBox::NoRole);
    msgBox.exec();
    if(msgBox.clickedButton() == yesButton)
    {
        qDebug() << "yesButton";
        //遍历LineEdit并清空内容
        for(int i = 0; i < lineEdits.size(); i++)
        {
            lineEdits[i]->clear();
        }
        for(int j = 0; j < checkBoxs.size(); j++)
        {
            checkBoxs[j]->setChecked(false);
        }
        //遍历checkBox并取消勾选
    }
    if(msgBox.clickedButton()== noButton)
    {
        qDebug() << "noButton";
    }

}

void Widget::on_pushButton_save_clicked()
{
    QDateTime now = QDateTime::currentDateTime();
    QString dateString = now.toString("yyyyMMdd_hhmmss");
    QString defaultFileName = QString("myFile_%1.txt").arg(dateString);

    QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"), "E:\\BaiduSyncdisk\\Embedded\\myQtLearningProject\\" + defaultFileName, tr("文本类型 (*.txt)"));

    QFile file;
    file.setFileName(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for(int i = 0; i < lineEdits.size(); i++)
    {
        out << checkBoxs[i]->isChecked() << "," << lineEdits[i]->text() << "\r\n";

    }
    file.close();

}

void Widget::on_pushButton_load_clicked()
{
    int i = 0;
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), "E:\\BaiduSyncdisk\\Embedded\\myQtLearningProject\\", tr("文本类型 (*.txt)"));

    if(!fileName.isEmpty())
    {
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;

        QTextStream in(&file);
        while(!in.atEnd() && i <= 7)
        {
            QString line = in.readLine();
            QStringList list = line.split(",");
            if(list.count() == 2)
            {
                checkBoxs[i]->setChecked(list[0].toInt());
                lineEdits[i]->setText(list[1]);
            }
            i++;
        }
    }
}
