//----------------------------------------------------------------------------
// Win32CInputGenerator.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Input/CInputGenerator.h"
#include "Engine/Mechanics/TComPtr.h"

#define DIRECTINPUT_VERSION 0x800
#include "dinput.h"
#include "XInput.h"

//----------------------------------------------------------------------------

class CInputGeneratorWiimote;

//----------------------------------------------------------------------------

class ENGINE_API CInputGenerator : public CBaseInputGenerator
{
public:
   CInputGenerator();
   ~CInputGenerator();

   virtual bool             HasInput(int const controllerNumber = 0) const;
   virtual CUserInput const GetInput(int const controllerNumber = 0);
   virtual void             ApplyRumbleSettings();
   virtual void             GetWiimoteInput(int const controllerNumber, int const numReadings, std::vector<CWiimoteState> &data) const;
   
private:
   static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );

   void UpdateKeyboard(CUserInput & input);
   bool const UpdateController(int const controllerNumber, XINPUT_STATE & xinput, DIJOYSTATE2 & state, CWiimoteState &wiimoteState);
   real32 const GetAxisValue(int const controllerNumber, int const axis, bool const bUseRawValue);

   EButtonState const GetButtonPress(int const controllerNumber, int const button);
   bool const IsButtonPressed_XInput(XINPUT_STATE const & input, int const button);
   bool const IsButtonPressed_Wiimote(CWiimoteState const & input, int const button);

   bool IsWiimoteConnected(int const controllerNumber) const;

private:
   TComPtr<IDirectInput8>        mpInput;

   TComPtr<IDirectInputDevice8>  mpKeyboard;
   uint8                         mPreviousState[256];

   // xbox 360 controller
   int                           mXInput_ControllerIndex[kMaxControllers];
   XINPUT_STATE                  mXInput_CurrentState[kMaxControllers];
   XINPUT_STATE                  mXInput_PreviousState[kMaxControllers];

   // ps2 controller
   int                           mEnumGamepadCount;
   TComPtr<IDirectInputDevice8>  mpGamepad[kMaxControllers];
   DIJOYSTATE2                   mCurrentGamepadState[kMaxControllers];
   DIJOYSTATE2                   mPreviousGamepadState[kMaxControllers];

   // Wiimote
   CInputGeneratorWiimote *      mpWiimote[kMaxControllers];
   CWiimoteState                 mCurrentWiimoteState[kMaxControllers];
   CWiimoteState                 mPreviousWiimoteState[kMaxControllers];
};

//----------------------------------------------------------------------------

