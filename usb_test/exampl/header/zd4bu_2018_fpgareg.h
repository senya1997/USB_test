#ifndef ZD4BU_2018_FPGAREG_H
#define ZD4BU_2018_FPGAREG_H

#define ZD4BU_2018_FPGARegNumber 128

typedef enum ZD4BU_2018_FPGAReg
{
    Adr_Reserved0,
    Adr_Test,        // 1 Регистр тестирования записи / чтения по USB
    Adr_Clr,         // 2 Сброс счетчиков адресации записи таблиц
    Adr_Reserved3,
    Adr_Reserved4,
    Adr_Reserved5,
    Adr_Reserved6,
    Adr_Reserved7,
    Adr_Reserved8,
    Adr_Reserved9,
    Adr_Reserved10,
    Adr_Reserved11,
    Adr_Reserved12,
    Adr_Reserved13,
    Adr_Reserved14,
    Adr_Reserved15,
    Adr_Reserved16,
    Adr_Reserved17,
    Adr_Reserved18,
    Adr_Reserved19,
    Adr_PTrF,           // 20 Частота импульса передатчика PW Doppler : 50 - 2MHz, 10 - 10MHz
    Adr_PTrN,           // 21 Ширина импульса передатчика(число периодов)
    Adr_Reserved22,
    Adr_Reserved23,
    Adr_Reserved24,
    Adr_Reserved25,
    Adr_DopMode,		// 26 0 - PW мода / 1 - CW мода
    Adr_CWF,            // 27 Частота излучения передатчика CW Doppler : 0 - 4MHz; 1 - 8MHz
    Adr_CWChN,          // 28 Выбранный канал CW : 0, 1
    Adr_PDChN,      	// 29 Выбранный канал PW : 0, 1, 2, 3
    Adr_CWEn,           // 30 0 - выключение / 1 - включение излучения CW
    Adr_PWTest,      	// 31   0 - в буфер записываются реальные данные с приёмника
                        //      1 - в буфер вместо данных записываются значения счётчика
    Adr_PWCh,          	// 32   0 - одноканальный PW режим
                        //      1 - двухканальный PW режим
    Adr_CWTest,         // 33   0 - в буфер записываются реальные данные с приёмника
                        //      1 - в буфер вместо данных записываются значения счётчика
    Adr_FirBPFilterTab, // 34   входной полосовой фильтр
    Adr_FirLPFilterTab, // 35	выходной антиэлайзинговый(НЧ) фильтр
    Adr_AdcSpiNum,      // 36   Колличесвто слов, загружаемых по SPI в AFE5809
    Adr_AdcSpiAddr,     // 37	Адрес регистра в AFE5809, куда хотим отправить или прочитать данные
    Adr_AdcSpiData,		// 38	Данные, которые хотим записать в AFE5809
    Adr_AdcReset,		// 39	Програмный сброс AFE5809
    Adr_InterpRatio,    // 40   Коэффициент интерполяции для CIC фильтра
    Adr_AdcPwsMode,		// 41   Энергосбережение AFE5809 0 - NORMAL, 0x0001 - PDN_GBL, 0x0002 - PDN_ADC, 0x0004 - PDN VCA
    Adr_ADCch_PW21,     // 42	0..7: выбор 1 из 8 каналов АЦП для первого канла PW режима
    Adr_SetPower,       // 43	Управление напряжением питания передатчика(+маска 0x8000 - PW, 0x4000 - CW; код - биты 11..0)
    Adr_TGC,			// 44	таблица TGC
    Adr_ADCch_PW22,		// 45   0..7: выбор 1 из 8 каналов АЦП для второго канла PW режима
    Adr_ADCch_PW10,     // 46	0..7: выбор 1 из 8 каналов АЦП для третьего канла PW режима
    Adr_DPeriod,        // 47	Длительность строки(определяет PRF - период повторения зондирований) (*20ns)
    Adr_Reserved48,
    Adr_Reserved49,
    Adr_Start_Delay,    // 50   Задержка между получением команды "старт" и стартом командоаппарата *1.3 мс
    Adr_Reserved51,
    Adr_Reserved52,
    Adr_Reserved53,
    Adr_Reserved54,
    Adr_Start_CA,       // 55   Управление командоаппаратом(КА) :
                        // 0x0000 - останов, 0x0001 - непрерывно(с периодом Adr_DPeriod), 0x0002 или 0x0003 - однократно
    Adr_Reserved56,
    Adr_Reserved57,
    Adr_BufSizeD,       // 58	Количество отсчетов, записываемых в буфер данных, БЕЗ хидера. (отсчеты следуют парами через 1 us)
    Adr_DataStartD,     // 59   Начало "окна" ввода данных(*20ns)
    Adr_DBufReset,		// 60	Сброс буфера данных для обмена с FT600
    Adr_AbortPipe,		// 61	Сброс буфера данных и завершение сессии(запись байта если висит TXE)
    Adr_Reserved62,
    Adr_Reserved63,
    Adr_Reserved64,
    Adr_Reserved65,
    Adr_Reserved66,
    Adr_Reserved67,
    Adr_Reserved68,
    Adr_GetInfo,        // 69	Чтение блока параметров(8 слов, те же, что в хидерах строк)
    Adr_SimFreq,        // 70   Частота сигнала на выходе ядра DDS
    Adr_SimAmpl,        // 71   Амплитуда сигнала на выходе ядра DDS
    Adr_SimMode,        // 72   0x0000 - для отладки: имитатор синуса генерирует непрерывный сигнал по всем глубинам
                        //      0x0001 - для отладки: имитатор синуса генерирует сигнал только с заданной глубины
    Adr_SimStDepth,     // 73   Начальная глубина для Adr_SimMode = 1
    Adr_SimEndDepth,    // 74   Конечная глубина для Adr_SimMode = 1
    Adr_SimEn,          // 75   0x0000 - на вход цифрового приёмника поступают реальные данные с АЦП
                        //      0x0001 - для отладки: на вход цифрового приёмника поступают данные с имитатора синуса
    Adr_Reserved76,
    Adr_Reserved77,
    Adr_Reserved78,
    Adr_PWPlayDiscBit,  // 79  Количество отбрасываемых бит после суммирования в PW - режиме
    Adr_PWPlayStDepth,  // 80  Начальная глубина измерительного объёма
    Adr_PWPlayEndDepth, // 81  Конечная глубина измерительного объёма
    Adr_Reserved82,
    Adr_Reserved83,
    Adr_Reserved84,
    Adr_Reserved85,
    Adr_Reserved86,
    Adr_Reserved87,
    Adr_Reserved88,
    Adr_Reserved89,
    Adr_Reserved90,
    Adr_Reserved91,
    Adr_Reserved92,
    Adr_Reserved93,
    Adr_Reserved94,
    Adr_Reserved95,
    Adr_Reserved96,
    Adr_Reserved97,
    Adr_Reserved98,
    Adr_Reserved99,
    Adr_Reserved100,
    Adr_Reserved101,
    Adr_Reserved102,
    Adr_Reserved103,
    Adr_Reserved104,
    Adr_Reserved105,
    Adr_Reserved106,
    Adr_Reserved107,
    Adr_Reserved108,
    Adr_Reserved109,
    Adr_Reserved110,
    Adr_Reserved111,
    Adr_Reserved112,
    Adr_Reserved113,
    Adr_Reserved114,
    Adr_Reserved115,
    Adr_Reserved116,
    Adr_DataCtrl,   // 117          0x0000 - в USB отправляются данные с выхода дециматора
                    // для отладки: 0x0001 - в USB отправляются данные с выхода АЦП
                    // для отладки: 0x0002 - в USB отправляются данные с выхода полосового фильтра
                    // для отладки: 0x0003 - в USB отправляются данные Real с выхода модулятора
                    // для отладки: 0x0004 - в USB отправляются данные Imag с выхода модулятора
                    // для отладки: 0x0005 - в USB отправляются данные Real с выхода ФНЧ
                    // для отладки: 0x0006 - в USB отправляются данные Imag с выхода ФНЧ
    Adr_Reserved118,
    Adr_Reserved119,
    Adr_Reserved120,
    Adr_Reserved121,
    Adr_CODPlayMult,   // 122 умножающий коэффициент перед отправкой данных в кодек
    Adr_CODReset,   // 123 сброс кодека TLV320AIC3254
    Adr_CODSpiData,	// 124 регистр данных для кодека TLV320AIC3254

    //Управление контроллером интерфейса 1-Wire
    Adr_1WireFIFO,	// 125  Регистр fifo для загрузки данных в устройство 1-WIRE
    Adr_1WireSel,	// 126  Регистр выбора текущего контроллера 1-WIRE
    Adr_Reserved127
} ZD4BU_2018_FPGAReg;

#endif // ZD4BU_2018_FPGAREG_H
