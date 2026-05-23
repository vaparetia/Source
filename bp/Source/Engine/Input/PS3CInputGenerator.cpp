//----------------------------------------------------------------------------
// PS3CInputGenerator.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <math.h>
#include "Engine/Math/MathUtils.h"
#include BPE_PLATFORM_SPECIFIC( CInputGenerator.h )

//----------------------------------------------------------------------------

#include "Engine/System/COsContext.h"

//----------------------------------------------------------------------------
SPS3ControllerSettings gPS3ControllerSettingsLow =
{
   SPS3ControllerSettings::kFM_IIR_20,       // mFilterMode
   20,                                       // mNumFramesBeforeRetrigger
   0.1f,                                     // mTriggerThreshold
   0.08f,                                    // mUntriggerThreshold
   0.03f,                                    // mCalibrationAdd
   0.02f,                                    // mMaxCalibrationDelta
   0.06f,                                    // mMaxCalibrationBias
   10,                                       // mRawMagForSpikeWithStickSpike
   50,                                       // mRawMagForSpikeNoStickSpike
   2,                                        // mNumFramesBeforeIgnoreSpike
   0.2f,                                     // mAccelerationForStickSpike
   2,                                        // mNumFramesBeforeIgnoreStickSpike
};

SPS3ControllerSettings gPS3ControllerSettingsHigh = 
{
   SPS3ControllerSettings::kFM_IIR_10,       // mFilterMode
   10,                                       // mNumFramesBeforeRetrigger
   0.092f,                                    // mTriggerThreshold
   0.08f,                                    // mUntriggerThreshold
   0.03f,                                    // mCalibrationAdd
   0.02f,                                    // mMaxCalibrationDelta
   0.06f,                                    // mMaxCalibrationBias
   10,                                       // mRawMagForSpikeWithStickSpike
   100,                                      // mRawMagForSpikeNoStick
   2,                                        // mNumFramesBeforeIgnoreSpike
   0.2f,                                     // mAccelerationForStickSpike
   2,                                        // mNumFramesBeforeIgnoreStickSpike
};

//----------------------------------------------------------------------------

CInputMovingAverage::CInputMovingAverage(int const averageBufferSize, real32 const initialValue)
{
   mBufferPos = 0;
   mAverageBuffer.resize(averageBufferSize, initialValue);
}

//----------------------------------------------------------------------------

CInputMovingAverage::~CInputMovingAverage()
{
}

//----------------------------------------------------------------------------

real32 CInputMovingAverage::GetValue(int32 const numValuesToAverage, int32 const offset) const
{
   BPE_ASSERT(numValuesToAverage <= mAverageBuffer.size(), "Too many values to average.");
   BPE_ASSERT(numValuesToAverage > 0, "Can't average zero or negative values.");

   int startingPos = mBufferPos - numValuesToAverage - offset - 1;
   real32 averageVal = 0;

   // We can do a faster implementation of this, but I just want it simple and correct for now
   for (int loop = 0; loop < numValuesToAverage; loop++)
   {
      int index = startingPos + loop;
      if (index < 0) index += mAverageBuffer.size();
      if (index >= mAverageBuffer.size()) index -= mAverageBuffer.size();
      averageVal += mAverageBuffer[index];
   }
   averageVal /= (real32) numValuesToAverage;
   return averageVal;
}

//----------------------------------------------------------------------------

real32 CInputMovingAverage::GetGradient(int32 const numValuesToAverage, int32 const offset) const
{
   BPE_ASSERT(numValuesToAverage <= mAverageBuffer.size(), "Too many values to average.");
   BPE_ASSERT(numValuesToAverage > 0, "Can't average zero or negative values.");

   int startingPos = mBufferPos - numValuesToAverage - offset - 1;
   real32 averageGradient = 0;
   int numSamples = 0;

   // We can do a faster implementation of this, but I just want it simple and correct for now
   for (int loop = 0; loop < numValuesToAverage; loop++)
   {
      int index = startingPos + loop;
      if (index < 0) index += mAverageBuffer.size();
      if (index >= mAverageBuffer.size()) index -= mAverageBuffer.size();
      real32 const gradient = mAverageBuffer[index] - mAverageBuffer[(index - 1 + mAverageBuffer.size()) % mAverageBuffer.size()];
      averageGradient += gradient;
      numSamples++;
   }
   if (numSamples == 0)
   {
      // No samples, just return zero
      return 0.0f;
   }
   averageGradient /= (real32) numSamples;
   return averageGradient;
}

//----------------------------------------------------------------------------

