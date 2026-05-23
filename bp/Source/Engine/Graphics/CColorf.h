//------------------------------------------------------------------------------------------
// CColorf.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector4.h"
#include "Engine/Graphics/CColor.h"

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class CColorf
{
public:
   enum EUninitialized
   {
      kConstructUnintialized
   };

public:
   CColorf(EUninitialized)
   {
   }

   CColorf()
   :  mR( 0 )
   ,  mG( 0 )
   ,  mB( 0 )
   ,  mA( 1 )
   {
   }

   ENGINE_API CColorf( CInputStream& stream );
   ENGINE_API void PutTo( COutputStream & stream ) const;

   explicit CColorf( CColor const & inColor )
   :  mR( inColor.GetR() * kOneOver255 )
   ,  mG( inColor.GetG() * kOneOver255 )
   ,  mB( inColor.GetB() * kOneOver255 )
   ,  mA( inColor.GetA() * kOneOver255 )
   {
   }

   explicit CColorf( const real32 r, const real32 g, const real32 b, const real32 a = 1.0f )
   :  mR( r )
   ,  mG( g )
   ,  mB( b )
   ,  mA( a )
   {
   }

   explicit CColorf( const CVector3& color, const real32 a = 1.0f )
   :  mR( color.GetX() )
   ,  mG( color.GetY() )
   ,  mB( color.GetZ() )
   ,  mA( a )
   {
   }

   explicit CColorf( const CVector4& color )
   :  mR( color.GetX() )
   ,  mG( color.GetY() )
   ,  mB( color.GetZ() )
   ,  mA( color.GetW() )
   {
   }

   ENGINE_API static CColorf FromHSV(real32 const h, real32 const s, real32 v, real32 const a = 1.0f);

   ENGINE_API CColor const AsLogLUV() const;
   
   CColorf operator * ( const real32 scalar ) const
   {
      return CColorf( GetR() * scalar,
                      GetG() * scalar,
                      GetB() * scalar,
                      GetA() * scalar );
   }

   CColorf& operator *= ( const real32 scalar )
   {
      mR *= scalar;
      mG *= scalar;
      mB *= scalar;
      mA *= scalar;
      return *this;
   }

   CColorf operator / ( const real32 scalar ) const
   {
      BPE_ASSERT( scalar != 0.0f, "div by zero" );
      const real32 oneOverScalar = 1.0f / scalar;
      return *this * oneOverScalar;
   }

   CColorf& operator /= ( const real32 scalar )
   {
      const real32 oneOverScalar = 1.0f / scalar;
      return *this *= oneOverScalar;
   }

   CColorf operator + ( const CColorf& other ) const
   {
      return CColorf( GetR() + other.GetR(),
                      GetG() + other.GetG(),
                      GetB() + other.GetB(),
                      GetA() + other.GetA()
                    );
   }

   CColorf& operator += ( const CColorf& other )
   {
      mR += other.GetR();
      mG += other.GetG();
      mB += other.GetB();
      mA += other.GetA();
      return *this;
   }

   CColorf operator - ( const CColorf& other ) const
   {
      return CColorf( GetR() - other.GetR(),
                      GetG() - other.GetG(),
                      GetB() - other.GetB(),
                      GetA() - other.GetA()
                    );
   }

   CColorf& operator -= ( const CColorf& other )
   {
      mR -= other.GetR();
      mG -= other.GetG();
      mB -= other.GetB();
      mA -= other.GetA();
      return *this;
   }

   CColorf operator * ( const CColorf& other ) const
   {
      return CColorf( GetR() * other.GetR(),
                      GetG() * other.GetG(),
                      GetB() * other.GetB(),
                      GetA() * other.GetA()
                    );
   }

   CColorf& operator *= ( const CColorf& other )
   {
      mR *= other.GetR();
      mG *= other.GetG();
      mB *= other.GetB();
      mA *= other.GetA();
      return *this;
   }

   ENGINE_API bool operator != ( CColorf const & rhs ) const;

   real32 GetR() const        { return mR; }
   real32 GetG() const        { return mG; }
   real32 GetB() const        { return mB; }
   real32 GetA() const        { return mA; }

   CVector3 AsVector3() const { return CVector3( mR, mG, mB ); }
   CVector4 AsVector4() const { return CVector4( mR, mG, mB, mA ); }
   bool IsTransparent() const { return (mA < (1.0f - kAlphaTolerance)); };          // Compensate for potential rounding errors
   ENGINE_API bool HasUnitAlpha() const;

   inline static CColorf const Lerp( CColorf const & src, CColorf const & dst, real32 const t ) { return src * (1.0f - t) + dst * t; }

   static CColorf const Zero()
   {
      return CColorf( 0, 0, 0, 0 );
   }

   static CColorf const White()
   {
      return CColorf( 1, 1, 1, 1 );
   }

   static CColorf const Black()
   {
      return CColorf( 0, 0, 0, 1 );
   }

   ENGINE_API static const real32 kOneOver255;
   ENGINE_API static const real32 kAlphaTolerance;

private:
   real32   mR, mG, mB, mA;
};
