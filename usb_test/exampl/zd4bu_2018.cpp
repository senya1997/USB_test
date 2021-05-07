#include <QFile>
#include <QThread>
#include "header/zd4bu_2018.h"

ZD4BU_2018::ZD4BU_2018()
{
    usb_device = new zd4bu_2018_usbprot();
    Q_ASSERT(usb_device);
    afe5809_drv = new AFE5809_driver();
    Q_ASSERT(afe5809_drv);
    afe5809_drv->SetUSBDevice(usb_device);
    tlv320AIC_drv = new TLV320AIC3254_driver();
    Q_ASSERT(tlv320AIC_drv);
    tlv320AIC_drv->SetUSBDevice(usb_device);
    IsLoggerInitted = false;
}

void ZD4BU_2018::SetLogger(logger* log)
{
    m_DbgLog = log;
    IsLoggerInitted = true;
    usb_device->SetLogger(log);
}

ZD4BU_2018::~ZD4BU_2018(void)
{
    if (usb_device)
    {
        delete usb_device;
        usb_device = nullptr;
    }
}

bool ZD4BU_2018::Initialize(QString RbfFileName)
{
    if (!usb_device->FT600_Connect())
        return false;

    if (!usb_device->FPGALoad(RbfFileName,&CurrFPGAVer))
        return false;

    UINT Wr_Data;							//	Массив, загружаемый в контроллер

    Wr_Data = 0;
    if (!usb_device->UsbWrite(Adr_ADCch_PW21, &Wr_Data, 2, false))        //	0 channel AFE is PW2_1 Channel
    {
        return false;
    }
    Wr_Data = 1;
    if (!usb_device->UsbWrite(Adr_ADCch_PW22, &Wr_Data, 2, false))        //	1 channel AFE is PW2_2 Channel
    {
        return false;
    }

    afe5809_drv->Init();
    tlv320AIC_drv->Init();
	InitTGC();

    // Устанавливаем начальные значения параметров
    m_PWPulseTime_us    = 2;
    m_PWDepth_mm        = ZD4BU2018_MAX_DEPTH_MM;
    m_PWSampleVolume_mm = ZD4BU2018_MIN_SV_MM;

    return true;
}