bool CInputMovingAverage::DetectSpike(int32 const numValuesToCheck, real32 const spikeThreshold) const
{
   BPE_ASSERT(numValuesToCheck <= mAverageBuffer.size(), "Too many values to check.");
   BPE_ASSERT(numValuesToCheck > 0, "Can't check zero or negative values.");

   int startingPos = mBufferPos - numValuesToCheck - 1;
   real32 prevGradient = 0;

   // We can do a faster implementation of this, but I just want it simple and correct for now
   for (int loop = 0; loop < numValuesToCheck; loop++)
   {
      int index = startingPos + loop;
      if (index < 0) index += mAverageBuffer.size();
      if (index >= mAverageBuffer.size()) index -= mAverageBuffer.size();
      real32 const gradient = mAverageBuffer[index] - mAverageBuffer[(index - 1 + mAverageBuffer.size()) % mAverageBuffer.size()];
      if (loop > 0)
      {
         if ((gradient > 0) ^ (prevGradient > 0))
         {
            // -ve/+ve gradients
            float const delta = fabsf(prevGradient - gradient);
            if (delta > spikeThreshold)
            {
               // Spike!
               return true;
            }
         }
      }
      prevGradient = gradient;
   }
   // No spike
   return false;
}

//----------------------------------------------------------------------------

void CInputMovingAverage::AddValue(real32 const value)
{
   mAverageBuffer[mBufferPos] = value;
   mBufferPos++;
   if (mBufferPos == mAverageBuffer.size()) mBufferPos = 0;
}

//----------------------------------------------------------------------------

#define BPE_CELL_PAD_BTN_OFFSET_ADDITIONAL   (CELL_PAD_MAX_CODES - 1)
#define BPE_CELL_PAD_CTRL_TIP_LEFT		      (1 << 0)
#define BPE_CELL_PAD_CTRL_TIP_RIGHT		      (1 << 1)

//----------------------------------------------------------------------------

SPS3ControllerSettings *   CInputGenerator::spControllerSettings =   &gPS3ControllerSettingsHigh;
bool                       CInputGenerator::sDisplayDebugOutput =    false;

//----------------------------------------------------------------------------

CInputGenerator::CInputGenerator()
: CBaseInputGenerator()
{
   if (cellPadInit(kMaxControllers) != 0) 
   {
      bpe_debugger_printf("Controller initialization error.\n");
   }

   if( cellKbInit(1) != CELL_OK )
   {
      bpe_debugger_printf("Keyboard initialization error.\n");
   }
   else
   {
      cellKbSetReadMode(0, CELL_KB_RMODE_INPUTCHAR);
      cellKbSetCodeType(0, CELL_KB_CODETYPE_ASCII);
   }

   mControllerType = kCT_PS3;
   memset( &mCurrentGamepadState, 0, sizeof(mCurrentGamepadState) );
   memset( &mPreviousGamepadState, 0, sizeof(mPreviousGamepadState) );   

   for (int loop = 0; loop < kMaxControllers; loop++)
   {
      cellPadFilterIIRInit(&mFilterIIRSosX_10[loop], CELL_PADFILTER_IIR_CUTOFF_2ND_LPF_BT_010);
      cellPadFilterIIRInit(&mFilterIIRSosX_20[loop], CELL_PADFILTER_IIR_CUTOFF_2ND_LPF_BT_020);
      cellPadFilterIIRInit(&mFilterIIRSosX_50[loop], CELL_PADFILTER_IIR_CUTOFF_2ND_LPF_BT_050);
      mCalibrationX[loop] = 0.0f;
      mLastTriggerCount[loop] = 0;
      mSensorX[loop] = 0.0f;
      mRawSensorX[loop] = 512;
      mRawSensorX_SpikeInternal[loop] = 512;
      mFilteredSensorX[loop] = 512;
      mSpikeFrameCount[loop] = 0;
      mPrevLeftStickVelocity[loop] = 0;
      mPrevRightStickVelocity[loop] = 0;
      mStickSpikeFrameCount[loop] = 0;
      mHasRumble[loop] = false;
      mHighPrecisionAnalogStick[loop] = false;
   }
}

//----------------------------------------------------------------------------

CInputGenerator::~CInputGenerator()
{   
   cellPadEnd();
   cellKbEnd();
}

//----------------------------------------------------------------------------

