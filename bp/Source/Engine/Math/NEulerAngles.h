//----------------------------------------------------------------------------
// NEulerAngles.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//----------------------------------------------------------------------------

#include "CVector3.h"
#include "CMatrix3.h"
#include "CMatrix34.h"
#include "CMatrix4.h"

//----------------------------------------------------------------------------

class ENGINE_API NEulerAngles
{
public:
   static CVector3 FromMatrix3(CMatrix3 const & matrix);
   static CVector3 FromMatrix34(CMatrix34 const & matrix);
   static CVector3 FromMatrix4(CMatrix4 const & matrix);

   static CMatrix3   ToMatrix3(CVector3 const & eulerAngles);
   static CMatrix34  ToMatrix34(CVector3 const & eulerAngles);
   static CMatrix4   ToMatrix4(CVector3 const & eulerAngles);
};
