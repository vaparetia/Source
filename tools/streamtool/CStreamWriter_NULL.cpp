//----------------------------------------------------------------------------

#include "CStreamWriter_NULL.h"

//----------------------------------------------------------------------------

CStreamWriter_NULL::CStreamWriter_NULL()
{
}

bool CStreamWriter_NULL::ReplacementFileExistsForCurrStream(EPlatform curPlatform) const
{
   (void)curPlatform;
   return false;
}

void CStreamWriter_NULL::WriteOnePacket( FILE * const /*streamfp*/, EPlatform /*curPlatform*/, const int /*currTick */)
{
}

void CStreamWriter_NULL::EndStream(FILE * const /*streamfp*/,  EPlatform /*curPlatform*/, STREAM_TAG*)
{
}

bool CStreamWriter_NULL::ReplacesDataForCurrStream( const unsigned int type )
{
   //Always replace (strip) these stream types.
   return type == STREAM_TYPE_AC3;
}

unsigned int CStreamWriter_NULL::GetRebuildPlatformMask() const
{
   return kPlatformMask_Base;
}
