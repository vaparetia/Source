//----------------------------------------------------------------------------
// Win32CInputGenerator.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Win32CInputGenerator.h"
#include "Win32CInputGeneratorWiimote.h"

//----------------------------------------------------------------------------

#include "Engine/System/COsContext.h"

//----------------------------------------------------------------------------

CInputGenerator::CInputGenerator()
:  CBaseInputGenerator()
,  mpInput(NULL)
,  mpKeyboard(NULL)
,  mEnumGamepadCount(0)
{
   for (int loop = 0; loop < kMaxControllers; loop++)
   {
      mXInput_ControllerIndex[loop] = -1;
      mpWiimote[loop] = new CInputGeneratorWiimote();
      if (mpWiimote[loop]->IsConnected())
      {
         mControllerType = kCT_Wii;
      }
   }

   memset( &mPreviousState, 0, 256 );
   memset( &mCurrentGamepadState, 0, sizeof(mCurrentGamepadState) );
   memset( &mPreviousGamepadState, 0, sizeof(mPreviousGamepadState) );

   LPDIRECTINPUT8 input = NULL;
   DirectInput8Create( (HINSTANCE)OsContext()->mInstanceHandle, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&input, NULL );
   mpInput = TComPtr<IDirectInput8>(input);

   // initialize keyboard
   {
      LPDIRECTINPUTDEVICE8 keyboard = NULL;
      mpInput->CreateDevice( GUID_SysKeyboard, &keyboard, NULL ); 
   
      mpKeyboard = TComPtr<IDirectInputDevice8>( keyboard );
      mpKeyboard->SetDataFormat( &c_dfDIKeyboard );
   
      DIPROPDWORD  dipdw; 
      dipdw.diph.dwSize = sizeof(DIPROPDWORD); 
      dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
      dipdw.diph.dwObj = 0; 
      dipdw.diph.dwHow = DIPH_DEVICE; 
      dipdw.dwData = 10; 
   
      mpKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ); 
   
      mpKeyboard->SetCooperativeLevel( (HWND)OsContext()->mMainWindow, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE );
      mpKeyboard->Acquire();
   }

   // initialize gamepad if wiimote not connected
   if (mControllerType == kCT_None)
   {
      for( int i = 0; i < kMaxControllers; ++i )
      {
         XINPUT_STATE state;
         DWORD result = XInputGetState(i, &state);
         if( result == ERROR_SUCCESS )
         {
            mControllerType = kCT_Xbox360;
            mXInput_ControllerIndex[i] = i;
            ZeroMemory(&mXInput_PreviousState[i], sizeof(XINPUT_STATE));
         }
      }
      
      if( mControllerType == kCT_None )
      {
         mpInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);
         
         for( int i = 0; i < kMaxControllers; ++i )
         {
            if( mpGamepad[i] )
            {
               mControllerType = kCT_PS3;
               mpGamepad[i]->SetDataFormat(&c_dfDIJoystick2);
               mpGamepad[i]->SetCooperativeLevel((HWND)OsContext()->mMainWindow, DISCL_EXCLUSIVE|DISCL_BACKGROUND);
            }
         }
      }
   }
}

//----------------------------------------------------------------------------

CInputGenerator::~CInputGenerator()
{
   for (int loop = 0; loop < kMaxControllers; loop++)
   {
      delete mpWiimote[loop];
   }
}

//----------------------------------------------------------------------------

BOOL CALLBACK CInputGenerator::EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
   CInputGenerator* pThis = (CInputGenerator*)pContext;

   // Obtain an interface to the enumerated joystick.
   LPDIRECTINPUTDEVICE8 gamepad = NULL;
   HRESULT hr = pThis->mpInput->CreateDevice( pdidInstance->guidInstance, &gamepad, NULL);
   if(FAILED(hr)) 
      return DIENUM_CONTINUE;

   TComPtr<IDirectInputDevice8> pGamepad = TComPtr<IDirectInputDevice8>(gamepad);

   // Check to see if this is a usable gamepad
   DIDEVCAPS caps;
   caps.dwSize = sizeof(caps);

   pGamepad->GetCapabilities(&caps);

   if ((caps.dwAxes < 4) || (caps.dwButtons < 4))
   {
      // Nope, not enough buttons or axes
      return DIENUM_CONTINUE;
   }

   pThis->mpGamepad[pThis->mEnumGamepadCount] = pGamepad;
   pThis->mEnumGamepadCount++;

   if (pThis->mEnumGamepadCount < kMaxControllers)
   {
      return DIENUM_CONTINUE;
   }

   // Already got 4
   return DIENUM_STOP;
}

