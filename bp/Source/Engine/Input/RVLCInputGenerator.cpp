//----------------------------------------------------------------------------
// RVLCInputGenerator.cpp
// Copyright 2009
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include <math.h>
#include "Engine/Math/MathUtils.h"
#include BPE_PLATFORM_SPECIFIC( CInputGenerator.h )
#include "Engine/System/COsContext.h"

#include <revolution/wpad.h>

//----------------------------------------------------------------------------

namespace
{
   CInputGenerator *sInputGenerator = NULL;
}

//----------------------------------------------------------------------------

CInputGenerator::CInputGenerator()
: CBaseInputGenerator()
, mConnectedRemotes( 0 )
{
   BPE_VERIFY( sInputGenerator == NULL, false, "Cannot have multiple input generators" );

   sInputGenerator = this;

   for ( int channel = WPAD_CHAN0; channel <= WPAD_CHAN3; ++channel )
   {
      KPADSetConnectCallback( channel, &StaticControllerConnect );
      KPADEnableDPD( channel );
   }

   memset( mStatus, 0, sizeof( mStatus ) );

   mControllerType = kCT_Wii;
}

//----------------------------------------------------------------------------

CInputGenerator::~CInputGenerator()
{   
   BPE_VERIFY( sInputGenerator == this, false, "~CInputGenerator called when it's not the last one?" );

   KPADSetConnectCallback( WPAD_CHAN0, NULL );
   KPADSetConnectCallback( WPAD_CHAN1, NULL );
   KPADSetConnectCallback( WPAD_CHAN2, NULL );
   KPADSetConnectCallback( WPAD_CHAN3, NULL );

   sInputGenerator = NULL;
}

//----------------------------------------------------------------------------

