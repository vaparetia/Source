//----------------------------------------------------------------------------
// CInputGenerator.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "boost/optional.hpp"

//----------------------------------------------------------------------------

class CInputEvent
{
public:
   enum EType
   {
      kType_Char,
      kType_KeyDown,
      kType_KeyUp,

      kType_Invalid = -1
   };

   enum EKey
   {
      kKey_None,

      kKey_LButton,
      kKey_RButton,
      kKey_MButton,
      kKey_Back,
      kKey_Tab,
      kKey_Return,
      kKey_Shift,
      kKey_Control,
      kKey_Menu,
      kKey_Pause,
      kKey_Capital,
      kKey_Escape,
      kKey_Space,
      kKey_Prior,
      kKey_Next,
      kKey_End,
      kKey_Home,
      kKey_Left,
      kKey_Up,
      kKey_Right,
      kKey_Down,
      kKey_Select,
      kKey_Print,
      kKey_Execute,
      kKey_Snapshot,
      kKey_Insert,
      kKey_Delete,
      kKey_Help,

      kKey_1,
      kKey_2,
      kKey_3,
      kKey_4,
      kKey_5,
      kKey_6,
      kKey_7,
      kKey_8,
      kKey_9,
      kKey_0,

      kKey_A,
      kKey_B,
      kKey_C,
      kKey_D,
      kKey_E,
      kKey_F,
      kKey_G,
      kKey_H,
      kKey_I,
      kKey_J,
      kKey_K,
      kKey_L,
      kKey_M,
      kKey_N,
      kKey_O,
      kKey_P,
      kKey_Q,
      kKey_R,
      kKey_S,
      kKey_T,
      kKey_U,
      kKey_V,
      kKey_W,
      kKey_X,
      kKey_Y,
      kKey_Z,

      kKey_LWin,
      kKey_RWin,
      kKey_Apps,

      kKey_NumPad0,
      kKey_NumPad1,
      kKey_NumPad2,
      kKey_NumPad3,
      kKey_NumPad4,
      kKey_NumPad5,
      kKey_NumPad6,
      kKey_NumPad7,
      kKey_NumPad8,
      kKey_NumPad9,

      kKey_Multiply,
      kKey_Add,
      kKey_Separator,
      kKey_Subtract,
      kKey_Decimal,
      kKey_Divide,
      kKey_F1,
      kKey_F2,
      kKey_F3,
      kKey_F4,
      kKey_F5,
      kKey_F6,
      kKey_F7,
      kKey_F8,
      kKey_F9,
      kKey_F10,
      kKey_F11,
      kKey_F12,
      kKey_F13,
      kKey_F14,
      kKey_F15,
      kKey_F16,
      kKey_F17,
      kKey_F18,
      kKey_F19,
      kKey_F20,
      kKey_F21,
      kKey_F22,
      kKey_F23,
      kKey_F24,

      kKey_NumLock,
      kKey_Scroll
   };

public:
   ENGINE_API explicit CInputEvent( EType const type,
                                    int const data );

   EType const GetType() const { return mType; }

   char const GetChar() const { BPE_ASSERT( mType == kType_Char, "must be char event" ); return static_cast<char>( mData ); }
   EKey const GetKey() const { BPE_ASSERT( mType == kType_KeyDown || mType == kType_KeyUp, "must be key event" ); return static_cast<EKey>( mData ); }

private:
   EType mType;
   int   mData;
};

//----------------------------------------------------------------------------

class CUserInput
{
public:
   enum EAction
   {
      kScriptingSelection_Left,
      kScriptingSelection_Right,
      kScriptingSelection_RunScriptFile,
      kScriptingSelection_QuitApplication,

      kDevelopMode_ToggleConsole,   // Toggle console currently only enabled for keyboard input.
                                    // Pad input uses alternative kDevelopMode_CommandPressed clicks.

      kDevelopMode_CommandPressed,  // Multi function button for develop mode.
      kDevelopMode_CommandHeld,     // In-game:
                                    // * Double click: show console
                                    // * Double click and hold: quit scripting
                                    // Scripting Selection:
                                    // * Click: Exit application (uses kScriptingSelection_QuitApplication)


      kBF_ShakePX,
      kBF_ShakeNX,
      kBF_RepellerTriggered,

      // ------IMPORTANT SECTION BEGIN
      // These enums are equivalent to the enums specified in the button press component, make sure they match up.