CUserInput const CInputGenerator::GetInput(int const controllerNumber /* = 0 */)
{
   CUserInput input;

   // We'll get padinfo on controller == 0, though we should break this out into a generic update function
   if (controllerNumber == 0)
   {      
      if (cellPadGetInfo2(&mPadInfo) != 0)
	   {
		   bpe_debugger_printf("Error : cellPadGetPadInfo\n");
	   }

      CellPadInfo2 const &padInfo = mPadInfo;
      // Detect new pad connection
      for (int loop = 0; loop < kMaxControllers; loop++) 
      {
         if ((padInfo.port_status[loop] & CELL_PAD_STATUS_CONNECTED) != 0) 
         {
            // Work around for rumble caps not being detected correctly on controller reassignment in SDK200, always set sensor mode etc every frame.
            // https://ps3.scedev.net/forums/thread/32232 - Reassining Sixxaxis non vibration controller with a vibration controller and the vibration controller wont vibrate

            bool const bNewConnection = ((padInfo.port_status[loop] & CELL_PAD_STATUS_ASSIGN_CHANGES) != 0);
			   
            if (bNewConnection) bpe_debugger_printf("New gamepad %d is connected: device_type=%d\n", loop, padInfo.device_type[loop]);

            uint32_t portSettings = 0;
            if ((padInfo.device_capability[loop] & CELL_PAD_CAPABILITY_PRESS_MODE) != 0)
            {
               portSettings |= CELL_PAD_SETTING_PRESS_ON;               
            }
            if ((padInfo.device_capability[loop] & CELL_PAD_CAPABILITY_SENSOR_MODE) != 0)
            {
               portSettings |= CELL_PAD_SETTING_SENSOR_ON;               
            }
            
            // Enable button (and trigger) analog and accelerometers
            int ret = cellPadSetPortSetting(loop, portSettings);
            
            if ((padInfo.device_capability[loop] & CELL_PAD_CAPABILITY_ACTUATOR) != 0)            
            {
               if (bNewConnection) bpe_debugger_printf("Pad has rumble.\n");
               mHasRumble[loop] = true;
            }
            else
            {
               mHasRumble[loop] = false;
            }            
            
            if ((padInfo.device_capability[loop] & CELL_PAD_CAPABILITY_HP_ANALOG_STICK) != 0)            
            {
               mHighPrecisionAnalogStick[loop] = true;
            }
            else
            {
               mHighPrecisionAnalogStick[loop] = false;
            }            
         }
      }
      
      if (!gpOsContext->IsReleaseMode()) UpdateKeyboard();
   }

   CellPadData & state = mCurrentGamepadState[controllerNumber];
   CellPadData & prevState = mPreviousGamepadState[controllerNumber];

   bool const bGotControllerData = UpdateController(controllerNumber, state);
   
   // gamepad
   if (bGotControllerData)
   {
      UpdateMotionController(controllerNumber);
      for( TPadButtonMap::const_iterator it = mPadButtonMappings.begin(); it != mPadButtonMappings.end(); ++it )
      {
         CPadButtonMapping const & mapping = it->second;

         EButtonState const buttonState = GetButtonPress(controllerNumber, mapping.mButton);
         // We use bitflags to allow the Pressing state to be a combination of Pressed and Held.         
         if ((mapping.mButtonState & buttonState) != 0 )
         {
            input.SetValue( it->first, mapping.mValue );
         }
      }

      for( TPadAxisMap::const_iterator it = mPadAxisMappings.begin(); it != mPadAxisMappings.end(); ++it )
      {
         CPadAxisMapping const & mapping = it->second;

         input.SetValue( it->first, mapping.MapValue( GetAxisValue(state, controllerNumber, mapping.GetAxis(), mapping.IsRawAxis()) ) );
      }

      memcpy(&prevState, &state, sizeof(CellPadData));
   }

   return input;                                        
}

//----------------------------------------------------------------------------

