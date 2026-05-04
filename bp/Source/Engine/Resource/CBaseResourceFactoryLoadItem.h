//------------------------------------------------------------------------------------------
// CBaseResourceFactoryLoadItem.h
// Bluepoint
// Base class for loading resources.
// Copyright 2006
//------------------------------------------------------------------------------------------

#pragma once

//------------------------------------------------------------------------------------------

#include "Engine/BPEEngineAPI.h"
#include "Engine/Resource/CResourceFactory.h"
#include "Engine/Resource/CResId.h"

//----------------------------------------------------------------------------

class ENGINE_API CBaseResourceFactoryLoadItem
{
public:
   virtual ~CBaseResourceFactoryLoadItem();
   
   virtual void      StartAsyncLoad()                 { mIsAsyncLoadingStarted = true; };
   virtual int32     GetAsyncLoadingFileSize() const  { return 0; };
   virtual void      Update() = 0;

   bool              IsCancelled() const              { return mCancelled; }
   void              Cancel();
   void              Uncancel();
   bool              IsDone() const;
   bool              IsCompressed() const;
   bool              HasAsyncLoadingStarted() const   { return mIsAsyncLoadingStarted; };

   CResId const &    GetResourceId() const            { return mResourceId; }
   IResourcePool *   ResourcePool() const             { return mpResourcePool; }
   void * const      TargetResourcePtr() const        { return *mpTargetResourcePtr; }

   static CBaseResourceFactoryLoadItem * Factory(  CResId const & resource,
                                                   void ** ppResource,
                                                   CResourceFactory::SFactoryEntry const & factory,
                                                   IResourcePool * pResourcePool);

protected:
   explicit CBaseResourceFactoryLoadItem(CResId const & resource,
                                         void ** ppResource,
                                         CResourceFactory::SFactoryEntry const & factory,
                                         IResourcePool * pResourcePool);

   CResId                              mResourceId;
   CResourceFactory::SFactoryEntry     mFactory;
   IResourcePool *                     mpResourcePool;
   void **                             mpTargetResourcePtr;

   bool                                mIsDone : 1;
   bool                                mCancelled : 1;
   bool                                mIsCompressed : 1;
   bool                                mIsAsyncLoadingStarted : 1;
};

//----------------------------------------------------------------------------

