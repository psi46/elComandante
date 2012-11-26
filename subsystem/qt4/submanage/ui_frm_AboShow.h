/********************************************************************************
** Form generated from reading ui file 'frm_AboShow.ui'
**
** Created: Tue Jul 29 17:38:06 2008
**      by: Qt User Interface Compiler version 4.3.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_FRM_ABOSHOW_H
#define UI_FRM_ABOSHOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QLineEdit>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_frm_Aboview
{
public:
    QVBoxLayout *vboxLayout;
    QTextBrowser *txtShow;
    QLineEdit *lineEdit;

    void setupUi(QWidget *frm_Aboview)
    {
    if (frm_Aboview->objectName().isEmpty())
        frm_Aboview->setObjectName(QString::fromUtf8("frm_Aboview"));
    frm_Aboview->resize(353, 236);
    vboxLayout = new QVBoxLayout(frm_Aboview);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    txtShow = new QTextBrowser(frm_Aboview);
    txtShow->setObjectName(QString::fromUtf8("txtShow"));
    QFont font;
    font.setFamily(QString::fromUtf8("Courier"));
    txtShow->setFont(font);

    vboxLayout->addWidget(txtShow);

    lineEdit = new QLineEdit(frm_Aboview);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));

    vboxLayout->addWidget(lineEdit);


    retranslateUi(frm_Aboview);

    QMetaObject::connectSlotsByName(frm_Aboview);
    } // setupUi

    void retranslateUi(QWidget *frm_Aboview)
    {
    frm_Aboview->setWindowTitle(QApplication::translate("frm_Aboview", "abo - PKT_TYPE", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(frm_Aboview);
    } // retranslateUi

};

namespace Ui {
    class frm_Aboview: public Ui_frm_Aboview {};
} // namespace Ui

#endif // UI_FRM_ABOSHOW_H
