#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>

#include <QTimer>

#include "defines.h"
#include "FPGA_device.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_chbox_def_path_clicked(bool checked);

    void on_chbox_show_log_clicked(bool checked);

    void on_but_sel_rbf_clicked();

    void on_but_sel_hex_clicked();

    void on_but_prog_clicked();

    void on_but_test_clicked();

    void delay_tick();

    void UpdLog(QString log_str);
    void UpdProgBar(int prog_value);

    bool GetDataWidth();

    void ShowMsg(QString title, QString msg);

    void UpdVerFPGA(QString ver);
    void UpdDateFPGA(QString date);

private:
    Ui::MainWindow *ui;

    FPGA_device *fpga;

    bool log_on = false;

    QTimer delay;
};

#endif // MAINWINDOW_H
