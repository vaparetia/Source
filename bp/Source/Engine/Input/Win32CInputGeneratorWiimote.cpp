   //----------------------------------------------------------------------------
// Win32CInputGeneratorWiimote.cpp
// Copyright 2008
   //----------------------------------------------------------------------------

#include "StdAfx.h"
#include <Windows.h>
//----------------------------------------------------------------------------

#include "Engine/System/COsContext.h"
#include "Engine/Math/CMatrix34.h"
#include "KPAD.cpp"

//----------------------------------------------------------------------------

extern "C"
{
#include "ExtLibraries/hid/hidsdi.h"
};

#include <setupapi.h>
#include "Engine/System/CSyncCriticalSection.h"
#include "Engine/System/CStopWatch.h"
#include "Engine/Input/Win32CInputGeneratorWiimote.h"
#include "Engine/Input/Win32CInputGenerator.h"

//----------------------------------------------------------------------------

int CInputGeneratorWiimote::sHIDIndex = 0;
int CInputGeneratorWiimote::sCurrentControllerNum = 0;

//----------------------------------------------------------------------------
//#define BPE_WIIMOTE_VERBOSE_OUTPUT

#ifdef BPE_WIIMOTE_VERBOSE_OUTPUT
#define BPE_WIIMOTE_REPORT_ERROR(_CInputGeneratorWiimote) bpe_debugger_printf("Wiimote Error: %s failed for controller: %d, Handle: 0x%08x, Error: %d\n", __FUNCTION__, _CInputGeneratorWiimote->sCurrentControllerNum, _CInputGeneratorWiimote->mFileHandleRead, GetLastError())
#define BPE_WIIMOTE_REPORT_ERROR_MSG(msg, _CInputGeneratorWiimote) bpe_debugger_printf("Wiimote Error: %s in %s for controller: %d, Handle: 0x%08x, Error: %d\n", msg, __FUNCTION__, _CInputGeneratorWiimote->sCurrentControllerNum, _CInputGeneratorWiimote->mFileHandleRead, GetLastError())
#define BPE_WIIMOTE_REPORT_MSG(msg, _CInputGeneratorWiimote) bpe_debugger_printf("Wiimote: %s in %s for controller: %d, Handle: 0x%08x\n", msg, __FUNCTION__, _CInputGeneratorWiimote->sCurrentControllerNum, _CInputGeneratorWiimote->mFileHandleRead)
#else
#define BPE_WIIMOTE_REPORT_ERROR(_CInputGeneratorWiimote)            (_CInputGeneratorWiimote)
#define BPE_WIIMOTE_REPORT_ERROR_MSG(msg, _CInputGeneratorWiimote)   (msg); (_CInputGeneratorWiimote)
#define BPE_WIIMOTE_REPORT_MSG(msg, _CInputGeneratorWiimote)         (msg); (_CInputGeneratorWiimote)

#endif

//----------------------------------------------------------------------------

CWiimoteRawStatus_NC::CWiimoteRawStatus_NC(uint8 const * pReport)
{
   // See http://wiibrew.org/wiki/Wiimote 
   // and http://wiibrew.org/wiki/Wiimote/Extension_Controllers

   mButtons = (pReport[2] << 8) | pReport[1];

   mAccX = pReport[3];
   mAccY = pReport[4];
   mAccZ = pReport[5];

   {
      SWiimoteRawIRInfo &ir = mIRObjs[0];
      uint32 const x = pReport[6]  | ((((uint32) pReport[8] >> 4) & 0x03) << 8);
	   uint32 const y = pReport[7]  | ((((uint32) pReport[8] >> 6) & 0x03) << 8);
      bool const bFound = !((x == 1023) && (y == 1023));
      ir.mX = (int16) x;
      ir.mY = (int16) y;
      ir.mSize = bFound ? 0 : SWiimoteRawIRInfo::kIRNotVisible;
      ir.mId = 0;
   }

   {
      SWiimoteRawIRInfo &ir = mIRObjs[1];
      uint32 const x = pReport[9]  | ((((uint32) pReport[8] >> 0) & 0x03) << 8);
	   uint32 const y = pReport[10] | ((((uint32) pReport[8] >> 2) & 0x03) << 8);
      bool const bFound = !((x == 1023) && (y == 1023));
      ir.mX = (int16) x;
      ir.mY = (int16) y;
      ir.mSize = bFound ? 0 : SWiimoteRawIRInfo::kIRNotVisible;
      ir.mId = 0;
   }

   {
      SWiimoteRawIRInfo &ir = mIRObjs[2];
      uint32 const x = pReport[11] | ((((uint32) pReport[13] >> 4) & 0x03) << 8);
	   uint32 const y = pReport[12] | ((((uint32) pReport[13] >> 6) & 0x03) << 8);
      bool const bFound = !((x == 1023) && (y == 1023));
      ir.mX = (int16) x;
      ir.mY = (int16) y;
      ir.mSize = bFound ? 0 : SWiimoteRawIRInfo::kIRNotVisible;
      ir.mId = 0;
   }

   {
      SWiimoteRawIRInfo &ir = mIRObjs[3];
      uint32 const x = pReport[14] | ((((uint32) pReport[13] >> 0) & 0x03) << 8);
	   uint32 const y = pReport[15] | ((((uint32) pReport[13] >> 2) & 0x03) << 8);
      bool const bFound = !((x == 1023) && (y == 1023));
      ir.mX = (int16) x;
      ir.mY = (int16) y;
      ir.mSize = bFound ? 0 : SWiimoteRawIRInfo::kIRNotVisible;
      ir.mId = 0;
   }

   {
      uint8 ncBuf[6];
      for (int i = 0; i < 6; i++)
      {
         // DecryptBufferByte
         uint8 src = pReport[i + 16];
         ncBuf[i] = (((src ^ 0x17) + 0x17) & 0xff);
      }
      
      mNC_StickX  = ncBuf[0] - 127;
      mNC_StickY  = ncBuf[1] - 127;
      mNC_AccX    = ncBuf[2] + ((ncBuf[5] >> 2) & 0x3);
      mNC_AccY    = ncBuf[3] + ((ncBuf[5] >> 4) & 0x3);
      mNC_AccZ    = ncBuf[4] + ((ncBuf[5] >> 6) & 0x3);
      mNC_Buttons = ncBuf[5] & 0x3;
   }
}

