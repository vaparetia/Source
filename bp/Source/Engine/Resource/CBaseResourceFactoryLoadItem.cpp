//------------------------------------------------------------------------------------------
// CBaseResourceFactoryLoadItem.h
// Bluepoint
// Base class for loading resources.
// Copyright 2006
//------------------------------------------------------------------------------------------

#include "StdAfx.h"
#include "Engine/Resource/CBaseResourceFactoryLoadItem.h"

//----------------------------------------------------------------------------

using namespace std;

//----------------------------------------------------------------------------
      
CBaseResourceFactoryLoadItem::CBaseResourceFactoryLoadItem( CResId const & resource,
                              void ** ppResource,
                              CResourceFactory::SFactoryEntry const & factory,
                              IResourcePool * pResourcePool)
:  mResourceId( resource )
,  mFactory( factory )
,  mpResourcePool(pResourcePool)
,  mpTargetResourcePtr( ppResource )
,  mIsDone( false )
,  mCancelled( false )
,  mIsCompressed( false )
,  mIsAsyncLoadingStarted( false )
{
}
      
//----------------------------------------------------------------------------

CBaseResourceFactoryLoadItem::~CBaseResourceFactoryLoadItem()
{
}
      
//----------------------------------------------------------------------------

void CBaseResourceFactoryLoadItem::Cancel()
{
   mCancelled = true;
}

//----------------------------------------------------------------------------

void CBaseResourceFactoryLoadItem::Uncancel()
{
   mCancelled = false;
}
//----------------------------------------------------------------------------

bool CBaseResourceFactoryLoadItem::IsDone() const
{
   return mIsDone;
}

//----------------------------------------------------------------------------

bool CBaseResourceFactoryLoadItem::IsCompressed() const
{
   return mIsCompressed;
}

//----------------------------------------------------------------------------

#include BPE_PLATFORM_SPECIFIC( CResourceFactoryLoadItem.h )

CBaseResourceFactoryLoadItem * CBaseResourceFactoryLoadItem::Factory(CResId const & resource,
                                                                     void ** ppResource,
                                                                     CResourceFactory::SFactoryEntry const & factory,
                                                                     IResourcePool * pResourcePool)
{
   return new CResourceFactoryLoadItem(resource, ppResource, factory, pResourcePool);
}

//----------------------------------------------------------------------------