void ZD4BU_2018::SetProbe(ZD4BU2018_ePROBES Probe, ZD4BU2018_eMODES Mode)
{
    USHORT m_PWCordicModeAmpl = 1000;
    USHORT m_PWCordicMode = 0;
    USHORT m_PWCordicMode_StDepth = 50;
    USHORT m_PWCordicMode_EndDepth = 100;
    //
    USHORT m_PWCombatMode_DataCtrl = 0;

    UINT WrData;

    while (usb_device->USB_busy);
    afe5809_drv->InitProbe(Probe);
    if (Probe == PW2 || Probe == PW10)	//	PW режим
    {
        //Записываем частоту импульса передатчика
        WrData = Probe == PW2 ? 50 : 10;
        usb_device->UsbWrite(Adr_PTrF, &WrData, 2, false);

        //	DopMode = 0 (PW mode)
        WrData = 0;
        usb_device->UsbWrite(Adr_DopMode, &WrData, 2, false);

		//	Одноканальный режим
		WrData = 0;
        usb_device->UsbWrite(Adr_PWCh, &WrData, 2, false);

		//	Канал передатчика
		WrData = 0;
        usb_device->UsbWrite(Adr_PDChN, &WrData, 2, false);
    }
    else   //   CW режим
    {
        //	DopMode = 1 (CW mode)
        WrData = 1;
        usb_device->UsbWrite(Adr_DopMode, &WrData, 2, false);

        //	Adr_CWF задаём частоту передатчика
        WrData = Probe == CW4 ? 0 : 1;
        usb_device->UsbWrite(Adr_CWF, &WrData, 2, false);

        //	Adr_CWChN задаём канал
        WrData = Probe == CW4 ? 0 : 1;
        usb_device->UsbWrite(Adr_CWChN, &WrData, 2, false);
    }
    m_CurrProbe = Probe;
    switch(Mode)
    {
        case PW_TEST:   WrData = 1;
                        usb_device->UsbWrite(Adr_PWTest, &WrData, 2, false);
                        break;

        case PW_CORDIC: WrData = 0;
                        usb_device->UsbWrite(Adr_PWTest, &WrData, 2, false);

                        WrData = 1;
                        usb_device->UsbWrite(Adr_SimEn, &WrData, 2, false);

                        WrData = UINT(m_PWCordicModeAmpl);
                        usb_device->UsbWrite(Adr_SimAmpl, &WrData, 2, false);

                        WrData = m_PWCordicMode;
                        usb_device->UsbWrite(Adr_SimMode, &WrData, 2, false);

                        WrData = m_PWCordicMode_StDepth*50;
                        usb_device->UsbWrite(Adr_SimStDepth, &WrData, 2, false);

                        WrData = m_PWCordicMode_EndDepth*50;
                        usb_device->UsbWrite(Adr_SimEndDepth, &WrData, 2, false);
                        break;

        case PW_COMBAT:     WrData = 0;
                            usb_device->UsbWrite(Adr_PWTest, &WrData, 2, false);

                            WrData = 0;
                            usb_device->UsbWrite(Adr_SimEn, &WrData, 2, false);

                            WrData = m_PWCombatMode_DataCtrl;
                            usb_device->UsbWrite(Adr_DataCtrl, &WrData, 2, false);
                            break;

        case CW_TEST:       WrData = 1;
                            usb_device->UsbWrite(Adr_CWTest, &WrData, 2, false);
                            break;

        case CW_COMBAT:     WrData = 0;
                            usb_device->UsbWrite(Adr_CWTest, &WrData, 2, false);
                            break;
    }
    m_CurrProbeMode = Mode;
}

bool ZD4BU_2018::Start()
{    
    UINT Wr_Data;							//	Массив, загружаемый в контроллер
    Wr_Data = 0;
    if (!usb_device->UsbWrite(Adr_DBufReset, &Wr_Data, 2, false))			//	Reset FIFO
    {
        return false;
    }

    Wr_Data = 1;
    if (!usb_device->UsbWrite(Adr_Start_CA, &Wr_Data, 2, false))          //	Start
    {
        return false;
    }
    return true;
}

bool ZD4BU_2018::Stop()
{
    UINT Wr_Data;							//	Массив, загружаемый в контроллер

    while (usb_device->USB_busy);
    Wr_Data = 0;
    if (!usb_device->UsbWrite(Adr_Start_CA, &Wr_Data, 2, false))          //	Stop
    {
        return false;
    }
    return true;
}

// Загрузка значения длительности импульса (в микросекундах)
bool ZD4BU_2018::SetPWPulseTime_us (USHORT PulseTime_us)
{
    UINT Wr_Data;							//	Массив, загружаемый в контроллер

    while (usb_device->USB_busy);
    Wr_Data = PulseTime_us*2;
    if (!usb_device->UsbWrite(Adr_PTrN, &Wr_Data, 2, false))          //	Stop
    {
        return false;
    }
    m_PWPulseTime_us = PulseTime_us;
    // Пересчитываем и записываем все зависимые параметры
    SetDepParameters();
    return true;
}