//----------------------------------------------------------------------------

CWiimoteRawStatus_NC::CWiimoteRawStatus_NC()
{
   memset(this, 0, sizeof(*this));
}

//----------------------------------------------------------------------------

CInputGeneratorWiimote::CInputGeneratorWiimote()
   : mWriteMode(kWRM_Async)
   , mControllerNumber(-1)
   , mFileHandleRead(BPE_INVALID_HANDLE_VALUE)
   , mFileHandleWrite(BPE_INVALID_HANDLE_VALUE)
   , mReadBufferAsyncIndex(2) // Start at 2 just in case the application wants to read data immediately before async IO completes.
   , mReadBufferAccessIndex(2)
   , mWriteBufferAsyncIndex(0)
   , mWriteBufferPendingIOCount(0)
   , mLEDs(0)
   , mRumble(0)
   , mInputType(kIT_ButtonsAccelIRExtension)
   , mIRMode(kIRM_Basic)
   , mpMemoryBuffer(NULL)
   , mAccCalX0(0x82)    // Default calibration values
   , mAccCalY0(0x82)
   , mAccCalZ0(0x82)
   , mAccCalX1G(0x9C)
   , mAccCalY1G(0x9C)
   , mAccCalZ1G(0x9C)
{

   // Initialize read buffer pointers
   for (int loop = 0; loop < kReadBufferCount; loop++)
   {
      mReadBuffer[loop].mpWiimote = this;
   }
   // Initialize write buffer pointers
   for (int loop = 0; loop < kWriteBufferCount; loop++)
   {
      mWriteBuffer[loop].mpWiimote = this;
   }

   // Connection controller
   if (!Connect())
   {
      // Not connected
      return;
   }

   // Enable read
   AsyncRead();

   // Calibrate
   if (!CalibrateAccelerometers())
   {
      // Couldn't read wiimote memory, something is screwed up, disconnect
      Disconnect();
      return;
   }

   // Set controller number
   mControllerNumber = sCurrentControllerNum;
   sCurrentControllerNum++;

   // Memory read OK, continue
   bpe_debugger_printf("Found Wiimote %d.\n", mControllerNumber);

   // Init input
   InitInput();
   // Set LED to controller number
   SetLEDState(0x1 << mControllerNumber);
   SleepEx(1, TRUE);

   KPADInit();
   KPADiConnectCallback(mControllerNumber, WPAD_ERR_NONE);
}

//----------------------------------------------------------------------------

CInputGeneratorWiimote::~CInputGeneratorWiimote()
{
   Disconnect();
}

//----------------------------------------------------------------------------

