//----------------------------------------------------------------------------
// CRenderBounds.cpp
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "CRenderBounds.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------

BPE_CTASSERT(sizeof(CRenderBounds) < kMaxRenderObjectSize);

//----------------------------------------------------------------------------

CRenderBounds::~CRenderBounds()
{
}

//----------------------------------------------------------------------------

void CRenderBounds::AddChild(CRenderObject* pChild)
{
   if( mpChild != NULL )
   {
      CRenderObject* pCurrent = mpChild;
      
      while( pCurrent->Sibling() != NULL )
      {
         pCurrent = pCurrent->Sibling();
      }

      pCurrent->SetSibling(pChild);
   }
   else
   {
      mpChild = pChild;
   }
}
