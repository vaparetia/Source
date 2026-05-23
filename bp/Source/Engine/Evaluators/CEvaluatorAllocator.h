//------------------------------------------------------------------------------------------
// CEvaluatorAllocator.h
// Index based allocator plus helper classes for evaluators
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector4.h"
#include "Engine/Graphics/CColorf.h"

//------------------------------------------------------------------------------------------

class CEvaluatorAllocator;

//------------------------------------------------------------------------------------------

class ENGINE_API CEvalFloatData
{
public:
   CEvalFloatData(int32 const allocIndex)
   : mAllocIndex(allocIndex)
   {
   }

   int32    mAllocIndex; 
};

//------------------------------------------------------------------------------------------

class ENGINE_API CEvalVector3Data
{
public:
   CEvalVector3Data(int32 const allocIndex)
   : mAllocIndex(allocIndex)
   {
   }

   int32    mAllocIndex; 
};

//------------------------------------------------------------------------------------------

class ENGINE_API CEvalVector4Data
{
public:
   CEvalVector4Data(int32 const allocIndex)
   : mAllocIndex(allocIndex)
   {
   }

   int32    mAllocIndex; 
};

//------------------------------------------------------------------------------------------

class ENGINE_API CEvalColorfData
{
public:
   CEvalColorfData(int32 const allocIndex)
   : mAllocIndex(allocIndex)
   {
   }

   int32    mAllocIndex; 
};

//------------------------------------------------------------------------------------------

template<class T> class TEvalStruct
{
public:
   TEvalStruct(TEvalStruct const & rhs)
      : mAllocIndex(rhs.mAllocIndex)
   {
   }

   TEvalStruct(int32 const allocIndex)
      : mAllocIndex(allocIndex)
   {
   }

   int32    mAllocIndex; 
};

//------------------------------------------------------------------------------------------

class ENGINE_API CEvaluatorAllocator
{
public:
   CEvaluatorAllocator()
   : mAllocSize(0)
   {
   }

   CEvalFloatData const AllocFloat()      { return CEvalFloatData(Alloc(sizeof(real32)));   };
   CEvalVector3Data const AllocVector3()  { return CEvalVector3Data(Alloc(sizeof(CVector3)));   };
   CEvalVector4Data const AllocVector4()  { return CEvalVector4Data(Alloc(sizeof(CVector4)));   };
   CEvalColorfData const AllocCColorf()   { return CEvalColorfData(Alloc(sizeof(CColorf)));   };

   template< class T > TEvalStruct<T> const AllocStruct(const TType<T>&, int const alignment = 4) { return TEvalStruct<T>(Alloc(sizeof(T), alignment)); }

   int32    Alloc(int32 const numBytes, int32 const alignment = 4)
   {
      // Align start offset to passed in alignment
      int32 const alignMinusOne = alignment - 1;
      mAllocSize = (mAllocSize + alignMinusOne) & (~alignMinusOne);

      int32 const oldAllocSize = mAllocSize;

      // Align size to 4 bytes for now
      int const alignedSize = (numBytes + 3) & (~3);
      mAllocSize += alignedSize;
      
      return oldAllocSize;
   };

   int32    mAllocSize;
};

//------------------------------------------------------------------------------------------

class ENGINE_API CEvaluatorDataBlock
{
public:
   // Caller owns memory
   BPE_FORCEINLINE CEvaluatorDataBlock(void * pMem)
   : mpMem(reinterpret_cast<uint8*>(pMem))
   {
   }

   BPE_FORCEINLINE void SetValue(CEvalFloatData const &floatData, real32 const value)     { *reinterpret_cast<real32*>(mpMem + floatData.mAllocIndex) = value; }
   BPE_FORCEINLINE real32 & Value(CEvalFloatData const &floatData)                        { return *reinterpret_cast<real32*>(mpMem + floatData.mAllocIndex);  }
   BPE_FORCEINLINE real32 GetValue(CEvalFloatData const &floatData) const                 {  return *reinterpret_cast<real32*>(mpMem + floatData.mAllocIndex); }
                                                                                          
   BPE_FORCEINLINE void SetValue(CEvalVector3Data const &data, CVector3 const &value)     { *reinterpret_cast<CVector3*>(mpMem + data.mAllocIndex) = value;  }
   BPE_FORCEINLINE CVector3 & Value(CEvalVector3Data const &data)                         { return *reinterpret_cast<CVector3*>(mpMem + data.mAllocIndex);   }
   BPE_FORCEINLINE CVector3 const & GetValue(CEvalVector3Data const &data) const          { return *reinterpret_cast<CVector3*>(mpMem + data.mAllocIndex);   }
                                                                                          
   BPE_FORCEINLINE void SetValue(CEvalVector4Data const &data, CVector4 const &value)     { *reinterpret_cast<CVector4*>(mpMem + data.mAllocIndex) = value;  }
   BPE_FORCEINLINE CVector4 & Value(CEvalVector4Data const &data)                         { return *reinterpret_cast<CVector4*>(mpMem + data.mAllocIndex);   }
   BPE_FORCEINLINE CVector4 const & GetValue(CEvalVector4Data const &data) const          { return *reinterpret_cast<CVector4*>(mpMem + data.mAllocIndex);   }

   BPE_FORCEINLINE void SetValue(CEvalColorfData const &data, CColorf const &value)   	   { *reinterpret_cast<CColorf*>(mpMem + data.mAllocIndex) = value;  }
   BPE_FORCEINLINE CColorf & Value(CEvalColorfData const &data)                       	   { return *reinterpret_cast<CColorf*>(mpMem + data.mAllocIndex);   }
   BPE_FORCEINLINE CColorf const & GetValue(CEvalColorfData const &data) const        	   { return *reinterpret_cast<CColorf*>(mpMem + data.mAllocIndex);   }

   template <class T> BPE_FORCEINLINE void SetValue(TEvalStruct<T> const &data, T const &value) { *reinterpret_cast<T*>(mpMem + data.mAllocIndex) = value;  }
   template <class T> BPE_FORCEINLINE T & Value(TEvalStruct<T> const &data)                     { return *reinterpret_cast<T*>(mpMem + data.mAllocIndex);   }
   template <class T> BPE_FORCEINLINE T const & GetValue(TEvalStruct<T> const &data) const      { return *reinterpret_cast<T*>(mpMem + data.mAllocIndex);   }

   BPE_FORCEINLINE uint8 * const  GetMemPtr() const                                       { return mpMem; };
   BPE_FORCEINLINE uint8 * const  MemPtr()                                                { return mpMem; };
   BPE_FORCEINLINE void           OffsetMemPtr(int32 const offset)                        { mpMem += offset; };

private:
   uint8 *      mpMem;
};

//------------------------------------------------------------------------------------------