bool CInputGeneratorWiimote::Connect()
{
   bool bFound = false;
   GUID guid;

   // Get HID guid
   HidD_GetHidGuid(&guid);

   // Get handles to all HID devices
   HDEVINFO hDevInfo = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_DEVICEINTERFACE);

   // create a new interface data struct and initialize its size
   SP_DEVICE_INTERFACE_DATA diData;
   diData.cbSize = sizeof(diData);

   // get a device interface to a single device (enumerate all devices)
   while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, sHIDIndex, &diData))
   {
      DWORD size;

      // get the buffer size for this device detail instance (returned in the size parameter)
      SetupDiGetDeviceInterfaceDetail(hDevInfo, &diData, NULL, 0, &size, NULL);

      // create a detail struct and set its size
      TCHAR detailBuffer[1024];
      PSP_DEVICE_INTERFACE_DETAIL_DATA pDiDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA) detailBuffer;
      pDiDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

      // actually get the detail struct
      if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &diData, pDiDetail, sizeof(detailBuffer), &size, NULL))
      {
         // open a read/write handle to our device using the DevicePath returned
         //mFileHandleRead = CreateFile(pDiDetail->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
         mFileHandleRead = CreateFile(pDiDetail->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

         if (mFileHandleRead == INVALID_HANDLE_VALUE)
         {
            // Can't open this HID device
            BPE_WIIMOTE_REPORT_ERROR_MSG("CreateFile-Read failed", this);
            sHIDIndex++;
            continue;
         }

#ifdef BPE_WIIMOTE_VERBOSE_OUTPUT
         bpe_debugger_printf("Wiimote: HID enum: %d - %s : %d\n", sHIDIndex, pDiDetail->DevicePath, GetLastError());
#endif

         // create an attributes struct and initialize the size
         HIDD_ATTRIBUTES attrib;
         memset(&attrib, 0, sizeof(attrib));
         attrib.Size = sizeof(attrib);

         // get the attributes of the current device
         if (HidD_GetAttributes(mFileHandleRead, &attrib))
         {
            // if the vendor and product IDs match up
            if (attrib.VendorID == kVID && attrib.ProductID == kPID)
            {
#ifdef BPE_WIIMOTE_VERBOSE_OUTPUT
               bpe_debugger_printf("Found Wiimote: %d - %s : %d\n", sHIDIndex, pDiDetail->DevicePath, GetLastError());
#endif
               // Create a write file handle
               mFileHandleWrite = CreateFile(pDiDetail->DevicePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED, NULL);
               if (mFileHandleWrite != INVALID_HANDLE_VALUE)
               {
                  bFound = true;
                  sHIDIndex++;
                  break;
               }
               else
               {
                  BPE_WIIMOTE_REPORT_ERROR_MSG("CreateFile-Write failed", this);
               }
            }
            // otherwise this isn't the controller, so close up the file handle
            CloseHandle(mFileHandleRead);
            mFileHandleRead = BPE_INVALID_HANDLE_VALUE;
         }
      }
      else
      {
         // failed to get the detail struct
         bpe_debugger_printf("SetupDiGetDeviceInterfaceDetail failed on %d\n", sHIDIndex);
      }

      // move to the next device
      sHIDIndex++;
   }

   // clean up our list
   SetupDiDestroyDeviceInfoList(hDevInfo);

   return bFound; 
}

//----------------------------------------------------------------------------

bool CInputGeneratorWiimote::CalibrateAccelerometers()
{
   uint32 const kCalibrationDataAddress = 0x16;
   uint32 const kCalibrationDataSize = 7;

   uint8 buffer[kCalibrationDataSize];
   bool bResult = ReadWiimoteMemory(kCalibrationDataAddress, kCalibrationDataSize, buffer);
   if (!bResult)
   {
      // No read, try switching to sync mode for writes.
      mWriteMode = kWRM_Sync;

      // Try read again with new mode.
      bResult = ReadWiimoteMemory(kCalibrationDataAddress, kCalibrationDataSize, buffer);
      if (!bResult)
      {
         // Couldn't read memory in sync mode, return error.
         BPE_WIIMOTE_REPORT_ERROR(this);
         return false;
      }
   }

   // Get calibration data
   mAccCalX0 = buffer[0];
   mAccCalY0 = buffer[1];
   mAccCalZ0 = buffer[2];

   mAccCalX1G = buffer[4];
   mAccCalY1G = buffer[5];
   mAccCalZ1G = buffer[6];

   return true;
}