//----------------------------------------------------------------------------

CUserInput const CInputGenerator::GetInput(int const controllerNumber /* = 0 */)
{
   CUserInput input;

   if (controllerNumber == 0)
   {
      // Only apply keyboard on input 0
      UpdateKeyboard(input);
   }

   // Update wiimote
   mpWiimote[controllerNumber]->FrameUpdate();

   XINPUT_STATE & xinput = mXInput_CurrentState[controllerNumber];
   XINPUT_STATE & prevXInput = mXInput_PreviousState[controllerNumber];
      
   DIJOYSTATE2 & state = mCurrentGamepadState[controllerNumber];
   DIJOYSTATE2 & prevState = mPreviousGamepadState[controllerNumber];

   CWiimoteState &wiimoteState = mCurrentWiimoteState[controllerNumber];
   CWiimoteState &prevWiimoteState = mPreviousWiimoteState[controllerNumber];

   bool const bGotControllerData = UpdateController(controllerNumber, xinput, state, wiimoteState);
   
   // gamepad
   if (bGotControllerData)
   {
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

         input.SetValue( it->first, mapping.MapValue( GetAxisValue(controllerNumber, mapping.GetAxis(), mapping.IsRawAxis()) ) );
      }

      memcpy(&prevState, &state, sizeof(DIJOYSTATE2));
      memcpy(&prevXInput, &xinput, sizeof(XINPUT_STATE));
      memcpy(&prevWiimoteState, &wiimoteState, sizeof(CWiimoteState));
   }

   return input;                                        
}

//----------------------------------------------------------------------------

bool CInputGenerator::HasInput(int const controllerNumber) const
{
   switch (mControllerType)
   {
   case kCT_PS3:
      if (!mpGamepad[controllerNumber].IsNull())
      {
         return true;
      }
      break;
   case kCT_Xbox360:
      if (mXInput_ControllerIndex[controllerNumber] != -1)
      {
         return true;
      }
      break;
   case kCT_Wii:
      if (IsWiimoteConnected(controllerNumber))
      {
         return true;
      }
      break;
   }

   return false;
}

//----------------------------------------------------------------------------

void CInputGenerator::ApplyRumbleSettings()
{
   if (mControllerType == kCT_Xbox360)
   {
      // XInput
      for (int loop = 0; loop < kMaxControllers; loop++)
      {
         if (mXInput_ControllerIndex[loop] == -1) continue;

         SRumbleSettings const &rumble = mRumbleSettings[loop];
         XINPUT_VIBRATION vibration;

         vibration.wLeftMotorSpeed = (uint16) (0xFFFF * rumble.mLowFreqIntensity);
         vibration.wRightMotorSpeed = (uint16) (0xFFFF * rumble.mHighFreqIntensity);

         XInputSetState( mXInput_ControllerIndex[loop], &vibration);
      }
   }
   else if (mControllerType == kCT_Wii)
   {
      // Wiimote rumble
      for (int loop = 0; loop < kMaxControllers; loop++)
      {
         CInputGeneratorWiimote * pWiimote = mpWiimote[loop];
         if (!IsWiimoteConnected(loop))
         {
            continue;
         }
         SRumbleSettings const &rumble = mRumbleSettings[loop];
         bool const bRumbleOn = (rumble.mHighFreqIntensity > 0.5f);
         pWiimote->SetRumbleState(bRumbleOn);
      }
   }
}

//----------------------------------------------------------------------------

