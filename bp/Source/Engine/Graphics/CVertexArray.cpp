//------------------------------------------------------------------------------------------
// CVertexArray.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CVertexArray.h"

//------------------------------------------------------------------------------------------

#include <vector>
#include <boost/array.hpp>

#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector2.h"
#include "Engine/Math/CHalfFloat.h"

#include "Engine/Graphics/CColor.h"

#include "Renderer/Base/Primitive/EVertexDataType.h"

#include "Engine/Streams/CInputStream.h"

//------------------------------------------------------------------------------------------

namespace
{
   typedef bpe::vector_s< uint32 >   TRGBAColorArray;
   typedef bpe::vector_s< CColor >   TColorArray;
   typedef bpe::vector_s< CVector2 > TVector2Array;
   typedef bpe::vector_s< CVector3 > TVector3Array;
   typedef bpe::vector_s< CVector4 > TVector4Array;

   template<class T> void* ConvertToPlainMemoryAllocation(bpe::vector_s<T> const & input)
   {
      uint32 const size = sizeof(T) * input.size();
      void* pMemory = BPE_MALLOC_ALIGNED(16, size);
      memcpy(pMemory, &input.front(), size);
      
      return pMemory;
   }

   template<class T> void AddStreamFromVector(uint32 const id, EVertexDataType const dataType, bpe::vector_s<T> const & input, CVertexArray * pVertexArray )
   {
      if( !input.empty() )
      {
         void* pData = ConvertToPlainMemoryAllocation(input);
         pVertexArray->AddStream(id, dataType, CVertexArray::kFlags_ManagedMemory, pData, sizeof(T) * input.size());
      }
   }

   void ConvertColorToRGBA(TColorArray const & input, TRGBAColorArray & output)
   {
      output.clear();
      output.reserve(input.size());
      for( int i = 0; i < input.size(); ++i )
      {
         output.push_back(input[i].GetRGBA());
      }
   }

   void ConvertRGBAToVector4(TRGBAColorArray const & input, TVector4Array & output)
   {
      output.clear();
      output.reserve(input.size());

      for( int i = 0; i < input.size(); ++i )
      {
         uint32 const value = input[i];

         CVector4 const vec4( ((value & 0xFF000000) >> 24) / 255.0f,
                              ((value & 0x00FF0000) >> 16) / 255.0f,
                              ((value & 0x0000FF00) >>  8) / 255.0f,
                              ((value & 0x000000FF) >>  0) / 255.0f );

         output.push_back(vec4);
      }
   }

   // NOTE: This code has only been verified as working on Win32
   void Convert_S11S11S10_to_Vector3(uint32* pData, float* pOutput, int const count)
   {
      for( int i = 0; i < count; ++i )
      {
         union
         {
            struct
            {
               int32 x : 11;
               int32 y : 11;
               int32 z : 10;
            } mCompressed;
            
            uint32 mData;
         };

         mData = pData[i];

         int32 const x = mCompressed.x;
         int32 const y = mCompressed.y;
         int32 const z = mCompressed.z;

         float const fx = x / 1023.0f;
         float const fy = y / 1023.0f;
         float const fz = z / 511.0f;

         pOutput[i * 3 + 0] = fx;
         pOutput[i * 3 + 1] = fy;
         pOutput[i * 3 + 2] = fz;
      }
   }

   void Convert_real16_to_real32(uint16* pData, float* pOutput, int const count)
   {
      for( int i = 0; i < count; ++i )
      {
         real16 half(pData[i]);
         pOutput[i] = half.AsR32();
      }
   }

   void Convert_Vector3_To_Vector4(TVector3Array const & input, TVector4Array & output)
   {
      output.clear();
      output.reserve(input.size());

      for( int i = 0; i < input.size(); ++i )
      {
         CVector3 const value = input[i];
         output.push_back(CVector4(value.GetX(), value.GetY(), value.GetZ(), 1));
      }
   }

   void ByteSwapUint32(TColorArray & boneIndices)
   {
      if( !boneIndices.empty() )
      {
         uint32* pData = (uint32*)&boneIndices.front();

         for( int i = 0; i < boneIndices.size(); ++i )
         {
            uint32 const input = pData[i];
            
            pData[i] = (input & 0xFF000000) >> 24 |
                       (input & 0x00FF0000) >> 8 |
                       (input & 0x0000FF00) << 8  |
                       (input & 0x000000FF) << 16;

         }
      }
   }

   void ConvertSkinData(TVector4Array const & weights, TVector4Array & pointsArray)
   {
      for( int i = 0; i < weights.size(); ++i )
      {
         CVector4 const weight = weights[i];
         pointsArray[i].SetW(weight.GetX());
      }
   }

}

