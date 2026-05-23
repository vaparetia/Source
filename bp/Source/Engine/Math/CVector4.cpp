//------------------------------------------------------------------------------------------
// CVector4.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"

#include "CVector4.h"

//------------------------------------------------------------------------------------------

// Removed unneeded output code when compiling for SPU's
#if !defined(SPU)

//------------------------------------------------------------------------------------------
// Stream constructor
CVector4::CVector4( CInputStream & stream )
:  mX( stream.ReadReal32() )
,  mY( stream.ReadReal32() )
,  mZ( stream.ReadReal32() )
,  mW( stream.ReadReal32() )
{
}

//------------------------------------------------------------------------------------------

// put vector to stream
void CVector4::PutTo( COutputStream & stream ) const
{
   stream.WriteReal32( mX );
   stream.WriteReal32( mY );
   stream.WriteReal32( mZ );
   stream.WriteReal32( mW );
}

#endif

//------------------------------------------------------------------------------------------

