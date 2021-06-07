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

    connect(fpga, SIGNAL(SetDataWidth(bool)), this, SLOT(SetDataWidth(bool)));
    connect(fpga, SIGNAL(SetCycloneLEs(bool)), this, SLOT(SetCycloneLEs(bool)));

    connect(fpga, SIGNAL(UpdRbfPath(QString)), this, SLOT(UpdRbfPath(QString)));
    connect(fpga, SIGNAL(UpdFTDIDesc(QString)), this, SLOT(UpdFTDIDesc(QString)));

    connect(fpga, SIGNAL(UpdVerFPGA(QString)), this, SLOT(UpdVerFPGA(QString)));
    connect(fpga, SIGNAL(UpdDateFPGA(QString)), this, SLOT(UpdDateFPGA(QString)));

// start init:
    ui->lb_log->setStyleSheet("color: rgb(255, 0, 0)");

    switch (fpga->Initialize(false, false)) // without force reconf num of LEs don't care
    {
        case init_failed:       ui->lb_log->setText("FTDI init error!"); break;
        case init_wrong_desc:   ui->lb_log->setText("FTDI wrong descriptor!"); break;
        case init_ok:
            ui->lb_log->setStyleSheet("color: rgb(0, 255, 0)");
            ui->lb_log->setText("FTDI connected!");
            break;

        case init_err_reconf: break; // don't use because force reconf turned off
    }

    ui->lb_log->show();
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
    ui->but_sel_rbf->setEnabled(!checked);

    if(checked)
    {
        ui->tb_rbf_path->setText(RBF080_32BIT_PATH);
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

void MainWindow::on_but_prog_ftdi_clicked()
{
    if (QMessageBox::question(this, "Force reconfig FTDI", "Are you sure?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        ui->lb_log->hide();
        ui->tb_ftdi_desc->setText("");

        switch (fpga->Initialize(true, ui->radBut_LEs120k->isChecked()))
        {
            case init_err_reconf:
                ui->lb_log->setStyleSheet("color: rgb(255, 0, 0)");
                ui->lb_log->setText("Error reconfig FTDI!");
                break;
            case init_ok:
                ui->lb_log->setStyleSheet("color: rgb(0, 255, 0)");
                ui->lb_log->setText("Successful!");
                break;

            case init_failed:       break; // don't use because force reconf turned on
            case init_wrong_desc:   break; // don't use because force reconf turned on
        }

        ui->lb_log->show();
        if(log_on) ui->text_log->append(" ");
    }

    return;
}

void MainWindow::on_but_prog_clicked()
{
    QString rbf_path;
    eProgInfo prog_info;

    ui->lb_log->hide();

    ui->tb_ftdi_desc->setText("");
    ui->tb_fw_date->setText("");
    ui->tb_fw_ver->setText("");

    rbf_path = ui->tb_rbf_path->text();

    prog_info = fpga->Program(rbf_path, ui->chbox_def_path->isChecked(),
                                        ui->radBut_16bit->isChecked(),
                                        ui->radBut_LEs120k->isChecked());

    ui->lb_log->setStyleSheet("color: rgb(255, 0, 0)");

    switch (prog_info)
    {
        case prog_err_open_file:    ui->lb_log->setText("Error open '.rbf'!");      break;
        case prog_err_reconf:       ui->lb_log->setText("Error reconfig FTDI!");    break;
        case prog_err_wr:           ui->lb_log->setText("Error write FTDI!\nCheck power"); break;
        case prog_err_rd:           ui->lb_log->setText("Error read FTDI!\nCheck data width");        break;
        case prog_ok:
            ui->lb_log->setStyleSheet("color: rgb(0, 255, 0)");
            ui->lb_log->setText("Successful!");
            break;
    }

    ui->lb_log->show();
    if(log_on) ui->text_log->append(" ");

    return;
}

void MainWindow::on_but_test_clicked()
{
    QString hex_path;
    eTestInfo test_info;

    ui->lb_log->hide();

    hex_path = ui->tb_hex_path->text();

    test_info = fpga->StartTest(hex_path, ui->radBut_16bit->isChecked());

    ui->lb_log->setStyleSheet("color: rgb(255, 0, 0)");

    switch (test_info)
    {
        case test_err_open_file:    ui->lb_log->setText("Error open '.hex'!");  break;
        case test_err_arr_size:     ui->lb_log->setText("Error hex buf size!"); break;
        case test_err_usb_busy:     ui->lb_log->setText("Error USB busy!");     break;
        case test_err_wr:           ui->lb_log->setText("Error write FTDI!");   break;
        case test_err_rd:           ui->lb_log->setText("Error read FTDI!\nWas FPGA program?"); break;
        case test_failed:           ui->lb_log->setText("Test failed!\nOpen log and test again"); break;
        case test_ok:
            ui->lb_log->setStyleSheet("color: rgb(0, 255, 0)");
            ui->lb_log->setText("Successful!");
            break;
    }

    ui->lb_log->show();
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

void MainWindow::SetDataWidth(bool data_width)
{
    if(data_width)
    {
        ui->radBut_16bit->setChecked(true);
        ui->radBut_32bit->setChecked(false);
    }
    else
    {
        ui->radBut_16bit->setChecked(false);
        ui->radBut_32bit->setChecked(true);
    }
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