void CInputGenerator::UpdateMotionController(int const controllerNumber)
{
   CellPadData & state = mCurrentGamepadState[controllerNumber];
   CellPadData & prevState = mPreviousGamepadState[controllerNumber];
   
   // Get analog stick velocity and acceleration
   real32 maxStickAcceleration = 0.0f;
   bool bWasStickSpike = false;   
   {
      real32 const leftStickVelocity = GetStickDeflectionVelocity(prevState, state, kS_Left);
      real32 const rightStickVelocity = GetStickDeflectionVelocity(prevState, state, kS_Right);

      real32 const leftStickAcceleration = mPrevLeftStickVelocity[controllerNumber] - leftStickVelocity;
      real32 const rightStickAcceleration = mPrevRightStickVelocity[controllerNumber] - rightStickVelocity;
      maxStickAcceleration = bpe::max_val(fabsf(leftStickAcceleration), fabsf(rightStickAcceleration));
      mPrevLeftStickVelocity[controllerNumber] = leftStickVelocity;
      mPrevRightStickVelocity[controllerNumber] = rightStickVelocity;
      if (maxStickAcceleration > spControllerSettings->mAccelerationForStickSpike)
      {
         mStickSpikeFrameCount[controllerNumber] = spControllerSettings->mNumFramesBeforeIgnoreStickSpike;
      }
      if (mStickSpikeFrameCount[controllerNumber] > 0)
      {
         mStickSpikeFrameCount[controllerNumber]--;
         bWasStickSpike = true;
      }
   }

   // Get motion controller input
   uint32_t const rawInputUnmodified = state.button[CELL_PAD_BTN_OFFSET_SENSOR_X];      
   uint32_t rawInput = rawInputUnmodified;

   // Check for spike (we get +/-300 spikes on some controllers)
   int32 rawInputDelta = bpe::abs(((int32) rawInput) - mRawSensorX_SpikeInternal[controllerNumber]);

   // Store unmodified input so we can check for spikes next frame
   mRawSensorX_SpikeInternal[controllerNumber] = rawInput;

   // Check for spike threshold
   bool bMightBeSpike = (rawInputDelta > spControllerSettings->mRawMagForSpikeNoStickSpike);      
   // Lower threshold when stick spike
   bMightBeSpike |= bWasStickSpike && (rawInputDelta > spControllerSettings->mRawMagForSpikeWithStickSpike);      

   if (bMightBeSpike)
   {
      // Reset frame counter
      mSpikeFrameCount[controllerNumber] = spControllerSettings->mNumFramesBeforeIgnoreSpike;
   }

   // We ignore spikes for fixed number of frames, no real theory, just seems to work.
   bool bWasSpike = false;
   if (mSpikeFrameCount[controllerNumber] > 0)
   {
      // This is a spike
      // Count number of frames for spike
      mSpikeFrameCount[controllerNumber]--;         
      // Use previous frame raw data (this will ignore the spike)
      rawInput = mRawSensorX[controllerNumber];
      bWasSpike = true;
   }      

   uint32_t filteredInput = 0;

   if (!bWasSpike)
   {
      // We only run the filter if it wasn't a spike, otherwise we're feeding the filter bogus data
      switch (spControllerSettings->mFilterMode)
      {
      case SPS3ControllerSettings::kFM_IIR_10:
         filteredInput = cellPadFilterIIRFilter(&mFilterIIRSosX_10[controllerNumber], rawInput);
         cellPadFilterIIRFilter(&mFilterIIRSosX_20[controllerNumber], rawInput);
         cellPadFilterIIRFilter(&mFilterIIRSosX_50[controllerNumber], rawInput);
         break;
      case SPS3ControllerSettings::kFM_IIR_20:
         filteredInput = cellPadFilterIIRFilter(&mFilterIIRSosX_20[controllerNumber], rawInput);
         cellPadFilterIIRFilter(&mFilterIIRSosX_10[controllerNumber], rawInput);
         cellPadFilterIIRFilter(&mFilterIIRSosX_50[controllerNumber], rawInput);
         break;
      case SPS3ControllerSettings::kFM_IIR_50:
         filteredInput = cellPadFilterIIRFilter(&mFilterIIRSosX_50[controllerNumber], rawInput);
         cellPadFilterIIRFilter(&mFilterIIRSosX_10[controllerNumber], rawInput);
         cellPadFilterIIRFilter(&mFilterIIRSosX_20[controllerNumber], rawInput);
         break;
      }
   }
   else
   {
      // SPIKE!
      // Use the data from the last frame
      filteredInput = mFilteredSensorX[controllerNumber];
   }

   real32 const uncalibratedSensorX = ((filteredInput / 1023.0f) * 2.0f) - 1.0f;

   // Set value
   mSensorX[controllerNumber] = uncalibratedSensorX - mCalibrationX[controllerNumber];
   mRawSensorX[controllerNumber] = rawInput;
   mFilteredSensorX[controllerNumber] = filteredInput;

   // Reset counters
   bool bIsTriggering = false;
   if (mLastTriggerCount[controllerNumber] > 0)
   {
      bIsTriggering = true;
      mLastTriggerCount[controllerNumber]--;      
   }

   // Offset calibration (but only if we're not spiking or not triggering)
   if (!bWasSpike && !bIsTriggering)
   {
      // Clamp max delta to prevent rapid changes
      real32 const calibrationDeltaX = uncalibratedSensorX - mCalibrationX[controllerNumber];
      mCalibrationX[controllerNumber] += MathUtils::ClampMinMax(calibrationDeltaX * spControllerSettings->mCalibrationAdd, -spControllerSettings->mMaxCalibrationDelta, spControllerSettings->mMaxCalibrationDelta);

      // Clamp max bias to prevent extreme calibration offsets
      mCalibrationX[controllerNumber] = MathUtils::ClampMinMax(mCalibrationX[controllerNumber], -spControllerSettings->mMaxCalibrationBias, spControllerSettings->mMaxCalibrationBias);
   }

   

   if (sDisplayDebugOutput && (controllerNumber == 0))
   {
      bpe_debugger_printf("%d/%d/%0.3f/%0.3f/%0.3f/%d/%d%s%s\n", 
         rawInputUnmodified, 
         filteredInput, 
         mSensorX[controllerNumber],
         mCalibrationX[controllerNumber], 
         maxStickAcceleration,
         mLastTriggerCount[controllerNumber], 
         mSpikeFrameCount[controllerNumber], 
         bMightBeSpike ? " - SPIKE!" : "",
         bWasStickSpike ? " - STICK SPIKE!" : "");

   }
}

//----------------------------------------------------------------------------

