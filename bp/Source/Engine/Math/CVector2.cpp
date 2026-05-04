//------------------------------------------------------------------------------------------
// CVector2.cpp
// Bluepoint
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "CVector2.h"

//------------------------------------------------------------------------------------------
// Remove unneeded  code when compiling for SPU's
#if !defined(SPU)

#include "Engine/Streams/CInputStream.h"
#include "Engine/Streams/COutputStream.h"

CVector2::CVector2(CInputStream& stream)
:  mX(stream.ReadReal32())
,  mY(stream.ReadReal32())
{
}

//------------------------------------------------------------------------------------------

void CVector2::PutTo(COutputStream& stream) const
{
   stream.WriteReal32(mX);
   stream.WriteReal32(mY);
}

#endif

//------------------------------------------------------------------------------------------

