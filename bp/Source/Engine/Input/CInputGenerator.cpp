//----------------------------------------------------------------------------
// CInputGenerator.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CInputGenerator.h"
#include "Engine/Math/MathUtils.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

CBaseInputGenerator * gpInputGenerator = NULL;

//----------------------------------------------------------------------------

CInputEvent::CInputEvent( EType const type,
                          int const data )
:  mType( type )
,  mData( data )
{
}

//----------------------------------------------------------------------------

CUserInput::CUserInput()
{
   for( int i = 0; i < kAction_Count; ++i )
   {
      mValues[i] = 0.0f;
   }

   for( int i = 0; i < 256; ++i )
   {
      mKeyDown[i] = false;
   }
}

//----------------------------------------------------------------------------

CUserInput const & CUserInput::NullInput()
{
   static const CUserInput skNullUserInput;
   return skNullUserInput;
}

//----------------------------------------------------------------------------

CBaseInputGenerator::CBaseInputGenerator()
:  mControllerType( kCT_None )
{
   BPE_VERIFY(gpInputGenerator == NULL, false, "only one instance of input generator supported");
   gpInputGenerator = this;
   
   mKeyMappings.resize( CUserInput::kAction_Count, boost::optional<CKeyMapping>() );
   mRumbleSettings.resize(kMaxControllers, SRumbleSettings());

   SetTippingActive(true);
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::AddEvent( CInputEvent const & event )
{
   mKeyboardEvents.push_back( event );
}

//----------------------------------------------------------------------------
   
CInputEvent const CBaseInputGenerator::PopEvent()
{
   CInputEvent event = mKeyboardEvents[0];
   mKeyboardEvents.erase( mKeyboardEvents.begin() );

   return event;
}


//----------------------------------------------------------------------------

void CBaseInputGenerator::GetWiimoteInput(int const controllerNumber, int const numReadings, std::vector<CWiimoteState> &data) const
{
   // Default implementation just writes empty data. Platforms that support wiimote will override this.
   for (int loop = 0; loop < numReadings; loop++)
   {
      data.push_back(CWiimoteState());
   }
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::AddMapping( CUserInput::EAction const action, CPadButtonMapping const & padButtonMapping )
{ 
   mPadButtonMappings.insert( std::make_pair(action, padButtonMapping) );
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::AddMapping( CUserInput::EAction const action, CPadAxisMapping const & padAxisMapping )
{
   mPadAxisMappings.insert( std::make_pair(action, padAxisMapping) );
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::AddMapping( CUserInput::EAction const action, CKeyMapping const & keyMapping )
{
   mKeyMappings[action] = keyMapping;
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::SetTippingActive(int const controllerNumber, bool const bActive)
{ 
   mDisableTipping[controllerNumber] = !bActive;
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::SetTippingActive(bool const bActive)
{
   // Reset all controllers
   for (int loop = 0; loop < kMaxControllers; loop++)
   {
      SetTippingActive(loop, bActive);
   }
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::ResetRumble()
{
   foreach(SRumbleSettings &rumble, mRumbleSettings)   
   {
      rumble.mLowFreqIntensity = 0;
      rumble.mHighFreqIntensity = 0;
      rumble.mPriority = gkInt32Min;
      rumble.mTriggerUpdateCount = 0;
   }
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::SetRumble(int const controllerNumber, real32 const lowFreqIntensity, real32 const highFreqIntensity, int const priority, uint32 const triggerUpdateCount)
{
   SRumbleSettings &rumble = mRumbleSettings[controllerNumber];
   if (priority <= rumble.mPriority)
   {
      // priority too low
      return;
   }

   if ((priority == rumble.mPriority) && (triggerUpdateCount < rumble.mTriggerUpdateCount) )
   {
      // Older
      return;
   }

   rumble.mHighFreqIntensity = MathUtils::ClampMinMax(highFreqIntensity, 0, 1);
   rumble.mLowFreqIntensity = MathUtils::ClampMinMax(lowFreqIntensity, 0, 1);
   rumble.mPriority = priority;
   rumble.mTriggerUpdateCount = triggerUpdateCount;
}

//----------------------------------------------------------------------------

void CBaseInputGenerator::AddRumble(int const controllerNumber, real32 const lowFreqIntensity, real32 const highFreqIntensity)
{
   SRumbleSettings &rumble = mRumbleSettings[controllerNumber];
   if (rumble.mPriority > gkInt32Min)
   {
      // Someone's used SetRumble this frame, so we can't add to this value.
      return;
   }

   rumble.mHighFreqIntensity += MathUtils::ClampMinMax(highFreqIntensity, 0, 1);
   rumble.mLowFreqIntensity += MathUtils::ClampMinMax(lowFreqIntensity, 0, 1);

   return;
}

//----------------------------------------------------------------------------