bool CInputGenerator::HasInput(int const controllerNumber) const
{
   if ((mPadInfo.system_info & CELL_PAD_INFO_INTERCEPTED) != 0)
   {
      // Controller intercepted by OS (usually the impose screen)
      return false;
   }

   if ((mPadInfo.device_capability[controllerNumber] & CELL_PAD_CAPABILITY_PS3_CONFORMITY) != 0)   
   {
      // Don't allow input from non-ps3 controllers
      return true;
   }

   if ((mPadInfo.port_status[controllerNumber] & CELL_PAD_STATUS_CONNECTED) != 0)   
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

void CInputGenerator::ApplyRumbleSettings()
{
   for (int loop = 0; loop < kMaxControllers; loop++)
   {
      if (!HasInput(loop)) continue;
      if (!mHasRumble[loop]) continue;

      SRumbleSettings const &rumble = mRumbleSettings[loop];

      CellPadActParam actprm;
      memset(&actprm, 0, sizeof(actprm));

      static real32 const skMinValueForHighFrequencyRumble = 0.2f;
      actprm.motor[0] = (rumble.mHighFreqIntensity < skMinValueForHighFrequencyRumble) ? 0 : 1;
      actprm.motor[1] = (uint8) (0xFF * rumble.mLowFreqIntensity);

      cellPadSetActDirect(loop, &actprm);
   }
}

//----------------------------------------------------------------------------

bool const CInputGenerator::UpdateController(int const controllerNumber, CellPadData & input)
{  
   if (!HasInput(controllerNumber)) 
      return false;

   switch( mControllerType )
   {
   case kCT_None:
      return false;
      break;

   case kCT_PS3:
      {
         // Zero additional input data as cellPadGetData doesn't reset the additional flags that we're using.   
         input.button[BPE_CELL_PAD_BTN_OFFSET_ADDITIONAL] = 0;   
		   if (cellPadGetData(controllerNumber, &input) != 0)
         {
            return false;
         }
         BPE_ASSERT(input.len <= BPE_CELL_PAD_BTN_OFFSET_ADDITIONAL, "cellPadGetData overwrote additional data");
         return true;
         
      }
      break;

   case kCT_Xbox360:
      break;
   }

   return false;
}


//----------------------------------------------------------------------------

real32 const GetAxis( int const value, int const perpAxisValue, int const range, int const deadzone )
{
   // We only use axis deadzone if stick displacement is inside deadzone.
   // This fixes inaccurate aiming for small displacements on X/Y axis.
   int const newDeadzone = (((perpAxisValue * perpAxisValue) + (value * value)) < (deadzone * deadzone)) ? deadzone : 0;
   int const absValue = abs(value);
   if( absValue < newDeadzone )
   {
      return 0.0f;
   }
   else
   {
      real32 const sign = (value < 0) ? -1.0f : 1.0f;
      real32 const returnValue = sign * ( absValue - newDeadzone ) / (range - newDeadzone);
      return returnValue; 
   }
}

//----------------------------------------------------------------------------


real32 const CInputGenerator::GetAxisValue( CellPadData const & input, int const controllerNumber, int const axis, bool const bUseRawValue )
{
   switch( mControllerType )
   {
   case kCT_None:
      return 0.0f;

   case kCT_PS3:
      {
         int axisValue = 0;
         int perpAxisValue = 0;
         real32 const skPS_ACC_OG_Offset = 512.0f;
         real32 const skPS_ACC_1G = 133.0f;
         
         bool const bIsHighPrecisionStick = mHighPrecisionAnalogStick[controllerNumber];
         int const kDefaultAnalogStickDeadZone = 0x25;  // 37
         int const kHighPrecisionAnalogStickDeadZone = 0x11;  // 17
         int const kTriggerDeadZone = 0; // PS3 triggers don't need a dead zone
         
         int axisDeadZone = bIsHighPrecisionStick ? kDefaultAnalogStickDeadZone : kHighPrecisionAnalogStickDeadZone;
         int axisRange = gkInt8Max;

         switch( axis )
         {
         case kA_PS3_A1_LeftRight:
            axisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] - gkInt8Max;
            perpAxisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] - gkInt8Max;
            break;
         case kA_PS3_A1_UpDown:
            axisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] - gkInt8Max;
            perpAxisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] - gkInt8Max;
            break;
         case kA_PS3_A2_LeftRight:
            axisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] - gkInt8Max;
            perpAxisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] - gkInt8Max;
            break;
         case kA_PS3_A2_UpDown:
            axisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] - gkInt8Max;
            perpAxisValue = input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] - gkInt8Max;
            break;
         case kA_PS3_LeftTrigger:
            axisValue = input.button[CELL_PAD_BTN_OFFSET_PRESS_L2];
            perpAxisValue = 0;
            axisDeadZone = kTriggerDeadZone;
            axisRange = 0xFF;
            break;
         case kA_PS3_RightTrigger:
            axisValue = input.button[CELL_PAD_BTN_OFFSET_PRESS_R2];
            perpAxisValue = 0;
            axisDeadZone = kTriggerDeadZone;            
            axisRange = 0xFF;
            break;
         case kA_PS3_AccX:
            {
               int const val = input.button[CELL_PAD_BTN_OFFSET_SENSOR_X];
               real32 const axis = (val - skPS_ACC_OG_Offset) / skPS_ACC_1G;
               return axis;
            }
            break;
         case kA_PS3_AccY:
            {
               int const val = input.button[CELL_PAD_BTN_OFFSET_SENSOR_Y];
               real32 const axis = (val - skPS_ACC_OG_Offset) / skPS_ACC_1G;
               return axis;
            }
            break;
         case kA_PS3_AccZ:
            {
               int const val = input.button[CELL_PAD_BTN_OFFSET_SENSOR_Z];
               real32 const axis = (val - skPS_ACC_OG_Offset) / skPS_ACC_1G;
               return axis;
            }
            break;
         case kA_PS3_AccGyro:
            {
               real32 const skPS_ACC_Gyro_90Deg_per_Sec = 123.0f;
               int const val = input.button[CELL_PAD_BTN_OFFSET_SENSOR_G];
               real32 const axis = (val - skPS_ACC_OG_Offset) / skPS_ACC_Gyro_90Deg_per_Sec;
               return axis;
            }
            break;
         }
         
         return GetAxis(axisValue, perpAxisValue, axisRange, bUseRawValue ? 0 : axisDeadZone);
      }
      break;

   case kCT_Xbox360:
      {
      }
      break;
   }

   return 0.0f;
}

