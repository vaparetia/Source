//----------------------------------------------------------------------------
// CRenderObject.cpp
// Bluepoint
// Copyright 2005
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CRenderObject.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(CRenderObject) < kMaxRenderObjectSize);

//----------------------------------------------------------------------------

CRenderObject::CRenderObject(EType const type)
:  mType(type)
,  mpSibling(NULL)
{
}

//----------------------------------------------------------------------------

CRenderObject::~CRenderObject()
{
}

