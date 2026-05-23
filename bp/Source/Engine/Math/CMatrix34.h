//------------------------------------------------------------------------------------------
// CMatrix34.h
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

class CMatrix3;
class CMatrix4;
class CQuaternion;
class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class ENGINE_API CMatrix34
{
public:
   enum EUninitialized
   {
      kConstructUninitialized
   };

   typedef real32 const (*TConstCStyleArray)[4];

   inline explicit CMatrix34(real32 t00, real32 t01, real32 t02,
                             real32 t10, real32 t11, real32 t12,
                             real32 t20, real32 t21, real32 t22,
                             real32 t30, real32 t31, real32 t32 );

   inline explicit CMatrix34( CVector3 const & right,
                              CVector3 const & up,
                              CVector3 const & forward,
                              CVector3 const & translation );

   explicit CMatrix34(EUninitialized) {};
   explicit CMatrix34(CInputStream& stream);

   // put matrix to stream
   void PutTo(COutputStream& stream) const;

   bool const operator == ( CMatrix34 const & rhs ) const;

   // transform a vector (matrix is premultiplied)
   inline CVector3   operator * (CVector3 const & vector) const;
   inline CVector3   Rotate( CVector3 const & vector ) const;
   

   inline CVector3   TransposeMultiply(CVector3 const & vector) const;
   inline CVector3   TransposeRotate(CVector3 const & vector) const;
   
   // multiply with matrix
   inline CMatrix34  operator * (const CMatrix34& matrix) const;
   // Inplace multiply for performance, matOut != matA or matB
   static inline void MatrixMultiply_Inplace(CMatrix34 const &matA, CMatrix34 const &matB, CMatrix34 &matOut);
   // Rotate a matrix (matrix is premultiplied)
   CMatrix34          Rotate( CMatrix34 const & rotation ) const;

   // Doesn't deal with scale
   CMatrix34         QuickInverse() const;
   // Doesn't deal with scale
   CMatrix34         QuickRotationInverse() const;
   CMatrix34         Inverse() const;
   inline real32     Determinant() const;
   CMatrix34         Orthonormalized() const;
   void              Orthonormalize();
   
   CVector3          GetForward() const         { return CVector3( m20, m21, m22 ); }
   CVector3          GetLeft() const            { return CVector3( m00, m01, m02 ); }
   CVector3          GetUp() const              { return CVector3( m10, m11, m12 ); }
   inline CVector3   GetTranslation() const;
   inline CVector3   GetRow(uint32 const row) const;
   inline CVector3 & Row(uint32 const row);
   CMatrix34         GetRotation() const        { return CMatrix34( GetLeft(), GetUp(), GetForward(), CVector3::Zero() ); }

   inline real32     GetTX() const              { return m30; }
   inline real32     GetTY() const              { return m31; }
   inline real32     GetTZ() const              { return m32; }

   inline real32     Get00() const              { return m00; }
   inline real32     Get01() const              { return m01; }
   inline real32     Get02() const              { return m02; }
   inline real32     Get10() const              { return m10; }
   inline real32     Get11() const              { return m11; }
   inline real32     Get12() const              { return m12; }
   inline real32     Get20() const              { return m20; }
   inline real32     Get21() const              { return m21; }
   inline real32     Get22() const              { return m22; }
   inline real32     Get30() const              { return m30; }
   inline real32     Get31() const              { return m31; }
   inline real32     Get32() const              { return m32; }

   inline real32 *          Ptr()               { return &m00; }
   inline real32 const *    GetPtr() const      { return &m00; }

   inline TConstCStyleArray GetCStyleArray() const { return reinterpret_cast<real32 const (*) [4]>(&m00); }

   void              SetForward( CVector3 const & vec )  { m20 = vec.mX; m21 = vec.mY; m22 = vec.mZ; }
   void              SetLeft( CVector3 const & vec )     { m00 = vec.mX; m01 = vec.mY; m02 = vec.mZ; }
   void              SetUp( CVector3 const & vec )       { m10 = vec.mX; m11 = vec.mY; m12 = vec.mZ; }
   void              SetRotation( CMatrix34 const & rotation );
   void              SetRotation( CMatrix3 const & rotation );
   void              SetRotation( CMatrix4 const & rotation );
   inline void       SetTranslation( CVector3 const & translation );
   inline void       AddTranslation( CVector3 const &translationDelta );
   inline void       SetRow( uint32 const row, CVector3 const &value);

   inline void       SetTX(real32 const val)    { m30 = val; }
   inline void       SetTY(real32 const val)    { m31 = val; }
   inline void       SetTZ(real32 const val)    { m32 = val; }

   // matrix factory
   inline static CMatrix34 const & Identity();
   inline static CMatrix34 Translation(const CVector3& translation);
   inline static CMatrix34 Scale(const CVector3& scale);
   inline static CMatrix34 Scale( real32 const scale );
   inline static CMatrix34 RotateX(const CAngle& angle);
   inline static CMatrix34 RotateY(const CAngle& angle);
   inline static CMatrix34 RotateZ(const CAngle& angle);
   inline static CMatrix34 FromMatrix3(const CMatrix3& other);
   inline static CMatrix34 FromMatrix3(const CMatrix3& orientation, CVector3 const & translation);
   inline static CMatrix34 FromMatrix4(const CMatrix4& other);
   inline static CMatrix34 FromQuaternion(const CQuaternion& other);
   inline static CMatrix34 FromQuaternion(const CQuaternion& other, CVector3 const &translation);
   inline static CMatrix34 FromEulerAngles(CVector3 const & rotation);
   
   static CMatrix34        LookAt( CVector3 const & sourcePosition, CVector3 const & destPosition, CVector3 const &up = CVector3::YAxis() );
   static CMatrix34        Slerp( CMatrix34 const & src, CMatrix34 const & dest, real32 const t );
   static CMatrix34        MakeRotationFromTo(CVector3 const &from, CVector3 const &to);
   

private:
   // x-axis
   real32  m00, m01, m02;
   // y-axis
   real32  m10, m11, m12;
   // z-axis
   real32  m20, m21, m22;
   // translation
   real32  m30, m31, m32;

   static CMatrix34 const skIdentity;
};

//------------------------------------------------------------------------------------------

#include "Engine/Math/CMatrix34.inl"
