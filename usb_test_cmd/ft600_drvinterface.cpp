#include "ft600_drvinterface.h"
/*
 * FT600 Data Loopback Demo App
 *
 * Copyright (C) 2016 FTDI Chip
 *
 */

//#include "ft600_drvinterface.h"

 ////////////////////////////////////////////////////////////////////////////////////////////////////
 //
 // CFTDIDrvIface::CFTDIDrvIface
 //      This is the wrapper class to the driver.
 //      Currently this application uses the test driver - FT600 API over WinUSB.
 //      Once D2XX supports FT600, this file will be modified to use the D2XX APIs.
 //      D2XX driver is the USB driver for all FTDI USB products.
 //
 // Summary
 //
 // Parameters
 //
 // Return Value
 //
 // Notes
 //
 ////////////////////////////////////////////////////////////////////////////////////////////////////

FT600_DrvInterface::FT600_DrvInterface()
    : m_FTHandle(nullptr)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFTDIDrvIface::~CFTDIDrvIface
//
// Summary
//
// Parameters
//
// Return Value
//
// Notes
//
////////////////////////////////////////////////////////////////////////////////////////////////////

FT600_DrvInterface::~FT600_DrvInterface()
{
    Cleanup();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFTDIDrvIface::CFTDIDrvIface
//
// Summary
//      Initializes the driver
//
// Parameters
//      a_eTypeOpenBy   - Indicates how device will be opened
//      a_pvOpenBy      - Indicates the parameter to be used in opening the device
//
// Return Value
//      TRUE if device was initialized properly; otherwise returns FALSE
//
// Notes
//      None
//
// Проверка наличия нашего USB устройство,
// в случае успеха устройство открывается и возвращается TRUE
////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL FT600_DrvInterface::Initialize(
    EOPEN_BY a_eTypeOpenBy,
    PVOID a_pvOpenBy,
    PVOID a_pvParam1
)
{
    //BOOL bResult = FALSE;
    FT_STATUS ftStatus = FT_OK;
    //   int iDeviceNumber = 0;
    //   int iNumTries = 0;
    DWORD dwNumDevices = 0;
    //DWORD dwDeviceIndex = 0;
    FT_DEVICE_LIST_INFO_NODE *ptDevicesInfo = nullptr;

    char* p_chDeviceDescr = (char*)a_pvOpenBy; //Описание устройства
    char* chSerialNumber = (char*)a_pvParam1; //

    dwNumDevices = GetNumberOfDevicesConnected();
    if (dwNumDevices == 0)
    {
        StatusStr = "FT600_DrvInterface::Initialize Error! Not USB devices is connected!\n";
        //Устройства не обнаружено!
        return FALSE;
    }

    if (GetDevicesInfoList(&ptDevicesInfo) == 0)
    {
        StatusStr = "FT600_DrvInterface::Initialize Error! No device is connected!\n";
        return FALSE;
    }

    bool Detect_Ok = false;

    for (DWORD i = 0; i < dwNumDevices; i++)
    {
        //Свой/Чужой?
        if ((strcmp(ptDevicesInfo[i].Description, p_chDeviceDescr) == 0) &&
            (strcmp(ptDevicesInfo[i].SerialNumber, chSerialNumber) == 0))
        {
            //Устройство найдено
            Detect_Ok = true;
            break;
        }
    }

    ReleaseDevicesInfoList(ptDevicesInfo);

    //Устройство найдено
    if (!Detect_Ok)
    {
        StatusStr = "FT600_DrvInterface::Initialize Error! Not USB devices is connected!\n";
        return FALSE;  // Не нашли устройство, отваливаем
    }

    switch (a_eTypeOpenBy)
    {
       case EOPEN_BY_GUID:
       {
           ftStatus = FT_Create(a_pvOpenBy, FT_OPEN_BY_GUID, &m_FTHandle);
           break;
       }
       case EOPEN_BY_DESC:
       {
           // ftStatus = FT_Create(a_pvOpenBy, FT_OPEN_BY_DESCRIPTION, &m_FTHandle);
           ftStatus = FT_Create((PVOID)p_chDeviceDescr, FT_OPEN_BY_DESCRIPTION, &m_FTHandle);  // Наш вариант
           break;
       }
       case EOPEN_BY_SERIAL:
       {
           ftStatus = FT_Create(a_pvOpenBy, FT_OPEN_BY_SERIAL_NUMBER, &m_FTHandle);
           break;
       }
       case EOPEN_BY_INDEX:
       {
           ULONG ulIndex = atoi((CHAR*)a_pvOpenBy);
           FT_Create((PVOID)ulIndex, FT_OPEN_BY_INDEX, &m_FTHandle);
           break;
       }
    }

    if (FT_FAILED(ftStatus))
    {
        StatusStr = "FT600_DrvInterface::Initialize Error FT_Create() !\n";
        //Ошибка, не смогли открыть устройство
        return FALSE;  // отваливаем
    }

    /* Configure the GPIOs to OUTPUT mode.*/
    ftStatus = FT_EnableGPIO(m_FTHandle,
        (FT_GPIO_DIRECTION_OUT << FT_GPIO_0) | (FT_GPIO_DIRECTION_OUT << FT_GPIO_1),
        (FT_GPIO_DIRECTION_OUT << FT_GPIO_0) | (FT_GPIO_DIRECTION_OUT << FT_GPIO_1));

    if (FT_FAILED(ftStatus))
    {
        StatusStr = "FT600_DrvInterface::Initialize  Error! FT_EnableGPIO()!\n";
        Cleanup();
        return FALSE;  // отваливаем
    }

    /* Set the pins to LOW */
    ftStatus = FT_WriteGPIO(m_FTHandle,
        (FT_GPIO_VALUE_HIGH << FT_GPIO_0) | (FT_GPIO_VALUE_HIGH << FT_GPIO_1), /* mask - GPIO_0 and GPIO_1 are being written*/
        (FT_GPIO_VALUE_LOW << FT_GPIO_0) | (FT_GPIO_VALUE_LOW << FT_GPIO_1)); /* value for GPIO_0 and GPIO_1 */

    if (FT_FAILED(ftStatus))
    {
        StatusStr = "FT600_DrvInterface::Initialize Error! FT_WriteGPIO()!\n";
        Cleanup();
        return FALSE;  // отваливаем
    }

    return TRUE;
}