void CInputGenerator::GetWiimoteInput(int const controllerNumber, int const numReadings, std::vector<CWiimoteState> &data) const
{
   CInputGeneratorWiimote const * pWiimote = mpWiimote[controllerNumber];
   // Only allow to read half of buffer to prevent potential invalidation.
   int const numReadingsInternal = bpe::min_val(CInputGeneratorWiimote::kReadBufferCount / 2, numReadings);

   // Get current index that is being written to and offset for read.
   int32 const startingAccessIndex = (pWiimote->mReadBufferAsyncIndex - 1) - numReadingsInternal;

   for (int loop = 0; loop < numReadingsInternal; loop++)
   {
      int const currentAccessIndex = (startingAccessIndex + loop + CInputGeneratorWiimote::kReadBufferCount) % CInputGeneratorWiimote::kReadBufferCount;
      CWiimoteReadInfo const * const pReadInfoEntry = &pWiimote->mReadBuffer[currentAccessIndex];

      CWiimoteRawStatus_NC wr;
      pWiimote->ParseReadInfo(pReadInfoEntry, wr);
      CWiimoteState state;
      pWiimote->ConvertRawStatus(wr, state);
      data.push_back(state);
   }
}

//----------------------------------------------------------------------------

void CInputGenerator::UpdateKeyboard( CUserInput & input )
{
   // Don't read keyboard if don't have focus. 
   // Prevents confusion when game responds to 'Esc' when it's not focused.   
   if (!OsContext()->HaveFocus())
   {
      return;
   }

   uint8 state[256];

   HRESULT res = mpKeyboard->GetDeviceState( 256, &state );
   if( FAILED( res ) )
   {
      res = mpKeyboard->Acquire();

      while( res == DIERR_INPUTLOST ) 
      {
         res = mpKeyboard->Acquire();
      }
   }
   else
   {
      // check key mappings
      for( int j = 0; j < 256; ++j )
      {
         EButtonState buttonState = kBS_NotPressed;
         if( state[j] && !mPreviousState[j])       buttonState = kBS_Pressed;
         else if( state[j] && mPreviousState[j])   buttonState = kBS_Pressing;
         else if( !state[j] && mPreviousState[j])  buttonState = kBS_Released;

         for( uint32 i = 0; i < mKeyMappings.size(); ++i )
         {
            if( mKeyMappings[i] )
            {
               CKeyMapping const & mapping = *mKeyMappings[i];
               if( (mapping.mKeyCode == j) && (mapping.mButtonState == buttonState) )
               {
                  input.SetValue( static_cast<CUserInput::EAction>( i ), mapping.mValue );
               }
            }
         }
      }

      memcpy( &mPreviousState, &state, 256 );
   }
}

//----------------------------------------------------------------------------

