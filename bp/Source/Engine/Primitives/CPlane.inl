//------------------------------------------------------------------------------------------
// CPlane.inl
// Bluepoint
//------------------------------------------------------------------------------------------

#include <math.h>

//------------------------------------------------------------------------------------------

CPlane::CPlane( const CVector3& pnt1, const CVector3& pnt2, const CVector3& pnt3 )
:  mNormal( CVector3::Cross( pnt2 - pnt1, pnt3 - pnt1 ).Normalized() )
,  mConstant( CVector3::Dot( pnt1, mNormal ) )
{
}

//------------------------------------------------------------------------------------------

CPlane::CPlane( const CVector3& reference, const CVector3& normal )
:  mNormal( normal )
,  mConstant( CVector3::Dot( reference, mNormal ) )
{
}

//------------------------------------------------------------------------------------------

CPlane::CPlane( const real32 constant, const CVector3& normal )
:  mNormal( normal )
,  mConstant( constant )
{
}

//------------------------------------------------------------------------------------------

const real32 CPlane::GetConstant() const
{
   return mConstant;
}

//------------------------------------------------------------------------------------------

const CVector3& CPlane::GetNormal() const
{
   return mNormal;
}

//------------------------------------------------------------------------------------------

const bool CPlane::IsFacing( const CVector3& point ) const
{
   return CVector3::Dot( GetNormal(), point ) >= GetConstant();
}

//------------------------------------------------------------------------------------------

real32 const CPlane::GetDistance( CVector3 const & point ) const
{
   return CVector3::Dot( GetNormal(), point ) - GetConstant();
}

