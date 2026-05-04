//------------------------------------------------------------------------------------------
// CMatrixStack.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CAngle.h"
#include "Engine/Math/CMatrix4.h"

//------------------------------------------------------------------------------------------

class CMatrixStack
{
   enum { kDefaultTransformStackSize = 32 };

public:
   explicit CMatrixStack( const int defaultSize = kDefaultTransformStackSize )
   :  mInverseValid( false )
   , mCachedInverse( CMatrix4::Identity() )
   {
      mTransformStack.reserve( defaultSize );
      mTransformStack.push_back( CMatrix4::Identity() );
   }

   // transform stack
   ENGINE_API void PushMatrix();
   ENGINE_API void PopMatrix();
   ENGINE_API void LoadIdentityMatrix();
   ENGINE_API void LoadMatrix( CMatrix4 const & matrix );
   ENGINE_API void LoadMatrix( CMatrix34 const & matrix );
   ENGINE_API void MultMatrix( CMatrix4 const & matrix );
   
   ENGINE_API CMatrix4 const & GetCurrentTransform() const;
   ENGINE_API CMatrix4 const & GetCurrentTransformInverse() const;

private:
   std::vector<CMatrix4>      mTransformStack;
   mutable bool               mInverseValid : 1;
   mutable CMatrix4           mCachedInverse;
};

//------------------------------------------------------------------------------------------