//----------------------------------------------------------------------------
// Be very careful using this function as caller owns pointer.
// Also calling this function before a previous async read has completed is undefined.
bool CInputGeneratorWiimote::ReadWiimoteMemory(uint32 const address, uint32 const size, uint8 * pDestBuffer)
{
   // Read 7 bytes from 0x0016
   bool bResult = ReadWiimoteMemoryAsync(address, size, pDestBuffer);
   if (!bResult)
   {
      BPE_WIIMOTE_REPORT_ERROR_MSG("ReadWiimoteMemoryAsync", this);
      return false;
   }

   CStopWatch time;    
   // Wait for read to complete
   uint32 previousMemoryBufferPos = 0;

   uint32 const kReadTimeOutMS = 150;

   while (mMemoryBufferPos < size)
   {
      // Wait for reads to complete.
      SleepEx(10, TRUE);
      if (previousMemoryBufferPos != mMemoryBufferPos)
      {
         // Received data, reset timer
         time.Reset();
      }

      // Check for timeout (prevent infinite loop on disconnection)
      if (time.GetElapsedMilliseconds() > kReadTimeOutMS)
      {
         break;
      }

#ifdef BPE_WIIMOTE_VERBOSE_OUTPUT
      bpe_debugger_printf(".");
#endif
   }

   // Reset memory buffer pointer
   mpMemoryBuffer = NULL;

#ifdef BPE_WIIMOTE_VERBOSE_OUTPUT
   bpe_debugger_printf("!\n");
#endif

   // Wait for reads to complete.
   // Check to see if we read all the data
   if (mMemoryBufferPos != size)
   {
      // Nope
      BPE_WIIMOTE_REPORT_ERROR(this);
      CancelIo(mFileHandleWrite);
      return false;
   }

   return true;
}

