//----------------------------------------------------------------------------
// DCCVertexData.h
// Dreamcast stub — mirrors VTACVertexData layout.
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/Material/CShaderVertexDataBinding.h"

//----------------------------------------------------------------------------

class CVertexDataPrivate;

class RENDERER_API CVertexData
{
   friend class CVertexDataPrivate;
public:
   struct SVertexAttribute
   {
      SVertexAttribute()
         :  mBufferIndex(-1)
         ,  mOffset(0)
         ,  mType(kVDT_Invalid)
      {
      }

      size_t   mBufferIndex;
      uint32   mOffset;
      uint32   mType;
   };

   struct SBuffer
   {
      SBuffer()
         : mpBuffer( NULL )
         , mStride( 0 )
         , mInstanced( 0 )
      {
      }

      SBuffer( void *pBuffer, uint32 stride ) : mpBuffer( pBuffer ), mStride( stride ), mInstanced( 0 ) {}

      void *mpBuffer;
      uint32 mStride;
      uint32 mInstanced;
   };

   typedef bpe::reserved_vector<SVertexAttribute, kVDS_Count> TAttributes;
   typedef bpe::reserved_vector<SBuffer, 4 > TBuffers;
public:
   CVertexData();

   void ClearCachedVertexDeclaration() {}

   void ClearVertexBuffers() { mBuffers.clear(); }
   size_t AddBuffer( CVertexBuffer const *pBuffer, uint32 stride );
   size_t AddBuffer( CDynamicVertexBufferPoolChunk_RT const * pChunk, uint32 stride );
   size_t AddBuffer( CDynamicVertexBufferPoolChunk_UT const * pChunk, uint32 stride );
   size_t AddBuffer_Unsafe( void *pBuffer, uint32 stride );
   void   ChangeBufferAddress_Unsafe( size_t const bufferIndex, void *pBuffer ) { mBuffers[ bufferIndex ].mpBuffer = pBuffer; }

   void SetAttribute(EVertexDataStream const vertexStream, uint8 const offset, EVertexDataType const type, size_t const bufferIndex );
   void SetAttributeBufferIndex( EVertexDataStream const vertexStream, size_t const bufferIndex );

   uint32 const GetOffset(EVertexDataStream const vertexStream) const { return mAttributes[vertexStream].mOffset; }

   uint32 const GetStrideByBufferIndex( size_t const index ) const { return mBuffers[ index ].mStride; }
   void const *GetBufferPtrByBufferIndex( size_t const index ) const { return mBuffers[ index ].mpBuffer; }
   size_t const GetBufferCount() const { return mBuffers.size(); }
   uint32 const GetInstancedByBufferIndex( size_t const index ) const { return mBuffers[ index ].mInstanced; }

   bool const HasAttribute( EVertexDataStream const vertexStream ) const { return ( mValidStreamsBitMask & ( 1 << vertexStream ) ) != 0; }
   SVertexAttribute const &GetAttribute( EVertexDataStream const index ) const { return mAttributes[ index ]; }

   void SetInstanced(uint32 const isInstanced, size_t const bufferIndex) { mBuffers[ bufferIndex ].mInstanced = isInstanced; }

private:
   TAttributes mAttributes;
   TBuffers    mBuffers;
   uint32      mValidStreamsBitMask;
};

//----------------------------------------------------------------------------
