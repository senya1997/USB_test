#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent, Qt::FramelessWindowHint)
	, ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    DbgLog = new logger();
    ui->DebugOutput_textEdit->setReadOnly(true);
    DbgLog->SetTextEdit(ui->DebugOutput_textEdit);

    m_device = new ZD4BU_2018;
    m_device->SetLogger(DbgLog);

    if (!m_device->Initialize(GetRbfPath() + "v37.rbf"))
        return;

    m_DS->SetDevice(m_device);

    // Создаём потоки
    receiver = new USB_receiver(m_DS);
    receiver->SetDevice(m_device);
    receiver->SetLogger(DbgLog);
    RecThread = new QThread();
    connect(RecThread, SIGNAL(started()), receiver, SLOT(Start()));

    // Создаём painterы

    td_painter = new TimeDomainPainter(ui->TimeGraph_widget,m_DS);
    qRegisterMetaType<uint32_t>("uint32_t");
    //connect(receiver,SIGNAL(received(uint32_t)),td_painter,SLOT(paintBlock(uint32_t)));
    qRegisterMetaType<QVector<DataStructure::ShortComplex>>("QVector<DataStructure::ShortComplex>");
    connect(calculator,SIGNAL(filtred(QVector<DataStructure::ShortComplex>,int)),
            td_painter,SLOT(paintFiltBlock(QVector<DataStructure::ShortComplex>,int)));

    fr_painter = new FreqDomainPainter(ui->FreqGraph_widget,m_DS);
    connect(calculator,SIGNAL(calculated(uint32_t)),fr_painter,SLOT(paintFFTLine(uint32_t)));

    adctd_painter = new ADCtimedomainpainter(ui->ADCTimeGraph_widget,m_DS);
    connect(receiver,SIGNAL(ADCreceived(QByteArray)),adctd_painter,SLOT(paint(QByteArray)));

    QOpenGLWidget* m_openGLWidget = new QOpenGLWidget(this);
    m_openGLWidget->makeCurrent();
    ui->DoplerWin_gV->setViewport(m_openGLWidget);
    m_monitorScene = new MonitorScene(this,m_DS);
    ui->DoplerWin_gV->setScene(m_monitorScene);
    ui->DoplerWin_gV->setSceneRect(0,0,1030,350);
    ui->DoplerWin_gV->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    connect(calculator, SIGNAL(calculated(uint)), m_monitorScene, SLOT(drawLine(uint)));

    m_settingsWidget = new Settings(this);
    m_settingsWidget->setMaximumWidth(600);
    ui->Center_hLayout->addWidget(m_settingsWidget);
    m_settingsWidget->hide();
    m_settingsWidget->SetInteractClasses(m_device,m_DS,m_monitorScene);

    upd_timer = new QTimer();
    upd_timer->setInterval(1000);
    connect(upd_timer, SIGNAL(timeout()), this, SLOT(update()));
    ui->Time_lne->setText(0);

    ui->StartButton->setEnabled(true);
    isRun = false;
    ui->StartButton->setIcon(QIcon(GetIconsPath() + "play.svg"));
    ui->SettingsButton->setIcon(QIcon(GetIconsPath() + "gear.svg"));
    ui->ExitButton->setIcon(QIcon(GetIconsPath() + "Exit.svg"));
    isPlayAudio = false;
    ui->AudioButton->setIcon(QIcon(GetIconsPath() + "audio_off.svg"));
    ui->Graph_tab->setCurrentIndex(0);
}

void MainWindow::CreateDataStructure()
{
    m_DS = new DataStructure();

    // Создаём кольцевой буфер для исходных данных
    QByteArray* newByteArray = new QByteArray();
    newByteArray->resize(m_DS->BytesCountMax);
    newByteArray->fill(0);

    // Создаём каналы
    DataStructure::PWChannel* newPWChannel = new DataStructure::PWChannel();
    newPWChannel->DataBuff          = newByteArray;
    newPWChannel->DataBuffShift     = 0;
    newPWChannel->PulseTime_us      = 2;
    newPWChannel->Depth_mm          = 15;
    newPWChannel->SampleVolume_mm   = 20;
    newPWChannel->brightnessdB      = 10;  //  0..50 dB
    newPWChannel->thresholdBlackdB  = -30; //  -30..-10 dB
    newPWChannel->PlayShift         = 0;
    newPWChannel->CodecMult         = 1;
    newPWChannel->SummShift         = 0;
    newPWChannel->DigGain           = 1;
    // Создаём устройство
    DataStructure::DopChannels* newChannels = new DataStructure::DopChannels();
    newChannels->PW_2Mhz_1 = newPWChannel;

    // Указываем устройстве структуре данных
    m_DS->Channels = newChannels;
}

QString MainWindow::GetRbfPath()
{
    QDir CurrDir = QDir::currentPath();
    CurrDir.cd("..");
    CurrDir.cd("rbf");
    QString CurrPath = CurrDir.absolutePath();
    CurrPath += "/";
    return CurrPath;
}
