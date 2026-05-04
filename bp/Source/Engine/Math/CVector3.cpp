//------------------------------------------------------------------------------------------
// CVector3.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "CVector3.h"

//------------------------------------------------------------------------------------------
// Removed unneeded output code when compiling for SPU's
#if !defined(SPU)

CVector3::CVector3(CInputStream& stream)
:  mX(stream.ReadReal32())
,  mY(stream.ReadReal32())
,  mZ(stream.ReadReal32())
{
}

//------------------------------------------------------------------------------------------

void CVector3::PutTo(COutputStream& stream) const
{
   stream.WriteReal32(mX);
   stream.WriteReal32(mY);
   stream.WriteReal32(mZ);
}

//------------------------------------------------------------------------------------------

CVector3::CVector3( std::string const & textValue )
{
   int const numArgs = sscanf( textValue.c_str(), "%f %f %f", &mX, &mY, &mZ );
   BPE_VERIFY( numArgs == 3, false, "invalid text string" );
}

//------------------------------------------------------------------------------------------

std::string CVector3::ToString() const
{
   return CStringExtras::Stringize( "%f %f %f", mX, mY, mZ );
}

#endif

//------------------------------------------------------------------------------------------

real32 CVector3::TryNormalize()
{
   real32 const prevLength = GetLength();
   // Arbitrary epsilon
   if (prevLength < gkEpsilon32)
   {
      return 0;
   }

   *this /= prevLength;

   return prevLength;
}

//------------------------------------------------------------------------------------------

real32 CVector3::TryNormalize_WSDelta()
{
   real32 const prevLength = GetLength();
   // Arbitrary epsilon
   if (prevLength < gkEpsilon32 * 10000.0f)
   {
      return 0;
   }

   *this /= prevLength;

   return prevLength;
}

//------------------------------------------------------------------------------------------

int CVector3::GetDominantAxis() const
{
   real32 absX = fabsf(mX);
   real32 absY = fabsf(mY);
   real32 absZ = fabsf(mZ);

   if( absX < absY )
   {
      // X<Y && Z>Y
      if( absZ > absY )
      {
         return kAZ;
      }
      // X<Y && Z<Y
      else
      {
         return kAY;
      }
   }
   else // X > Y
   {
      // X>Y && Z>X
      if( absZ > absX )
      {
         return kAZ;
      }
      // X>Y && Z<X
      else
      {
         return kAX;
      }
   }
}

//------------------------------------------------------------------------------------------

int CVector3::GetMinorAxis() const
{
   real32 absX = fabsf(mX);
   real32 absY = fabsf(mY);
   real32 absZ = fabsf(mZ);

   if( absX < absY )
   {
      // X<Y && Z>Y
      if( absZ > absY )
      {
         return kAX;
      }
      // X<Y && Z<Y
      else
      {
         return kAZ;
      }
   }
   else // X > Y
   {
      // X>Y && Z>X
      if( absZ > absX )
      {
         return kAY;
      }
      // X>Y && Z<X
      else
      {
         return kAZ;
      }
   }
}

//------------------------------------------------------------------------------------------

int CVector3::GetMedianAxis() const
{
   real32 const absX = fabsf(mX);
   real32 const absY = fabsf(mY);
   real32 const absZ = fabsf(mZ);

   if( absX < absY )
   {
      // X<Y && Z>Y
      if( absZ > absY )
      {
         return kAY;
      }
      // X<Y && Z<Y
      else
      {
         return kAX;
      }
   }
   else // X > Y
   {
      // X>Y && Z>X
      if( absZ > absX )
      {
         return kAX;
      }
      // X>Y && Z<X
      else
      {
         return kAY;
      }
   }
}

//------------------------------------------------------------------------------------------