// Загрузка зависимых параметров
bool ZD4BU_2018::SetDepParameters()
{
    UINT Wr_Data;							//	Массив, загружаемый в контроллер

    // Время между окончанием импульса и началом приёма
    uint16_t PWMinDepth_us = (uint16_t)(ConvertDepth_mm_us(ZD4BU2018_MIN_DEPTH_MM));
    while (usb_device->USB_busy);
    //  В FPGA это время считается от начала импульса, поэтому надо добавить время импульса
    Wr_Data = (PWMinDepth_us+m_PWPulseTime_us) * ZD4BU2018_CLOCK_FREQUENCY_MHZ;
    if (!usb_device->UsbWrite(Adr_DataStartD, &Wr_Data, 2, false))
    {
        return false;
    }

    // Возможно изменилась начальная начальная точка измерительного объема, поэтому должна перерассчитаться
    // и загрузиться конечная
    uint16_t PWDepth_us = (uint16_t)(ConvertDepth_mm_us(m_PWDepth_mm));
    uint16_t SampleVolume_us = (uint16_t)(ConvertDepth_mm_us(m_PWSampleVolume_mm));
    //uint16_t PWDepth_points = PWDepth_us/ZD4BU2018_PW_TIME_PER_SAMPLE_US;           //  Преобразуем в номер точки
    //uint16_t SampleVolume_points = SampleVolume_us/ZD4BU2018_PW_TIME_PER_SAMPLE_US; //  Преобразуем в номер точки
    uint16_t PWDepth_points = PWDepth_us * ZD4BU2018_CLOCK_FREQUENCY_MHZ;             //  Преобразуем в номер точки
    uint16_t SampleVolume_points = SampleVolume_us * ZD4BU2018_CLOCK_FREQUENCY_MHZ;   //  Преобразуем в номер точки
    while (usb_device->USB_busy);
    Wr_Data = PWDepth_points + SampleVolume_points;
    if (!usb_device->UsbWrite(Adr_PWPlayEndDepth, &Wr_Data, 2, false))
    {
        return false;
    }
    m_EndRec_points = (PWDepth_us + SampleVolume_us) * ZD4BU2018_CLOCK_FREQUENCY_MHZ;

    // Вычисляем время приёма внутри цикла
    uint16_t PWRecTime_us = PWDepth_us + SampleVolume_us + 1;
    Wr_Data = (PWRecTime_us * ZD4BU2018_CLOCK_FREQUENCY_MHZ) - 1;   //  Вычисляем колличество тактов генератора
    while (usb_device->USB_busy);
    if (!usb_device->UsbWrite(Adr_BufSizeD, &Wr_Data, 2, false))
    {
        return false;
    }
    m_PWRecTime_us = PWRecTime_us;

    // Вычисляем какое надо PRF и загружаем его
    uint16_t PeriodTime = m_PWPulseTime_us + PWMinDepth_us + PWRecTime_us;
    float Freq = float (1.0F / PeriodTime) * 1000000;
    int16_t CurrPRF = ZD4BU2018_POS_PRF_NUM-1;
    uint16_t PRF = 0;
    while (CurrPRF >= 0)
    {
        if ((Freq - ZD4BU2018_PRF[CurrPRF]) > 1)
        {
            PRF = ZD4BU2018_PRF[CurrPRF];
            break;
        }
        CurrPRF--;
    }
    Q_ASSERT(PRF != 0);     //  Нет возможных PRF

    float Hz_to_us = float(1000000.F/PRF);
    Wr_Data = (Hz_to_us * ZD4BU2018_CLOCK_FREQUENCY_MHZ);
    while (usb_device->USB_busy);
    if (!usb_device->UsbWrite(Adr_DPeriod, &Wr_Data, 2, false))
    {
        return false;
    }
//    tlv320AIC_drv->SetDAC_ClockDivider(SHORT(ZD4BU2018_SOUNDCLOCK_FREQ_MHZ*1000000.F/PRF));
    tlv320AIC_drv->SetDAC_ClockDivider(SHORT(ZD4BU2018_CLOCK_FREQUENCY_MHZ*1000000.F/PRF));
    // Вычисляем коэффициент интерполяции и загружаем его
    Wr_Data = ZD4BU2018_SOUND_SAMPLING_FREQ/PRF;
    while (usb_device->USB_busy);
    if (!usb_device->UsbWrite(Adr_InterpRatio, &Wr_Data, 2, false))
    {
        return false;
    }

    return true;
}
