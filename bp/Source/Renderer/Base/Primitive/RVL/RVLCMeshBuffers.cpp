//----------------------------------------------------------------------------
// RVLCMeshBuffers.cpp
// Copyright 2009
//----------------------------------------------------------------------------

#include "stdafx.h"
#include "RVLCMeshBuffers.h"
#include "Engine\Graphics\CVertexArray.h"

namespace
{
   template <class T>
   size_t const get_rounded_vector_size( T const &vec )
   {
      return OSRoundUp32B( sizeof( vec[0] ) * vec.size() );
   }

   template <class T>
   uint8 *copy_vector( uint8 *dst, T const &vec, size_t const size )
   {
      if ( vec.size() > 0 )
      {
         memcpy( dst, &( vec[0] ), vec.size() * sizeof( vec[0] ) );
      }

      return dst + size;
   }
}

CMeshBuffers::CMeshBuffers( CVertexArray const & vertexArray, void const *pDisplayList, uint32 const displayListSize )
: mVertexBuffer( NULL )
, mpDisplayList( gpRenderBackend->AllocFixed( displayListSize, kRM_Video, kRM_System ) )
, mDisplayListSize( displayListSize )
{
   memcpy( mpDisplayList->mpAddress, pDisplayList, mDisplayListSize );
   DCFlushRange( mpDisplayList->mpAddress, mDisplayListSize );

   // For now, let's just copy everything to the vertex array, worry about memory later

   CVertexArray::TVector3Array const &positions = vertexArray.GetPositions();
   CVertexArray::TVector3Array const &normals = vertexArray.GetNormals();
   CVertexArray::TRGBAColorArray const &colors = vertexArray.GetColors();
   CVertexArray::TVector2Array const &texCoords = vertexArray.GetTextureCoordinates( 0 );
   CVertexArray::TVector2Array const &texCoords2 = vertexArray.GetTextureCoordinates( 1 );

   size_t const size_positions = get_rounded_vector_size( positions );
   size_t const size_normals = get_rounded_vector_size( normals );
   size_t const size_colors = get_rounded_vector_size( colors );
   size_t const size_texCoords = get_rounded_vector_size( texCoords );
   size_t const size_texCoords2 = get_rounded_vector_size( texCoords2 );

   size_t const size_total = size_positions + size_normals + size_colors + size_texCoords + size_texCoords2;
   mVertexBuffer.mpMemory = gpRenderBackend->AllocFixed( size_total,
      kRM_Video, kRM_System );

   uint8 *beginDst = mVertexBuffer.mpMemory->mpAddress;
   uint8 *positionsDst = beginDst;
   uint8 *normalsDst = copy_vector( positionsDst, positions, size_positions );
   uint8 *colorsDst = copy_vector( normalsDst, normals, size_normals );
   uint8 *texCoordsDst = copy_vector( colorsDst, colors, size_colors );
   uint8 *texCoordsDst2 = copy_vector( texCoordsDst, texCoords, size_texCoords );
   uint8 *end = copy_vector( texCoordsDst2, texCoords2, size_texCoords2 );

   DCFlushRange( mVertexBuffer.mpMemory->mpAddress, size_total );

   mVertexData.SetAttribute( kVDU_Position, positionsDst - beginDst, sizeof( CVector3 ), &mVertexBuffer );
   
   if ( size_normals )
   {
      mVertexData.SetAttribute( kVDU_Normal, normalsDst - beginDst, sizeof( CVector3 ), &mVertexBuffer );
   }

   if ( size_colors )
   {
      mVertexData.SetAttribute( kVDU_Color0, colorsDst - beginDst, sizeof( uint32 ), &mVertexBuffer );
   }

   if ( size_texCoords )
   {
      mVertexData.SetAttribute( kVDU_TexCoord0, texCoordsDst - beginDst, sizeof( CVector2 ), &mVertexBuffer );
   }

   if ( size_texCoords2 )
   {
      mVertexData.SetAttribute( kVDU_TexCoord1, texCoordsDst2 - beginDst, sizeof( CVector2 ), &mVertexBuffer );
   }
}

CMeshBuffers::~CMeshBuffers()
{
   gpRenderBackend->Free( mpDisplayList );
}

void CMeshBuffers::SetCurrent() const
{
   gpRenderBackend->SetVertexData( mVertexData, 0 );
}

//----------------------------------------------------------------------------

COverridenMeshBuffers::~COverridenMeshBuffers()
{
}