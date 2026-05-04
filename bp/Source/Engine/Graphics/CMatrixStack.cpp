//------------------------------------------------------------------------------------------
// CMatrixStack.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CMatrixStack.h"

//------------------------------------------------------------------------------------------

#include "Engine/Math/BPEMath.h"

//------------------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------------------

void CMatrixStack::PushMatrix()
{
   BPE_ASSERT( mTransformStack.size() >= 1, "transform stack is in invalid state! ( numTransforms < 1 )" );
   mTransformStack.push_back( mTransformStack[ mTransformStack.size() - 1 ] );
   mInverseValid = false;
}

//------------------------------------------------------------------------------------------

void CMatrixStack::PopMatrix()
{
   BPE_ASSERT( mTransformStack.size() > 1, "popping off more elements than stack got!" );
   mTransformStack.pop_back();
   mInverseValid = false;
}

//------------------------------------------------------------------------------------------

void CMatrixStack::LoadIdentityMatrix()
{
   mTransformStack[ mTransformStack.size() - 1 ] = CMatrix4::Identity();
   mInverseValid = false;
}

//------------------------------------------------------------------------------------------

void CMatrixStack::LoadMatrix( CMatrix4 const & matrix )
{
   mTransformStack[ mTransformStack.size() - 1 ] = matrix;
   mInverseValid = false;
}

//------------------------------------------------------------------------------------------

void CMatrixStack::LoadMatrix( CMatrix34 const & matrix )
{
   mTransformStack[ mTransformStack.size() - 1 ] = CMatrix4::FromMatrix34( matrix );
   mInverseValid = false;
}

//------------------------------------------------------------------------------------------

void CMatrixStack::MultMatrix( CMatrix4 const & matrix )
{
   mTransformStack[ mTransformStack.size() - 1 ] *= matrix;
   mInverseValid = false;
}

//------------------------------------------------------------------------------------------
   
CMatrix4 const & CMatrixStack::GetCurrentTransform() const
{
   return mTransformStack[ mTransformStack.size() - 1 ];
}

//------------------------------------------------------------------------------------------

CMatrix4 const & CMatrixStack::GetCurrentTransformInverse() const
{
   if( !mInverseValid )
   {
      mCachedInverse = mTransformStack[ mTransformStack.size() - 1 ].Inverse();
      mInverseValid = true;
   }
   return mCachedInverse;
}


