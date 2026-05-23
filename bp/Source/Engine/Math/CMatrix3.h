//------------------------------------------------------------------------------------------
// CMatrix3.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector3.h"
#include "Engine/Math/CAngle.h"

//------------------------------------------------------------------------------------------
// forward declaration

class CMatrix34;
class CMatrix4;
class CQuaternion;

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class CMatrix3
{
public:
   enum EUninitialized
   {
      kConstructUninitialized
   };

   typedef real32 const (*TConstCStyleArray)[3];

   // default ctor initializes to identity
   ENGINE_API explicit CMatrix3(CInputStream& stream);

   // put matrix to stream
   ENGINE_API void PutTo(COutputStream& stream) const;
   // Uninitialized construction
   ENGINE_API explicit CMatrix3(EUninitialized) {};

   ENGINE_API explicit CMatrix3(std::string const & value);
   ENGINE_API std::string const ToString() const;

   inline explicit CMatrix3(CVector3 const & right,
                            CVector3 const & up,
                            CVector3 const & forward );

   inline explicit CMatrix3(real32 t00, real32 t01, real32 t02,
                            real32 t10, real32 t11, real32 t12,
                            real32 t20, real32 t21, real32 t22);

   // transform a vector (matrix is premultplied)
   inline const CVector3 operator * (const CVector3& vector) const;
   
   // multiply with matrix
   inline const CMatrix3 operator * (const CMatrix3& matrix) const;

   // multiply with scalar
   inline const CMatrix3 operator * (const real32 scalar) const;

   inline const CMatrix3 operator + ( const CMatrix3& matrix) const;

   inline const CMatrix3 Transpose() const;
   ENGINE_API const CMatrix3 Inverse() const;
   ENGINE_API const CMatrix3 OrthoNormalized() const;
   ENGINE_API CMatrix3& OrthoNormalize();

   CVector3 const GetLeft() const { return CVector3( m00, m01, m02 ); }
   CVector3 const GetUp() const { return CVector3( m10, m11, m12 ); }
   CVector3 const GetForward() const { return CVector3( m20, m21, m22 ); }
   inline const CVector3 GetRow(uint32 const row) const;

   void SetLeft( CVector3 const & vec ) { m00 = vec.GetX(); m01 = vec.GetY(); m02 = vec.GetZ(); }
   void SetUp( CVector3 const & vec ) { m10 = vec.GetX(); m11 = vec.GetY(); m12 = vec.GetZ(); }
   void SetForward( CVector3 const & vec ) { m20 = vec.GetX(); m21 = vec.GetY(); m22 = vec.GetZ(); }
   inline void SetRow( uint32 const row, CVector3 const &value);

   inline CVector3 const GetX() const { return CVector3( m00, m01, m02 ); }
   inline CVector3 const GetY() const { return CVector3( m10, m11, m12 ); }
   inline CVector3 const GetZ() const { return CVector3( m20, m21, m22 ); }
   
   inline real32 const Get00() const { return m00; }
   inline real32 const Get01() const { return m01; }
   inline real32 const Get02() const { return m02; }
   inline real32 const Get10() const { return m10; }
   inline real32 const Get11() const { return m11; }
   inline real32 const Get12() const { return m12; }
   inline real32 const Get20() const { return m20; }
   inline real32 const Get21() const { return m21; }
   inline real32 const Get22() const { return m22; }
                 
   inline real32 *          Ptr()                  { return &m00; }
   inline real32 const *    GetPtr() const         { return &m00; }
   inline TConstCStyleArray GetCStyleArray() const { return reinterpret_cast<real32 const (*) [3]>(&m00); }

   // matrix factory
   inline static const CMatrix3 Identity();
   ENGINE_API static const CMatrix3 Scale(const CVector3& scale);
   ENGINE_API static const CMatrix3 RotateX(const CAngle& angle);
   ENGINE_API static const CMatrix3 RotateY(const CAngle& angle);
   ENGINE_API static const CMatrix3 RotateZ(const CAngle& angle);
   inline static const CMatrix3 FromMatrix34(const CMatrix34& other);
   inline static const CMatrix3 FromMatrix4(const CMatrix4& other);
   inline static const CMatrix3 FromQuaternion(const CQuaternion& other);
   inline static const CMatrix3 FromEulerAngles(CVector3 const & rotation);

public:
   // x-axis
   real32  m00, m01, m02;
   // y-axis
   real32  m10, m11, m12;
   // z-axis
   real32  m20, m21, m22;
};

//------------------------------------------------------------------------------------------

#include "Engine/Math/CMatrix3.inl"

