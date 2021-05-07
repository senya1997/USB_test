#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Devices/zd4bu_2018.h"
#include "usb_receiver.h"
#include "fft_calculator.h"
#include "Sound_player.h"
#include "monitorscene.h"
#include "Settings.h"
#include "datastructure.h"
#include "Painters/timedomainpainter.h"
#include "Painters/freqdomainpainter.h"
#include "Painters/adctimedomainpainter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    
private slots:

    void update();

    void on_StartButton_clicked();

    void on_SettingsButton_clicked();

    void on_ExitButton_clicked();

    void on_AudioButton_clicked();

    void on_TGP_Autoscale_chb_stateChanged(int arg1);

    void on_TGP_ChangeScale_btn_clicked();

    void on_FGP_Autoscale_chb_stateChanged(int arg1);

    void on_FGP_ChangeScale_btn_clicked();

    void on_ADFGP_Autoscale_chb_stateChanged(int arg1);

    void on_ADFGP_ChangeScale_btn_clicked();

private:
    Ui::MainWindow *ui;

	Settings*      m_settingsWidget;

    DataStructure* m_DS;                //  Класс структуры данных, принимаемых от блока
    void CreateDataStructure();         //  Функция создания элементов структуры данных

    ZD4BU_2018 *m_device;               //  USB - устройство, посылающее данные

    USB_receiver *receiver;
    QThread* RecThread;

    FFT_calculator *calculator;
    QThread* CalcThread;

    Sound_player* m_playThread;
    QThread* audioThread;
    bool isPlayAudio;

    TimeDomainPainter* td_painter;

    FreqDomainPainter* fr_painter;

    ADCtimedomainpainter* adctd_painter;

    MonitorScene*  m_monitorScene;

    QTimer* upd_timer;

    QDateTime   start;
    QDateTime   finish;
    uint32_t Time;

    bool  isRun;

    logger* DbgLog;                     //  Класс для отладочного вывода

	QString GetIconsPath();

    QString GetRbfPath();

};
#endif // MAINWINDOW_H