//----------------------------------------------------------------------------
// Be very careful using this function as caller owns pointer.
// Also calling this function before a previous async read has completed is undefined.
bool CInputGeneratorWiimote::ReadWiimoteMemoryAsync(uint32 const address, uint32 const size, uint8 * pDestBuffer)
{
   mpMemoryBuffer = pDestBuffer;
   mMemoryBufferPos = 0;
   mMemoryReadSize = size;

   CWiimoteWriteInfo * pInfo = GetNextWriteBuffer();

   pInfo->mData[0] = kOR_ReadMemory;
   pInfo->mData[1] = 0x00 | mRumble;
   pInfo->mData[2] = (uint8) ((address & 0x00FF0000) >> 16);
   pInfo->mData[3] = (uint8) ((address & 0x0000FF00) >> 8);
   pInfo->mData[4] = (uint8) ((address & 0x000000FF) >> 0);

   pInfo->mData[5] = (uint8) ((size & 0xFF00) >> 8);
   pInfo->mData[6] = (uint8) ((size & 0x00FF) >> 0);
   bool bResult = WriteWiimote(pInfo);
   return bResult;
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::SetRumbleState(bool const on)
{
   uint8 const kRumbleBit = 0x1;
   mRumble &= ~kRumbleBit;
   mRumble |= on ? kRumbleBit : 0;
   FlushLEDAndRumbleState();
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::SetLEDState(uint8 const leds)
{
   // LEDs use top 4 bits
   mLEDs = leds << 4;
   FlushLEDAndRumbleState();
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::FlushLEDAndRumbleState()
{
   CWiimoteWriteInfo * pInfo = GetNextWriteBuffer();
   pInfo->mData[0] = kOR_LEDsRumble;
   pInfo->mData[1] = mRumble | mLEDs;
   bool bResult = WriteWiimote(pInfo);
   if (!bResult)
   {
      // Write failed, disconnect wiimote
      Disconnect();
   }
}

//----------------------------------------------------------------------------
// Using mode 0x37, 100hz, Nunchuck, basic IR (10 bytes)
void CInputGeneratorWiimote::InitInput()
{
   // Enable IR      
   EnableIR(mIRMode);

   // Enable extension
   EnableExtension();

   // Get all data continuously
   CWiimoteWriteInfo * pInfo = GetNextWriteBuffer();
   pInfo->mData[0] = kOR_Type;
   uint8 const kContinuousData = 0x04;
   pInfo->mData[1] = kContinuousData | mRumble;      
   pInfo->mData[2] = mInputType;

   WriteWiimote(pInfo);
}

//----------------------------------------------------------------------------

bool CInputGeneratorWiimote::IsConnected() const
{
   if (mFileHandleRead != BPE_INVALID_HANDLE_VALUE)
   {
      return true;
   }
   return false;
}

//----------------------------------------------------------------------------
// Helper function for getting most recent data.
// Note that this is for basic functionality only. Advanced analysis would read all new data (multiple) since the last update.
// Also note that this pointer is only valid for a short period of time (> 1 frame, < 1 sec) as it will get overwritten when buffer wraps around.
CWiimoteReadInfo const * CInputGeneratorWiimote::GetCurrentReadInfo() const
{
   // Get current index that is being written to.
   uint32 const currentAccessIndex = mReadBufferAsyncIndex;

   // Last data will be 2 indices behind
   uint32 const validDataIndex = ((currentAccessIndex - 2) + kReadBufferCount) % kReadBufferCount;
   return &mReadBuffer[validDataIndex];
};

//----------------------------------------------------------------------------
// Spoofed wiimote read function for KPAD
void WPADRead( int32 chan, CInputGeneratorWiimote * pInput, void *status )                        
{  
   // TODO: Fill in the read data!
   if (chan == 0)
   {
      int const readIndex = pInput->mReadBufferAccessIndex % CInputGeneratorWiimote::kReadBufferCount;
      CWiimoteRawStatus_NC wr(pInput->mReadBuffer[readIndex].mData);


      WPADFSStatus nc;
      memset(&nc, 0, sizeof(nc));

      nc.button = 0;   // TODO
      nc.accX = wr.mAccX;
      nc.accY = wr.mAccY;
      nc.accZ = wr.mAccZ;
      for (int i = 0; i < 4; i++)
      {
         // No size or id in free style mode.
         nc.obj[i].x = wr.mIRObjs[i].mX;
         nc.obj[i].y = wr.mIRObjs[i].mY;
         if (wr.mIRObjs[i].mSize != SWiimoteRawIRInfo::kIRNotVisible)
         {
            // Fake out 'valid' size.
            // TODO: Check against real wii for correct size in 10 byte IR mode.
            nc.obj[i].size = 1;
         }
      }

      nc.dev = WPAD_DEV_FREESTYLE;
      nc.err = WPAD_ERR_NONE;                  

      nc.fsAccX = wr.mNC_AccX;               
      nc.fsAccY = wr.mNC_AccY;               
      nc.fsAccZ = wr.mNC_AccZ;               
      nc.fsStickX = wr.mNC_StickX;             
      nc.fsStickY = wr.mNC_StickY;

      memcpy(status, &nc, sizeof(nc));

#if 0
      {
         for (int i = 0; i < 4; i++)
         {
            DPDObject &dpd = nc.obj[i];
            if (dpd.size)
            {
               bpe_debugger_and_console_printf("%d - %4d / %4d \t", i, dpd.x, dpd.y);
            }
         }
      }
#endif
   }
}

// Call this function once per frame.
void CInputGeneratorWiimote::FrameUpdate()
{
   if (mControllerNumber == 0)
   {
      // Sleep to allow ReadFileEx IO to complete for all wii controllers.
      SleepEx(0, TRUE);
   }

   if (!IsConnected())
   {
      return;
   }

   // Callback KPAD
   int const readBufferAsyncIndex = mReadBufferAsyncIndex;
   
   while (mReadBufferAccessIndex < (readBufferAsyncIndex - 1))
   {
      KPADiSamplingCallback(mControllerNumber, this);
      mReadBufferAccessIndex++;
   }
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::EnableExtension()
{
   CWiimoteWriteInfo * pInfo = GetNextWriteBuffer();
   //REGISTER_EXTENSION_INIT			= 0x04a40040
   pInfo->mData[0] = kOR_WriteMemory;
   pInfo->mData[1] = 0x04;
   pInfo->mData[2] = 0xa4;
   pInfo->mData[3] = 0x00;
   pInfo->mData[4] = 0x40;

   pInfo->mData[5] = 0x01;

   pInfo->mData[6] = 0x00;
   WriteWiimote(pInfo);

   // Add code for reading calibration data from nunchuck here later if required. Seems OK at moment.
}

//----------------------------------------------------------------------------

// Turn on the IR sensor
void CInputGeneratorWiimote::EnableIR(EIRMode const mode)
{
   CWiimoteWriteInfo * pInfo = NULL;

   pInfo = GetNextWriteBuffer();
   pInfo->mData[0] = kOR_IR;
   pInfo->mData[1] = (0x04 | mRumble);
   WriteWiimote(pInfo);

   pInfo = GetNextWriteBuffer();
   pInfo->mData[0] = kOR_IR2;
   pInfo->mData[1] = (0x04 | mRumble);
   WriteWiimote(pInfo);

   pInfo = GetNextWriteBuffer();
   //REGISTER_IR				= 0x04b00030;
   pInfo->mData[0] = kOR_WriteMemory;
   pInfo->mData[1] = 0x04;
   pInfo->mData[2] = 0xb0;
   pInfo->mData[3] = 0x00;
   pInfo->mData[4] = 0x30;

   pInfo->mData[5] = 0x01;

   pInfo->mData[6] = 0x08;
   WriteWiimote(pInfo);

   // Wii level 1
   //uint8 const skSensitivitySettings[] = {0x02, 0x00, 0x00, 0x71, 0x01, 0x00, 0x64, 0x00, 0xfe, 0xfd, 0x05 };
   // Wii level 3
   //uint8 const skSensitivitySettings[] = {0x02, 0x00, 0x00, 0x71, 0x01, 0x00, 0xaa, 0x00, 0x64, 0x63, 0x03 };
   // Wii level 5
   uint8 const skSensitivitySettings[] = {0x07, 0x00, 0x00, 0x71, 0x01, 0x00, 0x72, 0x00, 0x20, 0x1f, 0x03 };

   // This settings seems to work best.
   //uint8 const skSensitivitySettings[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0xC0, 0x40, 0x00 };

   pInfo = GetNextWriteBuffer();
   //REGISTER_IR_SENSITIVITY_1	= 0x04b00000;
   pInfo->mData[0] = kOR_WriteMemory;
   pInfo->mData[1] = 0x04;
   pInfo->mData[2] = 0xb0;
   pInfo->mData[3] = 0x00;
   pInfo->mData[4] = 0x00;

   pInfo->mData[5] = 0x09;

   pInfo->mData[6] = skSensitivitySettings[0];
   pInfo->mData[7] = skSensitivitySettings[1];
   pInfo->mData[8] = skSensitivitySettings[2];
   pInfo->mData[9] = skSensitivitySettings[3];
   pInfo->mData[10] = skSensitivitySettings[4];
   pInfo->mData[11] = skSensitivitySettings[5];
   pInfo->mData[12] = skSensitivitySettings[6];
   pInfo->mData[13] = skSensitivitySettings[7];
   pInfo->mData[14] = skSensitivitySettings[8];
   WriteWiimote(pInfo);

   //REGISTER_IR_SENSITIVITY_2	= 0x04b0001a;
   pInfo = GetNextWriteBuffer();
   pInfo->mData[0] = kOR_WriteMemory;
   pInfo->mData[1] = 0x04;
   pInfo->mData[2] = 0xb0;
   pInfo->mData[3] = 0x00;
   pInfo->mData[4] = 0x1a;

   pInfo->mData[5] = 0x02;

   pInfo->mData[6] = skSensitivitySettings[9];
   pInfo->mData[7] = skSensitivitySettings[10];
   WriteWiimote(pInfo);

   pInfo = GetNextWriteBuffer();
   //REGISTER_IR_MODE			= 0x04b00033;
   pInfo->mData[0] = kOR_WriteMemory;
   pInfo->mData[1] = 0x04;
   pInfo->mData[2] = 0xb0;
   pInfo->mData[3] = 0x00;
   pInfo->mData[4] = 0x33;

   pInfo->mData[5] = 0x01;

   pInfo->mData[6] = mode;
   WriteWiimote(pInfo);

   pInfo = GetNextWriteBuffer();
   //REGISTER_IR				= 0x04b00030;
   pInfo->mData[0] = kOR_WriteMemory;
   pInfo->mData[1] = 0x04;
   pInfo->mData[2] = 0xb0;
   pInfo->mData[3] = 0x00;
   pInfo->mData[4] = 0x30;

   pInfo->mData[5] = 0x01;

   pInfo->mData[6] = 0x08;
   WriteWiimote(pInfo);
}

//----------------------------------------------------------------------------

// Disable the IR sensor
void CInputGeneratorWiimote::DisableIR()
{
   CWiimoteWriteInfo * pInfo = NULL;
   pInfo = GetNextWriteBuffer();
   pInfo->mData[0] = kOR_IR;
   pInfo->mData[1] = (0x00 | mRumble);
   WriteWiimote(pInfo);

   pInfo = GetNextWriteBuffer();
   pInfo->mData[0] = kOR_IR2;
   pInfo->mData[1] = (0x00 | mRumble);
   WriteWiimote(pInfo);
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::ParseReadInfo(CWiimoteReadInfo const * const pInfo, CWiimoteRawStatus_NC &wr_out) const
{
   switch (pInfo->mData[0])
   {
   case kIT_ButtonsAccelIRExtension:
      {
         wr_out = CWiimoteRawStatus_NC(pInfo->mData);
      }
      break;
   default:
      // Unhandled types
      break;
   }
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::ConvertRawStatus(CWiimoteRawStatus_NC const &wr, CWiimoteState &ws) const
{
   ws.mButtonState = wr.mButtons;
   ws.mAcceleration_Calibrated.mX = ((real32)wr.mAccX - mAccCalX0) / ((real32) mAccCalX1G - mAccCalX0);
   ws.mAcceleration_Calibrated.mY = ((real32)wr.mAccY - mAccCalY0) / ((real32) mAccCalY1G - mAccCalY0);
   ws.mAcceleration_Calibrated.mZ = ((real32)wr.mAccZ - mAccCalZ0) / ((real32) mAccCalZ1G - mAccCalZ0);

   // IR data is filled in by calling CalculateAverageIRPosition 
   ws.mButtonState_Nunchuck = wr.mNC_Buttons;
   ws.mAxisX_Nunchuck = wr.mNC_StickX;
   ws.mAxisY_Nunchuck = wr.mNC_StickY;

   // Current calibration values are hard coded - should be read from extension controller
   real32 const kAccZero = (real32) 0x82;
   real32 const kAcc1G = (real32) 0x9C;
   ws.mAcceleration_Calibrated_Nunchuck.mX = (wr.mNC_AccX - kAccZero) / (kAcc1G - kAccZero);
   ws.mAcceleration_Calibrated_Nunchuck.mY = (wr.mNC_AccY - kAccZero) / (kAcc1G - kAccZero);
   ws.mAcceleration_Calibrated_Nunchuck.mZ = (wr.mNC_AccZ - kAccZero) / (kAcc1G - kAccZero);
}

//----------------------------------------------------------------------------

// Calculate average sensor position (low quality! - production applications will need to use something a bit more advanced than this).      
void CInputGeneratorWiimote::CalculateAverageIRPosition(CWiimoteState &input) const
{
   KPADStatus kpadStatus[1];
   int ret = KPADRead(mControllerNumber, kpadStatus, sizeof(kpadStatus) / sizeof(KPADStatus));
   input.mIRSensorAverage.mX = kpadStatus[0].pos.x;
   input.mIRSensorAverage.mY = kpadStatus[0].pos.y;
}

//----------------------------------------------------------------------------

// Disconnect from the controller and stop reading data from it
void CInputGeneratorWiimote::Disconnect()
{
   if (mFileHandleRead != BPE_INVALID_HANDLE_VALUE)
   {
      CancelIo(mFileHandleRead);
      CloseHandle(mFileHandleRead);
      mFileHandleRead = BPE_INVALID_HANDLE_VALUE;
      CancelIo(mFileHandleWrite);
      CloseHandle(mFileHandleWrite);
      mFileHandleWrite = BPE_INVALID_HANDLE_VALUE;
   }
}

//----------------------------------------------------------------------------

CWiimoteReadInfo * CInputGeneratorWiimote::GetNextReadBuffer()
{
   CSyncCriticalSectionLocker cs(&mReadCS);
   CWiimoteReadInfo *pReadInfo = &mReadBuffer[mReadBufferAsyncIndex % kReadBufferCount];
   mReadBufferAsyncIndex++;

   return pReadInfo;
};

//----------------------------------------------------------------------------

CWiimoteWriteInfo * CInputGeneratorWiimote::GetNextWriteBuffer()
{
   while (mWriteBufferPendingIOCount == kWriteBufferCount)
   {
      // Wait for previous write to complete.
      // This allows us to have a max of 1 outstanding async write at a time which is better than stalling every write.
      SleepEx(1, TRUE);
   }

   CSyncCriticalSectionLocker cs(&mReadCS);
   CWiimoteWriteInfo *pWriteInfo = &mWriteBuffer[mWriteBufferAsyncIndex % kWriteBufferCount];
   pWriteInfo->Reset();

   mWriteBufferAsyncIndex++;
   mWriteBufferPendingIOCount++;
   
   return pWriteInfo;
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::InternalProcessRead(CWiimoteReadInfo * pInfo)
{
   switch (pInfo->mData[0])
   {
   case kIT_ReadData:  // Memory
      {
         BPE_WIIMOTE_REPORT_MSG("'kIT_ReadData' read", this);
         if ((pInfo->mData[3] &0x08) != 0)
         {
            BPE_WIIMOTE_REPORT_ERROR_MSG("read address not valid", this);
         }
         else if ((pInfo->mData[3] & 0x07) != 0)
         {
            BPE_WIIMOTE_REPORT_ERROR_MSG("cannot read write only registers", this);
         }
         else if (mpMemoryBuffer)
         {
            // Copy into buffer
            uint32 const copySize = bpe::min_val(mMemoryReadSize - mMemoryBufferPos, 16UL);
            memcpy(mpMemoryBuffer, &pInfo->mData[6], copySize);
            mMemoryBufferPos += copySize;
         }
      }
      break;

   default:
      // Do nothing, data will be parsed later
      //BPE_WIIMOTE_REPORT_MSG("'default' read", this);
      break;
   }
}

//----------------------------------------------------------------------------

VOID CALLBACK CInputGeneratorWiimote::ReadIOCompletionRoutine(DWORD dwErrorCode,
                                             DWORD dwNumberOfBytesTransfered,
                                             LPOVERLAPPED lpOverlapped)
{
   CWiimoteReadInfo * pReadInfo = reinterpret_cast<CWiimoteReadInfo*>(lpOverlapped);
   CInputGeneratorWiimote * pInput = pReadInfo->mpWiimote;
   if (dwErrorCode == ERROR_OPERATION_ABORTED)
   {
      // Don't trigger another read, we've cancelled IO.
      BPE_WIIMOTE_REPORT_ERROR_MSG("ERROR_OPERATION_ABORTED", pInput);
      return;
   }

   if (!GetOverlappedResult(pInput->mFileHandleRead, lpOverlapped, &dwNumberOfBytesTransfered, false))
   {
      BPE_WIIMOTE_REPORT_ERROR_MSG("GetOverlappedResult", pInput);
      return;
   }

   if (dwNumberOfBytesTransfered != CWiimoteReadInfo::kReportLength)
   {
      bpe_debugger_printf("Wiimote Error: ReadIOCompletionRoutine bytes %d\n", dwNumberOfBytesTransfered);
      return;
   }

   pInput->InternalProcessRead(pReadInfo);

   // Trigger another read
   CSyncCriticalSectionLocker cs(&pInput->mReadCS);
   pInput->AsyncRead();
}

//----------------------------------------------------------------------------

void CInputGeneratorWiimote::AsyncRead()
{
   {
      CSyncCriticalSectionLocker cs(&mReadCS);

      CWiimoteReadInfo * pReadBuf = GetNextReadBuffer();

      SetLastError(ERROR_SUCCESS); // Reset error code so we don't catch errors from other code in this thread.
      BOOL const result = ReadFileEx(mFileHandleRead, pReadBuf->mData, CWiimoteReadInfo::kReportLength, reinterpret_cast<OVERLAPPED *>(pReadBuf), &CInputGeneratorWiimote::ReadIOCompletionRoutine);
      DWORD error = GetLastError();
      if (!result)
      {
         BPE_WIIMOTE_REPORT_ERROR_MSG("Hard Error - AsyncRead", this);
         return;
      }
      if (error != ERROR_SUCCESS)
      {
         BPE_WIIMOTE_REPORT_ERROR_MSG("Soft Error - AsyncRead", this);
      }
   }
}

//----------------------------------------------------------------------------

bool CInputGeneratorWiimote::WriteWiimote(CWiimoteWriteInfo * pWriteBuf)
{
   // We use different write modes depending on the driver compatibility.
   bool bResult = false;
   switch (mWriteMode)
   {
   case kWRM_Async:
      bResult = AsyncWrite(pWriteBuf);
      break;
   case kWRM_Sync:
      bResult = SyncWrite(pWriteBuf);
      break;
   }
   return bResult;
}

//----------------------------------------------------------------------------

VOID CALLBACK CInputGeneratorWiimote::WriteIOCompletionRoutine(DWORD dwErrorCode,
                                             DWORD dwNumberOfBytesTransfered,
                                             LPOVERLAPPED lpOverlapped)
{
   CWiimoteReadInfo * pReadInfo = reinterpret_cast<CWiimoteReadInfo*>(lpOverlapped);
   CInputGeneratorWiimote * pInput = pReadInfo->mpWiimote;
   //BPE_WIIMOTE_REPORT_MSG("WriteIOCompletionRoutine", pInput);
   CSyncCriticalSectionLocker cs(&pInput->mWriteCS);

   // Decrement pending write IO count
   pInput->mWriteBufferPendingIOCount--;

   if (dwNumberOfBytesTransfered != CWiimoteReadInfo::kReportLength)
   {
      BPE_WIIMOTE_REPORT_ERROR_MSG("WriteIOCompletionRoutine", pInput);
      return;
   }
}

//----------------------------------------------------------------------------

bool CInputGeneratorWiimote::AsyncWrite(CWiimoteWriteInfo * pWriteBuf)
{        
   CSyncCriticalSectionLocker cs(&mWriteCS);

   BOOL const result = WriteFileEx(mFileHandleWrite, pWriteBuf->mData, CWiimoteWriteInfo::kReportLength, reinterpret_cast<OVERLAPPED *>(pWriteBuf), &CInputGeneratorWiimote::WriteIOCompletionRoutine);
   if (!result)
   {
      BPE_WIIMOTE_REPORT_ERROR_MSG("AsyncWrite", this);
      return false;
   }

   return true;
}

//----------------------------------------------------------------------------
// We use sync writes as a fallback if async writes using WriteFileEx aren't working because of driver problems.
bool CInputGeneratorWiimote::SyncWrite(CWiimoteWriteInfo * pWriteBuf)
{        
   CSyncCriticalSectionLocker cs(&mWriteCS);

   // Decrement pending write IO count as we're synchronous
   mWriteBufferPendingIOCount--;
   BOOL const result = HidD_SetOutputReport(mFileHandleWrite, pWriteBuf->mData, CWiimoteWriteInfo::kReportLength);
   if (!result)
   {
      BPE_WIIMOTE_REPORT_ERROR_MSG("WriteWiimote", this);
      return false;
   }


   //BPE_WIIMOTE_REPORT_MSG("WriteWiimote completed", this);
   return true;
}

//----------------------------------------------------------------------------

