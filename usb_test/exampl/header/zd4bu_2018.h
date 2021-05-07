#ifndef ZD4BU_2018_H
#define ZD4BU_2018_H

#include <QObject>
#include "zd4bu_2018_usbprot.h"
#include "zd4bu_2018_fpgareg.h"
#include "afe5809_driver.h"
#include "tlv320aic3254_driver.h"
#include "../src/logger.h"

#define ZD4BU2018_CLOCK_FREQUENCY_MHZ       50										//  Частота тактового генератора в Мгц
#define ZD4BU2018_SOUNDCLOCK_FREQ_MHZ       24.576									//  Частота тактового генератора звукового сигнала
#define ZD4BU2018_SOUND_SAMPLING_FREQ       48000									//  Частота дискретизации звукового сигнала

#define ZD4BU2018_MAX_DEPTH_MM              150										//  Максимальная глубина в мм
#define ZD4BU2018_STEP_DEPTH_MM				5										//  Шаг изменения глубины в мм
#define ZD4BU2018_MIN_DEPTH_MM              15										//  Минимальная глубина в мм

#define ZD4BU2018_POS_PRF_NUM               3                                       //  Колличество возможных PRF
const UINT16 ZD4BU2018_PRF[]                = {4000,8000,16000};                    //  Возможные PRF

#define ZD4BU2018_MIN_SV_MM					0.8										//  Минимальное значение измерительного объёма в мм
#define ZD4BU2018_STEP_SV_MM				1.54									//  Шаг изменения измерительного объёма в мм
#define ZD4BU2018_SV_NUMBER					14										//  Колличество возможных измерительных объёмов

#define ZD4BU2018_PW_TIME_PER_SAMPLE_US     1                                       //  Время между сэмплами в микросекундах
#define ZD4BU2018_PW_SAMPLES_IN_SECOND      1000000/ZD4BU2018_PW_TIME_PER_SAMPLE_US //  Колличество комплексных отсчётов в секунду
#define ZD4BU2018_PW_BYTES_IN_SAMPLE        4                                       //  Размер одного комплексного отсчёта в байтах (2 байта на каждую составляющую)
#define ZD4BU2018_BYTES_IN_HEADER           16                                      //  Размер заголовка пакета

enum ZD4BU2018_eMODES {
    PW_TEST,
    PW_CORDIC,
    PW_COMBAT,
    CW_TEST,
    CW_COMBAT
};

class ZD4BU_2018
{
    public:
        ZD4BU_2018();
        ~ZD4BU_2018();

        USHORT CurrFPGAVer;

        AFE5809_driver *afe5809_drv;

        TLV320AIC3254_driver *tlv320AIC_drv;

        zd4bu_2018_usbprot  *usb_device = nullptr;

        // Преобразует заданную глубину в мм во время поожидания после импульса в микросекундах
        short   ConvertDepth_mm_us(float Depth_mm);
        // Начальная инициализация устройства
        bool Initialize(QString RbfFileName);

        // Загрузка значения длительности импульса (в микросекундах)
        bool SetPWPulseTime_us (USHORT PWPulseTime_us);
        // Загрузка значения измеряемой глубины (в миллиметрах)
        bool SetPWDepth_mm(USHORT PWDepth_mm);
        // Загрузка значения измеряемого объёма (в миллиметрах)
        bool SetPWSampleVolume_mm(float PWSampleVolume_mm);
        // Загрузка значения умножающего коэффициента перед отправкой данных в кодек
        bool SetCodecMult (USHORT CodecMult);
        // Загрузка значения сдвига на выходе сумматора глубины
        bool SetSummShift (USHORT SummShift);

        //  Установка заданного датчика и режима его работы
        void SetProbe(ZD4BU2018_ePROBES Probe, ZD4BU2018_eMODES Mode);

        //	Устанавливает значение TGC для одной из 8 зон TGC TGCZoneNum
        void SetTGC(uint8_t TGCZoneNum, uint8_t TGC);

        //	Устанавливает коэффициент усиления импульса передатчика (TxGain = 0..100)
        void SetTxGain(uint8_t TxGain);
			   		
        // Запуск работы
        bool Start();
        // Запуск работы c задержкой (для отладочных режимов)
        bool Start_with_delay();

        // Остановка работы
        bool Stop();

        void SetLogger(logger*);                            //  Устанавливает класс для записи отладочных сообщений

        USHORT m_PWRecTime_us;                              //  Длительность окна приёма в микросекундах
        USHORT m_StRec_points;          //  Номер точки АЦП с начала приёма, после которой
                                        //  начинаем суммирование по глубине

        USHORT m_EndRec_points;         //  Номер точки АЦП с начала приёма, после которой
                                        //  заканчиваем суммирование по глубине

        USHORT m_MaxRec_points;         //  Максимальное колличество точек АЦП в окне приёма

    private:

        ZD4BU2018_ePROBES m_CurrProbe;                      //  Текущий выбранный датчик
        ZD4BU2018_eMODES  m_CurrProbeMode;                  //  Текущий режим работы выбранного датчика

		short m_TGC[256];									// Храним текущую таблицу TGC
		void InitTGC(void);									// Инициализация таблицы TGC и загрузка её в устройство
		void WriteTGC (void);                               // Загружает текущую таблицу TGC в память устройства
        bool SetDepParameters();                            // Загрузка зависимых параметров

        logger* m_DbgLog;
        bool IsLoggerInitted;

        USHORT m_PWPulseTime_us;                            //  Длительность импульса передатчика в микросекундах
        USHORT m_PWDepth_mm;                                //  Текущее значение измеряемой глубины в мм
        float m_PWSampleVolume_mm;                          //  Текущее значение измеряемого объёма в мм

};

#endif // ZD4BU_2018_H
