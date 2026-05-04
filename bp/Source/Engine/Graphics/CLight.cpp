//----------------------------------------------------------------------------
// CLight.cpp
// Bluepoint
// Copyright 2004
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Graphics/CLight.h"

//----------------------------------------------------------------------------

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"
#include "Engine/Math/CRandom.h"
#include "Engine/Math/CQuaternion.h"
#include "Engine/Math/CMatrix3.h"

//----------------------------------------------------------------------------

using namespace std;
using namespace bpe;

//----------------------------------------------------------------------------

CLight::CLight()
:  mTransform( CMatrix34::Identity() )
,  mType( kType_Invalid )
,  mColor( CVector3::kConstructUninitialized )

,  mRadius( 1.0f )
,  mFalloffExponent( 1.0f )

,  mCastShadows( true )
,  mShadowSoftRadius( 0.0f )
,  mIsStatic( false )

,  mConeAngle( CAngle::FromRadians(0.0) )
,  mPenumbraAngle( CAngle::FromRadians(0.0) )
,  mDropOff( 0.0f )
,  mLightSets(~(0UL))
,  mPriority(0)
{
}

//----------------------------------------------------------------------------
// AndyO:
// Hopefully we can get rid of the stream code later!

CLight::CLight( CInputStream & stream )
:  mTransform( stream )
,  mType( static_cast<EType>( stream.ReadInt32() ) )
,  mColor( stream )

,  mRadius( stream.ReadReal32() )
,  mFalloffExponent( stream.ReadReal32() )

,  mCastShadows( stream.ReadBool() )
,  mShadowSoftRadius( stream.ReadReal32() )
,  mIsStatic( stream.ReadBool() )

,  mConeAngle( stream )
,  mPenumbraAngle( stream )
,  mDropOff( stream.ReadReal32() )
,  mLightSets(stream.ReadUint32())
,  mPriority(stream.ReadInt32())
{
}

//----------------------------------------------------------------------------
void CLight::PutTo( COutputStream & stream ) const
{
   stream.Put(mTransform);
   stream.WriteInt32(mType);
   stream.Put(mColor);
   
   stream.WriteReal32(mRadius);
   stream.WriteReal32(mFalloffExponent);
   
   stream.WriteBool(mCastShadows);
   stream.WriteReal32(mShadowSoftRadius);
   stream.WriteBool(mIsStatic);
   
   stream.Put(mConeAngle);
   stream.Put(mPenumbraAngle);
   stream.WriteReal32(mDropOff);
   stream.WriteUint32(mLightSets);
   stream.WriteInt32(mPriority);
}

//----------------------------------------------------------------------------

real32 const kMayaNoDistanceAttenuationFactor                 = 1.0f / 2.0f;
real32 const kOneOverMayaNoDistanceAttenuationFactor          = 1.0f / kMayaNoDistanceAttenuationFactor;
real32 const kMayaLinearDistanceAttenuationFactor             = 1.0f / 250.0f;
real32 const kOneOverMayaLinearDistanceAttenuationFactor      = 1.0f / kMayaLinearDistanceAttenuationFactor;
real32 const kMayaQuadraticDistanceAttenuationFactor          = 1.0f / 25000.0f;
real32 const kOneOverMayaQuadraticDistanceAttenuationFactor   = 1.0f / kMayaQuadraticDistanceAttenuationFactor;

//----------------------------------------------------------------------------

real32 const CLight::GetAttenuationAtPoint_NoGamma( CVector3 const & position ) const
{
   CVector3 const vertexToLight = ( mTransform.GetTranslation() - position );

   // real thing, linear falloff
   real32 const vertexLightDistance = max_val( vertexToLight.GetLength(), gkEpsilon32 );

   real32 linearAttenuation = max_val( 0.0f, 1.0f - vertexLightDistance / mRadius );
   linearAttenuation = powf(linearAttenuation, mFalloffExponent);

   return linearAttenuation;
}

//----------------------------------------------------------------------------

real32 const CLight::GetAttenuationAtPoint_NoGamma( CVector3 const & position, CVector3 const &normal, bool const isDoubleSided ) const
{
   CVector3 const vertexToLight = ( mTransform.GetTranslation() - position );

   // Linear falloff
   real32 const vertexLightDistance = max_val( vertexToLight.GetLength(), gkEpsilon32 );

   CVector3 const vertexToLightUnit( vertexToLight / vertexLightDistance );
   
   real32 linearAttenuation = max_val( 0.0f, 1.0f - vertexLightDistance / mRadius );
   linearAttenuation = powf(linearAttenuation, mFalloffExponent);
   
   real32 angleAttenuation = 1.0f;

   switch( mType )
   {
      case kType_Directional:
         {
            linearAttenuation = 1.0f;
            angleAttenuation = CVector3::Dot( normal, -mTransform.GetForward() );
         }
         break;

      case kType_Spot:
         {
            // if angle is 180 degrees then we leave the spot factor at 1.0 as initialized
            if( !close_enough(mConeAngle.AsRadians(), gkPi32) )
            {
               real32 const dot = CVector3::Dot( vertexToLightUnit, -mTransform.GetForward() );
               real32 const modifiedDot = max_val(dot, 0.0f);
   
               // cosine cutoff angle for spot lights
               real32 const spotCosinePenumbraStart = cosf(mConeAngle.AsRadians());
               real32 const spotCosinePenumbraEnd = cosf(mConeAngle.AsRadians() + mPenumbraAngle.AsRadians());
               real32 const penumbraRange = bpe::max_val(spotCosinePenumbraStart - spotCosinePenumbraEnd, gkEpsilon32);

               if( modifiedDot >= spotCosinePenumbraEnd )
               {
                  real32 const lightDot = max_val( CVector3::Dot( vertexToLightUnit, normal ), 0.0f);
                  if( mDropOff )
                  {
                     angleAttenuation = lightDot * powf( modifiedDot, mDropOff );
                  }
                  else
                  {
                     angleAttenuation = lightDot;
                  }

                  real32 const penumbraFactor = bpe::max_val(0.0f, bpe::min_val(1.0f, (modifiedDot - spotCosinePenumbraEnd) / penumbraRange));
                  
                  angleAttenuation *= penumbraFactor;
               }
               else
               {
                  angleAttenuation = 0.0f;
               }                 
            }
         }
         break;

      case kType_Point:
         {
            angleAttenuation = CVector3::Dot( normal, vertexToLightUnit );            
         }
         break;

      case kType_Ambient:
      case kType_Occlusion:
         {
            // No attenuation
            angleAttenuation = 1.0f;
            linearAttenuation = 1.0f;
         }
         break;
   }

   return max_val(linearAttenuation * (isDoubleSided ? fabsf(angleAttenuation) : angleAttenuation), 0.0f);
}

