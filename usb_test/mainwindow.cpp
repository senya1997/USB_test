#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

// GUI:
    ui->setupUi(this);

    setWindowTitle("USB FTDI60x - FPGA test");
    setFixedSize(MAIN_WL, MAIN_H);

    ui->text_log->setReadOnly(true);

    ui->lb_log->hide();
    ui->prog_bar->setValue(0);

    ui->tb_ftdi_desc->setReadOnly(true);
    ui->tb_fw_date->setReadOnly(true);
    ui->tb_fw_ver->setReadOnly(true);

    ui->text_log->setCursor(Qt::OpenHandCursor);

    on_chbox_def_path_clicked(true);

// inst and connect:
    fpga = new FPGA_device();

    connect(fpga, SIGNAL(UpdLog(QString)), this, SLOT(UpdLog(QString)));
    connect(fpga, SIGNAL(UpdProgBar(int)), this, SLOT(UpdProgBar(int)));

    connect(fpga, SIGNAL(GetDataWidth()), this, SLOT(GetDataWidth()));
    connect(fpga, SIGNAL(GetCycloneLEs()), this, SLOT(GetCycloneLEs()));
    connect(fpga, SIGNAL(SetCycloneLEs(bool)), this, SLOT(SetCycloneLEs(bool)));

    connect(fpga, SIGNAL(GetChBoxDefPath()), this, SLOT(GetChBoxDefPath()));

    connect(fpga, SIGNAL(ShowMsg(QString,QString)), this, SLOT(ShowMsg(QString,QString)));

    connect(fpga, SIGNAL(UpdRbfPath(QString)), this, SLOT(UpdRbfPath(QString)));
    connect(fpga, SIGNAL(UpdFTDIDesc(QString)), this, SLOT(UpdFTDIDesc(QString)));

    connect(fpga, SIGNAL(UpdVerFPGA(QString)), this, SLOT(UpdVerFPGA(QString)));
    connect(fpga, SIGNAL(UpdDateFPGA(QString)), this, SLOT(UpdDateFPGA(QString)));
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
    if(!rbf_path.isNull()) ui->tb_rbf_path->setText(rbf_path);
}

void MainWindow::on_but_sel_hex_clicked()
{
    QString hex_path = QFileDialog::getOpenFileName(this, "Open file", "", "*.hex");
    if(!hex_path.isNull()) ui->tb_hex_path->setText(hex_path);
}

void MainWindow::on_chbox_def_path_clicked(bool checked)
{
    ui->tb_rbf_path->setEnabled(!checked);
    ui->tb_hex_path->setEnabled(!checked);

    ui->but_sel_rbf->setEnabled(!checked);
    ui->but_sel_hex->setEnabled(!checked);

    if(checked)
    {
        ui->tb_rbf_path->setText(RBF080_PATH);
        ui->tb_hex_path->setText(HEX_PATH);
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

    ui->lb_log->hide();

    ui->tb_ftdi_desc->setText("");
    ui->tb_fw_date->setText("");
    ui->tb_fw_ver->setText("");

    rbf_path = ui->tb_rbf_path->text();

    init_success = fpga->Initialize(rbf_path);

    ui->lb_log->show();
    if(init_success)
    {
        ui->lb_log->setStyleSheet("color: rgb(0, 255, 0)");
        ui->lb_log->setText("Successful!");
    }
    else
    {
        ui->lb_log->setStyleSheet("color: rgb(255, 0, 0)");
        ui->lb_log->setText("Failure!");
    }

    if(log_on) ui->text_log->append(" ");

    return;
}

void MainWindow::on_but_test_clicked()
{
    QString hex_path;
    bool test_success;

    ui->lb_log->hide();

    hex_path = ui->tb_hex_path->text();

    test_success = fpga->StartTest(hex_path);

    ui->lb_log->show();
    if(test_success)
    {
        ui->lb_log->setStyleSheet("color: rgb(0, 255, 0)");
        ui->lb_log->setText("Successful!");
    }
    else
    {
        ui->lb_log->setStyleSheet("color: rgb(255, 0, 0)");
        ui->lb_log->setText("Failure!");
    }

    if(log_on) ui->text_log->append(" ");

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

bool MainWindow::GetChBoxDefPath()
{
    return ui->chbox_def_path->isChecked();
}

bool MainWindow::GetDataWidth()
{
    if(ui->radBut_16bit->isChecked()) return true;
    else return false;
}

bool MainWindow::GetCycloneLEs()
{
    if(ui->radBut_LEs120k->isChecked()) return true;
    else return false;
}

void MainWindow::SetCycloneLEs(bool cyclone_LEs)
{
    if(cyclone_LEs)
    {
        ui->radBut_LEs120k->setChecked(true);
        ui->radBut_LEs080k->setChecked(false);
    }
    else
    {
        ui->radBut_LEs120k->setChecked(false);
        ui->radBut_LEs080k->setChecked(true);
    }
}

void MainWindow::ShowMsg(QString title, QString msg)
{
    QMessageBox::information(this, title, msg);
}

void MainWindow::UpdRbfPath(QString rbf_path)
{
    ui->tb_rbf_path->setText(rbf_path);
}

void MainWindow::UpdFTDIDesc(QString desc)
{
    ui->tb_ftdi_desc->setText(desc);
}

void MainWindow::UpdVerFPGA(QString ver)
{
    ui->tb_fw_ver->setText(ver);
}

void MainWindow::UpdDateFPGA(QString date)
{
    ui->tb_fw_date->setText(date);
}
