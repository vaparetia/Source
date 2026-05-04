//------------------------------------------------------------------------------------------
// CQuaternion.h
// Bluepoint
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

class CInputStream;
class COutputStream;

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Math/CVector4.h"

//------------------------------------------------------------------------------------------
// forward declaration

class CAxisAngle;
class CAngle;
class CMatrix3;
class CMatrix34;
class CMatrix4;

//------------------------------------------------------------------------------------------

MANAGED_PUBLIC class CQuaternion
{
public:
   enum EUninitialized
   {
      kConstructUninitialized
   };

public:
   inline explicit CQuaternion(CInputStream& stream);
   inline void PutTo(COutputStream& stream) const;

   inline explicit CQuaternion(EUninitialized) : mVector(CVector4::kConstructUninitialized) {}
   inline explicit CQuaternion(const CVector3& vector, const real32 scalar);
   inline explicit CQuaternion(const CVector4& vector);
   inline explicit CQuaternion(real32 const x, real32 const y, real32 const z, real32 const s);

   inline const CQuaternion operator * (const CQuaternion& other) const;
   inline CQuaternion& operator *= (const CQuaternion& other);
   
   inline const CVector3 operator * (const CVector3& vector) const;

   inline const CQuaternion operator * (const real32 other) const;
   inline CQuaternion& operator *= (const real32 other);

   inline const CQuaternion operator / (const real32 other) const;
   inline CQuaternion& operator /= (const real32 other);

   inline const CQuaternion operator + (const CQuaternion& other) const;
   inline CQuaternion& operator += (const CQuaternion& other);

   inline const CQuaternion operator - (const CQuaternion& other) const;
   inline CQuaternion& operator -= (const CQuaternion& other);

   // access operators
   real32 const & operator [] ( const int index ) const  { return mVector[index]; }
   real32 & operator [] ( const int index )              { return mVector[index]; }

   inline const CVector3& GetVector() const;
   inline const real32 GetScalar() const;

   inline const real32 Dot(const CQuaternion& other) const;

   inline const real32 GetLength() const;
   inline const real32 GetLengthSquared() const;

   inline void Normalize();
   inline const CQuaternion AsNormalized() const;

   inline CQuaternion const Inverse() const;
   CQuaternion const Exp() const;
   CQuaternion const Log() const;

   ENGINE_API bool const        IsLocalTo( CQuaternion const & other ) const;
   ENGINE_API void              MakeLocalTo( CQuaternion const & other );

   // quaternion factory
   inline static CQuaternion const Identity();
   inline static CQuaternion const RotateX(CAngle const & angle);
   inline static CQuaternion const RotateY(CAngle const & angle);
   inline static CQuaternion const RotateZ(CAngle const & angle);
   ENGINE_API static CQuaternion const FromAxisAngle(CAxisAngle const & other);
   ENGINE_API static CQuaternion const FromAxisAngle(CVector3 const & axis, CAngle const angle );
   ENGINE_API static real32            Angle(CQuaternion const &q1, CQuaternion const &q2);
   ENGINE_API static real32            Angle4D(CQuaternion const &q1, CQuaternion const &q2);
   ENGINE_API static CQuaternion const SlerpAccurate( CQuaternion const & src, CQuaternion const & dst, real32 const t );       // Not a local slerp, use MakeLocalTo for local slerp
   ENGINE_API static CQuaternion const SlerpAccurateLocal( CQuaternion const & src, CQuaternion const & dst, real32 const t );  
   inline static CQuaternion const Slerp( CQuaternion const & src, CQuaternion const & dst, real32 const t );
   inline static CQuaternion const SlerpLocal( CQuaternion const & src, CQuaternion const & dst, real32 const t );
   ENGINE_API static CQuaternion const Spline(CQuaternion const & q0, CQuaternion const & q1, CQuaternion const & q2, CQuaternion const & q3, real32 const t);  // spherical cubic spline interpolation - Watt and Watt, p366
   ENGINE_API static CQuaternion const Tangent(CQuaternion const &q0, CQuaternion const &q1, CQuaternion const &q2);
   ENGINE_API static CQuaternion const Squad(CQuaternion const & q1, CQuaternion const & qa, CQuaternion const & qb, CQuaternion const & q2, real32 const t);  // spherical quadrangle interpolation, you probably want to use spline instead!
   ENGINE_API static CQuaternion const LookAt(CVector3 const & src, CVector3 const & dst );
   inline static CQuaternion const BuildEquivalent( CQuaternion const & src );
   ENGINE_API static CQuaternion const FromMatrix3( CMatrix3 const & other );
   ENGINE_API static CQuaternion const FromMatrix34( CMatrix34 const & other );
   ENGINE_API static CQuaternion const FromMatrix4( CMatrix4 const & other );

private:
   CVector4    mVector;
};

//------------------------------------------------------------------------------------------

#include "Engine/Math/CQuaternion.inl"
