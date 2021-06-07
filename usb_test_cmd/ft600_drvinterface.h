#ifndef FT600_DRVINTERFACE_H
#define FT600_DRVINTERFACE_H

/*
 * FT600 Data Loopback Demo App
 *
 * Copyright (C) 2016 FTDI Chip
 *
 */

// Define when linking with static library
// Undefine when linking with dynamic library
//
#define FTD3XX_STATIC

#include  <ftd3xx.h>
#include  <string>

typedef enum _EOPEN_BY
{
    EOPEN_BY_GUID,
    EOPEN_BY_DESC,
    EOPEN_BY_SERIAL,
    EOPEN_BY_INDEX,

} EOPEN_BY;

class FT600_DrvInterface
{
   public:
        FT600_DrvInterface();
        ~FT600_DrvInterface();

        BOOL Initialize(EOPEN_BY a_eTypeOpenBy, PVOID a_pvOpenBy,   PVOID a_pvParam1);
        inline bool isInitialized() const {	return m_FTHandle != nullptr; }
        VOID Cleanup();
        BOOL ReadChipConfiguration(FT_60XCONFIGURATION * pConfiguration);
        BOOL WriteChipConfiguration(FT_60XCONFIGURATION * pConfiguration);
        BOOL SetStringDescriptors(UCHAR* pStringDescriptors, ULONG ulSize, const CHAR* pManufacturer, const CHAR* pProductDescription, const CHAR* pSerialNumber);
        BOOL WriteGPIO(UINT32 u32Mask,UINT32 u32Data);
        FT_STATUS WritePipe(UCHAR a_ucPipeID,PUCHAR a_pucBuffer,ULONG a_ulBufferLength,PULONG a_pulLengthTransferred,LPOVERLAPPED a_pOverlapped);
        FT_STATUS ReadPipe(UCHAR a_ucPipeID,PUCHAR a_pucBuffer,ULONG a_ulBufferLength,PULONG a_pulLengthTransferred,LPOVERLAPPED a_pOverlapped);

        DWORD GetDevicesInfoList(FT_DEVICE_LIST_INFO_NODE **pptDevicesInfo);

        std::string StatusStr;

   private:

        DWORD GetNumberOfDevicesConnected();
        VOID ReleaseDevicesInfoList(FT_DEVICE_LIST_INFO_NODE *ptDevicesInfo);

        FT_HANDLE   m_FTHandle = nullptr;  //Handle driver
};

#endif
