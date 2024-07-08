#include "myserialcombobox.h"

#include <QMouseEvent>

MySerialComboBox::MySerialComboBox(QWidget *parent):QComboBox(parent)
{

}

void MySerialComboBox::mousePressEvent(QMouseEvent *e)
{

    //发出一个信号
    emit refresh();
    QComboBox::mousePressEvent(e);
}
