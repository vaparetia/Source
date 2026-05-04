//------------------------------------------------------------------------------------------
// CColor.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector4.h"

#if BPE_TARGET==BPE_TARGET_RVL
#include <revolution/gx/GXStruct.h>
#endif

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class CColor
{
public:
   inline CColor()
   :  mARGB(0xff000000)
   {
   }

   explicit inline CColor(const uint8 r, const uint8 g, const uint8 b, const uint8 a = 255)
   {
      mARGB = static_cast<uint32>(a) << 24 | 
              static_cast<uint32>(r) << 16 | 
              static_cast<uint32>(g) << 8 |
              b;
   }

   static CColor const FromRGBA(uint32 const rgba)
   {
      uint32 const a___ = (rgba & 0xFF) << 24;
      uint32 const _rgb = (rgba & 0xFFFFFF00) >> 8;
      return CColor(a___ | _rgb);
   }

   static CColor const FromARGB(uint32 const argb)
   {
      return CColor(argb);
   }

   static CColor const FromFloatColor( real32 const r, real32 const g, real32 const b, real32 const a = 1.0f )
   {
      return CColor( static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, r * 255.0f ) ), 
                     static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, g * 255.0f ) ),
                     static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, b * 255.0f ) ),
                     static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, a * 255.0f ) ) );
   }

   static CColor const FromVector4( CVector4 const & color )
   {
      return CColor( static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, color[kAX] * 255.0f ) ), 
                     static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, color[kAY] * 255.0f ) ),
                     static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, color[kAZ] * 255.0f ) ),
                     static_cast<uint8>( 0.5f + bpe::min_val( 255.0f, color[kAW] * 255.0f ) ) );
   }
   
   ENGINE_API static CColor const FromRGB565( uint16 const rgb565 );

   ENGINE_API explicit CColor(CInputStream& inputStream);
   ENGINE_API void PutTo(COutputStream& outputStream) const;

   CColor const Pow( float power ) const
   {
      CVector4 temp = GetVector4();
      temp[kAX] = powf( temp[kAX], power );
      temp[kAY] = powf( temp[kAY], power );
      temp[kAZ] = powf( temp[kAZ], power );
      return CColor::FromVector4( temp );
   }

   inline uint32 GetARGB() const
   {
      return mARGB;
   }   
   
   inline void SetARGB(uint32 const argb)
   {
      mARGB = argb;
   }

   // this returns ARGB if this is on DirectX or ABGR if it's OpenGL
   ENGINE_API uint32 const GetUint32_PlatformSpecific() const;

   ENGINE_API uint32 GetRGBA() const;

   uint8 const GetA() const { return static_cast<uint8>( (mARGB & 0xff000000) >> 24 ); }
   uint8 const GetR() const { return static_cast<uint8>( (mARGB & 0xff0000) >> 16 ); }
   uint8 const GetG() const { return static_cast<uint8>( (mARGB & 0xff00) >> 8 ); }
   uint8 const GetB() const { return static_cast<uint8>( mARGB & 0xff ); }

   CVector3 const GetVector3() const { return CVector3( GetR() / 255.0f, GetG() / 255.0f, GetB() / 255.0f ); }
   CVector4 const GetVector4() const { return CVector4( GetR() / 255.0f, GetG() / 255.0f, GetB() / 255.0f, GetA() / 255.0f ); }

   // comparison operators
   inline bool const operator == ( const CColor& rhs ) const;
   inline bool const operator != ( const CColor& rhs ) const;

   CColor & operator *= ( CColor const & rhs )
   {
      *this = CColor( static_cast<uint8>( ( static_cast<uint32>( GetR() ) * static_cast<uint32>( rhs.GetR() ) ) >> 8 ),
                      static_cast<uint8>( ( static_cast<uint32>( GetG() ) * static_cast<uint32>( rhs.GetG() ) ) >> 8 ),
                      static_cast<uint8>( ( static_cast<uint32>( GetB() ) * static_cast<uint32>( rhs.GetB() ) ) >> 8 ),
                      static_cast<uint8>( ( static_cast<uint32>( GetA() ) * static_cast<uint32>( rhs.GetA() ) ) >> 8 ) );
      return *this;
   }

   CColor const operator * ( CColor const & rhs ) const
   {
      CColor cpy( *this );
      cpy *= rhs;
      return cpy;
   }

   CColor const operator + ( CColor const & rhs ) const
   {
      return CColor( bpe::min_val( static_cast<uint8>( 255 ), static_cast<uint8>( GetR() + rhs.GetR() ) ),
                     bpe::min_val( static_cast<uint8>( 255 ), static_cast<uint8>( GetG() + rhs.GetG() ) ),
                     bpe::min_val( static_cast<uint8>( 255 ), static_cast<uint8>( GetB() + rhs.GetB() ) ),
                     bpe::min_val( static_cast<uint8>( 255 ), static_cast<uint8>( GetA() + rhs.GetA() ) ) );
   }

   CColor & operator += ( CColor const & rhs )
   {
      *this = (*this) + rhs;
      
      return *this;
   }

   CColor const operator * ( real32 const factor ) const
   {
      return CColor( bpe::min_val( static_cast<uint8>(255), static_cast<uint8>( GetR() * factor ) ),
                     bpe::min_val( static_cast<uint8>(255), static_cast<uint8>( GetG() * factor ) ),
                     bpe::min_val( static_cast<uint8>(255), static_cast<uint8>( GetB() * factor ) ),
                     bpe::min_val( static_cast<uint8>(255), static_cast<uint8>( GetA() * factor ) ) );
   }

   // 0 = A, 1 = R, 2 = G, 3 = B
   uint8 const operator [] ( int const index ) const
   {
      BPE_ASSERT(index < 4, "Index out of range.");
      return *(((uint8*)this) + (3 - index));
   }

   inline static CColor const Lerp( CColor const & src, CColor const & dst, real32 const t ) 
   {
      // We'll do this in high precision float for now, speed it up later
      // Could use Mul8Bit 'int t = a * b + 128; return (t + (t >> 8)) >> 8;' but needs testing.
      // See http://code.google.com/p/nvidia-texture-tools/source/browse/branches/2.0/src/nvtt/SingleColorLookup.h?r=562

      return CColor::FromVector4(src.GetVector4() * (1.0f - t) + dst.GetVector4() * t);
   }

