//----------------------------------------------------------------------------
// CIndexBuffer.h
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

//----------------------------------------------------------------------------

enum EIndexType
{
   kIT_Uint32,
   kIT_Uint16,
   kIT_Invalid = -1
};

inline int const GetSizeForIndexType(EIndexType type)
{
   switch(type)
   {
   case kIT_Uint16:
      return 2;
   case kIT_Uint32:
      return 4;
   default:
      return 0;
   }
}

//----------------------------------------------------------------------------

#include BPE_PLATFORM_PATH2(Renderer/Base/Primitive, CIndexBuffer.h)

