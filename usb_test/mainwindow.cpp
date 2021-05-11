#include <QDialog>
#include <QFileDialog>

#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("USB FTDI600 - FPGA test");
    setFixedSize(MAIN_WL, MAIN_H);

    ui->progress_com->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_but_sel_rbf_clicked()
{
    QString rbf_path = QFileDialog::getOpenFileName(this, "Open file", "", "*.rbf");
    ui->tb_rbf_path->setText(rbf_path);
}

void MainWindow::on_but_sel_hex_clicked()
{
    QString hex_path = QFileDialog::getOpenFileName(this, "Open file", "", "*.hex");
    ui->tb_hex_path->setText(hex_path);
}

void MainWindow::on_chbox_def_path_clicked(bool checked)
{
    QString rbf_path, hex_path;

    ui->tb_rbf_path->setEnabled(!checked);
    ui->tb_hex_path->setEnabled(!checked);

    ui->but_sel_rbf->setEnabled(!checked);
    ui->but_sel_hex->setEnabled(!checked);

    if(checked)
    {
        rbf_path = RBF_PATH;
        hex_path = HEX_PATH;

        ui->tb_rbf_path->setText(rbf_path);
        ui->tb_hex_path->setText(hex_path);
    }
}

void MainWindow::on_chbox_show_log_clicked(bool checked)
{
    if(checked) QWidget::setFixedSize(MAIN_WH, MAIN_H);
    else QWidget::setFixedSize(MAIN_WL, MAIN_H);
}

void MainWindow::on_but_prog_clicked()
{
    QString rbf_path;

    rbf_path = ui->tb_rbf_path->text();

    if(rbf_path.isEmpty() || rbf_path.isNull() || (rbf_path.size() == rbf_path.count(' ')))
    {
        QMessageBox::information(this, "Path is empty", "Choose '.rbf'");
        return;
    }
}

void MainWindow::on_but_test_clicked()
{
    QString hex_path;

    hex_path = ui->tb_hex_path->text();

    if(hex_path.isEmpty() || hex_path.isNull() || (hex_path.size() == hex_path.count(' ')))
    {
        QMessageBox::information(this, "Path is empty", "Choose '.hex'");
        return;
    }
}