      // General
      kGeneralP_Back,
      kGeneralR_Back,
      kGeneralH_Back,
      kGeneralP_Start,
      kGeneralR_Start,
      kGeneralH_Start,
      kGeneralP_X,
      kGeneralR_X,
      kGeneralH_X,
      kGeneralP_Y,
      kGeneralR_Y,
      kGeneralH_Y,
      kGeneralP_B,
      kGeneralR_B,
      kGeneralH_B,
      kGeneralP_A,
      kGeneralR_A,
      kGeneralH_A,
      kGeneralP_LeftShoulder,
      kGeneralR_LeftShoulder,
      kGeneralH_LeftShoulder,
      kGeneralP_RightShoulder,
      kGeneralR_RightShoulder,
      kGeneralH_RightShoulder,
      kGeneralP_LeftThumb,
      kGeneralR_LeftThumb,
      kGeneralH_LeftThumb,
      kGeneralP_RightThumb,
      kGeneralR_RightThumb,
      kGeneralH_RightThumb,
      kGeneralP_LeftTrigger,
      kGeneralR_LeftTrigger,
      kGeneralH_LeftTrigger,
      kGeneralP_RightTrigger,
      kGeneralR_RightTrigger,
      kGeneralH_RightTrigger,
      kGeneralP_DPad_Up,
      kGeneralR_DPad_Up,
      kGeneralH_DPad_Up,
      kGeneralP_DPad_Right,
      kGeneralR_DPad_Right,
      kGeneralH_DPad_Right,
      kGeneralP_DPad_Down,
      kGeneralR_DPad_Down,
      kGeneralH_DPad_Down,
      kGeneralP_DPad_Left,
      kGeneralR_DPad_Left,
      kGeneralH_DPad_Left,

      // These are mapped to A/B (X/0) based on region settings
      kGeneralP_CmdBack,
      kGeneralR_CmdBack,
      kGeneralH_CmdBack,
      kGeneralP_CmdSelect,
      kGeneralR_CmdSelect,
      kGeneralH_CmdSelect,

      // 'Binary' versions for analog stick inputs
      kGeneralP_LStick_Up,
      kGeneralR_LStick_Up,
      kGeneralH_LStick_Up,
      kGeneralP_LStick_Right,
      kGeneralR_LStick_Right,
      kGeneralH_LStick_Right,
      kGeneralP_LStick_Down,
      kGeneralR_LStick_Down,
      kGeneralH_LStick_Down,
      kGeneralP_LStick_Left,
      kGeneralR_LStick_Left,
      kGeneralH_LStick_Left,

      kGeneralP_RStick_Up,
      kGeneralR_RStick_Up,
      kGeneralH_RStick_Up,
      kGeneralP_RStick_Right,
      kGeneralR_RStick_Right,
      kGeneralH_RStick_Right,
      kGeneralP_RStick_Down,
      kGeneralR_RStick_Down,
      kGeneralH_RStick_Down,
      kGeneralP_RStick_Left,
      kGeneralR_RStick_Left,
      kGeneralH_RStick_Left,

      // ------IMPORTANT SECTION END (READ ABOVE FOR COMMENT)


      kGeneral_Accel_X1,
      kGeneral_Accel_Y1,
      kGeneral_Accel_Z1,

      kGeneral_Accel_X2,
      kGeneral_Accel_Y2,
      kGeneral_Accel_Z2,

      kGeneral_IR_Sensor_X,
      kGeneral_IR_Sensor_Y,

      kGeneral_LS_Raw_X,
      kGeneral_LS_Raw_Y,
      kGeneral_LT_Raw,

      kGeneral_RS_Raw_X,
      kGeneral_RS_Raw_Y,
      kGeneral_RT_Raw,

      kGeneral_LS_Filtered_X,
      kGeneral_LS_Filtered_Y,
      kGeneral_LT_Filtered,

      kGeneral_RS_Filtered_X,
      kGeneral_RS_Filtered_Y,
      kGeneral_RT_Filtered,

      kAction_Count
   };

public:
   ENGINE_API CUserInput();

   bool const GetDigitalValue( EAction const action ) const { return mValues[action] > 0.99f; }
   real32 const GetAnalogValue( EAction const action ) const { return mValues[action]; }

   void SetValue( EAction const action, real32 const value ) { mValues[action] = value; }
   
   void SetKeyDown( int const key, bool const down ) { mKeyDown[key] = down; }
   bool const IsKeyDown( int const key ) const { return mKeyDown[key]; }

   ENGINE_API static CUserInput const & NullInput();    // 'Fake' zero input, useful for debugging etc.
   
private:
   real32   mValues[kAction_Count];
   bool     mKeyDown[256];
};

//----------------------------------------------------------------------------

