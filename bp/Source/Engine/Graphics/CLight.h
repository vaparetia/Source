//----------------------------------------------------------------------------
// CLight.h
// Bluepoint
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Graphics/CColor.h"
#include "Engine/Math/CMatrix34.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CRay.h"
#include "Engine/Primitives/CAABox.h"

//----------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//----------------------------------------------------------------------------

class CLight
{
public:
   enum EType
   {
      kType_Occlusion,
      kType_Ambient,
      kType_Point,
      kType_Directional,
      kType_Spot,

      kType_Invalid = -1
   };

public:
   ENGINE_API explicit CLight();

   ENGINE_API explicit CLight( CInputStream & stream );
   ENGINE_API void PutTo( COutputStream & stream ) const;

   ENGINE_API real32 const GetAttenuationAtPoint_NoGamma( CVector3 const & position ) const;
   ENGINE_API real32 const GetAttenuationAtPoint_NoGamma( CVector3 const & position, CVector3 const &normal, bool const isDoubleSided ) const;

   ENGINE_API CRay const GetShadowRay( CVector3 const & destination,
                                       real32 & rayLength,
                                       CVector3 const & lightJitterOffset ) const;

   ENGINE_API CAABox const GetLightBounds() const;

   CMatrix34 const & GetTransform() const { return mTransform; }
   void SetTransform( CMatrix34 const & transform ) { mTransform = transform; }

   void SetCastShadow(bool const value) { mCastShadows = value; }
   bool const DoesCastShadow() const { return mCastShadows; }

   real32 const GetSoftShadowRadius() const { return mShadowSoftRadius; }

   // Note, if you call this function it is up to you to ensure all the required variables are initialized
   void SetType( EType const type ) { mType = type; }
   EType const GetType() const { return mType; }

   void SetFalloffExponent( real32 const falloffExponent ) { mFalloffExponent = falloffExponent; }
   real32 const GetFalloffExponent() const { return mFalloffExponent; }

   CVector3 const & GetColor() const { return mColor; }
   void SetColor( CVector3 const & color ) { mColor = color; }

   void SetRadius( real32 const radius ) { mRadius = radius; }
   real32 const GetRadius() const { return mRadius; }

   bool const IsStaticLight() const { return mIsStatic; }
   void SetStaticLight( bool const isStatic ) { mIsStatic = isStatic; }

   ENGINE_API static CLight const CreateAmbient( CVector3 const & color );

   ENGINE_API static CLight const CreatePoint( CMatrix34 const & transform,
                                               CVector3 const & color,
                                               real32 const radius,
                                               real32 const falloffExponent,
                                               bool const castShadows,
                                               real32 const shadowSoftRadius );

   ENGINE_API static CLight const CreateDirectional( CMatrix34 const & transform,
                                                     CVector3 const & color,
                                                     real32 const radius,
                                                     real32 const falloffExponent,
                                                     bool const castShadows,
                                                     real32 const shadowSoftRadius );

   ENGINE_API static CLight const CreateSpot( CMatrix34 const & transform,
                                              CVector3 const & color,
                                              real32 const radius,
                                              real32 const falloffExponent,
                                              CAngle const & coneAngle,
                                              CAngle const & penumbraAngle,
                                              real32 const dropOff,
                                              bool const castShadows,
                                              real32 const shadowSoftRadius );

   ENGINE_API static CLight const CreateOcclusion( CVector3 const & color );

protected:
   ENGINE_API explicit CLight( CMatrix34 const & transform,
                               EType const type,
                               CVector3 const & color,
                               real32 const radius,
                               real32 const falloffExponent,
                               CAngle const & coneAngle,
                               CAngle const & penumbraAngle,
                               real32 const dropOff,
                               bool const castShadows,
                               real32 const shadowSoftRadius );

protected:
   CMatrix34   mTransform;
   
   EType       mType;

   CVector3    mColor;
              
   real32      mRadius;
   real32      mFalloffExponent;
              
   bool        mCastShadows;
   real32      mShadowSoftRadius;
   bool        mIsStatic;

public:
   // spot specific
   CAngle      mConeAngle;
   CAngle      mPenumbraAngle;
   real32      mDropOff;

   uint32      mLightSets; // Used with CMaterialFlags to check if a model is affected by this light.
   int32       mPriority;
};

//----------------------------------------------------------------------------


