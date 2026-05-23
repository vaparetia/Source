//------------------------------------------------------------------------------------------
// CIndexArray.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CIndexArray.h"

//------------------------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

//------------------------------------------------------------------------------------------

CIndexArray::CIndexArray( CInputStream& inStream )
:  mIndices( inStream )
{
}

//------------------------------------------------------------------------------------------

void CIndexArray::PutTo( COutputStream& outStream ) const
{
   outStream.Put( mIndices );
}

//------------------------------------------------------------------------------------------
   
int const CIndexArray::Append( CIndexArray const & other )
{
   int const firstIndex = mIndices.size();
   
   mIndices.insert( mIndices.end(), other.GetIndices().begin(), other.GetIndices().end() );

   return firstIndex;
}