//----------------------------------------------------------------------------

real32 CInputGenerator::GetStickDeflectionVelocity(CellPadData const & prev, CellPadData const & current, EStick const stick) const
{
   real32 prevX = 0;
   real32 prevY = 0;
   real32 x = 0;
   real32 y = 0;

   switch (stick)
   {
      case kS_Left:
         prevX = ((real32) (prev.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] - gkInt8Max)) / gkInt8Max;
         prevY = ((real32) (prev.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] - gkInt8Max)) / gkInt8Max;
         x = ((real32) (current.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] - gkInt8Max)) / gkInt8Max;
         y = ((real32) (current.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] - gkInt8Max)) / gkInt8Max;
         break;

      case kS_Right:
         prevX = ((real32) (prev.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] - gkInt8Max)) / gkInt8Max;
         prevY = ((real32) (prev.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] - gkInt8Max)) / gkInt8Max;
         x = ((real32) (current.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] - gkInt8Max)) / gkInt8Max;
         y = ((real32) (current.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] - gkInt8Max)) / gkInt8Max;
         break;
   }
   real32 const deltaX = prevX - x;
   real32 const deltaY = prevY - y;
   real32 const velocity = (deltaX * deltaX) + (deltaY * deltaY);

   return velocity;
}

//----------------------------------------------------------------------------

EButtonState const CInputGenerator::GetButtonPress( int const controllerNumber, int const button )
{

   CellPadData const & state = mCurrentGamepadState[controllerNumber];
   CellPadData const & prevState = mPreviousGamepadState[controllerNumber];

   switch( mControllerType )
   {
   case kCT_None:
      return kBS_NotPressed;

   case kCT_Xbox360:
      {
         return kBS_NotPressed;
      }

   case kCT_PS3:
      {
         bool pressedNow = IsButtonPressed( state, button );
         bool const wasPressed = IsButtonPressed( prevState, button );
         // If it was pressed last frame, then reduce the threshold for a trigger to give hysteresis
         real32 const triggerThreshold = wasPressed ? spControllerSettings->mUntriggerThreshold : spControllerSettings->mTriggerThreshold;

         // Special code case for tip

         // Use 'wasPressed' to prevent continual tipping when tilted and disabled
         if (!mDisableTipping[controllerNumber] || wasPressed)
         {
            if (button == kPB_PS3_TipLeft)
            {
               if (mSensorX[controllerNumber] > triggerThreshold)
               {
                  if ( (mLastTriggerCount[controllerNumber] == 0) || wasPressed)
                  {
                     // It's tipped
                     pressedNow = true;
                     mLastTriggerCount[controllerNumber] = spControllerSettings->mNumFramesBeforeRetrigger;
                     // Set state as pressed so pressed/released works
                     mCurrentGamepadState[controllerNumber].button[BPE_CELL_PAD_BTN_OFFSET_ADDITIONAL] |= BPE_CELL_PAD_CTRL_TIP_LEFT;
                  }
               }
            }         

            if (button == kPB_PS3_TipRight)
            {
               if (mSensorX[controllerNumber] < -triggerThreshold)
               {
                  if ( (mLastTriggerCount[controllerNumber] == 0) || wasPressed)
                  {
                     // It's tipped
                     pressedNow = true;
                     mLastTriggerCount[controllerNumber] = spControllerSettings->mNumFramesBeforeRetrigger;
                     // Set state as pressed so pressed/released works
                     mCurrentGamepadState[controllerNumber].button[BPE_CELL_PAD_BTN_OFFSET_ADDITIONAL] |= BPE_CELL_PAD_CTRL_TIP_RIGHT;
                  }
               }
            }         
         }
         
         if( pressedNow != wasPressed )
         {
            if( pressedNow )
               return kBS_Pressed;
            else
               return kBS_Released;
         }
         else
         {
            if( pressedNow )
               return kBS_Held;
            else
               return kBS_NotPressed;
         }
      }
      break;
   }

   return kBS_NotPressed;
}

