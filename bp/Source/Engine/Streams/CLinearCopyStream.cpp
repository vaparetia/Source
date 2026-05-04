//----------------------------------------------------------------------------
// CLinearCopyStream.h
// Bluepoint
// Copyright 2006
//
// Used for making deep copies of evaluators into a linear buffer.
// This is used for copying PPU data for use on the SPU.
//----------------------------------------------------------------------------

#include "Stdafx.h"
#include "Engine/Streams/CLinearCopyStream.h"

//----------------------------------------------------------------------------


CLinearCopyStream::CLinearCopyStream( int maxBufferSize, uint8 * pBuffer, int targetBufferOffset )
: mMaxBufferSize(maxBufferSize)
, mpBuffer(pBuffer)
, mTargetBufferOffset(targetBufferOffset)
, mCurrentBufferPos(0)
{
}

//----------------------------------------------------------------------------

CLinearCopyStream::~CLinearCopyStream()
{
}

//----------------------------------------------------------------------------