bool const CInputGenerator::UpdateController(int const controllerNumber, XINPUT_STATE & xinput, DIJOYSTATE2 & state, CWiimoteState &wiimoteState )
{
   if (!HasInput(controllerNumber)) return false;

   switch( mControllerType )
   {
   case kCT_None:
      return false;
      break;

   case kCT_PS3:
      {
         HRESULT res = mpGamepad[controllerNumber]->Poll();
         if( FAILED(res) )
         {
            res = mpGamepad[controllerNumber]->Acquire();

            while(res == DIERR_INPUTLOST)
            {
               res = mpGamepad[controllerNumber]->Acquire();
            }
         }
         else
         {
            if( SUCCEEDED( mpGamepad[controllerNumber]->GetDeviceState(sizeof(DIJOYSTATE2), &state) ) )
            {
               return true;
            }
         }

         return false;
         
      }
      break;

   case kCT_Xbox360:
      {
         return XInputGetState( mXInput_ControllerIndex[controllerNumber], &xinput) == ERROR_SUCCESS;
      }
      break;

   case kCT_Wii:
      {
         if (IsWiimoteConnected(controllerNumber))
         {        
            wiimoteState = CWiimoteState(); // Clear current state

            CInputGeneratorWiimote *pWiimote = mpWiimote[controllerNumber];
            CWiimoteRawStatus_NC wr;
            pWiimote->ParseReadInfo(pWiimote->GetCurrentReadInfo(), wr);
            pWiimote->ConvertRawStatus(wr, wiimoteState);
            pWiimote->CalculateAverageIRPosition(wiimoteState);
            return true;
         }
      }
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

real32 const CInputGenerator::GetAxisValue(int const controllerNumber, int const axis, bool const bUseRawValue )
{
   switch( mControllerType )
   {
   case kCT_None:
      return 0.0f;

   case kCT_PS3:
      {
         DIJOYSTATE2 const & state = mCurrentGamepadState[controllerNumber];

         int axisValue = 0;
         int perpAxisValue = 0;

         int tempAxisValue = state.lZ - gkInt16Max;
         int tempPerpAxisValue = state.lRz - gkInt16Max;

         switch( axis )
         {
         case kA_PS3_A1_LeftRight:
            axisValue = state.lX - gkInt16Max;
            perpAxisValue = state.lY - gkInt16Max;
            break;
         case kA_PS3_A1_UpDown:
            axisValue = state.lY - gkInt16Max;
            perpAxisValue = state.lX - gkInt16Max;
            break;
         case kA_PS3_A2_LeftRight:
            axisValue = tempAxisValue;
            perpAxisValue = tempPerpAxisValue;
            break;
         case kA_PS3_A2_UpDown:
            axisValue = tempPerpAxisValue;
            perpAxisValue = tempAxisValue;
            break;
            
         case kA_PS3_LeftTrigger:
               // Currently no analog triggers for PS3 pad under Win32
               return 0.0f;
         case kA_PS3_RightTrigger:
               // Currently no analog triggers for PS3 pad under Win32               
               return 0.0f;
         }
         if (bUseRawValue)
         {
            // No dead zone
            return GetAxis(axisValue, perpAxisValue, gkInt16Max, 0);
         }

         // Use dead zone
         return GetAxis(axisValue, perpAxisValue, gkInt16Max, gkInt16Max / 5);
      }
      break;

   case kCT_Xbox360:
      {
         XINPUT_STATE const & xinput = mXInput_CurrentState[controllerNumber];
         switch( axis )
         {
            case kA_Xbox360_Left_LeftRight:
               return GetAxis( (int)xinput.Gamepad.sThumbLX, xinput.Gamepad.sThumbLY, gkInt16Max, bUseRawValue ? 0 : XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
               break;
            case kA_Xbox360_Left_UpDown:
               return -GetAxis( (int)xinput.Gamepad.sThumbLY, xinput.Gamepad.sThumbLX, gkInt16Max, bUseRawValue ? 0 : XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE );
               break;
            case kA_Xbox360_Right_LeftRight:
               return GetAxis( (int)xinput.Gamepad.sThumbRX, xinput.Gamepad.sThumbRY, gkInt16Max, bUseRawValue ? 0 : XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
               break;
            case kA_Xbox360_Right_UpDown:
               return -GetAxis( (int)xinput.Gamepad.sThumbRY, xinput.Gamepad.sThumbRX, gkInt16Max, bUseRawValue ? 0 : XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE );
               break;

            case kA_Xbox360_LeftTrigger:
               return GetAxis( (int)xinput.Gamepad.bLeftTrigger, 0, gkUint8Max, bUseRawValue ? 0 : XINPUT_GAMEPAD_TRIGGER_THRESHOLD );
               break;
            case kA_Xbox360_RightTrigger:
               return GetAxis( (int)xinput.Gamepad.bRightTrigger, 0, gkUint8Max, bUseRawValue ? 0 : XINPUT_GAMEPAD_TRIGGER_THRESHOLD );
               break;
         }
      }
      break;
   case kCT_Wii:
      {
         // Note that accelerometer axis Y/Z are switched so they correspond to -Z being into screen.
         // This matches the PS3 controller acc. values.
         CWiimoteState const &state = mCurrentWiimoteState[controllerNumber];
         switch (axis)
         {
         case kA_Wiimote_AccX:
            return state.mAcceleration_Calibrated.mX;
            break;
         case kA_Wiimote_AccY:
            return state.mAcceleration_Calibrated.mZ;
            break;
         case kA_Wiimote_AccZ:
            return state.mAcceleration_Calibrated.mY;
            break;

         case kA_Wiimote_IRSensorX:
            return state.mIRSensorAverage.mX;
            break;
         case kA_Wiimote_IRSensorY:
            return state.mIRSensorAverage.mY;
            break;

         case kA_Wiimote_AccX_Nunchuck:
            return state.mAcceleration_Calibrated_Nunchuck.mX;
            break;
         case kA_Wiimote_AccY_Nunchuck:
            return state.mAcceleration_Calibrated_Nunchuck.mZ;
            break;
         case kA_Wiimote_AccZ_Nunchuck:
            return state.mAcceleration_Calibrated_Nunchuck.mY;
            break;

         case kA_Wiimote_Nunchuck_StickX:
            return GetAxis(state.mAxisX_Nunchuck, state.mAxisY_Nunchuck, 100, bUseRawValue ? 0 : 16);
            break;
         case kA_Wiimote_Nunchuck_StickY:
            return GetAxis(state.mAxisY_Nunchuck, state.mAxisX_Nunchuck, 100, bUseRawValue ? 0 : 16);
            break;
         }
      }
      break;
   }

   // Unknown axis
   return 0.0f;
}

//----------------------------------------------------------------------------

EButtonState const CInputGenerator::GetButtonPress( int const controllerNumber, int const button )
{
   switch( mControllerType )
   {
   case kCT_None:
      return kBS_NotPressed;

   case kCT_PS3:
      {
         DIJOYSTATE2 const & state = mCurrentGamepadState[controllerNumber];
         DIJOYSTATE2 const & prevState = mPreviousGamepadState[controllerNumber];

         bool isPov = false;
         int povValue = 0;
      
         switch( button )
         {
         case kPB_PS3_Up:
            isPov = true; povValue = 0; break;
         case kPB_PS3_Right:
            isPov = true; povValue = 9000; break;
         case kPB_PS3_Down:
            isPov = true; povValue = 18000; break;
         case kPB_PS3_Left:
            isPov = true; povValue = 27000; break;

         case kPB_PS3_LStick_Up:
         case kPB_PS3_LStick_Right:
         case kPB_PS3_LStick_Down:
         case kPB_PS3_LStick_Left:
         case kPB_PS3_RStick_Up:
         case kPB_PS3_RStick_Right:
         case kPB_PS3_RStick_Down:
         case kPB_PS3_RStick_Left:
            // Not supported under Win32 for PS3 controller.
            return kBS_NotPressed;
            break;

         default:
            {
               if( state.rgbButtons[button] != prevState.rgbButtons[button] )
               {
                  if( state.rgbButtons[button] != 0 )
                     return kBS_Pressed;
                  else
                     return kBS_Released;
               }
               else
               {
                  if( state.rgbButtons[button] != 0 )
                     return kBS_Held;
                  else
                     return kBS_NotPressed;
               }
            }
            break;
         }
      
         if( isPov )
         {
            if( state.rgdwPOV[0] != prevState.rgdwPOV[0] )
            {
               if( state.rgdwPOV[0] == povValue )
               {
                  return kBS_Pressed;
               }
               else if( prevState.rgdwPOV[0] == povValue )
               {
                  return kBS_Released;
               }
            }
            else
            {
               if( state.rgdwPOV[0] == povValue )
                  return kBS_Held;
               else
                  return kBS_NotPressed;
            }
         }
      }

   case kCT_Xbox360:
      {
         XINPUT_STATE const & xinput = mXInput_CurrentState[controllerNumber];
         XINPUT_STATE const & prevXInput = mXInput_PreviousState[controllerNumber];

         bool const pressedNow = IsButtonPressed_XInput( xinput, button );
         bool const wasPressed = IsButtonPressed_XInput( prevXInput, button );
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
   case kCT_Wii:
      {
         CWiimoteState const &state = mCurrentWiimoteState[controllerNumber];
         CWiimoteState const &prevState = mPreviousWiimoteState[controllerNumber];
         bool const pressedNow = IsButtonPressed_Wiimote( state, button );
         bool const wasPressed = IsButtonPressed_Wiimote( prevState, button );
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

         return kBS_NotPressed;
      }
      break;
   }

   // Unknown button
   return kBS_NotPressed;
}

//----------------------------------------------------------------------------

bool const CInputGenerator::IsButtonPressed_XInput( XINPUT_STATE const & input, int const button )
{
   int const kBinaryStickThreshold = gkInt16Max / 2;

   switch( button )
   {
   case kPB_Xbox360_Back:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
   case kPB_Xbox360_Start:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
   case kPB_Xbox360_X:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
   case kPB_Xbox360_Y:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
   case kPB_Xbox360_B:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
   case kPB_Xbox360_A:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
   case kPB_Xbox360_LeftShoulder:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
   case kPB_Xbox360_RightShoulder:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
   case kPB_Xbox360_LeftThumb:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
   case kPB_Xbox360_RightThumb:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
   case kPB_Xbox360_LeftTrigger:
      return (input.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
   case kPB_Xbox360_RightTrigger:
      return (input.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
   case kPB_Xbox360_DPad_Up:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
   case kPB_Xbox360_DPad_Right:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
   case kPB_Xbox360_DPad_Down:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
   case kPB_Xbox360_DPad_Left:
      return (input.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;

   case kPB_Xbox360_LStick_Up:
      return (input.Gamepad.sThumbLY > kBinaryStickThreshold);
   case kPB_Xbox360_LStick_Down:
      return (input.Gamepad.sThumbLY < -kBinaryStickThreshold);
   case kPB_Xbox360_LStick_Right:
      return (input.Gamepad.sThumbLX > kBinaryStickThreshold);
   case kPB_Xbox360_LStick_Left:
      return (input.Gamepad.sThumbLX < -kBinaryStickThreshold);

   case kPB_Xbox360_RStick_Up:
      return (input.Gamepad.sThumbRY > kBinaryStickThreshold);
   case kPB_Xbox360_RStick_Down:
      return (input.Gamepad.sThumbLY < -kBinaryStickThreshold);
   case kPB_Xbox360_RStick_Right:
      return (input.Gamepad.sThumbRX > kBinaryStickThreshold);
   case kPB_Xbox360_RStick_Left:
      return (input.Gamepad.sThumbRX < -kBinaryStickThreshold);
   }

   return false;
}

//----------------------------------------------------------------------------

bool const CInputGenerator::IsButtonPressed_Wiimote(CWiimoteState const & input, int const button)
{
   uint16 const skUp    = 0x8;
   uint16 const skDown  = 0x4;
   uint16 const skLeft  = 0x1;
   uint16 const skRight = 0x2;

   switch( button )
   {
   case kPB_Wiimote_A:
      return ((input.mButtonState & (0x0800)) != 0);
      break;
   case kPB_Wiimote_B:
      return ((input.mButtonState & (0x0400)) != 0);
      break;
   case kPB_Wiimote_Minus:
      return ((input.mButtonState & (0x1000)) != 0);
      break;
   case kPB_Wiimote_Plus:
      return ((input.mButtonState & (0x0010)) != 0);
      break;
   case kPB_Wiimote_Home:
      return ((input.mButtonState & (0x8000)) != 0);
      break;
   case kPB_Wiimote_1:
      return ((input.mButtonState & (0x0200)) != 0);
      break;
   case kPB_Wiimote_2:
      return ((input.mButtonState & (0x0100)) != 0);
      break;
   case kPB_Wiimote_DPad_Up:
      return ((input.mButtonState & (skUp)) != 0);
      break;
   case kPB_Wiimote_DPad_Right:
      return ((input.mButtonState & (skRight)) != 0);
      break;
   case kPB_Wiimote_DPad_Down:
      return ((input.mButtonState & (skDown)) != 0);
      break;
   case kPB_Wiimote_DPad_Left:
      return ((input.mButtonState & (skLeft)) != 0);
      break;
   case kPB_Wiimote_C:
      return ((input.mButtonState_Nunchuck & (0x02)) == 0); // NOTE: Nunchuck buttons are inverted.
      break;
   case kPB_Wiimote_Z:
      return ((input.mButtonState_Nunchuck & (0x01)) == 0);
      break;
   }

   return false;
}

//----------------------------------------------------------------------------

bool CInputGenerator::IsWiimoteConnected(int const controllerNumber) const
{
   CInputGeneratorWiimote *pWiimote = mpWiimote[controllerNumber];
   if (pWiimote && pWiimote->IsConnected())
   {
      return true;
   }
   return false;
}

//----------------------------------------------------------------------------
// Platform specific implementation, resolved at link time
//----------------------------------------------------------------------------

CInputEvent::EKey CBaseInputGenerator::ConvertVirtualKey(int const inKey)
{
   switch( inKey )
   {
      case VK_LBUTTON:     return CInputEvent::kKey_LButton; 
      case VK_RBUTTON:     return CInputEvent::kKey_RButton; 
      case VK_MBUTTON:     return CInputEvent::kKey_MButton; 
      case VK_BACK:        return CInputEvent::kKey_Back; 
      case VK_TAB:         return CInputEvent::kKey_Tab; 
      case VK_RETURN:      return CInputEvent::kKey_Return; 
      case VK_SHIFT:       return CInputEvent::kKey_Shift; 
      case VK_CONTROL:     return CInputEvent::kKey_Control; 
      case VK_MENU:        return CInputEvent::kKey_Menu; 
      case VK_PAUSE:       return CInputEvent::kKey_Pause; 
      case VK_CAPITAL:     return CInputEvent::kKey_Capital; 
      case VK_ESCAPE:      return CInputEvent::kKey_Escape; 
      case VK_SPACE:       return CInputEvent::kKey_Space; 
      case VK_PRIOR:       return CInputEvent::kKey_Prior; 
      case VK_NEXT:        return CInputEvent::kKey_Next; 
      case VK_END:         return CInputEvent::kKey_End; 
      case VK_HOME:        return CInputEvent::kKey_Home; 
      case VK_LEFT:        return CInputEvent::kKey_Left; 
      case VK_UP:          return CInputEvent::kKey_Up; 
      case VK_RIGHT:       return CInputEvent::kKey_Right; 
      case VK_DOWN:        return CInputEvent::kKey_Down; 
      case VK_SELECT:      return CInputEvent::kKey_Select; 
      case VK_PRINT:       return CInputEvent::kKey_Print; 
      case VK_EXECUTE:     return CInputEvent::kKey_Execute; 
      case VK_SNAPSHOT:    return CInputEvent::kKey_Snapshot; 
      case VK_INSERT:      return CInputEvent::kKey_Insert; 
      case VK_DELETE:      return CInputEvent::kKey_Delete; 
      case VK_HELP:        return CInputEvent::kKey_Help; 
      case '0':            return CInputEvent::kKey_0; 
      case '1':            return CInputEvent::kKey_1; 
      case '2':            return CInputEvent::kKey_2; 
      case '3':            return CInputEvent::kKey_3; 
      case '4':            return CInputEvent::kKey_4; 
      case '5':            return CInputEvent::kKey_5; 
      case '6':            return CInputEvent::kKey_6; 
      case '7':            return CInputEvent::kKey_7; 
      case '8':            return CInputEvent::kKey_8; 
      case '9':            return CInputEvent::kKey_9; 
      case 'A':            return CInputEvent::kKey_A; 
      case 'B':            return CInputEvent::kKey_B; 
      case 'C':            return CInputEvent::kKey_C; 
      case 'D':            return CInputEvent::kKey_D; 
      case 'E':            return CInputEvent::kKey_E; 
      case 'F':            return CInputEvent::kKey_F; 
      case 'G':            return CInputEvent::kKey_G; 
      case 'H':            return CInputEvent::kKey_H; 
      case 'I':            return CInputEvent::kKey_I; 
      case 'J':            return CInputEvent::kKey_J; 
      case 'K':            return CInputEvent::kKey_K; 
      case 'L':            return CInputEvent::kKey_L; 
      case 'M':            return CInputEvent::kKey_M; 
      case 'N':            return CInputEvent::kKey_N; 
      case 'O':            return CInputEvent::kKey_O; 
      case 'P':            return CInputEvent::kKey_P; 
      case 'Q':            return CInputEvent::kKey_Q; 
      case 'R':            return CInputEvent::kKey_R; 
      case 'S':            return CInputEvent::kKey_S; 
      case 'T':            return CInputEvent::kKey_T; 
      case 'U':            return CInputEvent::kKey_U; 
      case 'V':            return CInputEvent::kKey_V; 
      case 'W':            return CInputEvent::kKey_W; 
      case 'X':            return CInputEvent::kKey_X; 
      case 'Y':            return CInputEvent::kKey_Y; 
      case 'Z':            return CInputEvent::kKey_Z; 
      case VK_LWIN:        return CInputEvent::kKey_LWin; 
      case VK_RWIN:        return CInputEvent::kKey_RWin; 
      case VK_APPS:        return CInputEvent::kKey_Apps; 
      case VK_NUMPAD0:     return CInputEvent::kKey_NumPad0; 
      case VK_NUMPAD1:     return CInputEvent::kKey_NumPad1; 
      case VK_NUMPAD2:     return CInputEvent::kKey_NumPad2; 
      case VK_NUMPAD3:     return CInputEvent::kKey_NumPad3; 
      case VK_NUMPAD4:     return CInputEvent::kKey_NumPad4; 
      case VK_NUMPAD5:     return CInputEvent::kKey_NumPad5; 
      case VK_NUMPAD6:     return CInputEvent::kKey_NumPad6; 
      case VK_NUMPAD7:     return CInputEvent::kKey_NumPad7; 
      case VK_NUMPAD8:     return CInputEvent::kKey_NumPad8; 
      case VK_NUMPAD9:     return CInputEvent::kKey_NumPad9; 
      case VK_MULTIPLY:    return CInputEvent::kKey_Multiply; 
      case VK_ADD:         return CInputEvent::kKey_Add; 
      case VK_SEPARATOR:   return CInputEvent::kKey_Separator; 
      case VK_SUBTRACT:    return CInputEvent::kKey_Subtract; 
      case VK_DECIMAL:     return CInputEvent::kKey_Decimal; 
      case VK_DIVIDE:      return CInputEvent::kKey_Divide; 
      case VK_F1:          return CInputEvent::kKey_F1; 
      case VK_F2:          return CInputEvent::kKey_F2; 
      case VK_F3:          return CInputEvent::kKey_F3; 
      case VK_F4:          return CInputEvent::kKey_F4; 
      case VK_F5:          return CInputEvent::kKey_F5; 
      case VK_F6:          return CInputEvent::kKey_F6; 
      case VK_F7:          return CInputEvent::kKey_F7; 
      case VK_F8:          return CInputEvent::kKey_F8; 
      case VK_F9:          return CInputEvent::kKey_F9; 
      case VK_F10:         return CInputEvent::kKey_F10; 
      case VK_F11:         return CInputEvent::kKey_F11; 
      case VK_F12:         return CInputEvent::kKey_F12; 
      case VK_F13:         return CInputEvent::kKey_F13; 
      case VK_F14:         return CInputEvent::kKey_F14; 
      case VK_F15:         return CInputEvent::kKey_F15; 
      case VK_F16:         return CInputEvent::kKey_F16; 
      case VK_F17:         return CInputEvent::kKey_F17; 
      case VK_F18:         return CInputEvent::kKey_F18; 
      case VK_F19:         return CInputEvent::kKey_F19; 
      case VK_F20:         return CInputEvent::kKey_F20; 
      case VK_F21:         return CInputEvent::kKey_F21; 
      case VK_F22:         return CInputEvent::kKey_F22; 
      case VK_F23:         return CInputEvent::kKey_F23; 
      case VK_F24:         return CInputEvent::kKey_F24; 
      case VK_NUMLOCK:     return CInputEvent::kKey_NumLock; 
      case VK_SCROLL:      return CInputEvent::kKey_Scroll; 
      default:             return CInputEvent::kKey_None; 
   }
}

//----------------------------------------------------------------------------

CBaseInputGenerator * CBaseInputGenerator::Factory()
{
   return new CInputGenerator();
}

//----------------------------------------------------------------------------


