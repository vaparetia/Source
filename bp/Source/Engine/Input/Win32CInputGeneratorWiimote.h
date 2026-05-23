//----------------------------------------------------------------------------
// Win32CInputGeneratorWiimote.h
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/System/CSyncCriticalSection.h"

//----------------------------------------------------------------------------

class CInputGeneratorWiimote;
class CWiimoteState;

//----------------------------------------------------------------------------

struct SWiimoteRawIRInfo
{
   enum { kIRNotVisible = 0xFFFF };
   int16    mX;
   int16    mY;
   uint16   mSize;
   uint8    mId;
};

class CWiimoteRawStatus_NC
{
public:
   CWiimoteRawStatus_NC(uint8 const * pReport);
   CWiimoteRawStatus_NC();

   enum { kMaxIRObjs = 4 };

   uint16               mButtons;
   int16                mAccX;
   int16                mAccY;
   int16                mAccZ;
   SWiimoteRawIRInfo    mIRObjs[kMaxIRObjs];

   int8                 mNC_StickX;
   int8                 mNC_StickY;   
   int16                mNC_AccX;
   int16                mNC_AccY;
   int16                mNC_AccZ;
   uint8                mNC_Buttons;
};

//----------------------------------------------------------------------------

class CWiimoteReadInfo
{
public:
   CWiimoteReadInfo()
      : mpWiimote(NULL)
   {
      Reset();
   };

   ~CWiimoteReadInfo()
   {
   };

   void Reset()
   {
      memset(mData, 0, kReportLength);
      memset(&mOverlappedIO, 0, sizeof(mOverlappedIO));
   };

   // Note: OVERLAPPED MUST be first
   OVERLAPPED     mOverlappedIO;

   static int const kReportLength    = 22;
   uint8                      mData[kReportLength];

   // Pointer to original wiimote data
   CInputGeneratorWiimote *   mpWiimote;
};

//----------------------------------------------------------------------------

class CWiimoteWriteInfo
{
public:
   CWiimoteWriteInfo()
      : mpWiimote(NULL)
   {
      Reset();
   };

   ~CWiimoteWriteInfo()
   {
   };

   void Reset()
   {
      memset(mData, 0, kReportLength);
      memset(&mOverlappedIO, 0, sizeof(mOverlappedIO));
   };

   // Note: OVERLAPPED MUST be first
   OVERLAPPED     mOverlappedIO;

   static int const kReportLength    = 22;
   uint8          mData[kReportLength];

   // Pointer to original wiimote data
   CInputGeneratorWiimote *   mpWiimote;
};

//----------------------------------------------------------------------------
// HID for Wiimote via Bluetooth
// See http://wiibrew.org/index.php?title=Wiimote for technical details.

class CInputGeneratorWiimote
{
public:
   // Wiimote output commands
   enum EOutputReport
   {
      kOR_None          = 0x00,
      kOR_LEDsRumble    = 0x11,
      kOR_Type          = 0x12,
      kOR_IR            = 0x13,
      kOR_Status        = 0x15,
      kOR_WriteMemory   = 0x16,
      kOR_ReadMemory    = 0x17,
      kOR_IR2           = 0x1a,

      kOR_Terminator
   };

   // Data returned from the wii controller
   enum EInputType
   {
      kIT_Status				= 0x20,  // Status report
      kIT_ReadData			= 0x21,     // Read data from memory location
      kIT_Buttons				= 0x30,  // Button data only
      kIT_ButtonsAccel		= 0x31,  // Button and accelerometer data
      kIT_ButtonsAccelIR				= 0x33,  // IR sensor and accelerometer data
      kIT_ButtonsExtension	= 0x34,  // Button and extension controller data
      kIT_ButtonsAccelExtension		= 0x35,  // Extension and accelerometer data
      kIT_ButtonsAccelIRExtension	= 0x37,  // IR sensor, extension controller and accelerometer data

      kIT_Terminator
   };

   enum EIRMode
   {
      kIRM_Off			= 0x00,  // IR sensor off
      kIRM_Basic		= 0x01,	// Basic mode, 10 bytes
      kIRM_Extended	= 0x03,	// Extended mode, 12 bytes
      kIRM_Full		= 0x05,	// Full mode (unsupported), 16 bytes * 2 (format unknown)

      kIRM_Terminator
   };

   // VID = Nintendo, PID = Wiimote
	static int const     kVID = 0x057e;
   static int const     kPID = 0x0306;