enum EPadButton_PS3
{
   kPB_PS3_Triangle,
   kPB_PS3_Circle,
   kPB_PS3_X,
   kPB_PS3_Square,
   kPB_PS3_L2,
   kPB_PS3_R2,
   kPB_PS3_L1,
   kPB_PS3_R1,
   kPB_PS3_Start,
   kPB_PS3_Select,
   kPB_PS3_LeftThumb,
   kPB_PS3_RightThumb,

   kPB_PS3_Up,
   kPB_PS3_Right,
   kPB_PS3_Down,
   kPB_PS3_Left,

   kPB_PS3_LStick_Up,
   kPB_PS3_LStick_Down,
   kPB_PS3_LStick_Right,
   kPB_PS3_LStick_Left,

   kPB_PS3_RStick_Up,
   kPB_PS3_RStick_Down,
   kPB_PS3_RStick_Right,
   kPB_PS3_RStick_Left,

   kPB_PS3_TipLeft,
   kPB_PS3_TipRight
};

enum EPadButton_Xbox360
{
   kPB_Xbox360_Back,
   kPB_Xbox360_Start,
   kPB_Xbox360_X,
   kPB_Xbox360_Y,
   kPB_Xbox360_B,
   kPB_Xbox360_A,
   kPB_Xbox360_LeftShoulder,
   kPB_Xbox360_RightShoulder,
   kPB_Xbox360_LeftThumb,
   kPB_Xbox360_RightThumb,
   kPB_Xbox360_LeftTrigger,
   kPB_Xbox360_RightTrigger,
   kPB_Xbox360_DPad_Up,
   kPB_Xbox360_DPad_Right,
   kPB_Xbox360_DPad_Down,
   kPB_Xbox360_DPad_Left,

   kPB_Xbox360_LStick_Up,
   kPB_Xbox360_LStick_Down,
   kPB_Xbox360_LStick_Right,
   kPB_Xbox360_LStick_Left,

   kPB_Xbox360_RStick_Up,
   kPB_Xbox360_RStick_Down,
   kPB_Xbox360_RStick_Right,
   kPB_Xbox360_RStick_Left

}; 

enum EPadButton_Wiimote
{
   // These members are based on defines in 
   // wpad.h, so please do not change them unless 
   // wpad.h changes
   kPB_Wiimote_DPad_Left    = 0,  // 0x0001
   kPB_Wiimote_DPad_Right   = 1,  // 0x0002
   kPB_Wiimote_DPad_Down    = 2,  // 0x0004
   kPB_Wiimote_DPad_Up      = 3,  // 0x0008
   kPB_Wiimote_Plus         = 4,  // 0x0010
   kPB_Wiimote_2            = 8,  // 0x0100
   kPB_Wiimote_1            = 9,  // 0x0200
   kPB_Wiimote_B            = 10, // 0x0400
   kPB_Wiimote_A            = 11, // 0x0800
   kPB_Wiimote_Minus        = 12, // 0x1000
   kPB_Wiimote_Home         = 15, // 0x8000
   kPB_Wiimote_Z            = 13, // 0x2000
   kPB_Wiimote_C            = 14  // 0x4000
};

enum EAxis_PS3
{
   kA_PS3_A1_UpDown,
   kA_PS3_A1_LeftRight,
   kA_PS3_A2_UpDown,
   kA_PS3_A2_LeftRight,

   kA_PS3_LeftTrigger,
   kA_PS3_RightTrigger,
   
   kA_PS3_AccX,
   kA_PS3_AccY,
   kA_PS3_AccZ,
   kA_PS3_AccGyro

};

enum EAxis_Xbox360
{
   kA_Xbox360_Left_UpDown,
   kA_Xbox360_Left_LeftRight,
   kA_Xbox360_Right_UpDown,
   kA_Xbox360_Right_LeftRight,

   kA_Xbox360_LeftTrigger,
   kA_Xbox360_RightTrigger
};

enum EAxis_Wiimote
{
   kA_Wiimote_AccX,
   kA_Wiimote_AccY,
   kA_Wiimote_AccZ,

   kA_Wiimote_IRSensorX,
   kA_Wiimote_IRSensorY,

   kA_Wiimote_AccX_Nunchuck,
   kA_Wiimote_AccY_Nunchuck,
   kA_Wiimote_AccZ_Nunchuck,

   kA_Wiimote_Nunchuck_StickX,
   kA_Wiimote_Nunchuck_StickY
};

class CPadAxisMapping
{
public:
   explicit CPadAxisMapping( uint32 const axis,
                             real32 const minValue,
                             real32 const maxValue,
                             real32 const multiplier,
                             bool const bGetRawAxisData = false)
   :  mAxis( axis )
   ,  mMinValue( minValue )
   ,  mMaxValue( maxValue )
   ,  mMultiplier( multiplier )
   {
      if (bGetRawAxisData)
      {
         // Set flag to indicate that we want raw axis data
         mAxis |= skUseRawAxisData;
      }
   }

