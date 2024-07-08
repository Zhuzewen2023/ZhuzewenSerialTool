#ifndef MYSERIALCOMBOBOX_H
#define MYSERIALCOMBOBOX_H

#include <QComboBox>



class MySerialComboBox : public QComboBox
{
    Q_OBJECT
public:
    MySerialComboBox(QWidget* parent);

private:
    void mousePressEvent(QMouseEvent *e) override;

signals:
    void refresh();


};

#endif // MYSERIALCOMBOBOX_H
