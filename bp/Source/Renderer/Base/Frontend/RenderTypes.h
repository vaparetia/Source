//------------------------------------------------------------------------------------------
// RenderTypes.h
// Bluepoint
// Copyright 2005
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"
#include "Renderer/Base/Frontend/CLightState.h"

//----------------------------------------------------------------------------

class CDrawableRenderEntity;
class CDrawableMasterRenderEntity;
class CLightRenderEntity;
class CRenderBounds;
class CRenderObject;
struct SGOWMaterial;

//------------------------------------------------------------------------------------------

struct SRect
{
   int x1;
   int y1;
   int x2;
   int y2;

   SRect()
   {
   }

   SRect(int _x1, int _y1, int _x2, int _y2)
      : x1(_x1)
      , y1(_y1)
      , x2(_x2)
      , y2(_y2)
   {
   }

   void MultiplyInto(SRect const & src, float mul)
   {
      x1 = (int)(src.x1*mul);
      y1 = (int)(src.y1*mul);
      x2 = (int)(src.x2*mul);
      y2 = (int)(src.y2*mul);
   }
};

struct SRectf
{
   float x1;
   float y1;
   float x2;
   float y2;
};

//------------------------------------------------------------------------------------------

class TRenderHandle
{
public:
   TRenderHandle(uint16 const objectCount, uint16 const id)
      :  mObjectCount(objectCount)
      ,  mId(id)
   {
   }

   static TRenderHandle & FromUint32(uint32 & data) { return *((TRenderHandle*)&data); }
   uint32 ToUint32() const { return *(uint32*)this; }

   uint16 const GetObjectCount() const { return mObjectCount; }
   uint16 const GetId() const { return mId; }

   bool const operator == (TRenderHandle const & rhs) const
   {
      return mData == rhs.mData;
   }

   bool const operator != (TRenderHandle const & rhs) const
   {
      return mData != rhs.mData;
   }

   bool const operator < (TRenderHandle const & rhs) const
   {
      return mData < rhs.mData;
   }

private:
   union
   {
      struct
      {
         uint16   mObjectCount;
         uint16   mId;
      };
      uint32   mData;
   };
};

BPE_CTASSERT(sizeof(TRenderHandle) == 4);

extern RENDERER_API TRenderHandle const kInvalidRenderHandle;

//----------------------------------------------------------------------------

typedef std::vector<CDrawableRenderEntity*>        TRenderEntities;
typedef std::vector<CDrawableMasterRenderEntity*>  TMasterRenderEntities;

//----------------------------------------------------------------------------

struct SRenderEntry
{
   SRenderEntry()
   {
   }

   SRenderEntry(uint64 const sortKey, SGOWMaterial const * pMaterial, CDrawableRenderEntity* pObject)
      :  mSortKey(sortKey)
      ,  mpMaterial(pMaterial)
      ,  mpObject(pObject)
   {
   }

   uint64                  mSortKey;
   SGOWMaterial const *    mpMaterial;
   CDrawableRenderEntity*  mpObject;
};

//----------------------------------------------------------------------------

struct SRenderList
{
   static int const kRenderListCount = 16 * 1024;

   SRenderEntry & Add()
   {
      BPE_VERIFY(mSize < kRenderListCount, false, "Trying to add too many entries.");
      ++mSize;

      return mRenderEntries[mSize - 1];
   }

   // Returns number of used handles
   // Useful for misc sorting
   uint32   GetCount() const  { return mSize; };

   void Reset()
   {
      mSize = 0;
   }

   SRenderEntry   mRenderEntries[kRenderListCount];
   int            mSize;
};

//----------------------------------------------------------------------------

struct SSortedRenderList
{
   static int const kSortedRenderListCount = 16 * 1024;

   SSortedRenderList()
   : mSize(0)
   {
   }

   SRenderEntry & Add()
   {
      BPE_VERIFY(mSize < kSortedRenderListCount, false, "Trying to add too many entries.");
      ++mSize;

      return mRenderEntries[mSize - 1];
   }
   void Reset()
   {
      mSize = 0;
   }

   SRenderEntry   mRenderEntries[kSortedRenderListCount];
   int            mSize;
};

//----------------------------------------------------------------------------

typedef int TRenderTargetId;
TRenderTargetId const kInvalidRenderTargetId = -1;
