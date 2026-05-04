//------------------------------------------------------------------------------------------
// IEvaluator.h
// Interface classes for evaluators
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CVector3.h"

//------------------------------------------------------------------------------------------

class CEvaluatorUpdateData;
class CGameObjectApplyPropertiesData;
class CEvaluatorAllocator;
class CColorf;
class CLinearCopyStream;
class IEvaluatorTypeToVTable;

//------------------------------------------------------------------------------------------

class ENGINE_API IEvaluator
{
public:
   virtual ~IEvaluator() {};
   virtual void   ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator) = 0;
   virtual void   PostLoadUpdate() {};    // We'll make this pure virtual later when we've updated the code gen for all evaluators.
   virtual uint32 GetType() const = 0;
   // Returns buffer 'ptr' to new copy.
   virtual uint32 CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const { return 0; };

   // Helper function used to update existing evaluator properties, returns true if properties were updated
   static bool    UpdateProperties( IEvaluator * pEvaluator,
                                    CGameObjectApplyPropertiesData &applyProperties, 
                                    uint16 const size,
                                    CEvaluatorAllocator &allocator);
   // Poor mans RTTI.
   enum EEvaluatorClassTypes
   {
      kECT_Float,
      kECT_Bool,
      kECT_Vector,
      kECT_Color,
      kECT_Modifier,
      kECT_Emitter,
      kECT_Custom,
      kECT_UserData,

      kECT_Count,
      kECT_Invalid
   };
   
   struct ENGINE_API STypeInfo
   {
      STypeInfo();
      ~STypeInfo();
      
      real32                  mTimeRangeStart;
      real32                  mTimeRangeEnd;
      EEvaluatorClassTypes    mClassType;
      bool                    mIsConst : 1;
   };
   
   // Function returns evaluator specific information that can be useful at runtime.
   // Examples include getting time range of a spline, optimizations for constant evaluators etc.
   virtual void   GetInfo(STypeInfo &typeInfo) const = 0;

   // This is last so that the vtable entry goes next to the 'GetValue' function.
   virtual void   Initialize(CEvaluatorUpdateData &updateData) const = 0;
};

//------------------------------------------------------------------------------------------

class ENGINE_API IFloatEvaluator : public IEvaluator
{
public:
   virtual ~IFloatEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;      
   
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const = 0;
};

//------------------------------------------------------------------------------------------

class ENGINE_API IBoolEvaluator : public IEvaluator
{
public:
   virtual ~IBoolEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;   
   
   virtual bool GetValue(CEvaluatorUpdateData const &updateData) const = 0;
};

//------------------------------------------------------------------------------------------

class ENGINE_API IVectorEvaluator : public IEvaluator
{
public:
   virtual ~IVectorEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;         
   
   virtual CVector3 GetValue(CEvaluatorUpdateData const &updateData) const = 0;
};

//------------------------------------------------------------------------------------------

class ENGINE_API IColorEvaluator : public IEvaluator
{
public:
   virtual ~IColorEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;         
   
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const = 0;
};

//------------------------------------------------------------------------------------------
class IModifierTemporaryAllocator;

class ENGINE_API IModifierPVEvaluator : public IEvaluator
{
public:
   virtual ~IModifierPVEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;         

   enum EModifierFlags  // Returned by ApplyModifier
   {
      kMF_None,
      kMF_DeleteParticle,

      kMF_Count
   };

   virtual uint32 ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const = 0;
   // Call before calling 'ApplyModifier', returns offset into it's temp data block.
   virtual uint32 InitializeForUpdate(CEvaluatorUpdateData &, IModifierTemporaryAllocator &) const { return 0; };
};

//------------------------------------------------------------------------------------------

class ENGINE_API IEmitterPVEvaluator : public IEvaluator
{
public:
   virtual ~IEmitterPVEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;         
   
   virtual void GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const = 0;
};


//------------------------------------------------------------------------------------------

class ENGINE_API ICustomEvaluator : public IEvaluator
{
public:
   virtual ~ICustomEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;   
};

//------------------------------------------------------------------------------------------

class ENGINE_API IUserDataEvaluator : public IEvaluator
{
public:
   virtual ~IUserDataEvaluator() {};
   virtual void GetInfo(STypeInfo &typeInfo) const;   
   
   enum EDataType
   {
      kDT_Bool,         // Writes 1 byte with 0,1
      kDT_Float,        // Writes 4 bytes with float
      kDT_Int,          // Writes 4 bytes with int
      kDT_String,       // Writes 4 bytes with pointer to string
      kDT_Vector,       // Writes 12 bytes with x, y, z floats
      kDT_Color,        // Writes 16 bytes with r, g, b, a floats
      kDT_Resource,     // Copy constructs CResource into pBufferOut, requires sizeof(CResource)
      kDT_Branch,       // Writes up to 20 bytes, 4 byes with number of entries followed by N entries of 'IUserDataEvaluator *'
      kDT_Custom,       // Evaluator specific
      
      kDT_Count
   };
   
   virtual char const * GetIdentifier() const = 0;
   virtual EDataType    GetUserDataType() const = 0;
   // Function will modify data pointed at pOutData, see notes EData
   virtual void         GetUserData(CEvaluatorUpdateData const &updateData, void * pOutBuffer) const = 0;
};

//------------------------------------------------------------------------------------------
// Support classes
//------------------------------------------------------------------------------------------
// Allocated memory is used as a temporary cache for precalculated data.
// Any allocated memory can be released after we are done with using the modifier for this batch of particles.
class ENGINE_API IModifierTemporaryAllocator
{
public:
   virtual ~IModifierTemporaryAllocator() {};
   // Returns an offset into the temp buffer
   virtual int32  Allocate(int32 const size) = 0;
   virtual void * GetPtr(int32 const offset) = 0;
};

//------------------------------------------------------------------------------------------

class ENGINE_API CSimpleModifierTemporaryAllocator : public IModifierTemporaryAllocator
{
public:
   CSimpleModifierTemporaryAllocator(void * pMem, int32 const size, bool const bClassOwnsMemory = false)
      : mMemRemaining(size)
      , mpCurrentMemPtr((uint8*)pMem)
      , mpOriginalMemPtr((uint8*)pMem)
      , mpDeleteMemPtr((uint8*)(bClassOwnsMemory ? pMem : NULL))
   {
   };

   ~CSimpleModifierTemporaryAllocator();

   virtual int32     Allocate(int32 const size);
   virtual void *    GetPtr(int32 const offset) { return (void*)(mpOriginalMemPtr + offset); }

   int32       mMemRemaining;
   uint8 *     mpCurrentMemPtr;
   uint8 *     mpOriginalMemPtr;
   uint8 *     mpDeleteMemPtr;
};
 
//------------------------------------------------------------------------------------------

