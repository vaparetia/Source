//------------------------------------------------------------------------------------------
// close_enough.h
// Bluepoint
//------------------------------------------------------------------------------------------
// Checks for absolute difference between 2 values (close_enough) or zero (small_enough)
// Note that using these functions is not robust, see:
// http://realtimecollisiondetection.net/pubs/GDC07_Ericson_Physics_Tutorial_Numerical_Robustness.ppt
// and others for more info.
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/Math/BPEMath.h"
#include "Engine/Math/CQuaternion.h"
#include "Engine/Graphics/CColor.h"
#include "Engine/Graphics/CColorf.h"

//------------------------------------------------------------------------------------------

template< typename T > bool const close_enough( T const &lhs, T const &rhs, real32 const epsilon = gkEpsilon32 );

template<> inline bool const close_enough( real32 const &lhs, real32 const &rhs, real32 const epsilon )
{
   return fabsf( rhs - lhs ) < epsilon;
}

template<> inline bool const close_enough( real64 const &lhs, real64 const &rhs, real32 const epsilon )
{
   return fabs( rhs - lhs ) < static_cast<real64>( epsilon );
}

template<> inline bool const close_enough( CVector2 const &lhs, CVector2 const &rhs, real32 const epsilon )
{
   return close_enough( lhs.GetX(), rhs.GetX(), epsilon ) &&
          close_enough( lhs.GetY(), rhs.GetY(), epsilon );
}

template<> inline bool const close_enough( CVector3 const &lhs, CVector3 const &rhs, real32 const epsilon )
{
   return close_enough( lhs.GetX(), rhs.GetX(), epsilon ) &&
          close_enough( lhs.GetY(), rhs.GetY(), epsilon ) &&
          close_enough( lhs.GetZ(), rhs.GetZ(), epsilon );
}

template<> inline bool const close_enough( CVector4 const &lhs, CVector4 const &rhs, real32 const epsilon )
{
   return close_enough( lhs.GetX(), rhs.GetX(), epsilon ) &&
          close_enough( lhs.GetY(), rhs.GetY(), epsilon ) &&
          close_enough( lhs.GetZ(), rhs.GetZ(), epsilon ) &&
          close_enough( lhs.GetW(), rhs.GetW(), epsilon );
}

template<> inline bool const close_enough( CQuaternion const &lhs, CQuaternion const &rhs, real32 const epsilon )
{
   return close_enough( lhs.GetVector(), rhs.GetVector(), epsilon ) &&
          close_enough( lhs.GetScalar(), rhs.GetScalar(), epsilon );
}

template<> inline bool const close_enough( uint8 const &lhs, uint8 const &rhs, real32 const epsilon )
{
   return fabsf( static_cast<real32>( rhs ) - static_cast<real32>( lhs ) ) < epsilon;
}

template<> inline bool const close_enough( CColor const &lhs, CColor const &rhs, real32 const epsilon )
{
   return close_enough( lhs.GetR(), rhs.GetR(), epsilon ) &&
          close_enough( lhs.GetG(), rhs.GetG(), epsilon ) &&
          close_enough( lhs.GetB(), rhs.GetB(), epsilon ) &&
          close_enough( lhs.GetA(), rhs.GetA(), epsilon );
}

template<> inline bool const close_enough( CColorf const &lhs, CColorf const &rhs, real32 const epsilon )
{
   return close_enough( lhs.GetR(), rhs.GetR(), epsilon ) &&
          close_enough( lhs.GetG(), rhs.GetG(), epsilon ) &&
          close_enough( lhs.GetB(), rhs.GetB(), epsilon ) &&
          close_enough( lhs.GetA(), rhs.GetA(), epsilon );
}

template<> inline bool const close_enough( CAngle const &lhs, CAngle const &rhs, real32 const epsilon )
{
   return close_enough( lhs.AsRadians(), rhs.AsRadians(), epsilon );
}

//------------------------------------------------------------------------------------------
// Basic 'is zero' checks.
template< typename T > bool const small_enough( T const &lhs, real32 const epsilon = gkEpsilon32 );

template<> inline bool const small_enough( real32 const &lhs, real32 const epsilon )
{
   return fabsf( lhs ) < epsilon;
}

template<> inline bool const small_enough( real64 const &lhs, real32 const epsilon )
{
   return fabs( lhs ) < static_cast<real64>( epsilon );
}

template<> inline bool const small_enough( CVector2 const &lhs, real32 const epsilon )
{
   return small_enough( lhs.GetX(), epsilon ) &&
          small_enough( lhs.GetY(), epsilon );
}

template<> inline bool const small_enough( CVector3 const &lhs, real32 const epsilon )
{
   return small_enough( lhs.GetX(), epsilon ) &&
          small_enough( lhs.GetY(), epsilon ) &&
          small_enough( lhs.GetZ(), epsilon );
}

template<> inline bool const small_enough( CVector4 const &lhs, real32 const epsilon )
{
   return small_enough( lhs.GetX(), epsilon ) &&
          small_enough( lhs.GetY(), epsilon ) &&
          small_enough( lhs.GetZ(), epsilon ) &&
          small_enough( lhs.GetW(), epsilon );
}

template<> inline bool const small_enough( CQuaternion const &lhs, real32 const epsilon )
{
   return small_enough( lhs.GetVector(), epsilon ) &&
          small_enough( lhs.GetScalar(), epsilon );
}

template<> inline bool const small_enough( uint8 const &lhs, real32 const epsilon )
{
   return lhs < static_cast<uint8>(epsilon);
}

template<> inline bool const small_enough( CColor const &lhs, real32 const epsilon )
{
   return small_enough( lhs.GetG(), epsilon ) &&
          small_enough( lhs.GetG(), epsilon ) &&
          small_enough( lhs.GetB(), epsilon ) &&
          small_enough( lhs.GetA(), epsilon );
}

template<> inline bool const small_enough( CColorf const &lhs, real32 const epsilon )
{
   return small_enough( lhs.GetR(), epsilon ) &&
          small_enough( lhs.GetG(), epsilon ) &&
          small_enough( lhs.GetB(), epsilon ) &&
          small_enough( lhs.GetA(), epsilon );
}

template<> inline bool const small_enough( CAngle const &lhs, real32 const epsilon )
{
   return small_enough( lhs.AsRadians(), epsilon );
}

//------------------------------------------------------------------------------------------