   real32 MapValue( real32 const inputValue ) const
   {
      return mMultiplier * bpe::min_val(mMaxValue, bpe::max_val(mMinValue, inputValue));
   }

   uint32   GetAxis() const      { return (mAxis & ~skUseRawAxisData); }
   bool     IsRawAxis() const    { return ((mAxis & skUseRawAxisData) != 0); }

   static CPadAxisMapping const MakePositiveValueMapping( uint32 const axis, bool const bGetRawAxisData = false )
   {
      return CPadAxisMapping(axis, 0.0f, 1.0f, 1.0f, bGetRawAxisData);
   }

   static CPadAxisMapping const MakeNegativeValueMapping( uint32 const axis, bool const bGetRawAxisData = false )
   {
      return CPadAxisMapping(axis, -1.0f, 0.0f, -1.0f, bGetRawAxisData);
   }

   static CPadAxisMapping const MakeFullRangeValueMapping( uint32 const axis, bool const bGetRawAxisData = false )
   {
      return CPadAxisMapping(axis, gkReal32MaxNeg, gkReal32Max, 1.0f, bGetRawAxisData);
   }

   static CPadAxisMapping const MakeFullRangeValueMappingInverted( uint32 const axis, bool const bGetRawAxisData = false )
   {
      // Internal analog Y axis stick inverted, compensate.
      return CPadAxisMapping(axis, gkReal32MaxNeg, gkReal32Max, -1.0f, bGetRawAxisData);
   }

public:
   real32   mMinValue;
   real32   mMaxValue;
   real32   mMultiplier;

private:
   static uint32 const  skUseRawAxisData        = 0x80000000;  // No dead zone or other internal modifications will be applied to
                                                    // the returned data, though it will still use applied range.
   uint32   mAxis;
};

//----------------------------------------------------------------------------

// State of controller buttons.
// We use bitflags to allow the Pressing state to be a combination of Pressed and Held.
enum EButtonState
{
   kBS_NotPressed    = 0,
   kBS_Pressed       = 1 << 1,
   kBS_Held          = 1 << 2,
   kBS_Pressing      = kBS_Pressed | kBS_Held,
   kBS_Released      = 1 << 3,
   kBS_Terminator
};
   
//----------------------------------------------------------------------------

class CPadButtonMapping
{
public:

   explicit CPadButtonMapping( int const button, EButtonState const buttonState = kBS_Pressing, real32 const value = 1.0f )
   :  mButton( button )
   ,  mValue( value )
   ,  mButtonState( buttonState )
   {
   }

   int            mButton;
   real32         mValue;
   EButtonState   mButtonState;
};

//----------------------------------------------------------------------------

class CKeyMapping
{
public:
   explicit CKeyMapping( uint8 const keyCode, EButtonState const buttonState = kBS_Pressing, real32 const value = 1.0f )
   :  mKeyCode( keyCode )
   ,  mValue( value )
   ,  mButtonState( buttonState )
   {
   }

   uint8          mKeyCode;
   real32         mValue;
   EButtonState   mButtonState;
};


//----------------------------------------------------------------------------
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector2.h"

class CWiimoteState
{
public:
   struct SIRSensorReading
   {
      static uint32 const skNotVisible = 0xFFFFFFFF;

      SIRSensorReading()
         : mRawX(0)
         , mRawY(0)
         , mRawSize(skNotVisible)
         , mCalibratedPos(CVector2::Zero())
         , mCalibratedSize(0.0f)
      {
      };

      bool IsVisible() const
      {
         bool const bVisible = (mRawSize != skNotVisible);
         return bVisible;
      };

      void SetCalibratedValues()
      {
         if (IsVisible())
         {
            mCalibratedSize = mRawSize / 15.0f;

            mCalibratedPos.mX = mRawX /  1023.0f;            
            mCalibratedPos.mY = mRawY /  767.0f;
            // Map pos into -1 to 1 range
            mCalibratedPos *= 2.0f;
            mCalibratedPos -= CVector2::One();
         }
      }

      uint32         mRawX;
      uint32         mRawY;
      uint32         mRawSize;
      CVector2       mCalibratedPos;
      real32         mCalibratedSize;
   };

