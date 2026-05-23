//------------------------------------------------------------------------------------------
// CMatrix4.h
// Bluepoint
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/Math/CVector3.h"
#include "Engine/Math/CVector4.h"
#include "Engine/Math/CAngle.h"

//------------------------------------------------------------------------------------------
// forward declaration

class CMatrix3;
class CMatrix34;
class CQuaternion;

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class CMatrix4
{
public:
   enum EUninitialized
   {
      kConstructUninitialized
   };

   typedef real32 const (*TConstCStyleArray)[4];

   // default ctor initializes to identity
   explicit CMatrix4(CInputStream& stream);

   // put matrix to stream
   void PutTo(COutputStream& stream) const;

   explicit CMatrix4(real32 t00, real32 t01, real32 t02, real32 t03,
                     real32 t10, real32 t11, real32 t12, real32 t13,
                     real32 t20, real32 t21, real32 t22, real32 t23,
                     real32 t30, real32 t31, real32 t32, real32 t33 );

   explicit CMatrix4( CVector3 const & right, CVector3 const & up, CVector3 const & forward, CVector3 const & translation );
   explicit CMatrix4( CVector4 const &row0, CVector4 const &row1, CVector4 const &row2, CVector4 const &row3 );
   explicit CMatrix4(EUninitialized) {};

   // transform a vector (matrix is premultplied)
   const CVector3 operator * (const CVector3& vector) const;
   const CVector4 operator * (const CVector4& vector) const;
   
   // multiply with matrix
   const CMatrix4 operator * (const CMatrix4& matrix) const;
   CMatrix4& operator *= (const CMatrix4& matrix);

   const CMatrix4 Transpose() const;
   const CMatrix4 Inverse() const;
   real32 const Determinant() const;

   CVector3 const GetLeft() const { return CVector3( m00, m01, m02 ); }
   CVector3 const GetUp() const { return CVector3( m10, m11, m12 ); }
   CVector3 const GetForward() const { return CVector3( m20, m21, m22 ); }
   CVector3 const GetTranslation() const { return CVector3( m30, m31, m32 ); }
   inline const CVector4 GetRow(uint32 const row) const;

   void SetLeft( CVector3 const & vec ) { m00 = vec.GetX(); m01 = vec.GetY(); m02 = vec.GetZ(); }
   void SetUp( CVector3 const & vec ) { m10 = vec.GetX(); m11 = vec.GetY(); m12 = vec.GetZ(); }
   void SetForward( CVector3 const & vec ) { m20 = vec.GetX(); m21 = vec.GetY(); m22 = vec.GetZ(); }
   void SetTranslation( CVector3 const & translation ) { m30 = translation.GetX(); m31 = translation.GetY(); m32 = translation.GetZ(); }
   inline void SetRow( uint32 const row, CVector4 const &value);

   const real32 Get00() const { return m00; }
   const real32 Get01() const { return m01; }
   const real32 Get02() const { return m02; }
   const real32 Get03() const { return m03; }
   const real32 Get10() const { return m10; }
   const real32 Get11() const { return m11; }
   const real32 Get12() const { return m12; }
   const real32 Get13() const { return m13; }
   const real32 Get20() const { return m20; }
   const real32 Get21() const { return m21; }
   const real32 Get22() const { return m22; }
   const real32 Get23() const { return m23; }
   const real32 Get30() const { return m30; }
   const real32 Get31() const { return m31; }
   const real32 Get32() const { return m32; }
   const real32 Get33() const { return m33; }

   void Set00( real32 value ) { m00 = value; }
   void Set01( real32 value ) { m01 = value; }
   void Set02( real32 value ) { m02 = value; }
   void Set03( real32 value ) { m03 = value; }
   void Set10( real32 value ) { m10 = value; }
   void Set11( real32 value ) { m11 = value; }
   void Set12( real32 value ) { m12 = value; }
   void Set13( real32 value ) { m13 = value; }
   void Set20( real32 value ) { m20 = value; }
   void Set21( real32 value ) { m21 = value; }
   void Set22( real32 value ) { m22 = value; }
   void Set23( real32 value ) { m23 = value; }
   void Set30( real32 value ) { m30 = value; }
   void Set31( real32 value ) { m31 = value; }
   void Set32( real32 value ) { m32 = value; }
   void Set33( real32 value ) { m33 = value; }

   inline real32 *          Ptr()          { return &m00; }
   inline real32 const *    GetPtr() const { return &m00; }
   
   inline TConstCStyleArray GetCStyleArray() const { return reinterpret_cast<real32 const (*) [4]>(&m00); }

   // matrix factory
   static CMatrix4 const Identity();
   static CMatrix4 const Translation(const CVector3& translation);
   static CMatrix4 const Scale(const CVector3& scale);
   static CMatrix4 const RotateX(const CAngle& angle);
   static CMatrix4 const RotateY(const CAngle& angle);
   static CMatrix4 const RotateZ(const CAngle& angle);
   static CMatrix4 const FromMatrix3(const CMatrix3& other);
   static CMatrix4 const FromMatrix34(const CMatrix34& other);
   static CMatrix4 const FromQuaternion(const CQuaternion& other);
   static CMatrix4 const FromEulerAngles(CVector3 const & rotation);

   CMatrix4 const ScaleThenTranslate(CVector3 const &sc, CVector3 const &tr) const;

   ENGINE_API static CMatrix4 const Perspective(CAngle const & fov, real32 const aspect, real32 const minClip, real32 const maxClip);
   ENGINE_API static CMatrix4 const Orthographic(real32 const width, real32 const height, real32 const minClip, real32 const maxClip);
   ENGINE_API static CMatrix4 const OrthographicOffset(real32 const width, real32 const height, real32 const offsetX, real32 const offsetY, real32 const minClip, real32 const maxClip);
   ENGINE_API static CMatrix4 const PerspectiveNegZ(CAngle const & fov, real32 const aspect, real32 const minClip, real32 const maxClip);
   ENGINE_API static CMatrix4 const OrthographicNegZ(real32 const width, real32 const height, real32 const minClip, real32 const maxClip);
   ENGINE_API static CMatrix4 const OrthographicOffsetNegZ(real32 const width, real32 const height, real32 const offsetX, real32 const offsetY, real32 const minClip, real32 const maxClip);
   
   ENGINE_API bool const operator == ( CMatrix4 const & rhs ) const;

private:
   inline real32* const GetData();
   inline const real32* const GetData() const;

private:
   // x-axis/w
   real32  m00, m01, m02, m03;
   // y-axis/w
   real32  m10, m11, m12, m13;
   // z-axis/w
   real32  m20, m21, m22, m23;
   // translation/w
   real32  m30, m31, m32, m33;
};

//------------------------------------------------------------------------------------------

#include "Engine/Math/CMatrix4.inl"
