/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.0.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *but_sel_rbf;
    QScrollArea *area_log;
    QWidget *scrollAreaWidgetContents;
    QLabel *RBF_path;
    QLineEdit *tb_rbf_path;
    QPushButton *but_prog;
    QPushButton *but_test;
    QProgressBar *progress_com;
    QLabel *HEX_path;
    QLineEdit *tb_hex_path;
    QPushButton *but_sel_hex;
    QCheckBox *chbox_def_path;
    QGroupBox *groupBox;
    QRadioButton *radBut_16bit;
    QRadioButton *radBut_32bit;
    QCheckBox *chbox_show_log;
    QStatusBar *statusbar;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(284, 334);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        but_sel_rbf = new QPushButton(centralwidget);
        but_sel_rbf->setObjectName(QString::fromUtf8("but_sel_rbf"));
        but_sel_rbf->setGeometry(QRect(200, 30, 75, 23));
        area_log = new QScrollArea(centralwidget);
        area_log->setObjectName(QString::fromUtf8("area_log"));
        area_log->setEnabled(false);
        area_log->setGeometry(QRect(290, 10, 281, 291));
        area_log->setLineWidth(2);
        area_log->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 279, 289));
        area_log->setWidget(scrollAreaWidgetContents);
        RBF_path = new QLabel(centralwidget);
        RBF_path->setObjectName(QString::fromUtf8("RBF_path"));
        RBF_path->setGeometry(QRect(10, 10, 71, 16));
        QFont font;
        font.setPointSize(10);
        RBF_path->setFont(font);
        tb_rbf_path = new QLineEdit(centralwidget);
        tb_rbf_path->setObjectName(QString::fromUtf8("tb_rbf_path"));
        tb_rbf_path->setGeometry(QRect(12, 30, 181, 20));
        but_prog = new QPushButton(centralwidget);
        but_prog->setObjectName(QString::fromUtf8("but_prog"));
        but_prog->setGeometry(QRect(10, 230, 91, 23));
        but_test = new QPushButton(centralwidget);
        but_test->setObjectName(QString::fromUtf8("but_test"));
        but_test->setGeometry(QRect(10, 270, 75, 23));
        progress_com = new QProgressBar(centralwidget);
        progress_com->setObjectName(QString::fromUtf8("progress_com"));
        progress_com->setGeometry(QRect(100, 270, 181, 23));
        progress_com->setValue(24);
        HEX_path = new QLabel(centralwidget);
        HEX_path->setObjectName(QString::fromUtf8("HEX_path"));
        HEX_path->setGeometry(QRect(10, 60, 121, 16));
        HEX_path->setFont(font);
        tb_hex_path = new QLineEdit(centralwidget);
        tb_hex_path->setObjectName(QString::fromUtf8("tb_hex_path"));
        tb_hex_path->setGeometry(QRect(12, 80, 181, 20));
        but_sel_hex = new QPushButton(centralwidget);
        but_sel_hex->setObjectName(QString::fromUtf8("but_sel_hex"));
        but_sel_hex->setGeometry(QRect(200, 80, 75, 23));
        chbox_def_path = new QCheckBox(centralwidget);
        chbox_def_path->setObjectName(QString::fromUtf8("chbox_def_path"));
        chbox_def_path->setGeometry(QRect(10, 110, 131, 17));
        chbox_def_path->setFont(font);
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 150, 161, 51));
        groupBox->setFont(font);
        radBut_16bit = new QRadioButton(groupBox);
        radBut_16bit->setObjectName(QString::fromUtf8("radBut_16bit"));
        radBut_16bit->setEnabled(true);
        radBut_16bit->setGeometry(QRect(20, 20, 51, 17));
        radBut_16bit->setChecked(true);
        radBut_32bit = new QRadioButton(groupBox);
        radBut_32bit->setObjectName(QString::fromUtf8("radBut_32bit"));
        radBut_32bit->setGeometry(QRect(90, 20, 51, 17));
        chbox_show_log = new QCheckBox(centralwidget);
        chbox_show_log->setObjectName(QString::fromUtf8("chbox_show_log"));
        chbox_show_log->setGeometry(QRect(200, 230, 81, 21));
        MainWindow->setCentralWidget(centralwidget);
        groupBox->raise();
        but_sel_rbf->raise();
        area_log->raise();
        RBF_path->raise();
        tb_rbf_path->raise();
        but_prog->raise();
        but_test->raise();
        progress_com->raise();
        HEX_path->raise();
        tb_hex_path->raise();
        but_sel_hex->raise();
        chbox_def_path->raise();
        chbox_show_log->raise();
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 284, 21));
        MainWindow->setMenuBar(menubar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        but_sel_rbf->setText(QCoreApplication::translate("MainWindow", "Select RBF", nullptr));
        RBF_path->setText(QCoreApplication::translate("MainWindow", "RBF path:", nullptr));
        but_prog->setText(QCoreApplication::translate("MainWindow", "Program FPGA", nullptr));
        but_test->setText(QCoreApplication::translate("MainWindow", "Start test", nullptr));
        HEX_path->setText(QCoreApplication::translate("MainWindow", "HEX test data path:", nullptr));
        but_sel_hex->setText(QCoreApplication::translate("MainWindow", "Select HEX", nullptr));
        chbox_def_path->setText(QCoreApplication::translate("MainWindow", "Use default path", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "FTDI 16/32 bit select:", nullptr));
        radBut_16bit->setText(QCoreApplication::translate("MainWindow", "16 bit", nullptr));
        radBut_32bit->setText(QCoreApplication::translate("MainWindow", "32 bit", nullptr));
        chbox_show_log->setText(QCoreApplication::translate("MainWindow", "Show log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
