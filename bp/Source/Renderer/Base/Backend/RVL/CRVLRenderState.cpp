#include "stdafx.h"
#include "CRVLRenderState.h"

CRVLRenderState::CStateHandle *CRVLRenderState::CStateHandle::mpHead = NULL;

CRVLRenderState::CStateHandle::CStateHandle()
: mPtr( NULL )
, mSize( 0 )
, mpNext( NULL )
{
}

void CRVLRenderState::CStateHandle::Construct( void const *ptr, int const size )
{
   mPtr = ptr;
   mSize = size;

   // First, see if we match something else
   for ( CStateHandle *pHandle = mpHead; pHandle != NULL; pHandle = pHandle->mpNext )
   {
      if ( mSize == pHandle->mSize && !memcmp( mPtr, pHandle->mPtr, mSize ) )
      {
         // We match, set our handle to the one we're looking at and punt
         mpUniqueId = pHandle;
         return;
      }
   }

   // If we've fallen through here, we didn't have a handle.  In this case, hook us up to be 
   // the new head and make our "handle" ourself
   mpNext = mpHead;
   mpHead = this;
   mpUniqueId = this;
}

CRVLRenderState::TTexcoordGenHandle const &CRVLRenderState::GetDefaultTexcoordGenForTexture( int const textureParamIndex )
{
   static const CRVLRenderState::TTexcoordGenHandle skDefaultTexGenHandles[] = 
   {
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY ) ),
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY ) ),
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY ) ),
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX3, GX_IDENTITY ) ),
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX4, GX_IDENTITY ) ),
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX5, GX_IDENTITY ) ),
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX6, GX_IDENTITY ) ),
      CRVLRenderState::TTexcoordGenHandle( CRVLRenderState::STexCoordGen( GX_TG_MTX2x4, GX_TG_TEX7, GX_IDENTITY ) )
   };

   return skDefaultTexGenHandles[ textureParamIndex ];
}

CRVLRenderState::STevOp const &CRVLRenderState::GetDefaultTevOp()
{
   static STevOp const skDefaultTevOp( GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV );

   return skDefaultTevOp;
}

CRVLRenderState::TVariableUnitStatesHandle const &CRVLRenderState::GetUnitStatesForSameTevsAndTexGensNoChan( int const tevAndTgCount )
{
   static const TVariableUnitStatesHandle skUnitStatesHandles[] =
   {
      TVariableUnitStatesHandle( SVariableUnitStates( 0, 0, 0, 0 ) ),
      TVariableUnitStatesHandle( SVariableUnitStates( 1, 0, 1, 0 ) ),
      TVariableUnitStatesHandle( SVariableUnitStates( 2, 0, 2, 0 ) ),
      TVariableUnitStatesHandle( SVariableUnitStates( 3, 0, 3, 0 ) ),
      TVariableUnitStatesHandle( SVariableUnitStates( 4, 0, 4, 0 ) ),
      TVariableUnitStatesHandle( SVariableUnitStates( 5, 0, 5, 0 ) ),
      TVariableUnitStatesHandle( SVariableUnitStates( 6, 0, 6, 0 ) ),
      TVariableUnitStatesHandle( SVariableUnitStates( 7, 0, 7, 0 ) )
   };

   return skUnitStatesHandles[ tevAndTgCount ];
}