//----------------------------------------------------------------------------

CAABox const CLight::GetLightBounds() const
{
   CVector3 const radiusVector( mRadius, mRadius, mRadius );
   CVector3 const lightPos = mTransform.GetTranslation();
   return CAABox( lightPos - radiusVector, lightPos + radiusVector );
}

//----------------------------------------------------------------------------

CLight const CLight::CreateAmbient( CVector3 const & color )
{
   return CLight( CMatrix34::Identity(),
                  kType_Ambient,
                  color,
                  0.0f,
                  1.0f,
                  CAngle::FromDegrees( 0.0f ),
                  CAngle::FromDegrees( 0.0f ),
                  0.0f,
                  false,
                  0.0f );
}

//----------------------------------------------------------------------------

CLight const CLight::CreatePoint( CMatrix34 const & transform,
                                  CVector3 const & color,
                                  real32 const radius,
                                  real32 const falloffExponent,
                                  bool const castShadows,
                                  real32 const shadowSoftRadius )
{
   return CLight( transform,
                  kType_Point,
                  color,
                  radius,
                  falloffExponent,
                  CAngle::FromDegrees( 0.0f ),
                  CAngle::FromDegrees( 0.0f ),
                  0.0f,
                  castShadows,
                  shadowSoftRadius );
}

//----------------------------------------------------------------------------

CLight const CLight::CreateDirectional( CMatrix34 const & transform,
                                        CVector3 const & color,
                                        real32 const radius,
                                        real32 const falloffExponent,
                                        bool const castShadows,
                                        real32 const shadowSoftRadius )
{
   return CLight( transform,
                  kType_Directional,
                  color,
                  radius,
                  falloffExponent,
                  CAngle::FromDegrees( 0.0f ),
                  CAngle::FromDegrees( 0.0f ),
                  0.0f,
                  castShadows,
                  shadowSoftRadius );
}

//----------------------------------------------------------------------------

CLight const CLight::CreateSpot( CMatrix34 const & transform,
                                 CVector3 const & color,
                                 real32 const radius,
                                 real32 const falloffExponent,
                                 CAngle const & coneAngle,
                                 CAngle const & penumbraAngle,
                                 real32 const dropOff,
                                 bool const castShadows,
                                 real32 const shadowSoftRadius )
{
   return CLight( transform,
                  kType_Spot,
                  color,
                  radius,
                  falloffExponent,
                  coneAngle,
                  penumbraAngle,
                  dropOff,
                  castShadows,
                  shadowSoftRadius );
}

//----------------------------------------------------------------------------
   
CLight const CLight::CreateOcclusion( CVector3 const & color )
{
   return CLight( CMatrix34::Identity(),
                  kType_Occlusion,
                  color,
                  1.0f,
                  1.0f,
                  CAngle::FromDegrees( 0.0f ),
                  CAngle::FromDegrees( 0.0f ),
                  0.0f,
                  false,
                  0.0f );
}

//----------------------------------------------------------------------------

CLight::CLight( CMatrix34 const & transform,
                EType const type,
                CVector3 const & color,
                real32 const radius,
                real32 const falloffExponent,
                CAngle const & coneAngle,
                CAngle const & penumbraAngle,
                real32 const dropOff,
                bool const castShadows,
                real32 const shadowSoftRadius )
:  mTransform( transform )
,  mType( type )
,  mColor( color )
,  mRadius( radius )
,  mFalloffExponent( falloffExponent )
,  mConeAngle( coneAngle )
,  mPenumbraAngle( penumbraAngle )
,  mDropOff( dropOff )
,  mCastShadows( castShadows )
,  mShadowSoftRadius( shadowSoftRadius )
,  mIsStatic( false )
{
}

//----------------------------------------------------------------------------

CRay const CLight::GetShadowRay(CVector3 const & destination, 
                                real32 & rayLength,
                                CVector3 const & lightJitterOffset) const
{
   CVector3 lightPos(CVector3::kConstructUninitialized);

   real32 lightOffsetScale = 1.0f;

   if( mType != kType_Directional )
   {
      lightPos = mTransform.GetTranslation();
   }
   else
   {
      lightOffsetScale = 4000.0f;
      lightPos = destination - mTransform.GetForward() * 4000.0f;
   }

   CQuaternion const orient = CQuaternion::LookAt( CVector3::ZAxis(), CVector3(destination - lightPos).Normalized() );

   CVector3 const scaledLightJitterOffset = lightJitterOffset * mShadowSoftRadius;
   CVector3 const lightOffset = orient * scaledLightJitterOffset * lightOffsetScale;
   lightPos += lightOffset;

   CVector3 direction = (destination - lightPos);
   rayLength = direction.Normalize();

   return CRay( lightPos, direction );

}