VOID FT600_DrvInterface::Cleanup()
{
    if (m_FTHandle)
    {
        FT_Close(m_FTHandle);
        m_FTHandle = nullptr;
    }
}

BOOL FT600_DrvInterface::ReadChipConfiguration(FT_60XCONFIGURATION * pConfiguration)
{
    FT_STATUS ftResult;
    ftResult = FT_GetChipConfiguration(m_FTHandle, (PVOID)pConfiguration);

    if (FT_FAILED(ftResult))
        return FALSE;
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
BOOL FT600_DrvInterface::SetStringDescriptors(UCHAR* pStringDescriptors, ULONG ulSize, const CHAR* pManufacturer, const CHAR* pProductDescription, const CHAR* pSerialNumber)
{
    ULONG lLen = 0; UCHAR bLen = 0;
    UCHAR* pPtr = pStringDescriptors;
    if (ulSize != 128 || pStringDescriptors == NULL)
        return FALSE;
    if (pManufacturer == NULL || pProductDescription == NULL || pSerialNumber == NULL)
        return FALSE;
    // Verify input parameters
    {
        // Manufacturer: Should be 15 bytes maximum printable characters
        lLen = ULONG(strlen(pManufacturer));
        if (lLen < 1 || lLen >= 16)
            return FALSE;
        for (ULONG i = 0; i < lLen; i++)
            if (!isprint(pManufacturer[i]))
                return FALSE;
        // Product Description: Should be 31 bytes maximum printable characters
        lLen = ULONG(strlen(pProductDescription));
        if (lLen < 1 || lLen >= 32)
            return FALSE;
        for (ULONG i = 0; i < lLen; i++)
            if (!isprint(pProductDescription[i]))
                return FALSE;
        // Serial Number: Should be 15 bytes maximum alphanumeric characters
        lLen = ULONG(strlen(pSerialNumber));
        if (lLen < 1 || lLen >= 16)
            return FALSE;
        for (ULONG i = 0; i < lLen; i++)
            if (!isalnum(pSerialNumber[i]))
                return FALSE;
    }
    // Construct the string descriptors
    {
        // Manufacturer
        bLen = UCHAR(strlen(pManufacturer));
        pPtr[0] = bLen * 2 + 2; pPtr[1] = 0x03;
        for (ULONG i = 2, j = 0; i < pPtr[0]; i += 2, j++)
        {
            pPtr[i] = pManufacturer[j];
            pPtr[i + 1] = '\0';
        }
        pPtr += pPtr[0];
        // Product Description
        bLen = UCHAR(strlen(pProductDescription));
        pPtr[0] = bLen * 2 + 2; pPtr[1] = 0x03;
        for (ULONG i = 2, j = 0; i < pPtr[0]; i += 2, j++)
        {
            pPtr[i] = pProductDescription[j];
            pPtr[i + 1] = '\0';
        }
        pPtr += pPtr[0];
        // Serial Number
        bLen = UCHAR(strlen(pSerialNumber));
        pPtr[0] = bLen * 2 + 2; pPtr[1] = 0x03;
        for (ULONG i = 2, j = 0; i < pPtr[0]; i += 2, j++)
        {
            pPtr[i] = pSerialNumber[j];
            pPtr[i + 1] = '\0';
        }
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFTDIDrvIface::WritePipe
//
// Summary
//      Writes specified data to a specified endpoint
//
// Parameters
//      a_ucPipeID              - Indicates the endpoint where data will be sent
//      a_pucBuffer             - Contains the data that will be sent
//      a_ulBufferLength        - Specifies the number of bytes of data that will be sent
//      a_pulLengthTransferred  - Pointer to contain the number of bytes sent
//
// Return Value
//      TRUE if successful, FALSE otherwise
//
// Notes
//      None
//
////////////////////////////////////////////////////////////////////////////////////////////////////

FT_STATUS FT600_DrvInterface::WritePipe(
    UCHAR a_ucPipeID,
    PUCHAR a_pucBuffer,
    ULONG a_ulBufferLength,
    PULONG a_pulLengthTransferred,
    LPOVERLAPPED a_pOverlapped
)
{
    return FT_WritePipe(
        m_FTHandle,
        a_ucPipeID,
        a_pucBuffer,
        a_ulBufferLength,
        a_pulLengthTransferred,
        a_pOverlapped
    );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CFTDIDrvIface::ReadPipe
//
// Summary
//      Read data from a specified endpoint
//
// Parameters
//      a_ucPipeID              - Indicates the endpoint where data will be received
//      a_pucBuffer             - Contains the pointer that will contain the data received
//      a_ulBufferLength        - Specifies the number of bytes of data that will be received
//      a_pulLengthTransferred  - Pointer to contain the number of bytes received
//
// Return Value
//      TRUE if successful, FALSE otherwise
//
// Notes
//      None
//
////////////////////////////////////////////////////////////////////////////////////////////////////

FT_STATUS FT600_DrvInterface::ReadPipe(
    UCHAR a_ucPipeID,
    PUCHAR a_pucBuffer,
    ULONG a_ulBufferLength,
    PULONG a_pulLengthTransferred,
    LPOVERLAPPED a_pOverlapped
)
{
    return FT_ReadPipe(
        m_FTHandle,
        a_ucPipeID,
        a_pucBuffer,
        a_ulBufferLength,
        a_pulLengthTransferred,
        a_pOverlapped
    );
}

//////////////////////////////////////////////////////////////////////////////////////
BOOL FT600_DrvInterface::WriteChipConfiguration(FT_60XCONFIGURATION * pConfiguration)
{
    FT_STATUS ftResult;
    ftResult = FT_SetChipConfiguration(m_FTHandle, (PVOID)pConfiguration);

    if (FT_FAILED(ftResult))
        return FALSE;
    return TRUE;
}

//*
BOOL FT600_DrvInterface::WriteGPIO(UINT32 u32Mask, UINT32 u32Data)
{
    FT_STATUS ftResult;

    ftResult = FT_WriteGPIO(m_FTHandle, u32Mask, u32Data);
    if (FT_FAILED(ftResult))
    {
        return FALSE;
    }

    return TRUE;
}

DWORD FT600_DrvInterface::GetNumberOfDevicesConnected()
{
    FT_STATUS ftStatus = FT_OK;
    DWORD dwNumDevices = 0;

    ftStatus = FT_ListDevices(&dwNumDevices, NULL, FT_LIST_NUMBER_ONLY);
    if (FT_FAILED(ftStatus))
    {
        return 0;
    }

    return dwNumDevices;
}

DWORD FT600_DrvInterface::GetDevicesInfoList(FT_DEVICE_LIST_INFO_NODE **pptDevicesInfo)
{
    FT_STATUS ftStatus = FT_OK;
    DWORD dwNumDevices = 0;

    ftStatus = FT_CreateDeviceInfoList(&dwNumDevices);
    if (FT_FAILED(ftStatus))
    {
        return 0;
    }

    *pptDevicesInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * dwNumDevices);
    if (!(*pptDevicesInfo))
    {
        return 0;
    }

    ftStatus = FT_GetDeviceInfoList(*pptDevicesInfo, &dwNumDevices);
    if (FT_FAILED(ftStatus))
    {
        free(*pptDevicesInfo);
        *pptDevicesInfo = NULL;
        return 0;
    }

    return dwNumDevices;
}

VOID FT600_DrvInterface::ReleaseDevicesInfoList(FT_DEVICE_LIST_INFO_NODE *ptDevicesInfo)
{
    if (ptDevicesInfo)
    {
        free(ptDevicesInfo);
        ptDevicesInfo = NULL;
    }
}