//------------------------------------------------------------------------------------------

static const uint32 kVDT_ElementSizes[] =
{
   8,    // 2D float expanded to (value, value, 0, 1)
   12,   // 3D float expanded to (value, value, value, 1)
   16,   // 4D float

   4,    // Each of 4 bytes is normalized by dividing to 255.0
   4,    // 4D unsigned byte

   4,    // Two 16-bit floating point values, expanded to (value, value, 0, 1)
   8,    // Four 16-bit floating point values

   4,    // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
   8,    // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)

   4,    // 3d signed normalized (11:11:10 on PS3, 10:10:10 on D3D) expanded to (value, value, value, 1)
   
   4,    // kVDT_Int32
   
   4,    // 2D signed short expanded to (value, value, 0., 1.)
   8,    // 4D signed short
};

BPE_CTASSERT(BPE_ARRAY_SIZE(kVDT_ElementSizes) == kVDT_Count);

uint32 CVertexArray::Stream::GetElementSize() const
{
   return kVDT_ElementSizes[mType];
}

//------------------------------------------------------------------------------------------

CVertexArray::~CVertexArray()
{
   for( int i = 0; i < mStreamCount; ++i )
   {
      if( mStreams[i].mFlags & kFlags_ManagedMemory )
      {
         BPE_FREE_ALIGNED( mStreams[i].GetDataPtr() );
      }
   }
}

//------------------------------------------------------------------------------------------

void CVertexArray::operator delete (void* pData)
{
   BPE_FREE_ALIGNED(pData);
}

//------------------------------------------------------------------------------------------

CVertexArray* CVertexArray::CreateVertexArray(uint32 const streamCount)
{
   CVertexArray* pVertexArray = new ( BPE_MALLOC_ALIGNED(16, sizeof(CVertexArray) + sizeof(CVertexArray::Stream) * (streamCount - 1)) ) CVertexArray;
   return pVertexArray;
}

//------------------------------------------------------------------------------------------

void CVertexArray::AddStream(uint32 const id, uint16 const type, uint16 const flags, void* pData, uint32 const size)
{
   mStreams[mStreamCount].mId = id;
   mStreams[mStreamCount].mType = type;
   mStreams[mStreamCount].mFlags = flags;
   mStreams[mStreamCount].mData = (uint32)pData;
   mStreams[mStreamCount].mSize = size;
   ++mStreamCount;
}

//------------------------------------------------------------------------------------------

CVertexArray::Stream const * CVertexArray::GetStream(uint32 const id) const
{
   for( int i = 0; i < mStreamCount; ++i )
   {
      if( mStreams[i].mId == id )
      {
         return mStreams + i;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------------------------

CVertexArray * CVertexArray::Load(CInputStream & stream)
{
   CVertexArray* pOutput = NULL;

   uint32 const vertexArraySize = stream.ReadUint32();

   uint8* pBaseAddress = stream.ReadAsMemoryStream(vertexArraySize);
   pOutput = (CVertexArray*)pBaseAddress;

   // patch up data offset to become ptrs.
   for( int i = 0; i < pOutput->mStreamCount; ++i )
   {
      pOutput->mStreams[i].mData += (uint32)pBaseAddress;
      pOutput->mStreams[i].mFlags &= ~kFlags_IsOffset;
   }

   if( pOutput != NULL )
   {
      for( int i = 0; i < pOutput->GetStreamCount(); ++i )
      {
         CVertexArray::Stream * pStream = const_cast<CVertexArray::Stream*>(pOutput->GetStreamByIndex(i));

#if BPE_TARGET == BPE_TARGET_WIN32
         if( pStream->mType == kVDT_Packed3N )
         {
            // Convert packed normals to float3 on Win32 because of lacking support on certain hardware (including GTX260!)
            uint32* pData = (uint32*)pStream->GetDataPtr();

            int const normalCount = pStream->mSize / 4;

            float* pFloatNormals = (float*)BPE_MALLOC_ALIGNED(16, normalCount * 12);
            Convert_S11S11S10_to_Vector3(pData, pFloatNormals, normalCount);

            pStream->mType = kVDT_Float3;
            pStream->mFlags |= kFlags_ManagedMemory;
            pStream->mData = (uint32)pFloatNormals;
            pStream->mSize = normalCount * 12;
         }
#endif

#if BPE_TARGET==BPE_TARGET_VITA
         BPE_VERIFY( pStream->mType != kVDT_Packed3N, false, "Packed3N Unsupported on Vita" );
//         BPE_VERIFY( pStream->mType != kVDT_Int32, false, "kVDT_Int32 Unsupported on Vita" );
#endif
      }
   }
   return pOutput;
}