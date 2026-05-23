//----------------------------------------------------------------------------
// PS3CInputGenerator.h
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/Input/CInputGenerator.h"
#include "boost/scoped_array.hpp"

#include <revolution/kpad.h>

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
   static void StaticControllerConnect( s32 chan, s32 reason );

   void ControllerConnect( s32 chan, s32 reason );

   uint32 mConnectedRemotes;
   KPADStatus mStatus[4];
};

//----------------------------------------------------------------------------

