//------------------------------------------------------------------------------------------
// CGameObjectApplyProperties.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class IGameObjectFactory;
class IEvaluatorFactory;
class CInputStream;

//------------------------------------------------------------------------------------------

class ENGINE_API CGameObjectApplyPropertiesData
{
public:
   enum EFlags
   {
      kNone          = 0,
      kUpdateOnly    = 1 << 0,

      kTerminator    = 0xFFFFFFFF
   };

   CGameObjectApplyPropertiesData(CInputStream &stream, IEvaluatorFactory const &factory, uint32 const flags = kNone)
      : mStream(stream)
      , mFactory(factory)
      , mFlags(flags)
   {
   };

   bool  UpdateOnly() const { return (mFlags & kUpdateOnly) != 0; };

   CInputStream &             mStream;
   IEvaluatorFactory const &  mFactory;
   uint32                     mFlags;

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CGameObjectApplyPropertiesData);
};

//------------------------------------------------------------------------------------------

