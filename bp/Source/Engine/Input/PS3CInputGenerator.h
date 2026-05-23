//----------------------------------------------------------------------------
// PS3CInputGenerator.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Input/CInputGenerator.h"
#include "boost/scoped_array.hpp"

#include <cell/pad/libpad.h>      // USB Gamepad Library
#include <cell/padfilter.h>
#include <cell/keyboard.h>


//----------------------------------------------------------------------------
class ENGINE_API CInputMovingAverage
{
public:
   CInputMovingAverage(int const averageBufferSize, real32 const initialValue);
   ~CInputMovingAverage();

   real32                  GetValue(int32 const numValuesToAverage, int32 const offset) const;
   real32                  GetGradient(int32 const numValuesToAverage, int32 const offset) const;
   bool                    DetectSpike(int32 const numValuesToCheck, real32 const spikeThreshold) const;
   void                    AddValue(real32 const value);

   std::vector<real32>     mAverageBuffer; 
   int32                   mBufferPos;
};

//----------------------------------------------------------------------------

struct SPS3ControllerSettings
{
   enum EFilterMode
   {
      kFM_IIR_10,
      kFM_IIR_20,
      kFM_IIR_50,

      kFM_Count
   };

   EFilterMode            mFilterMode;
   int                    mNumFramesBeforeRetrigger;
   real32                 mTriggerThreshold;
   real32                 mUntriggerThreshold;
   real32                 mCalibrationAdd;   
   real32                 mMaxCalibrationDelta;
   real32                 mMaxCalibrationBias;
   int32                  mRawMagForSpikeWithStickSpike;
   int32                  mRawMagForSpikeNoStickSpike;
   int32                  mNumFramesBeforeIgnoreSpike;
   real32                 mAccelerationForStickSpike;   
   int32                  mNumFramesBeforeIgnoreStickSpike;
};

extern SPS3ControllerSettings gPS3ControllerSettingsLow;
extern SPS3ControllerSettings gPS3ControllerSettingsHigh;

//----------------------------------------------------------------------------

class ENGINE_API CInputGenerator : public CBaseInputGenerator
{
public:
   CInputGenerator();
   ~CInputGenerator();

   virtual bool             HasInput(int const controllerNumber = 0) const;
   virtual CUserInput const GetInput(int const controllerNumber = 0);
   virtual void             ApplyRumbleSettings();

private:
   bool const UpdateController(int const controllerNumber, CellPadData & input);
   real32 const GetAxisValue(CellPadData const & input, int const controllerNumber, int const axis, bool const bUseRawValue );
   enum EStick
   {
      kS_Left,
      kS_Right,
      kS_Count
   };

   real32 GetStickDeflectionVelocity(CellPadData const & prev, CellPadData const & current, EStick const stick) const;
   void UpdateMotionController(int const controllerNumber);

   EButtonState const GetButtonPress(int const controllerNumber, int const button);
   bool const IsButtonPressed(CellPadData const & input, int const button);
   
   void UpdateKeyboard();

private:
   CellKbData                    mKeyboardData;

   CellPadData                   mCurrentGamepadState[kMaxControllers];    // Gamepad data buffer
   CellPadData                   mPreviousGamepadState[kMaxControllers];   // Gamepad data buffer
   CellPadInfo2                  mPadInfo;
   bool                          mHasRumble[kMaxControllers];
   bool                          mHighPrecisionAnalogStick[kMaxControllers];

   // Filtering
   CellPadFilterIIRSos           mFilterIIRSosX_10[kMaxControllers];
   CellPadFilterIIRSos           mFilterIIRSosX_20[kMaxControllers];
   CellPadFilterIIRSos           mFilterIIRSosX_50[kMaxControllers];
   real32                        mCalibrationX[kMaxControllers];
   int32                         mRawSensorX[kMaxControllers];
   int32                         mRawSensorX_SpikeInternal[kMaxControllers];
   int32                         mFilteredSensorX[kMaxControllers];
   real32                        mSensorX[kMaxControllers];
   int                           mLastTriggerCount[kMaxControllers];
   int                           mSpikeFrameCount[kMaxControllers];
   real32                        mPrevLeftStickVelocity[kMaxControllers];
   real32                        mPrevRightStickVelocity[kMaxControllers];
   int                           mStickSpikeFrameCount[kMaxControllers];

public:   

   static SPS3ControllerSettings *  spControllerSettings;
   static bool                      sDisplayDebugOutput;
};

//----------------------------------------------------------------------------