   // Wiimote controller iteration
   static int           sHIDIndex;
   static int           sCurrentControllerNum;

   
   enum EWiimoteWriteMode
   {
      kWRM_Async,    // WriteFileEx (default)
      kWRM_Sync,     // HidD_SetReport

      kWRM_Count
   };

   EWiimoteWriteMode    mWriteMode;
   int                  mControllerNumber;
   BPE_HANDLE           mFileHandleRead;
   BPE_HANDLE           mFileHandleWrite;

   static int const     kReadBufferCount = 64;

   CWiimoteReadInfo     mReadBuffer[kReadBufferCount];
   int                  mReadBufferAsyncIndex;
   int                  mReadBufferAccessIndex;
   CSyncCriticalSection mReadCS;

   static int const     kWriteBufferCount = 1;  // More than one buffer isn't safe, would need to use multiple mOverlappedWrite members
   CWiimoteWriteInfo    mWriteBuffer[kWriteBufferCount];
   int                  mWriteBufferAsyncIndex;
   volatile int         mWriteBufferPendingIOCount;
   CSyncCriticalSection mWriteCS;

   // Controller persistent state
   uint8                mLEDs;
   uint8                mRumble;

   // Data and IR mode, note that these need to match!
   EInputType           mInputType;
   EIRMode              mIRMode;

   // Read wiimote memory data buffer (data can be async read into this buffer)
   uint8 *              mpMemoryBuffer;
   volatile uint32      mMemoryBufferPos;
   uint32               mMemoryReadSize;  // Requested size of read

   // Wiimote acceleration calibration
   // Offset
   uint8                mAccCalX0;
   uint8                mAccCalY0;
   uint8                mAccCalZ0;

   // 1G magnitude
   uint8                mAccCalX1G;
   uint8                mAccCalY1G;
   uint8                mAccCalZ1G;

   CInputGeneratorWiimote();
   ~CInputGeneratorWiimote();

   void SetRumbleState(bool const on);
   void SetLEDState(uint8 const leds);
   void FlushLEDAndRumbleState();
   void InitInput();
   bool IsConnected() const;

   // Helper function for getting most recent data.
   // Note that this is for basic functionality only. Advanced analysis would read all new data (multiple) since the last update.
   // Also note that this pointer is only valid for a short period of time (> 1 frame, < 1 sec) as it will get overwritten when buffer wraps around.
   CWiimoteReadInfo const * GetCurrentReadInfo() const;

   // Call this function once per frame.
   void FrameUpdate();

   void ParseReadInfo(CWiimoteReadInfo const * const pInfo, CWiimoteRawStatus_NC &wr_out) const;
   void ConvertRawStatus(CWiimoteRawStatus_NC const &wr, CWiimoteState &ws_out) const;

   void CalculateAverageIRPosition(CWiimoteState &input) const;

   void EnableExtension();
   void EnableIR(EIRMode const mode);
   void DisableIR();

   // Used by extension controllers
   bool CalibrateAccelerometers();

   // Be very careful using this function as caller owns pointer.
   // Also calling this function before a previous async read has completed is undefined.
   bool ReadWiimoteMemory(uint32 const address, uint32 const size, uint8 * pDestBuffer);

   // Be very careful using this function as caller owns pointer.
   // Also calling this function before a previous async read has completed is undefined.
   bool ReadWiimoteMemoryAsync(uint32 const address, uint32 const size, uint8 * pDestBuffer);
   bool Connect();
   // Disconnect from the controller and stop reading data from it
   void Disconnect();
   CWiimoteReadInfo * GetNextReadBuffer();
   CWiimoteWriteInfo * GetNextWriteBuffer();

   bool  WriteWiimote(CWiimoteWriteInfo * pWriteBuf);
private:
   void InternalProcessRead(CWiimoteReadInfo * pInfo);
   static VOID CALLBACK ReadIOCompletionRoutine(DWORD dwErrorCode,
                                                DWORD dwNumberOfBytesTransfered,
                                                LPOVERLAPPED lpOverlapped);
   void AsyncRead();
   static VOID CALLBACK WriteIOCompletionRoutine(DWORD dwErrorCode,
                                                DWORD dwNumberOfBytesTransfered,
                                                LPOVERLAPPED lpOverlapped);
   bool AsyncWrite(CWiimoteWriteInfo * pWriteBuf);
   bool SyncWrite(CWiimoteWriteInfo * pWriteBuf);
};

//----------------------------------------------------------------------------