//----------------------------------------------------------------------------

bool const CInputGenerator::IsButtonPressed( CellPadData const & input, int const button )
{
   int const kBinaryStickThreshold = (gkInt8Max * 2) / 3;

   switch( button )
   {
   case kPB_PS3_Select:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_SELECT) != 0;
   case kPB_PS3_Start:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_START) != 0;
   case kPB_PS3_Square:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_SQUARE) != 0;
   case kPB_PS3_Circle:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CIRCLE) != 0;
   case kPB_PS3_X:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_CROSS) != 0;
   case kPB_PS3_Triangle:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_TRIANGLE) != 0;
   case kPB_PS3_L1:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L1) != 0;
   case kPB_PS3_L2:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_L2) != 0;
   case kPB_PS3_R1:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R1) != 0;
   case kPB_PS3_R2:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL2] & CELL_PAD_CTRL_R2) != 0;
   case kPB_PS3_Up:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_UP) != 0;
   case kPB_PS3_Right:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_RIGHT) != 0;
   case kPB_PS3_Down:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_DOWN) != 0;
   case kPB_PS3_Left:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_LEFT) != 0;
   case kPB_PS3_LeftThumb:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_L3) != 0;
   case kPB_PS3_RightThumb:
      return (input.button[CELL_PAD_BTN_OFFSET_DIGITAL1] & CELL_PAD_CTRL_R3) != 0;
   case kPB_PS3_TipLeft:
      return (input.button[BPE_CELL_PAD_BTN_OFFSET_ADDITIONAL] & BPE_CELL_PAD_CTRL_TIP_LEFT) != 0;
   case kPB_PS3_TipRight:
      return (input.button[BPE_CELL_PAD_BTN_OFFSET_ADDITIONAL] & BPE_CELL_PAD_CTRL_TIP_RIGHT) != 0;

   case kPB_PS3_LStick_Up:
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] < (gkInt8Max - kBinaryStickThreshold));
   case kPB_PS3_LStick_Down:
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_Y] > (gkInt8Max + kBinaryStickThreshold));
   case kPB_PS3_LStick_Right:
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] < (gkInt8Max - kBinaryStickThreshold));
   case kPB_PS3_LStick_Left:
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_LEFT_X] > (gkInt8Max + kBinaryStickThreshold));

   case kPB_PS3_RStick_Up:
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] < (gkInt8Max - kBinaryStickThreshold));
   case kPB_PS3_RStick_Down:                                     
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_Y] > (gkInt8Max + kBinaryStickThreshold));
   case kPB_PS3_RStick_Right:                                    
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] < (gkInt8Max - kBinaryStickThreshold));
   case kPB_PS3_RStick_Left:                                     
      return (input.button[CELL_PAD_BTN_OFFSET_ANALOG_RIGHT_X] > (gkInt8Max + kBinaryStickThreshold));
   }

   return false;
}


//----------------------------------------------------------------------------
// Platform specific implementation, resolved at link time
//----------------------------------------------------------------------------

