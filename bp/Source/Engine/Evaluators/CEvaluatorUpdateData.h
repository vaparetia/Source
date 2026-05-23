//------------------------------------------------------------------------------------------
// CEvaluatorUpdateData.h
// Data that's passed to evaluators to allow update
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Evaluators/CEvaluatorAllocator.h"

//------------------------------------------------------------------------------------------

class CRandom;
class CBaseGameManager;
class CGameObjectComponent;

//------------------------------------------------------------------------------------------

class ENGINE_API CEvaluatorUpdateData
{
public:
   // Caller owns all pointers
   CEvaluatorUpdateData(CRandom *pRandom,
                        CEvaluatorDataBlock const & dataBlock,
                        real64 const universeTime,
                        real64 const instanceTime,
                        real64 const localTime,
                        CBaseGameManager const * const pManager = NULL,
                        CGameObjectComponent const * const pComponent = NULL)
   : mpRandom(pRandom)
   , mDataBlock(dataBlock)
   , mUniverseTime64(universeTime)
   , mUniverseTime((real32)universeTime)
   , mInstanceTime64(instanceTime)
   , mInstanceTime((real32)instanceTime)
   , mLocalTime64(localTime)
   , mLocalTime((real32)localTime)
   , mpManager(pManager)
   , mpComponent(pComponent)
   {
   };

   enum EUninitialized { kConstructUninitialized };
   CEvaluatorUpdateData(EUninitialized)
      : mDataBlock(NULL)
   {
   };

   void SetTime(  real64 const universeTime,
                  real64 const instanceTime,
                  real64 const localTime)
   {
      mUniverseTime64 = universeTime;
      mUniverseTime = (real32)universeTime;
      mInstanceTime64 = instanceTime;
      mInstanceTime = (real32)instanceTime;
      mLocalTime64 = localTime;
      mLocalTime = (real32)localTime;
   };

   void SetTime_Local(real64 const localTime)
   {
      mLocalTime64 = localTime;
      mLocalTime = (real32)localTime;
   };

   CRandom *                     mpRandom;
   CEvaluatorDataBlock           mDataBlock;
   real64                        mUniverseTime64;
   real32                        mUniverseTime;
   real64                        mInstanceTime64;
   real32                        mInstanceTime;
   real64                        mLocalTime64;
   real32                        mLocalTime;
   CBaseGameManager const *      mpManager;
   CGameObjectComponent const *  mpComponent;
};

//------------------------------------------------------------------------------------------
// Very similar to above, except that the memory associated with the data block will be deleted on destruction.
// Intended for use with transient message objects with evaluators.
class ENGINE_API CManagedEvaluatorUpdateData : public CEvaluatorUpdateData
{
public:
   CManagedEvaluatorUpdateData(CEvaluatorUpdateData &updateData)
      : CEvaluatorUpdateData(updateData)
      , mpData((uint8*)mDataBlock.MemPtr())
   {
   };

private:
   boost::shared_ptr<uint8>    mpData;
};

//------------------------------------------------------------------------------------------

