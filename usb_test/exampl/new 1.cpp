// 1) ЗАГРУЗКА АЛЬТЕРЫ
 
/*** Загрузка Альтеры из rbf-файла, бросает исключения */
void CzdtHardware::loadCyclone()
{
    Q_ASSERT(mP_IsHardwarePresent);
    //формируем имя
QString fullFilename = QCoreApplication::applicationDirPath() + "/rbf/";
    switch(m_AlteraType)
    {
        case CYCLONE_10CL080:
            fullFilename += "Z64";
            break;
        case CYCLONE_10CL120:
            fullFilename += "Z64_120";
            break;
        default:
            Q_UNREACHABLE();
    }
    fullFilename += ".rbf";
QFile firmwareFile(fullFilename);
 
    //открываем
    if(!firmwareFile.open(QIODevice::ReadOnly))
    {//нет такого файла
    QString strErr = tr("File '%1' not found!").arg(fullFilename);
        qDebug() << strErr;
        throw std::runtime_error(strErr.toUtf8());
    }
 
    //размер
qint64 fileSize = firmwareFile.size();
    //читаем
Ipp8u* databuf = ippsMalloc_8u((int)fileSize);
    if(firmwareFile.read((char*)databuf, fileSize) != fileSize)
    {//что-то не заладилось с чтением
    QString strErr = tr("Can not read firmware file Z64.rbf!");
        qDebug() << strErr;
        throw std::runtime_error(strErr.toUtf8());
    }
    firmwareFile.close();
 
scanWord* databuf2Load;
static const quint32 WORDS_4SEND_BYTE = 16; //сколько нужно послать слов, чтобы передать один байт
 
#ifdef FT601
    databuf2Load = ippsMalloc_32u((int)fileSize * WORDS_4SEND_BYTE);
scanWord DATA1_CLOCK0 = 0x40000000; //бит 30
scanWord DATA1_CLOCK1 = 0xC0000000; //биты 30, 31
scanWord DATA0_CLOCK0 = 0x00000000;
scanWord DATA0_CLOCK1 = 0x80000000; //бит 31
    if(m_AlteraType == CYCLONE_10CL120)
    {
         DATA1_CLOCK0 = 0x00008000; //бит 15
		         DATA1_CLOCK1 = 0x0000A000; //биты 15, 13
		         DATA0_CLOCK0 = 0x00000000;
		         DATA0_CLOCK1 = 0x00002000; //бит 13
    }
#else
    databuf2Load = ippsMalloc_16u((int)fileSize * WORDS_4SEND_BYTE);
static const scanWord DATA1_CLOCK0 = 0x0400;
static const scanWord DATA1_CLOCK1 = 0x0C00;
static const scanWord DATA0_CLOCK0 = 0x0000;
static const scanWord DATA0_CLOCK1 = 0x0800;
#endif
 
    quint32 k = 0;
	    for(quint32 j = 0; j < fileSize; ++j)
	    {
        for(quint32 i = 0; i < 8; ++i)
        {
			             if((databuf[j] >> i) & 1)
             {
				                 databuf2Load[k++] = DATA1_CLOCK0;  //data = 1 clock = 0
				                 databuf2Load[k++] = DATA1_CLOCK1;  //data = 1 clock = 1
             }
			             else
			             {
				                 databuf2Load[k++] = DATA0_CLOCK0;  //data = 0 clock = 0
                 databuf2Load[k++] = DATA0_CLOCK1;  //data = 0 clock = 1
			             }
        }
    }
	    SAFE_IPPFREE_PTR(databuf);
	    Q_ASSERT(m_pUSBDrv != nullptr);
 
	    //NConfig
	    m_pUSBDrv->WriteGPIO(1, 0);
	    QThread::msleep(2);
	    m_pUSBDrv->WriteGPIO(1, 1);
	    m_pUSBDrv->AbortPipe(WRITEPIPE_ID);
	    m_pUSBDrv->FlushPipe(WRITEPIPE_ID);
	    QThread::msleep(2);
	    //заливаем
	    m_pUSBDrv->SetPipeTimeout(WRITEPIPE_ID, 10000); //Почему? Патамушта!
ULONG ulActualBytesWritten = 0;
FT_STATUS ftStatus = m_pUSBDrv->WritePipe(0x02, (PUCHAR)databuf2Load, (ULONG)fileSize * WORDS_4SEND_BYTE * sizeof(scanWord), &ulActualBytesWritten, nullptr);
	    if(ftStatus != FT_OK)
	    {//что-то не заладилось с загрузкой
	    QString strErr = tr("Can not LOAD firmware into Altera!");
		        qDebug() << strErr;
		        throw std::runtime_error(strErr.toUtf8());
	    }
	    SAFE_IPPFREE_PTR(databuf2Load);
}
 
 
// 2) ИНИЦИАЛИЗАЦИЯ
void CzdtHardware::initHardware()
{
    Q_ASSERT(mP_IsHardwarePresent);
	    //Драйвер USB
	    m_pUSBDrv = new CFTDIDrvIface();
OPEN:
	    //Открываем устройство по индексу
	    if(!m_pUSBDrv->Initialize(EOPEN_BY_INDEX, (PVOID)"0"))
	    {//не получилось
		    QString err(tr("Can not initialize USB driver!"));
		        throw std::runtime_error(err.toUtf8());
	    }
	    //Проверяем и при необходимости корректируем конфигурацию FTDI
FT_60XCONFIGURATION oConfigurationData;
bool isCorrected = false;
	    //Читаем конфигурацию
	    if(!m_pUSBDrv->ReadChipConfiguration(&oConfigurationData))
	    {//не получилось
		    QString err(tr("Can not read FTDI configuration!"));
		        throw std::runtime_error(err.toUtf8());
	    }
	    //Проверяем конфигурацию
	    if(oConfigurationData.OptionalFeatureSupport != (CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN |     CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL))
	    {//Session underrun
		        oConfigurationData.OptionalFeatureSupport = CONFIGURATION_OPTIONAL_FEATURE_DISABLECANCELSESSIONUNDERRUN | CONFIGURATION_OPTIONAL_FEATURE_DISABLEUNDERRUN_INCHALL;
		        isCorrected = true;
	    }
	    if(oConfigurationData.FIFOClock != CONFIGURATION_FIFO_CLK_66)
	    {//FIFO clock
		        oConfigurationData.FIFOClock = CONFIGURATION_FIFO_CLK_66;
		        isCorrected = true;
	    }
	    if(oConfigurationData.FIFOMode != CONFIGURATION_FIFO_MODE_245)
	    {//FIFO mode
		        oConfigurationData.FIFOMode = CONFIGURATION_FIFO_MODE_245;
		        isCorrected = true;
	    }
	    if(oConfigurationData.ChannelConfig != CONFIGURATION_CHANNEL_CONFIG_1)
	    {//Num of pipes
		        oConfigurationData.ChannelConfig = CONFIGURATION_CHANNEL_CONFIG_1;
		        isCorrected = true;
	    }
 
	    //Пишем, если корректировалась
	    if(isCorrected)
	    {
		        if(!m_pUSBDrv->WriteChipConfiguration(&oConfigurationData))
		        {//прошивка не задалась
			        QString err(tr("Can not setup FTDI chip configuration!"));
			            throw std::runtime_error(err.toUtf8());
		        }
		        QThread::msleep(1000);
		        goto OPEN;
    }
 
FT_DEVICE_DESCRIPTOR deviceDescr;
	    if(!m_pUSBDrv->GetDeviceDescriptor(&deviceDescr))
	    {//не получилось
	    QString err(tr("Can not read FTDI device descriptor!"));
		        throw std::runtime_error(err.toUtf8());
	    }
 
	    //Проверяем, что работаем именно на USB3
	    if(deviceDescr.bcdUSB != 0x310)
	    {//не USB3
	    QString err(tr("FTDI device does not meet USB specification 3.1! Check all cable connections."));
		        throw std::runtime_error(err.toUtf8());
	    }
 
	    //Тип Альтеры
    FT_STRING_DESCRIPTOR stringDescr;
	    if(!m_pUSBDrv->GetStringDescriptor(deviceDescr.iProduct, &stringDescr))
	    {//не получилось
	    QString err(tr("Can not read FTDI string descriptor!"));
		        throw std::runtime_error(err.toUtf8());
	    }
	    if(stringDescr.szString == QString(CYCLONE_10CL080_CONFIG_STRING).toStdWString())
		        m_AlteraType = CYCLONE_10CL080;
	    else
	    {
		        if(stringDescr.szString == QString(CYCLONE_10CL120_CONFIG_STRING).toStdWString())
			            m_AlteraType = CYCLONE_10CL120;
		        else
		        {//не задан тип Альтеры
		        QString err(tr("FPGA type not detected! Check FTDI configuration."));
			            throw std::runtime_error(err.toUtf8());
		        }
	    }
	    //включаем питание
	    switchPowerSupply(true);
	    //загружаем Альтеру
	    loadCyclone();
	    //инициализация бимформера
	    initBeamformer();
	    //инициализация коммутатора
	    initCommutator();
}