//----------------------------------------------------------------------------
// RVLCVertexData.cpp
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Renderer/Base/Primitive/CVertexData.h"

//----------------------------------------------------------------------------

CVertexData::CVertexData(int baseVertexIndex)
:  mBaseVertexIndex(baseVertexIndex)
,  mAttributes(TAttributes::static_capacity, SVertexAttribute())
{
}

//----------------------------------------------------------------------------

void CVertexData::OverrideAttributes(CVertexData const & rhs)
{
   for( int i = 0; i < TAttributes::static_capacity; ++i )
   {
      if( rhs.mAttributes[i].mIsValid )
         mAttributes[i] = rhs.mAttributes[i];
   }
}

//----------------------------------------------------------------------------

void CVertexData::OverrideStreams(CVertexBuffer const * pBuffer)
{
   for( int i = 0; i < TAttributes::static_capacity; ++i )
   {
      if( mAttributes[i].mIsValid )
         mAttributes[i].mpBuffer = pBuffer;
   }
}

