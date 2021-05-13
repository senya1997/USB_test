#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("USB FTDI600 - FPGA test");
    setFixedSize(MAIN_WL, MAIN_H);

    ui->text_log->setReadOnly(true);

    ui->lb_log->hide();
    ui->prog_bar->setValue(0);

    ui->text_log->setCursor(Qt::OpenHandCursor);

    fpga = new FPGA_device();

    connect(&delay, SIGNAL(timeout()), this, SLOT(delay_tick()));

    connect(fpga, SIGNAL(UpdLog(QString)), this, SLOT(UpdLog(QString)));
    connect(fpga, SIGNAL(UpdProgBar(int)), this, SLOT(UpdProgBar(int)));

    connect(fpga, SIGNAL(GetDataWidth()), this, SLOT(GetDataWidth()));
    connect(fpga, SIGNAL(ShowMsg(QString,QString)), this, SLOT(ShowMsg(QString,QString)));
}

MainWindow::~MainWindow()
{
    if(fpga)
    {
        delete fpga;
        fpga = nullptr;
    }

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
    if(checked)
    {
        QWidget::setFixedSize(MAIN_WH, MAIN_H);
        log_on = true;

        ui->text_log->show();
    }
    else
    {
        QWidget::setFixedSize(MAIN_WL, MAIN_H);
        log_on = false;

        ui->text_log->hide();
    }
}

void MainWindow::on_but_prog_clicked()
{
    QString rbf_path;
    bool init_success;

    rbf_path = ui->tb_rbf_path->text();

    init_success = fpga->Initialize(rbf_path);

    ui->lb_log->show();
    if(init_success)    ui->lb_log->setText("Successful");
    else                ui->lb_log->setText("Failure");

    if(log_on) ui->text_log->append(" ");
    delay.start(3000); // show label on this time

    return;
}

void MainWindow::on_but_test_clicked()
{
    QString hex_path;
    bool test_success;

    hex_path = ui->tb_hex_path->text();

    test_success = fpga->StartTest(hex_path);

    ui->lb_log->show();
    if(test_success)    ui->lb_log->setText("Successful");
    else                ui->lb_log->setText("Failure");

    if(log_on) ui->text_log->append(" ");
    delay.start(3000); // show label on this time

    return;
}

void MainWindow::UpdLog(QString log_str)
{
    if(log_on) ui->text_log->append(log_str);
}

void MainWindow::UpdProgBar(int prog_value)
{
    ui->prog_bar->setValue(prog_value);
}

bool MainWindow::GetDataWidth()
{
    if(ui->radBut_16bit->isChecked()) return true;
    else return false;
}

void MainWindow::delay_tick()
{
    ui->lb_log->hide();
}

void MainWindow::ShowMsg(QString title, QString msg)
{
    QMessageBox::information(this, title, msg);
}
