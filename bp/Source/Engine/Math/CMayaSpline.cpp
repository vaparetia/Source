//----------------------------------------------------------------------------
// CMayaSpline.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CMayaSpline.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

real32 const kMaxTan = 5729577.9485111479f;

//----------------------------------------------------------------------------

CSpline const CMayaSpline::BuildSpline() const
{
   real32      py, ny, dx;
   real32      length;

   std::vector<CSplineKey> outKeys;

   // initialize the animation curve parameters
   //animCurve->isStatic = kEngineTRUE;

   // compute tangents
   for( int index = 0; index < mKeys.size(); ++index )
   {
      CMayaSplineKey const * const key = &mKeys[index];
      CMayaSplineKey const * const prevKey = (index > 0) ? &mKeys[index - 1] : NULL;
      CMayaSplineKey const * const nextKey = (index < (mKeys.size() - 1)) ? &mKeys[index + 1] : NULL;

      bool hasSmooth = false;

      CMayaSplineKey::ETangentType inTangentType = key->mInTangentType;
      real32      inTanX, inTanY;
      {
         // compute the in-tangent values
         // kTangentClamped
         if( (inTangentType == CMayaSplineKey::kTangent_Clamped) && (prevKey != NULL) )
         {
            py = prevKey->mValue - key->mValue;
            if( py < 0.0 ) 
               py = -py;

            ny = (nextKey == NULL ? py : nextKey->mValue - key->mValue);

            if( ny < 0.0 ) 
               ny = -ny;

            if( (ny <= 0.05) || (py <= 0.05) )
            {
               inTangentType = CMayaSplineKey::kTangent_Flat;
            }
         }

         switch( inTangentType )
         {
         case CMayaSplineKey::kTangent_Fixed:
            inTanX = key->mInWeight * cosf(key->mInAngle.AsRadians()) * 3.0f;
            inTanY = key->mInWeight * sinf(key->mInAngle.AsRadians()) * 3.0f;
            break;

         case CMayaSplineKey::kTangent_Linear:
            if( prevKey == NULL )
            {
               inTanX = 1.0f;
               inTanY = 0.0f;
            }
            else
            {
               inTanX = key->mTime - prevKey->mTime;
               inTanY = key->mValue - prevKey->mValue;
            }
            break;

         case CMayaSplineKey::kTangent_Flat:
            if( prevKey == NULL )
            {
               inTanX = (nextKey == NULL ? 0.0f : nextKey->mTime - key->mTime);
               inTanY = 0.0f;
            }
            else
            {
               inTanX = key->mTime - prevKey->mTime;
               inTanY = 0.0f;
            }
            break;

         case CMayaSplineKey::kTangent_Step:
            inTanX = 0.0f;
            inTanY = 0.0f;
            break;

         case CMayaSplineKey::kTangent_Slow:
         case CMayaSplineKey::kTangent_Fast:
            inTangentType = CMayaSplineKey::kTangent_Smooth;
            if( prevKey == NULL )
            {
               inTanX = 1.0f;
               inTanY = 0.0f;
            }
            else
            {
               inTanX = key->mTime - prevKey->mTime;
               inTanY = key->mValue - prevKey->mValue;
            }
            break;

         case CMayaSplineKey::kTangent_Plateau:
         case CMayaSplineKey::kTangent_Smooth:
         case CMayaSplineKey::kTangent_Clamped:
            inTangentType = CMayaSplineKey::kTangent_Smooth;
            hasSmooth = true;
            break;
         }
      }

      CMayaSplineKey::ETangentType outTangentType = key->mOutTangentType;
      real32 outTanX, outTanY;
      {
         // compute the out-tangent values
         // kTangentClamped
         if( (outTangentType == CMayaSplineKey::kTangent_Clamped) && (nextKey != NULL) )
         {
            ny = nextKey->mValue - key->mValue;
            if( ny < 0.0 ) 
               ny = -ny;
            
            py = (prevKey == NULL ? ny : prevKey->mValue - key->mValue);
            
            if( py < 0.0 ) 
               py = -py;
            
            if( (ny <= 0.05) || (py <= 0.05) )
            {
               outTangentType = CMayaSplineKey::kTangent_Flat;
            }
         }
         
         switch( outTangentType )
         {
         case CMayaSplineKey::kTangent_Fixed:
            outTanX = key->mOutWeight * cosf(key->mOutAngle.AsRadians()) * 3.0f;
            outTanY = key->mOutWeight * sinf(key->mOutAngle.AsRadians()) * 3.0f;
            break;

         case CMayaSplineKey::kTangent_Linear:
            if( nextKey == NULL )
            {
               outTanX = 1.0f;
               outTanY = 0.0f;
            }
            else
            {
               outTanX = nextKey->mTime - key->mTime;
               outTanY = nextKey->mValue - key->mValue;
            }
            break;

         case CMayaSplineKey::kTangent_Flat:
            if( nextKey == NULL )
            {
               outTanX = (prevKey == NULL ? 0.0f : key->mTime - prevKey->mTime);
               outTanY = 0.0f;
            }
            else
            {
               outTanX = nextKey->mTime - key->mTime;
               outTanY = 0.0f;
            }
            break;

         case CMayaSplineKey::kTangent_Step:
            outTanX = 0.0f;
            outTanY = 0.0f;
            break;

         case CMayaSplineKey::kTangent_Slow:
         case CMayaSplineKey::kTangent_Fast:
            outTangentType = CMayaSplineKey::kTangent_Smooth;
            if( nextKey == NULL )
            {
               outTanX = 1.0f;
               outTanY = 0.0f;
            }
            else
            {
               outTanX = nextKey->mTime - key->mTime;
               outTanY = nextKey->mValue - key->mValue;
            }
            break;

         case CMayaSplineKey::kTangent_Plateau:
         case CMayaSplineKey::kTangent_Smooth:
         case CMayaSplineKey::kTangent_Clamped:
            outTangentType = CMayaSplineKey::kTangent_Smooth;
            hasSmooth = true;
            break;
         }
      }

      // compute smooth tangents (if necessary)
      {
         real32 inTanXs, inTanYs, outTanXs, outTanYs;
         
         if( hasSmooth )
         {
            if( (prevKey == NULL) && (nextKey != NULL) )
            {
               outTanXs = nextKey->mTime - key->mTime;
               outTanYs = nextKey->mValue - key->mValue;
               inTanXs = outTanXs;
               inTanYs = outTanYs;
            }
            else if( (prevKey != NULL) && (nextKey == NULL) )
            {
               outTanXs = key->mTime - prevKey->mTime;
               outTanYs = key->mValue - prevKey->mValue;
               inTanXs = outTanXs;
               inTanYs = outTanYs;
            }
            else if( (prevKey != NULL) && (nextKey != NULL) )
            {
               // There is a CV before and after this one
               // Find average of the adjacent in and out tangents.
   
               dx = nextKey->mTime - prevKey->mTime;
               if( dx < 0.0001 )
               {
                  outTanYs = kMaxTan;
               }
               else
               {
                  outTanYs = (nextKey->mValue - prevKey->mValue) / dx;
               }
   
               outTanXs = nextKey->mTime - key->mTime;
               inTanXs = key->mTime - prevKey->mTime;
               inTanYs = outTanYs * inTanXs;
               outTanYs *= outTanXs;
            }
            else
            {
               inTanXs = 1.0f;
               inTanYs = 0.0f;
               outTanXs = 1.0f;
               outTanYs = 0.0f;
            }
   
            if( inTangentType == CMayaSplineKey::kTangent_Smooth )
            {
               inTanX = inTanXs;
               inTanY = inTanYs;
            }
            if( outTangentType == CMayaSplineKey::kTangent_Smooth )
            {
               outTanX = outTanXs;
               outTanY = outTanYs;
            }
         }
      }

      // make sure the computed tangents are valid
      {
         if( inTanX < 0.0f ) 
            inTanX = 0.0f;
         if( outTanX < 0.0f ) 
            outTanX = 0.0f;

         if( !mIsWeighted )
         {
            // validate in tangents
            length = sqrtf((inTanX * inTanX) + (inTanY * inTanY));
            if( length != 0.0f )
            {
               // zero lengths can come from step tangents
               inTanX /= length;
               inTanY /= length;
            }
            if( (inTanX == 0.0f) && (inTanY != 0.0f) )
            {
               inTanX = 0.0001f;
               inTanY = (inTanY < 0.0f ? -1.0f : 1.0f) * (inTanX * kMaxTan);
            }

            // validate out tangents
            length = sqrtf((outTanX * outTanX) + (outTanY * outTanY));
            if( length != 0.0f )
            { 
               // zero lengths can come from step tangents
               outTanX /= length;
               outTanY /= length;
            }
            if( (outTanX == 0.0f) && (outTanY != 0.0f) )
            {
               outTanX = 0.0001f;
               outTanY = (outTanY < 0.0f ? -1.0f : 1.0f) * (outTanX * kMaxTan);
            }
         }
      }

      // construct the final key
      outKeys.push_back(CSplineKey(key->mTime, 
                                   key->mValue, 
                                   CVector2(inTanX, inTanY), 
                                   CVector2(outTanX, outTanY)));
      
      // check whether or not this animation curve is static (i.e. all the
      // key values are the same)
      /*
      if( animCurve->isStatic )
      {
         if( (prevKey != NULL) && (prevKey->mValue != key->mValue) )
         {
            animCurve->isStatic = kEngineFALSE;
         }
         else if( (inTanY != 0.0) || (outTanY != 0.0) )
         {
            animCurve->isStatic = kEngineFALSE;
         }
      }
      */
   }
   /*
   if( animCurve->isStatic )
   {
      if( (prevKey != NULL) && (key != NULL) && (prevKey->mValue != key->mValue) )
      {
         animCurve->isStatic = kEngineFALSE;
      }
   }
   */
   
   CSpline spline( mIsWeighted, mPreInfinity, mPostInfinity );
   
   for( int i = 0; i < outKeys.size(); ++i )
   {
      spline.AddKey( outKeys[i] );
   }

   return spline;
}


