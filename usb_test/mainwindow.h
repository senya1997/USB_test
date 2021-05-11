#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

const QString RBF_PATH = "D:/SS/asdf.rbf";
const QString HEX_PATH = "D:/SS/asdf.hex";

const int MAIN_H = 296;
const int MAIN_WL = 285;
const int MAIN_WH = 584;

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

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