#if BPE_TARGET==BPE_TARGET_RVL
   GXColor const &AsGXColor() const 
   {
      return *reinterpret_cast<GXColor const *>(this);
   }
#endif

   static CColor const White()            { return CColor( 0xffffffff ); }
   static CColor const Black()            { return CColor( 0xff000000 ); }
   static CColor const Zero()             { return CColor( 0x00000000 ); }
   static CColor const Red()              { return CColor( 255,   0,   0, 255 ); }
   static CColor const Green()            { return CColor(   0, 255,   0, 255 ); }
   static CColor const Blue()             { return CColor(   0,   0, 255, 255 ); }
   static CColor const Yellow()           { return CColor( 255, 255,   0, 255 ); }
   static CColor const Purple()           { return CColor( 255,   0, 255, 255 ); }
   static CColor const Cyan()             { return CColor(   0, 255, 255, 255 ); }   
   static CColor const DimGray()          { return CColor( 105, 105, 105, 255 ); }
   static CColor const Grey()             { return CColor( 128, 128, 128, 255 ); }
   static CColor const Orange()           { return CColor( 255, 165,   0, 255 ); }
   static CColor const Brown()            { return CColor( 165,  42,  42, 255 ); }
   // Lots of cool colors at 
   // http://www.tayloredmktg.com/rgb/
   // http://www.nemahaweb.com/nemahaweb/colors.htm
   static CColor const Violet()           { return CColor( 238, 130, 238, 255 ); }
   static CColor const Beige()            { return CColor( 245, 245, 220, 255 ); }
   static CColor const Gold()             { return CColor( 255, 215,   0, 255 ); }
   static CColor const LightYellow()      { return CColor( 255, 255, 224, 255 ); }
   static CColor const Aquamarine()       { return CColor( 127, 255, 212, 255 ); }
   static CColor const Turquoise()        { return CColor(  64, 224, 208, 255 ); }
   static CColor const Khaki()            { return CColor( 240, 230, 140, 255 ); }
   static CColor const Lavender()         { return CColor( 230, 230, 250, 255 ); }
   static CColor const Chocolate()        { return CColor( 210, 105,  30, 255 ); }
   static CColor const Chartreuse()	      { return CColor( 127, 255,   0, 255 ); }
   static CColor const Salmon()           { return CColor( 250, 128, 114, 255 ); }
   static CColor const HotPink()          { return CColor( 255, 105, 180, 255 ); }
   static CColor const DeepPink()         { return CColor( 255,  20, 147, 255 ); }
   static CColor const OliveDrab()        { return CColor( 107, 142,  35, 255 ); }
   static CColor const Tomato()           { return CColor( 255,  99,  71, 255 ); }
   static CColor const Wheat()            { return CColor( 245, 222, 179, 255 ); }
   static CColor const Azure()            { return CColor( 240, 255, 255, 255 ); }
   static CColor const SpringGreen()      { return CColor(   0, 255, 127, 255 ); }
   static CColor const PowderBlue()       { return CColor( 176, 224, 230, 255 ); }
   static CColor const SteelBlue()        { return CColor(  70, 130, 180, 255 ); }
   static CColor const DodgerBlue()       { return CColor(  30, 144, 255, 255 ); }
   static CColor const DeepSkyBlue()      { return CColor(   0, 191, 255, 255 ); }
   static CColor const SlateGray()        { return CColor( 112, 138, 144, 255 ); }
   static CColor const LightSlateGray()   { return CColor( 119, 136, 153, 255 ); }
   static CColor const Snow()             { return CColor( 255, 250, 250, 255 ); }
   static CColor const Ivory()            { return CColor( 255, 255, 240, 255 ); }
   static CColor const GhostWhite()       { return CColor( 248, 248, 255, 255 ); }
   static CColor const PeachPuff()        { return CColor( 255, 218, 185, 255 ); }
   static CColor const MintCream()        { return CColor( 245, 255, 250, 255 ); }
   

private:
   explicit inline CColor( const uint32 argb )
      :  mARGB( argb )
   {
   }

private:
   uint32   mARGB;
};

//------------------------------------------------------------------------------------------

const bool CColor::operator == ( const CColor& rhs ) const
{
   return ( mARGB == rhs.mARGB );
}

//------------------------------------------------------------------------------------------

const bool CColor::operator != ( const CColor& rhs ) const
{
   return ( !( *this == rhs ) );
}

//------------------------------------------------------------------------------------------


