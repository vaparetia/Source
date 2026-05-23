//------------------------------------------------------------------------------------------
// CIndexArray.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include <vector>

//------------------------------------------------------------------------------------------

class CIndexArray
{
public:
   explicit CIndexArray()
   {
   }

   explicit CIndexArray( std::vector<uint32> const & indices )
   :  mIndices( indices )
   {
   }

   ENGINE_API explicit CIndexArray( CInputStream& inStream );
   ENGINE_API void PutTo( COutputStream& outStream ) const;

   std::vector<uint32> const & GetIndices() const { return mIndices; }

   ENGINE_API int const Append( CIndexArray const & other );

private:
   bpe::vector_s<uint32>   mIndices;
};