   CWiimoteState()
      : mButtonState(0)
      , mAcceleration_Raw(CVector3::Zero())
      , mAcceleration_Calibrated(CVector3::Zero())
      , mIRSensorAverage(CVector2::Zero())
      , mButtonState_Nunchuck(0)
      , mAxisX_Nunchuck(0)
      , mAxisY_Nunchuck(0)
      , mAcceleration_Raw_Nunchuck(CVector3::Zero())
      , mAcceleration_Calibrated_Nunchuck(CVector3::Zero())
   {
   };

   ~CWiimoteState()
   {
   };

   // Main controller
   uint16                              mButtonState;
   CVector3                            mAcceleration_Raw;
   CVector3                            mAcceleration_Calibrated;
   SIRSensorReading                    mIRSensor[4];
   CVector2                            mIRSensorAverage;

   // Nunchuck
   uint16                              mButtonState_Nunchuck;
   int16                               mAxisX_Nunchuck;
   int16                               mAxisY_Nunchuck;
   CVector3                            mAcceleration_Raw_Nunchuck;
   CVector3                            mAcceleration_Calibrated_Nunchuck;
};

//----------------------------------------------------------------------------

class ENGINE_API CBaseInputGenerator
{
public:
   enum EControllerType
   {
      kCT_None,
      kCT_PS3,
      kCT_Xbox360,
      kCT_Wii
   };

   static int const kMaxControllers = 4;

public:
   virtual ~CBaseInputGenerator() {}
   
   EControllerType GetControllerType() const { return mControllerType; }

   // returns true if there is input for the given controller
   virtual bool             HasInput(int const controllerNumber = 0) const = 0;
   // retrieves input for the given controller
   virtual CUserInput const GetInput(int const controllerNumber = 0) = 0;
   // Actives rumble based on recently applied settings
   virtual void             ApplyRumbleSettings() = 0;
   // Returns 'n' readings from internal wiimote buffer. Appends state readings to 'data'. Controller is sampled at 100hz.   
   virtual void             GetWiimoteInput(int const controllerNumber, int const numReadings, std::vector<CWiimoteState> &data) const;

   // action mapping interface
   void AddMapping( CUserInput::EAction const action, CKeyMapping const & keyMapping );
   void AddMapping( CUserInput::EAction const action, CPadButtonMapping const & padButtonMapping );
   void AddMapping( CUserInput::EAction const action, CPadAxisMapping const & padAxisMapping );

   // event interface
   void AddEvent( CInputEvent const & event );

   bool const HasEvents() const { return !mKeyboardEvents.empty(); }
   CInputEvent const PopEvent();
   void SetTippingActive(int const controllerNumber, bool const bActive);
   void SetTippingActive(bool const bActive);

   void ResetRumble();     // Sets rumble setting to zero, call at start of frame

   // Intensity 0-1. More recent triggers override older ones if higher priority.
   // Recommend using AddRumble instead unless this is a special case, as rumble can be considered similar to sound which is additive.
   void SetRumble(int const controllerNumber, real32 const lowFreqIntensity, real32 const highFreqIntensity, int const priority, uint32 const triggerUpdateCount);
   // Adds the to the current rumble intensity.
   void AddRumble(int const controllerNumber, real32 const lowFreqIntensity, real32 const highFreqIntensity);
   
   // Implemented in platform specific file
   static CInputEvent::EKey       ConvertVirtualKey(int const inKey);

   // Factory for getting platform specific implementation
   static CBaseInputGenerator * Factory();

protected:
   explicit CBaseInputGenerator();
   
protected:
   typedef std::vector< boost::optional<CKeyMapping> > TKeyMappings;
   typedef std::multimap<CUserInput::EAction,CPadButtonMapping> TPadButtonMap;
   typedef std::multimap<CUserInput::EAction,CPadAxisMapping> TPadAxisMap;
   
protected:
   EControllerType            mControllerType;
   TKeyMappings               mKeyMappings;
   TPadButtonMap              mPadButtonMappings;
   TPadAxisMap                mPadAxisMappings;
   std::vector<CInputEvent>   mKeyboardEvents;
   // When true, tipping is disabled
   // Allows finer controller over triggering
   bool                       mDisableTipping[kMaxControllers];

   // Per controller rumble settings.
   struct SRumbleSettings
   {        
      real32         mLowFreqIntensity;
      real32         mHighFreqIntensity;
      int            mPriority;
      uint32         mTriggerUpdateCount;
   };

   bpe::reserved_vector<SRumbleSettings, kMaxControllers>   mRumbleSettings;
};

//----------------------------------------------------------------------------

extern ENGINE_API CBaseInputGenerator * gpInputGenerator;

//----------------------------------------------------------------------------

BPE_FORCEINLINE CBaseInputGenerator * InputGenerator()
{
   return gpInputGenerator;
}

//----------------------------------------------------------------------------

