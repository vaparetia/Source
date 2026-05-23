//------------------------------------------------------------------------------------------
// CVertexArray.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CInputStream;

//------------------------------------------------------------------------------------------

class ENGINE_API CVertexArray
{
public:
   enum EFlags
   {
      kFlags_IsOffset      = (1 << 1),       // If this flag is set, then the data is an offset that needs to be resolved to a ptr.
      kFlags_ManagedMemory = (1 << 2)        // If this flag is set, then the data ptr is managed memory and needs to be freed when the vertex array is destroyed.
   };

   struct ENGINE_API Stream
   {
      // Get number of elements in this stream
      uint32 GetElementCount() const { return mSize / GetElementSize(); }
      
      // Get byte size of one element of the stream
      uint32 GetElementSize() const;
      
      // Get ptr to data.
      void* GetDataPtr() const { return (void*)mData; }

      uint32   mId;     // FourCC of this stream id
      uint16   mType;   // One of EVertexDataType (kVDT_)
      uint16   mFlags;  // Combination of EFlags
      uint32   mData;   // Offset/Ptr to data.
      uint32   mSize;   // Size in bytes of data
   };

public:
   ~CVertexArray();

   static CVertexArray* CreateVertexArray(uint32 const streamCount);
   void AddStream(uint32 const id, uint16 const type, uint16 const flags, void* pData, uint32 const size);

   Stream const * GetStream(uint32 const id) const;
   Stream const * GetStreamByIndex(int const index) const { return mStreams + index; }

   uint32 GetStreamCount() const { return mStreamCount; }

   static CVertexArray * Load(CInputStream & stream);

   static void operator delete (void* pData);

private:
   CVertexArray() : mStreamCount(0) {}

private:
   uint32   mStreamCount;
   Stream   mStreams[1];

private:
   BPE_DISABLE_COPY_AND_ASSIGNMENT(CVertexArray);
};
