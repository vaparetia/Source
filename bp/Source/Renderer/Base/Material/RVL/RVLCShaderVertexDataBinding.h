#pragma once

//----------------------------------------------------------------------------

#include "Renderer/Base/BPERendererAPI.h"

//----------------------------------------------------------------------------

#ifdef BPE_RVL_TOOLS_SUPPORT
namespace bpe_rvl
{
#endif

enum EVertexDataUsage
{
   // These orders match the orders for the GXAttr enum
   kVDU_Position,
   kVDU_Normal,
   kVDU_Color0,
   kVDU_Color1,
   kVDU_TexCoord0,
   kVDU_TexCoord1,
   kVDU_TexCoord2,
   kVDU_TexCoord3,
   kVDU_TexCoord4,
   kVDU_TexCoord5,
   kVDU_TexCoord6,
   kVDU_TexCoord7,
   kVDU_NBT = kVDU_TexCoord7 + 5,

   kVDU_CountBeforeNBT = kVDU_TexCoord7 + 1,
   kVDU_Count = kVDU_CountBeforeNBT + 1,
   kVDU_LastValid = kVDU_NBT
};

//----------------------------------------------------------------------------

class CShaderVertexDataBinding
{
public:
   enum EMatrixIndexAttribute
   {
      // These orders match the orders for the GXAttr enum
      kMIA_PositionNormal,
      kMIA_Tex0,
      kMIA_Tex1,
      kMIA_Tex2,
      kMIA_Tex3,
      kMIA_Tex4,
      kMIA_Tex5,
      kMIA_Tex6,
      kMIA_Tex7,

      kMIA_Count
   };

   enum EAttributeType
   {
      // These orders match the orders for the GXAttrType enum
      kAT_None,
      kAT_Direct,
      kAT_Index8,
      kAT_Index16
   };

   CShaderVertexDataBinding( CInputStream &stream ) 
      : mMatrixIndexAttributes( stream.ReadUint32() )
      , mVertexAttributes( stream.ReadUint32() )
   {
   }

   CShaderVertexDataBinding()
      : mMatrixIndexAttributes( 0 )
      , mVertexAttributes( 0 )
   {
      SetVertexAttribute( kVDU_Position, kAT_Direct );
   }

   CShaderVertexDataBinding( EVertexDataUsage const );
   CShaderVertexDataBinding( EVertexDataUsage const, EVertexDataUsage const );
   CShaderVertexDataBinding( EVertexDataUsage const, EVertexDataUsage const, EVertexDataUsage const );
   CShaderVertexDataBinding( EVertexDataUsage const, EVertexDataUsage const, EVertexDataUsage const, EVertexDataUsage const );
   CShaderVertexDataBinding( EVertexDataUsage const, EVertexDataUsage const, EVertexDataUsage const, EVertexDataUsage const, EVertexDataUsage const );

   void PutTo( COutputStream &stream )
   {
      stream.WriteUint32( mMatrixIndexAttributes );
      stream.WriteUint32( mVertexAttributes );
   }


   void SetMatrixIndexAttributeUsed( EMatrixIndexAttribute attribute, bool isUsed )
   {
      uint32 const attributeBit = 1 << attribute;
      uint32 const attributeBitValue = uint32( isUsed ) << attribute;

      mMatrixIndexAttributes = ( mMatrixIndexAttributes & ( ~attributeBit ) ) | attributeBitValue;
   }

   void SetVertexAttribute( EVertexDataUsage attribute, EAttributeType attributeType )
   {
      uint32 const attributeBitIndex = attribute << 1;
      uint32 const attributeMask = 0x3 << attributeBitIndex;
      uint32 const attributeValue = uint32( attributeType ) << attributeBitIndex;

      mVertexAttributes = ( mVertexAttributes & ( ~attributeMask ) ) | attributeValue;
   }

   EAttributeType const GetVertexAttributeType( EVertexDataUsage attribute ) const 
   {
      uint32 const attributeBitIndex = attribute << 1;

      return EAttributeType( ( mVertexAttributes >> attributeBitIndex ) & 0x3 );
   }

   bool const GetMatrixIndexAttributeUsed( EMatrixIndexAttribute attribute ) const
   {
      return bool( ( mMatrixIndexAttributes >> attribute ) & 1 );
   }

   static CShaderVertexDataBinding const None()
   {
      return CShaderVertexDataBinding();
   }

   bool const operator==( CShaderVertexDataBinding const &rhs ) const
   {
      return mMatrixIndexAttributes == rhs.mMatrixIndexAttributes && mVertexAttributes == rhs.mVertexAttributes;
   }

private:
   uint32 mMatrixIndexAttributes;
   uint32 mVertexAttributes;
};

//----------------------------------------------------------------------------

inline CShaderVertexDataBinding::CShaderVertexDataBinding( 
   EVertexDataUsage const arg1
   )
: mMatrixIndexAttributes( 0 )
, mVertexAttributes( 0 )
{
   SetVertexAttribute( arg1, kAT_Direct );
}

//----------------------------------------------------------------------------

inline CShaderVertexDataBinding::CShaderVertexDataBinding( 
   EVertexDataUsage const arg1
  ,EVertexDataUsage const arg2
   )
   : mMatrixIndexAttributes( 0 )
   , mVertexAttributes( 0 )
{
   SetVertexAttribute( arg1, kAT_Direct );
   SetVertexAttribute( arg2, kAT_Direct );
}

//----------------------------------------------------------------------------

inline CShaderVertexDataBinding::CShaderVertexDataBinding( 
   EVertexDataUsage const arg1
  ,EVertexDataUsage const arg2
  ,EVertexDataUsage const arg3
   )
   : mMatrixIndexAttributes( 0 )
   , mVertexAttributes( 0 )
{
   SetVertexAttribute( arg1, kAT_Direct );
   SetVertexAttribute( arg2, kAT_Direct );
   SetVertexAttribute( arg3, kAT_Direct );
}

//----------------------------------------------------------------------------

inline CShaderVertexDataBinding::CShaderVertexDataBinding( 
   EVertexDataUsage const arg1
  ,EVertexDataUsage const arg2
  ,EVertexDataUsage const arg3
  ,EVertexDataUsage const arg4
   )
   : mMatrixIndexAttributes( 0 )
   , mVertexAttributes( 0 )
{
   SetVertexAttribute( arg1, kAT_Direct );
   SetVertexAttribute( arg2, kAT_Direct );
   SetVertexAttribute( arg3, kAT_Direct );
   SetVertexAttribute( arg4, kAT_Direct );
}

//----------------------------------------------------------------------------

inline CShaderVertexDataBinding::CShaderVertexDataBinding( 
   EVertexDataUsage const arg1
  ,EVertexDataUsage const arg2
  ,EVertexDataUsage const arg3
  ,EVertexDataUsage const arg4
  ,EVertexDataUsage const arg5
   )
   : mMatrixIndexAttributes( 0 )
   , mVertexAttributes( 0 )
{
   SetVertexAttribute( arg1, kAT_Direct );
   SetVertexAttribute( arg2, kAT_Direct );
   SetVertexAttribute( arg3, kAT_Direct );
   SetVertexAttribute( arg4, kAT_Direct );
   SetVertexAttribute( arg5, kAT_Direct );
}

//----------------------------------------------------------------------------

#ifdef BPE_RVL_TOOLS_SUPPORT
} // end namespace bpe_rvl
#endif