CUserInput const CInputGenerator::GetInput(int const controllerNumber /* = 0 */)
{
   KPADStatus currentStatus;

   // If we are reading a disconnected remote, then we zero out the current status
   // If we read and get no samples (or error), then use the last samples
   // If we read and get samplers, then use those
   if ( ( mConnectedRemotes & ( 1 << controllerNumber ) ) == 0 )
   {
      memset( &currentStatus, 0, sizeof( KPADStatus ) );
   }
   else
   {
      s32 storedSetCount = KPADRead( controllerNumber, &currentStatus, 1 );

      if ( storedSetCount == 0 || currentStatus.wpad_err != WPAD_ERR_NONE )
      {
         memcpy( &currentStatus, &mStatus[ controllerNumber ], sizeof( currentStatus ) );
      }
   }

   // For each button, set a bit if we need to fire a "pressed" or "released" event
   uint32 pressStateChanged = currentStatus.hold ^ mStatus[ controllerNumber ].hold;
   
   u32 controllerType = WPAD_DEV_CORE;
   bool hasDPD = false;
   bool hasAcc = false;
   bool isFreestyle = false;
   if ( currentStatus.wpad_err == WPAD_ERR_NONE )
   {
      switch ( currentStatus.data_format )
      {
      case WPAD_FMT_CORE_ACC_DPD:
      case WPAD_FMT_FREESTYLE_ACC_DPD:
      case WPAD_FMT_CLASSIC_ACC_DPD:
         hasDPD = ( currentStatus.dpd_valid_fg > 0 );
         break;
      }

      switch ( currentStatus.data_format )
      {
      case WPAD_FMT_CORE_ACC:
      case WPAD_FMT_CORE_ACC_DPD:
      case WPAD_FMT_FREESTYLE_ACC:
      case WPAD_FMT_FREESTYLE_ACC_DPD:
      case WPAD_FMT_CLASSIC_ACC:
      case WPAD_FMT_CLASSIC_ACC_DPD:
         hasAcc = true;
         break;
      }

      switch ( currentStatus.data_format )
      {
      case WPAD_FMT_FREESTYLE:
      case WPAD_FMT_FREESTYLE_ACC:
      case WPAD_FMT_FREESTYLE_ACC_DPD:
         isFreestyle = ( currentStatus.dev_type == WPAD_DEV_FREESTYLE );
         break;
      }
   }

   CUserInput input;

   for( TPadButtonMap::const_iterator it = mPadButtonMappings.begin(); it != mPadButtonMappings.end(); ++it )
   {  
      CUserInput::EAction const action = it->first;
      CPadButtonMapping const & mapping = it->second;
      int const button = mapping.mButton;
      uint32 myButtonState = 0;
      bool held = ( currentStatus.hold & ( 1 << button ) ) != 0;
      bool toggled = ( pressStateChanged & ( 1 << button ) ) != 0;

      if ( held )
      {
         myButtonState |= kBS_Held;
         if ( toggled )
         {
            myButtonState |= kBS_Pressed;
         }
      }
      else if ( toggled )
      {
         myButtonState = kBS_Released;
      }

      if ( myButtonState & mapping.mButtonState )
      {
         input.SetValue( action, mapping.mValue );
      }
   }

   for ( TPadAxisMap::const_iterator it = mPadAxisMappings.begin(); it != mPadAxisMappings.end(); ++it )
   {
      CUserInput::EAction const action = it->first;
      CPadAxisMapping const &mapping = it->second;

      real32 axisValue = 0.f;
      bool valueValid = false;

      switch ( mapping.GetAxis() )
      {
      case kA_Wiimote_AccX:
         if ( hasAcc )
         {
            axisValue = currentStatus.acc.x;
            valueValid = true;
         }
         break;
      case kA_Wiimote_AccY:
         if ( hasAcc )
         {
            axisValue = currentStatus.acc.y;
            valueValid = true;
         }
         break;
      case kA_Wiimote_AccZ:
         if ( hasAcc )
         {
            axisValue = currentStatus.acc.z;
            valueValid = true;
         }
         break;
      case kA_Wiimote_IRSensorX:
         if ( hasDPD )
         {
            axisValue = currentStatus.pos.x;
            valueValid = true;
         }
         break;
      case kA_Wiimote_IRSensorY:
         if ( hasDPD )
         {
#pragma BPE_TODOMSG( "Fix for RVL - Negate y" )

            axisValue = -currentStatus.pos.y;
            valueValid = true;
         }
         break;
      case kA_Wiimote_AccX_Nunchuck:
         if ( isFreestyle && hasAcc )
         {
            axisValue = currentStatus.ex_status.fs.acc.x;
            valueValid = true;
         }
         break;
      case kA_Wiimote_AccY_Nunchuck:
         if ( isFreestyle && hasAcc )
         {
            axisValue = currentStatus.ex_status.fs.acc.y;
            valueValid = true;
         }
         break;
      case kA_Wiimote_AccZ_Nunchuck:
         if ( isFreestyle && hasAcc )
         {
            axisValue = currentStatus.ex_status.fs.acc.z;
            valueValid = true;
         }
         break;

      case kA_Wiimote_Nunchuck_StickX:
         if ( isFreestyle )
         {
            axisValue = currentStatus.ex_status.fs.stick.x;
            valueValid = true;
         }
         break;
      case kA_Wiimote_Nunchuck_StickY:
         if ( isFreestyle )
         {
            axisValue = currentStatus.ex_status.fs.stick.y;
            valueValid = true;
         }
         break;
      }

      if ( valueValid )
      {
         input.SetValue( action, mapping.MapValue( axisValue ) );
      }
   }

   memcpy( &mStatus[ controllerNumber ], &currentStatus, sizeof( currentStatus ) );

   return input;
}

//----------------------------------------------------------------------------

bool CInputGenerator::HasInput(int const controllerNumber) const
{
   if ( mConnectedRemotes & ( 1 << controllerNumber ) )
   {
      return true;
   }

   return false;
}

//----------------------------------------------------------------------------

void CInputGenerator::ApplyRumbleSettings()
{
}

//----------------------------------------------------------------------------

CBaseInputGenerator * CBaseInputGenerator::Factory()
{
   return new CInputGenerator();
}

//----------------------------------------------------------------------------

void CInputGenerator::StaticControllerConnect( s32 chan, s32 reason )
{
   sInputGenerator->ControllerConnect( chan, reason );
}

//----------------------------------------------------------------------------

void CInputGenerator::ControllerConnect( s32 chan, s32 reason )
{
   switch ( reason )
   {
   case WPAD_ERR_NO_CONTROLLER:
      mConnectedRemotes &= ~( 1 << chan );
      break;
   case WPAD_ERR_NONE:
      mConnectedRemotes |= ( 1 << chan );
      break;
   default:
      // Ignore any other reasons
      break;
   }
}