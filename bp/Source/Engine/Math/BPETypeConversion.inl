//------------------------------------------------------------------------------------------
// BPETypeConversion.inl
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/Math/CAngle.h"
#include "Engine/Math/CAxisAngle.h"
#include "Engine/Math/NEulerAngles.h"
#include "Engine/Math/CMatrix3.h"
#include "Engine/Math/CMatrix34.h"
#include "Engine/Math/CMatrix4.h"
#include "Engine/Math/CVector3.h"
#include "Engine/Math/CQuaternion.h"

//------------------------------------------------------------------------------------------
// CAxisAngle
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// CMatrix3
//------------------------------------------------------------------------------------------

const CMatrix3 CMatrix3::FromMatrix34(const CMatrix34& other)
{
   return CMatrix3( other.Get00(), other.Get01(), other.Get02(),
                    other.Get10(), other.Get11(), other.Get12(),
                    other.Get20(), other.Get21(), other.Get22()
                    );

}

const CMatrix3 CMatrix3::FromMatrix4(const CMatrix4& other)
{
   return CMatrix3( other.Get00(), other.Get01(), other.Get02(),
                    other.Get10(), other.Get11(), other.Get12(),
                    other.Get20(), other.Get21(), other.Get22()
                    );
}

const CMatrix3 CMatrix3::FromQuaternion(CQuaternion const & other)
{
   const real32 s = other.GetScalar();

   const real32 a  = 2.0f;

   const real32 ax = a * other.GetVector().GetX();
   const real32 ay = a * other.GetVector().GetY();
   const real32 az = a * other.GetVector().GetZ();

   const real32 sx = s * ax;
   const real32 sy = s * ay;
   const real32 sz = s * az;

   const real32 xx = ax * other.GetVector().GetX(); 
   const real32 xy = ax * other.GetVector().GetY(); 
   const real32 xz = ax * other.GetVector().GetZ(); 

   const real32 yy = ay * other.GetVector().GetY();
   const real32 yz = ay * other.GetVector().GetZ();

   const real32 zz = az * other.GetVector().GetZ();

   return CMatrix3(  1.0f - yy - zz,
                     xy + sz,
                     xz - sy,

                     xy - sz,
                     1.0f - xx - zz,
                     yz + sx,

                     xz + sy,                        
                     yz - sx,
                     1.0f - xx - yy
                  );
}

const CMatrix3 CMatrix3::FromEulerAngles(CVector3 const & rotation)
{
   return NEulerAngles::ToMatrix3(rotation);
}

//------------------------------------------------------------------------------------------
// CMatrix34
//------------------------------------------------------------------------------------------

CMatrix34 CMatrix34::FromMatrix3(const CMatrix3& other)
{
   return CMatrix34( other.Get00(), other.Get01(), other.Get02(),
                     other.Get10(), other.Get11(), other.Get12(),
                     other.Get20(), other.Get21(), other.Get22(),
                     real32(0),     real32(0),     real32(0)
                     );
}

CMatrix34 CMatrix34::FromMatrix3(CMatrix3 const & orientation, CVector3 const & translation)
{
   return CMatrix34( orientation.Get00(), orientation.Get01(), orientation.Get02(),
                     orientation.Get10(), orientation.Get11(), orientation.Get12(),
                     orientation.Get20(), orientation.Get21(), orientation.Get22(),
                     translation.GetX(),  translation.GetY(),  translation.GetZ() );
}

CMatrix34 CMatrix34::FromMatrix4(const CMatrix4& other)
{
   return CMatrix34( other.Get00(), other.Get01(), other.Get02(),
                        other.Get10(), other.Get11(), other.Get12(),
                        other.Get20(), other.Get21(), other.Get22(),
                        other.Get30(), other.Get31(), other.Get32()
                       );
}

CMatrix34 CMatrix34::FromQuaternion(const CQuaternion& other)
{
   return CMatrix34::FromMatrix3( CMatrix3::FromQuaternion( other ) );
}

CMatrix34 CMatrix34::FromQuaternion(const CQuaternion& other, CVector3 const &translation)
{
   return CMatrix34::FromMatrix3( CMatrix3::FromQuaternion( other ), translation );
}

CMatrix34 CMatrix34::FromEulerAngles(CVector3 const & rotation)
{
   return NEulerAngles::ToMatrix34(rotation);
}

//------------------------------------------------------------------------------------------
// CMatrix4
//------------------------------------------------------------------------------------------

inline const CMatrix4 CMatrix4::FromMatrix3(const CMatrix3& other)
{
   return CMatrix4(other.Get00(), other.Get01(), other.Get02(), real32(0),
                   other.Get10(), other.Get11(), other.Get12(), real32(0),
                   other.Get20(), other.Get21(), other.Get22(), real32(0),
                   real32(0),     real32(0),     real32(0),     real32(1)
                   );
}

inline const CMatrix4 CMatrix4::FromMatrix34(const CMatrix34& other)
{
   return CMatrix4(other.Get00(), other.Get01(), other.Get02(), real32(0),
                   other.Get10(), other.Get11(), other.Get12(), real32(0),
                   other.Get20(), other.Get21(), other.Get22(), real32(0),
                   other.Get30(), other.Get31(), other.Get32(), real32(1)
                   );
}

inline const CMatrix4 CMatrix4::FromQuaternion(const CQuaternion& other)
{
   return CMatrix4::FromMatrix3( CMatrix3::FromQuaternion( other ) );
}

inline CMatrix4 const CMatrix4::FromEulerAngles(CVector3 const & rotation)
{
   return NEulerAngles::ToMatrix4(rotation);
}