CInputEvent::EKey CBaseInputGenerator::ConvertVirtualKey(int const inKey)
{
   if( inKey & CELL_KB_KEYPAD )
   {
      switch( inKey & ~CELL_KB_KEYPAD)
      {
      case CELL_KEYC_KPAD_NUMLOCK:  return CInputEvent::kKey_NumLock;
      case CELL_KEYC_KPAD_SLASH:    return CInputEvent::kKey_Divide;
      case CELL_KEYC_KPAD_ASTERISK: return CInputEvent::kKey_Multiply;
      case CELL_KEYC_KPAD_MINUS:    return CInputEvent::kKey_Subtract;
      case CELL_KEYC_KPAD_PLUS:     return CInputEvent::kKey_Add;
      case CELL_KEYC_KPAD_ENTER:    return CInputEvent::kKey_Return;
      case CELL_KEYC_KPAD_1:        return CInputEvent::kKey_NumPad1;
      case CELL_KEYC_KPAD_2:        return CInputEvent::kKey_NumPad2;
      case CELL_KEYC_KPAD_3:        return CInputEvent::kKey_NumPad3;
      case CELL_KEYC_KPAD_4:        return CInputEvent::kKey_NumPad4;
      case CELL_KEYC_KPAD_5:        return CInputEvent::kKey_NumPad5;
      case CELL_KEYC_KPAD_6:        return CInputEvent::kKey_NumPad6;
      case CELL_KEYC_KPAD_7:        return CInputEvent::kKey_NumPad7;
      case CELL_KEYC_KPAD_8:        return CInputEvent::kKey_NumPad8;
      case CELL_KEYC_KPAD_9:        return CInputEvent::kKey_NumPad9;
      case CELL_KEYC_KPAD_0:        return CInputEvent::kKey_NumPad0;
      case CELL_KEYC_KPAD_PERIOD:   return CInputEvent::kKey_Decimal;
      }
   }
   else if( inKey & CELL_KB_RAWDAT )
   {
      switch( inKey & 0x7f )
      {
      case CELL_KEYC_BS:            return CInputEvent::kKey_Back; 
      case CELL_KEYC_TAB:           return CInputEvent::kKey_Tab; 
      case CELL_KEYC_ENTER:         return CInputEvent::kKey_Return; 
      case CELL_KEYC_PAUSE:         return CInputEvent::kKey_Pause; 
      case CELL_KEYC_ESCAPE:        return CInputEvent::kKey_Escape; 
      case CELL_KEYC_PAGE_UP:       return CInputEvent::kKey_Prior; 
      case CELL_KEYC_PAGE_DOWN:     return CInputEvent::kKey_Next; 
      case CELL_KEYC_END:           return CInputEvent::kKey_End; 
      case CELL_KEYC_HOME:          return CInputEvent::kKey_Home; 
      case CELL_KEYC_LEFT_ARROW:    return CInputEvent::kKey_Left; 
      case CELL_KEYC_UP_ARROW:      return CInputEvent::kKey_Up; 
      case CELL_KEYC_RIGHT_ARROW:   return CInputEvent::kKey_Right; 
      case CELL_KEYC_DOWN_ARROW:    return CInputEvent::kKey_Down; 
      case CELL_KEYC_PRINTSCREEN:   return CInputEvent::kKey_Print; 
      case CELL_KEYC_INSERT:        return CInputEvent::kKey_Insert; 
      case CELL_KEYC_DELETE:        return CInputEvent::kKey_Delete; 

      case CELL_KEYC_F1:            return CInputEvent::kKey_F1; 
      case CELL_KEYC_F2:            return CInputEvent::kKey_F2; 
      case CELL_KEYC_F3:            return CInputEvent::kKey_F3; 
      case CELL_KEYC_F4:            return CInputEvent::kKey_F4; 
      case CELL_KEYC_F5:            return CInputEvent::kKey_F5; 
      case CELL_KEYC_F6:            return CInputEvent::kKey_F6; 
      case CELL_KEYC_F7:            return CInputEvent::kKey_F7; 
      case CELL_KEYC_F8:            return CInputEvent::kKey_F8; 
      case CELL_KEYC_F9:            return CInputEvent::kKey_F9; 
      case CELL_KEYC_F10:           return CInputEvent::kKey_F10; 
      case CELL_KEYC_F11:           return CInputEvent::kKey_F11; 
      case CELL_KEYC_F12:           return CInputEvent::kKey_F12; 
      }
   }
   else
   {
      switch( inKey )
      {
      case 8:                       return CInputEvent::kKey_Back;
      case 9:                       return CInputEvent::kKey_Tab;
      case 10:                      return CInputEvent::kKey_Return;
      }
   }


   return CInputEvent::kKey_None;
}

//----------------------------------------------------------------------------

void CInputGenerator::UpdateKeyboard()
{
   CellKbInfo info;

   if( cellKbGetInfo(&info) != CELL_KB_OK )
      return;

   for( int i = 0; i < CELL_KB_MAX_KEYBOARDS; ++i )
   {
      if( info.status[i] == CELL_KB_STATUS_DISCONNECTED )
         continue;
   
      cellKbRead(i, &mKeyboardData);

      for( int j = 0; j < mKeyboardData.len; ++j )
      {
         if( mKeyboardData.keycode[j] != (CELL_KB_RAWDAT | CELL_KEYC_NO_EVENT) )
         {
            int key = ConvertVirtualKey(mKeyboardData.keycode[j]);
            
            if( key != CInputEvent::kKey_None )
            {
               AddEvent(CInputEvent(CInputEvent::kType_KeyDown, key));
            }
            else
            {
               key = cellKbCnvRawCode(CELL_KB_MAPPING_101, mKeyboardData.mkey, mKeyboardData.led, mKeyboardData.keycode[j]);
               AddEvent(CInputEvent(CInputEvent::kType_Char, mKeyboardData.keycode[j]));
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

CBaseInputGenerator * CBaseInputGenerator::Factory()
{
   return new CInputGenerator();
}

//----------------------------------------------------------------------------

