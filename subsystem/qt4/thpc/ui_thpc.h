/********************************************************************************
** Form generated from reading ui file 'thpc.ui'
**
** Created: Mon May 5 13:44:46 2008
**      by: Qt User Interface Compiler version 4.3.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_THPC_H
#define UI_THPC_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QProgressBar>
#include <QtGui/QWidget>

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGroupBox *sensor_1;
    QLCDNumber *temp_1;
    QLCDNumber *humi_1;
    QLabel *label_3;
    QLabel *label_4;
    QGroupBox *sensor_2;
    QLCDNumber *humi_2;
    QLCDNumber *temp_2;
    QLabel *label;
    QLabel *label_2;
    QLCDNumber *clock;
    QGroupBox *sensor_3;
    QLCDNumber *buffer_pres;
    QLCDNumber *ambient_pres;
    QProgressBar *useable_pres;
    QLabel *label_5;
    QLabel *label_6;

    void setupUi(QMainWindow *MainWindow)
    {
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->setWindowModality(Qt::NonModal);
    MainWindow->resize(742, 375);
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    sensor_1 = new QGroupBox(centralwidget);
    sensor_1->setObjectName(QString::fromUtf8("sensor_1"));
    sensor_1->setGeometry(QRect(10, 10, 221, 241));
    temp_1 = new QLCDNumber(sensor_1);
    temp_1->setObjectName(QString::fromUtf8("temp_1"));
    temp_1->setEnabled(true);
    temp_1->setGeometry(QRect(10, 30, 201, 91));
    temp_1->setFrameShape(QFrame::StyledPanel);
    temp_1->setSmallDecimalPoint(true);
    temp_1->setSegmentStyle(QLCDNumber::Flat);
    temp_1->setProperty("value", QVariant(0));
    humi_1 = new QLCDNumber(sensor_1);
    humi_1->setObjectName(QString::fromUtf8("humi_1"));
    humi_1->setGeometry(QRect(10, 140, 201, 91));
    humi_1->setFrameShape(QFrame::StyledPanel);
    humi_1->setFrameShadow(QFrame::Plain);
    humi_1->setSmallDecimalPoint(true);
    humi_1->setSegmentStyle(QLCDNumber::Flat);
    label_3 = new QLabel(sensor_1);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setGeometry(QRect(130, 10, 89, 20));
    label_4 = new QLabel(sensor_1);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setGeometry(QRect(130, 120, 91, 20));
    sensor_2 = new QGroupBox(centralwidget);
    sensor_2->setObjectName(QString::fromUtf8("sensor_2"));
    sensor_2->setGeometry(QRect(240, 10, 221, 241));
    humi_2 = new QLCDNumber(sensor_2);
    humi_2->setObjectName(QString::fromUtf8("humi_2"));
    humi_2->setGeometry(QRect(10, 140, 201, 91));
    humi_2->setFrameShape(QFrame::StyledPanel);
    humi_2->setFrameShadow(QFrame::Plain);
    humi_2->setSmallDecimalPoint(true);
    humi_2->setSegmentStyle(QLCDNumber::Flat);
    temp_2 = new QLCDNumber(sensor_2);
    temp_2->setObjectName(QString::fromUtf8("temp_2"));
    temp_2->setGeometry(QRect(10, 30, 201, 91));
    temp_2->setFrameShape(QFrame::StyledPanel);
    temp_2->setFrameShadow(QFrame::Plain);
    temp_2->setSmallDecimalPoint(true);
    temp_2->setSegmentStyle(QLCDNumber::Flat);
    label = new QLabel(sensor_2);
    label->setObjectName(QString::fromUtf8("label"));
    label->setGeometry(QRect(130, 120, 91, 20));
    label_2 = new QLabel(sensor_2);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setGeometry(QRect(125, 10, 89, 20));
    clock = new QLCDNumber(centralwidget);
    clock->setObjectName(QString::fromUtf8("clock"));
    clock->setGeometry(QRect(20, 260, 441, 91));
    clock->setFrameShape(QFrame::StyledPanel);
    clock->setFrameShadow(QFrame::Plain);
    clock->setNumDigits(8);
    clock->setSegmentStyle(QLCDNumber::Flat);
    sensor_3 = new QGroupBox(centralwidget);
    sensor_3->setObjectName(QString::fromUtf8("sensor_3"));
    sensor_3->setGeometry(QRect(470, 10, 261, 341));
    buffer_pres = new QLCDNumber(sensor_3);
    buffer_pres->setObjectName(QString::fromUtf8("buffer_pres"));
    buffer_pres->setGeometry(QRect(10, 140, 201, 91));
    buffer_pres->setFrameShape(QFrame::StyledPanel);
    buffer_pres->setFrameShadow(QFrame::Plain);
    buffer_pres->setSmallDecimalPoint(true);
    buffer_pres->setSegmentStyle(QLCDNumber::Flat);
    ambient_pres = new QLCDNumber(sensor_3);
    ambient_pres->setObjectName(QString::fromUtf8("ambient_pres"));
    ambient_pres->setGeometry(QRect(10, 30, 201, 91));
    ambient_pres->setFrameShape(QFrame::StyledPanel);
    ambient_pres->setFrameShadow(QFrame::Plain);
    ambient_pres->setSmallDecimalPoint(true);
    ambient_pres->setSegmentStyle(QLCDNumber::Flat);
    useable_pres = new QProgressBar(sensor_3);
    useable_pres->setObjectName(QString::fromUtf8("useable_pres"));
    useable_pres->setGeometry(QRect(220, 20, 31, 311));
    useable_pres->setMaximum(1);
    useable_pres->setValue(1);
    useable_pres->setOrientation(Qt::Vertical);
    label_5 = new QLabel(sensor_3);
    label_5->setObjectName(QString::fromUtf8("label_5"));
    label_5->setGeometry(QRect(76, 10, 133, 20));
    label_6 = new QLabel(sensor_3);
    label_6->setObjectName(QString::fromUtf8("label_6"));
    label_6->setGeometry(QRect(88, 120, 131, 20));
    MainWindow->setCentralWidget(centralwidget);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "THPC", 0, QApplication::UnicodeUTF8));
    sensor_1->setTitle(QApplication::translate("MainWindow", "Sensor  1", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("MainWindow", "Temperature \302\260C", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("MainWindow", "Humidity % rel.", 0, QApplication::UnicodeUTF8));
    sensor_2->setTitle(QApplication::translate("MainWindow", "Sensor 2", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("MainWindow", "Humidity % rel.", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("MainWindow", "Temperature \302\260C", 0, QApplication::UnicodeUTF8));
    sensor_3->setTitle(QApplication::translate("MainWindow", "Pressure", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("MainWindow", "ambient pressure mbara", 0, QApplication::UnicodeUTF8));
    label_6->setText(QApplication::translate("MainWindow", "buffer pressure mbara", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(MainWindow);
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

#endif // UI_THPC_H
